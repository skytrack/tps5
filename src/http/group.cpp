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
#include "group.h"
#include "../core/jparse.h"
#include "response.h"
#include "object.h"

#define GROUP_PUT_VALIDATORS_COUNT 2

static JKEY_VALIDATOR group_put_validators[GROUP_PUT_VALIDATORS_COUNT] = 
{
	{ "name",					4,	JPARSE_VALUE_TYPE_STRING,	true,	0,	256},
	{ "custom",					6,	JPARSE_VALUE_TYPE_STRING,	false,	0,	4096}
};

#define GROUP_POST_VALIDATORS_COUNT 2

static JKEY_VALIDATOR group_post_validators[GROUP_POST_VALIDATORS_COUNT] = 
{
	{ "name",					4,	JPARSE_VALUE_TYPE_STRING,	true,	0,	256},
	{ "custom",					6,	JPARSE_VALUE_TYPE_STRING,	false,	0,	4096}
};

static JKEY type_key = { "type", 4, JPARSE_VALUE_TYPE_NUMBER, {}, 0, NULL, NULL};

/////////////////////////////////////////////////////////////////////////////////////////

int group_put(HTTP_SESSION *s, unsigned int id, unsigned char **d, size_t *l)
{
	unsigned char *incoming_key_buffer = *d;
	const size_t incoming_key_buffer_size = *l / 2;
	unsigned char *new_key_buffer = incoming_key_buffer + incoming_key_buffer_size;
	const size_t new_key_buffer_size = *l / 2;

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

	if (jparse_validate_object(&incoming_key.value.object_val, group_put_validators, GROUP_PUT_VALIDATORS_COUNT)) {

		unsigned char *error = jparse_get_error(s->http_body, s->zero_init.body_len);

		response_fail_with_message(*d, l, error, strlen((char *)error), s->zero_init.keep_alive > 0);

		jparse_free_error(error);

		return 0;
	}

	jparse_minify_object(&incoming_key.value.object_val, group_put_validators, GROUP_PUT_VALIDATORS_COUNT);

	type_key.value.int_val = OBJECT_GROUP;

	if (jparse_set_key(&incoming_key, &type_key))
		return 500;

	size_t json_buffer_size = new_key_buffer_size;

	if (jparse_build_json(&incoming_key, new_key_buffer, &json_buffer_size))
		return 500;

	db_object.type				= OBJECT_GROUP;
	db_object.module_data		= NULL;
	db_object.module_data_size	= 0;
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

int group_post(HTTP_SESSION *s, DB_OBJECT *object, unsigned char **d, size_t *l)
{
	unsigned char *existing_key_buffer = *d;
	const size_t existing_key_buffer_size = *l / 3;
	unsigned char *incoming_key_buffer = existing_key_buffer + existing_key_buffer_size;
	const size_t incoming_key_buffer_size = *l / 3;
	unsigned char *new_key_buffer = incoming_key_buffer + incoming_key_buffer_size;
	const size_t new_key_buffer_size = *l / 3;

	JKEY incoming_key;

	if (jparse_parse(s->http_body, s->zero_init.body_len, incoming_key_buffer, incoming_key_buffer_size, &incoming_key)) {

		unsigned char *error = jparse_get_error(s->http_body, s->zero_init.body_len);

		response_fail_with_message(*d, l, error, strlen((char *)error), s->zero_init.keep_alive > 0);

		jparse_free_error(error);

		return 0;
	}

	if (incoming_key.value_type != JPARSE_VALUE_TYPE_OBJECT)
		return 500;

	if (jparse_validate_object(&incoming_key.value.object_val, group_post_validators, GROUP_POST_VALIDATORS_COUNT)) {

		unsigned char *error = jparse_get_error(s->http_body, s->zero_init.body_len);

		response_fail_with_message(*d, l, error, strlen((char *)error), s->zero_init.keep_alive > 0);

		jparse_free_error(error);

		return 0;
	}

	jparse_minify_object(&incoming_key.value.object_val, group_post_validators, GROUP_POST_VALIDATORS_COUNT);

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