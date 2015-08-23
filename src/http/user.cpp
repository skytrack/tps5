//******************************************************************************
//
// File Name : user.cpp
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#include <stdio.h>
#include <string.h>
#include "api.h"
#include "rest.h"
#include "sha2.h"
#include "user.h"
#include "../core/jparse.h"
#include "response.h"
#include "object.h"

#define USER_PUT_VALIDATORS_COUNT 9

static JKEY_VALIDATOR user_put_validators[USER_PUT_VALIDATORS_COUNT] = 
{
	{ "login",					5,	JPARSE_VALUE_TYPE_STRING,	true,	1,	32},
	{ "password",				8,	JPARSE_VALUE_TYPE_STRING,	true,	1,	32},
	{ "custom",					6,	JPARSE_VALUE_TYPE_STRING,	false,	0,	4096},
	{ "company",				7,	JPARSE_VALUE_TYPE_STRING,	true,	0,	256},
	{ "name",					4,	JPARSE_VALUE_TYPE_STRING,	true,	0,	256},
	{ "phone",					5,	JPARSE_VALUE_TYPE_STRING,	true,	0,	256},
	{ "email",					5,	JPARSE_VALUE_TYPE_STRING,	false,	0,	256},
	{ "permission_admin",		16,	JPARSE_VALUE_TYPE_BOOLEAN,	true,	0,	0},
	{ "permission_login",		16,	JPARSE_VALUE_TYPE_BOOLEAN,	true,	0,	0}
};

#define USER_POST_VALIDATORS_COUNT 9

static JKEY_VALIDATOR user_post_validators[USER_POST_VALIDATORS_COUNT] = 
{
	{ "login",					5,	JPARSE_VALUE_TYPE_STRING,	true,	1,	32},
	{ "password",				8,	JPARSE_VALUE_TYPE_STRING,	false,	1,	32},
	{ "custom",					6,	JPARSE_VALUE_TYPE_STRING,	false,	0,	4096},
	{ "company",				7,	JPARSE_VALUE_TYPE_STRING,	true,	0,	256},
	{ "name",					4,	JPARSE_VALUE_TYPE_STRING,	true,	0,	256},
	{ "phone",					5,	JPARSE_VALUE_TYPE_STRING,	true,	0,	256},
	{ "email",					5,	JPARSE_VALUE_TYPE_STRING,	false,	0,	256},
	{ "permission_admin",		16,	JPARSE_VALUE_TYPE_BOOLEAN,	true,	0,	0},
	{ "permission_login",		16,	JPARSE_VALUE_TYPE_BOOLEAN,	true,	0,	0}
};

static JKEY hash_key = { "hash", 4, JPARSE_VALUE_TYPE_STRING, {}, 0, NULL, NULL};
static JKEY type_key = { "type", 4, JPARSE_VALUE_TYPE_NUMBER, {}, 0, NULL, NULL};

/////////////////////////////////////////////////////////////////////////////////////////

static void base64_encode(const unsigned char *data, size_t input_length, char *encoded_data, size_t *output_length) 
{
	static char encoding_table[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
									'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
									'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
									'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
									'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
									'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
									'w', 'x', 'y', 'z', '0', '1', '2', '3',
									'4', '5', '6', '7', '8', '9', '+', '/'};
	
	static int mod_table[] = {0, 2, 1};

	*output_length = 4 * ((input_length + 2) / 3);

	for (size_t i = 0, j = 0; i < input_length;) {

		uint32_t octet_a = i < input_length ? (unsigned char)data[i++] : 0;
		uint32_t octet_b = i < input_length ? (unsigned char)data[i++] : 0;
		uint32_t octet_c = i < input_length ? (unsigned char)data[i++] : 0;

		uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

		encoded_data[j++] = encoding_table[(triple >> 3 * 6) & 0x3F];
		encoded_data[j++] = encoding_table[(triple >> 2 * 6) & 0x3F];
		encoded_data[j++] = encoding_table[(triple >> 1 * 6) & 0x3F];
		encoded_data[j++] = encoding_table[(triple >> 0 * 6) & 0x3F];
	}

	for (int i = 0; i < mod_table[input_length % 3]; i++)
		encoded_data[*output_length - 1 - i] = '=';
}

/////////////////////////////////////////////////////////////////////////////////////////

typedef struct enum_ctx
{
	unsigned char *login;
	size_t login_len;
	DB_OBJECT *object;
} ENUM_CTX;

static int user_callback(DB_OBJECT *object, void *c)
{
	ENUM_CTX *ctx = (ENUM_CTX *)c;

	if ((object == ctx->object)||(object->type != OBJECT_USER))
		return 0;

	JKEY key;
	if (jparse_extract_key((unsigned char *)"login", 5, (unsigned char *)object->core_data, object->core_data_size, &key))
		return 0;

	if ((key.value_type == JPARSE_VALUE_TYPE_STRING)&&(key.str_len == ctx->login_len)&&(memcmp(key.value.str_val, ctx->login, ctx->login_len) == 0))
		return 1;

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int user_put(HTTP_SESSION *s, unsigned int id, unsigned char **d, size_t *l)
{
	unsigned char auth[514];

	unsigned char *incoming_key_buffer = *d;
	const size_t incoming_key_buffer_size = *l / 2;
	unsigned char *new_key_buffer = incoming_key_buffer + incoming_key_buffer_size;
	const size_t new_key_buffer_size = *l / 2;

	ENUM_CTX ctx;

	DB_OBJECT db_object, *parent_object;
	parent_object = api_db_get_object(id);

	if ((parent_object == NULL)||((parent_object->type != OBJECT_USER)&&(parent_object->type != OBJECT_GROUP)))
		return 404;

	REST_USER *user = rest_find_user(s);
	if (user == NULL)
		return 401;

	if (!rest_check_access_to_object(user, parent_object))
		return 401;

	JKEY incoming_key;

	if (jparse_parse(s->http_body, s->zero_init.body_len, incoming_key_buffer, incoming_key_buffer_size, &incoming_key)) {

		unsigned char *error = jparse_get_error(s->http_body, s->zero_init.body_len);

		response_fail_with_message(*d, l, error, strlen((char *)error), s->zero_init.keep_alive > 0);

		jparse_free_error(error);

		return 0;
	}

	if (incoming_key.value_type != JPARSE_VALUE_TYPE_OBJECT)
		return 500;

	if (jparse_validate_object(&incoming_key.value.object_val, user_put_validators, USER_PUT_VALIDATORS_COUNT)) {

		unsigned char *error = jparse_get_error(s->http_body, s->zero_init.body_len);

		response_fail_with_message(*d, l, error, strlen((char *)error), s->zero_init.keep_alive > 0);

		jparse_free_error(error);

		return 0;
	}

	ctx.object = NULL;
	ctx.login = incoming_key.value.object_val.first_key->value.str_val;
	ctx.login_len = incoming_key.value.object_val.first_key->str_len;

	if (api_db_enum_objects(user_callback, &ctx)) {
		response_fail_with_message(*d, l, (unsigned char *)"Such login already exists", 25, s->zero_init.keep_alive > 0);
		return 0;
	}

	// prevent password being saved to json as plain text
	user_put_validators[1].value_type = JPARSE_VALUE_TYPE_UNKNOWN;
	jparse_minify_object(&incoming_key.value.object_val, user_put_validators, USER_PUT_VALIDATORS_COUNT);
	user_put_validators[1].value_type = JPARSE_VALUE_TYPE_STRING;

	memcpy(auth, user_put_validators[0].jkey->value.str_val, user_put_validators[0].jkey->str_len);
	auth[user_put_validators[0].jkey->str_len] = ':';
	memcpy(auth + user_put_validators[0].jkey->str_len + 1, user_put_validators[1].jkey->value.str_val, user_put_validators[1].jkey->str_len);

	size_t base64_len;
	char base64[256];
	base64_encode(auth, user_put_validators[0].jkey->str_len + 1 + user_put_validators[1].jkey->str_len, base64, &base64_len);

	SHA256_CTX ctx256;
	SHA256_Init(&ctx256);
	SHA256_Update(&ctx256, (unsigned char *)base64, base64_len);
	SHA256_End(&ctx256, (char *)hash_key.value.str_val);
	hash_key.str_len = 64;
	
	if (jparse_set_key(&incoming_key, &hash_key))
		return 500;

	type_key.value.int_val = 0;

	if (jparse_set_key(&incoming_key, &type_key))
		return 500;

	db_object.type				= OBJECT_USER;
	db_object.module_data		= NULL;
	db_object.module_data_size	= 0;

	size_t json_buffer_size = new_key_buffer_size;

	if (jparse_build_json(&incoming_key, new_key_buffer, &json_buffer_size))
		return 500;

	db_object.core_data = (unsigned char *)malloc(json_buffer_size + 33);
	if (db_object.core_data == NULL)
		return 500;

	memcpy(db_object.core_data, new_key_buffer, json_buffer_size - 1);
	db_object.core_data_size = json_buffer_size - 1;

	DB_OBJECT *pObject = &db_object;
	int res = api_db_put_object(&pObject, parent_object);

	if (res == DB_OK) {

		on_object_create(pObject);

		object_report(s, pObject, d, l);

		return 0;
	}

	unsigned char *error = (unsigned char *)api_db_get_error();

	response_fail_with_message(*d, l, error, strlen((char *)error), s->zero_init.keep_alive > 0);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int user_post(HTTP_SESSION *s, DB_OBJECT *object, unsigned char **d, size_t *l)
{
	unsigned char auth[514];

	unsigned char *existing_key_buffer = *d;
	const size_t existing_key_buffer_size = *l / 3;
	unsigned char *incoming_key_buffer = existing_key_buffer + existing_key_buffer_size;
	const size_t incoming_key_buffer_size = *l / 3;
	unsigned char *new_key_buffer = incoming_key_buffer + incoming_key_buffer_size;
	const size_t new_key_buffer_size = *l / 3;

	ENUM_CTX ctx;

	JKEY incoming_key;

	if (jparse_parse(s->http_body, s->zero_init.body_len, incoming_key_buffer, incoming_key_buffer_size, &incoming_key)) {

		unsigned char *error = jparse_get_error(s->http_body, s->zero_init.body_len);

		response_fail_with_message(*d, l, error, strlen((char *)error), s->zero_init.keep_alive > 0);

		jparse_free_error(error);

		return 0;
	}

	if (incoming_key.value_type != JPARSE_VALUE_TYPE_OBJECT)
		return 500;

	if (jparse_validate_object(&incoming_key.value.object_val, user_post_validators, USER_POST_VALIDATORS_COUNT)) {

		unsigned char *error = jparse_get_error(s->http_body, s->zero_init.body_len);

		response_fail_with_message(*d, l, error, strlen((char *)error), s->zero_init.keep_alive > 0);

		jparse_free_error(error);

		return 0;
	}

	ctx.object = object;
	ctx.login = incoming_key.value.object_val.first_key->value.str_val;
	ctx.login_len = incoming_key.value.object_val.first_key->str_len;

	if (api_db_enum_objects(user_callback, &ctx)) {
		response_fail_with_message(*d, l, (unsigned char *)"Such login already exists", 25, s->zero_init.keep_alive > 0);
		return 0;
	}

	// prevent password being saved to json as plain text
	user_post_validators[1].value_type = JPARSE_VALUE_TYPE_UNKNOWN;
	jparse_minify_object(&incoming_key.value.object_val, user_post_validators, USER_POST_VALIDATORS_COUNT);
	user_post_validators[1].value_type = JPARSE_VALUE_TYPE_STRING;

	JKEY existing_key;

	if (jparse_parse((unsigned char *)object->core_data, object->core_data_size, existing_key_buffer, existing_key_buffer_size, &existing_key))
		return 500;

	if (existing_key.value_type != JPARSE_VALUE_TYPE_OBJECT)
		return 500;

	JKEY *jobjkey = incoming_key.value.object_val.first_key;
	while (jobjkey) {
		JKEY *jobjkey_next = jobjkey->next_key;
		if (jparse_set_key(&existing_key, jobjkey))
			return 500;
		jobjkey = jobjkey_next;
	}

	if (user_post_validators[1].jkey != NULL) {

		memcpy(auth, user_post_validators[0].jkey->value.str_val, user_post_validators[0].jkey->str_len);
		auth[user_post_validators[0].jkey->str_len] = ':';
		memcpy(auth + user_post_validators[0].jkey->str_len + 1, user_post_validators[1].jkey->value.str_val, user_post_validators[1].jkey->str_len);

		size_t base64_len;
		char base64[256];
		base64_encode(auth, user_post_validators[0].jkey->str_len + 1 + user_post_validators[1].jkey->str_len, base64, &base64_len);

		SHA256_CTX ctx256;
		SHA256_Init(&ctx256);
		SHA256_Update(&ctx256, (unsigned char *)base64, base64_len);
		SHA256_End(&ctx256, (char *)hash_key.value.str_val);
		hash_key.str_len = 64;
	
		if (jparse_set_key(&existing_key, &hash_key))
			return 500;

	}

	size_t json_buffer_size = new_key_buffer_size;
	if (jparse_build_json(&existing_key, new_key_buffer, &json_buffer_size))
		return 500;

	unsigned char *new_core_data = (unsigned char *)realloc(object->core_data, json_buffer_size);

	if (new_core_data == NULL)
		return 500;

	object->core_data = new_core_data;
	object->core_data_size = json_buffer_size;

	memcpy(object->core_data, new_key_buffer, json_buffer_size);

	int res = api_db_update_object(object);

	if (res == DB_OK) {

		object_report(s, object, d, l);

		return 0;
	}

	unsigned char *error = (unsigned char *)api_db_get_error();

	response_fail_with_message(*d, l, error, strlen((char *)error), s->zero_init.keep_alive > 0);

	return 0;
}