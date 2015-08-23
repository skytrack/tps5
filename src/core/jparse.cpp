//******************************************************************************
//
// File Name : jparse.cpp
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include "jparse.h"
#include "../lib/yajl/src/api/yajl_parse.h"

typedef struct ctx
{
	unsigned char *ptr;
	size_t bytes_left;
	JOBJECT *current_object;
	JKEY *current_key;
	size_t nest_level;
	bool extract_key;
} CTX;

static yajl_handle parser;
static yajl_handle extractor;
static CTX context;

static unsigned char error[1024];

static int parse_null(void *ctx)  
{
	context.current_key->value_type = JPARSE_VALUE_TYPE_NULL;
	context.current_key = context.current_key->parent_key;

	return 1;
}  
  
static int parse_boolean(void * ctx, int boolean)  
{  
	context.current_key->value_type = (boolean) ? JPARSE_VALUE_TYPE_TRUE : JPARSE_VALUE_TYPE_FALSE;
	context.current_key = context.current_key->parent_key;

	return 1;
}  
  
static int parse_number(void * ctx, const char * s, size_t l)  
{  
	char *ptr;

	context.current_key->value.int_val = strtol(s, &ptr, 10);

	if (*ptr != '.') {
		context.current_key->value_type = JPARSE_VALUE_TYPE_NUMBER;
	}
	else {
		context.current_key->value.float_val = (float)strtod(s, &ptr);
		context.current_key->value_type = JPARSE_VALUE_TYPE_FLOAT;
	}
	context.current_key = context.current_key->parent_key;

	return 1;
}  
  
static int parse_string(void * ctx, const unsigned char * stringVal, size_t stringLen)  
{  
	if (stringLen > sizeof(context.current_key->value.str_val)) {
		context.current_key->str_len = sizeof(context.current_key->value.str_val);
	}
	else {
		context.current_key->str_len = stringLen;
	}

	memcpy(context.current_key->value.str_val, stringVal, context.current_key->str_len);

	context.current_key->value_type = JPARSE_VALUE_TYPE_STRING;

	context.current_key = context.current_key->parent_key;

	return 1;
}  
  
static int parse_map_key(void * ctx, const unsigned char * stringVal, size_t stringLen)  
{
	JKEY *this_key = (JKEY *)context.ptr;
	context.ptr += sizeof(JKEY);

	this_key->parent_key = context.current_key;
	this_key->next_key = NULL;

	this_key->value_type = JPARSE_VALUE_TYPE_UNKNOWN;

	if (stringLen > sizeof(this_key->key)) {
		this_key->key_len = sizeof(this_key->key);
	}
	else {
		this_key->key_len = stringLen;
	}
	
	memcpy(this_key->key, stringVal, this_key->key_len);

	JOBJECT *object;

	if (context.current_key->value_type == JPARSE_VALUE_TYPE_OBJECT)
		object = &context.current_key->value.object_val;
	else
	if (context.current_key->value_type == JPARSE_VALUE_TYPE_ARRAY)
		object = context.current_key->value.array_val.last_object;

	if (object->first_key == NULL) {
		object->first_key = this_key;
	}
	else {
		object->last_key->next_key = this_key;
	}

	object->last_key = this_key;

	context.current_key = this_key;

	return 1;
}  
  
static int parse_start_map(void * ctx)  
{
	if (context.current_key->value_type == JPARSE_VALUE_TYPE_UNKNOWN) {

		context.current_key->value_type = JPARSE_VALUE_TYPE_OBJECT;

		context.current_key->value.object_val.first_key = NULL;
		context.current_key->value.object_val.last_key = NULL;
	}
	else
	if (context.current_key->value_type == JPARSE_VALUE_TYPE_ARRAY) {

		JOBJECT *this_object = (JOBJECT *)context.ptr;
		context.ptr += sizeof(JOBJECT);

		this_object->first_key = NULL;
		this_object->last_key = NULL;
		this_object->next_object = NULL;

		if (context.current_key->value.array_val.first_object == NULL) {
			context.current_key->value.array_val.first_object = this_object;
		}
		else {
			context.current_key->value.array_val.last_object->next_object = this_object;
		}

		context.current_key->value.array_val.last_object = this_object;
	}

	return 1;
}  
   
static int parse_end_map(void * ctx)  
{
	if (context.current_key->value_type == JPARSE_VALUE_TYPE_OBJECT)
		context.current_key = context.current_key->parent_key;

	return 1;
}  
  
static int parse_start_array(void * ctx)  
{
	context.current_key->value_type = JPARSE_VALUE_TYPE_ARRAY;
	context.current_key->value.array_val.first_object = NULL;
	context.current_key->value.array_val.last_object = NULL;

	return 1;
}  
  
static int parse_end_array(void * ctx)  
{  	
	context.current_key = context.current_key->parent_key;

	return 1;
}  

static yajl_callbacks callbacks = 
{  
	parse_null,  
	parse_boolean,  
	NULL,  
	NULL,  
	parse_number,  
	parse_string,  
	parse_start_map,  
	parse_map_key,  
	parse_end_map,  
	parse_start_array,  
	parse_end_array  
};  
    
int jparse_parse(unsigned char *json, size_t json_len, unsigned char *buffer, size_t buffer_size, JKEY *result)
{
	yajl_status stat; 
 
	yajl_clear(parser);

	*error = '\0';

	context.ptr = buffer;
	context.bytes_left = buffer_size;
	context.current_key = result;

	result->value_type = JPARSE_VALUE_TYPE_UNKNOWN;
	result->next_key = NULL;
	result->parent_key = NULL;
	result->key_len = 0;

	stat = yajl_parse(parser, json, json_len);  
    
	if (stat == yajl_status_ok) {

		stat = yajl_complete_parse(parser);  

		if (stat == yajl_status_ok) {

			return 0;
		}
	}
  
	return -1; 
}

static int extractor_parse_null(void *ctx)  
{
	if (context.extract_key) {
		context.current_key->value_type = JPARSE_VALUE_TYPE_NULL;
		return 0;
	}
	return 1;
}  
  
static int extractor_parse_boolean(void * ctx, int boolean)  
{  
	if (context.extract_key) {
		context.current_key->value_type = (boolean) ? JPARSE_VALUE_TYPE_TRUE : JPARSE_VALUE_TYPE_FALSE;
		return 0;
	}
	return 1;
}  
  
static int extractor_parse_number(void * ctx, const char * s, size_t l)  
{  
	if (context.extract_key) {

		char *ptr;

		context.current_key->value.int_val = strtol(s, &ptr, 10);

		if (*ptr != '.') {
			context.current_key->value_type = JPARSE_VALUE_TYPE_NUMBER;
		}
		else {
			context.current_key->value.float_val = (float)strtod(s, &ptr);
			context.current_key->value_type = JPARSE_VALUE_TYPE_FLOAT;
		}

		return 0;
	}

	return 1;
}  
  
static int extractor_parse_string(void * ctx, const unsigned char * stringVal, size_t stringLen)  
{  
	if (context.extract_key) {

		if (stringLen > sizeof(context.current_key->value.str_val)) {
			context.current_key->str_len = sizeof(context.current_key->value.str_val);
		}
		else {
			context.current_key->str_len = stringLen;
		}

		memcpy(context.current_key->value.str_val, stringVal, context.current_key->str_len);

		context.current_key->value_type = JPARSE_VALUE_TYPE_STRING;

		return 0;
	}

	return 1;
}  
  
static int extractor_parse_map_key(void * ctx, const unsigned char * stringVal, size_t stringLen)  
{
	if ((context.nest_level == 1)&&(stringLen == context.bytes_left)&&(memcmp(stringVal, context.ptr, stringLen) == 0))
		context.extract_key = true;
	else
		context.extract_key = false;

	return 1;
}  
  
static int extractor_parse_start_map(void * ctx)  
{
	context.nest_level++;
	return 1;
}  
   
static int extractor_parse_end_map(void * ctx)  
{
	context.nest_level--;
	return 1;
}  
  
static int extractor_parse_start_array(void * ctx)  
{
	context.nest_level++;
	return 1;
}  
  
static int extractor_parse_end_array(void * ctx)  
{  	
	context.nest_level--;
	return 1;
}  

static yajl_callbacks extractor_callbacks = 
{  
	extractor_parse_null,  
	extractor_parse_boolean,  
	NULL,  
	NULL,  
	extractor_parse_number,  
	extractor_parse_string,  
	extractor_parse_start_map,  
	extractor_parse_map_key,  
	extractor_parse_end_map,  
	extractor_parse_start_array,  
	extractor_parse_end_array  
};  

int jparse_extract_key(unsigned char *key, size_t key_len, unsigned char *json, size_t json_len, JKEY *result)
{
	yajl_clear(extractor);

	context.ptr = key;
	context.bytes_left = key_len;
	context.current_key = result;
	context.nest_level = 0;

	result->value_type = JPARSE_VALUE_TYPE_UNKNOWN;
	result->next_key = NULL;
	result->parent_key = NULL;
	result->key_len = 0;

	yajl_parse(extractor, json, json_len);  
    
	yajl_complete_parse(extractor);  
  
	return 0; 
}

static int jparse_build_key(JKEY *key, unsigned char **b, size_t *buffer_size)
{
	int i;
	JKEY *k;
	JOBJECT *o;
	unsigned char *buffer = *b;
	size_t size = *buffer_size;
	size_t l;
	bool first_item;

	if (key->value_type == JPARSE_VALUE_TYPE_UNKNOWN)
		return 0;

	if (key->key_len > 0) {
		if (size < (key->key_len + 3))
			return -1;
		*buffer++ = '\"';
		memcpy(buffer, key->key, key->key_len);
		buffer += key->key_len;
		*buffer++ = '\"';
		*buffer++ = ':';
		size -= key->key_len + 3;
	}

	switch (key->value_type) {
	case JPARSE_VALUE_TYPE_OBJECT:
		if (size == 0)
			return -1;
		size--;
		*buffer++ = '{';
		k = key->value.object_val.first_key;
		first_item = true;
		while (k) {

			if (k->value_type != JPARSE_VALUE_TYPE_UNKNOWN) {

				if (first_item == true) {
					first_item = false;
				}
				else {

					if (size == 0)
						return -1;
					size--;
					*buffer++ = ',';
				}

				if (jparse_build_key(k, &buffer, &size))
					return -1;
			}

			k = k->next_key;
		}
		if (size == 0)
			return -1;
		size--;
		*buffer++ = '}';
		break;
	case JPARSE_VALUE_TYPE_ARRAY:
		if (size == 0)
			return -1;
		size--;
		*buffer++ = '[';
		o = key->value.array_val.first_object;
		while (o) {
			if (size == 0)
				return -1;
			size--;
			*buffer++ = '{';
			k = o->first_key;
			while (k) {
				if (jparse_build_key(k, &buffer, &size))
					return -1;
				k = k->next_key;
				if (k) {
					if (size == 0)
						return -1;
					size--;
					*buffer++ = ',';
				}
			}
			if (size == 0)
				return -1;
			size--;
			*buffer++ = '}';
			o = o->next_object;
			if (o) {
				if (size == 0)
					return -1;
				size--;
				*buffer++ = ',';
			}
		}
		if (size == 0)
			return -1;
		size--;
		*buffer++ = ']';
		break;
	case JPARSE_VALUE_TYPE_STRING:

		if (size == 0)
			return -1;

		size--;

		*buffer++ = '\"';

		for (i = 0; i < key->str_len; i++) {

			unsigned char ch = key->value.str_val[i];
			switch(ch) {
			case '\\':
			case '"':
			case '/':
				if (size < 2)
					return -1;

				size -= 2;

				*buffer++ = '\\';
				*buffer++ = ch;
				break;
			case '\b':
				if (size == 0)
					return -1;

				size--;		
				*buffer++ = '\b';
				break;
			case '\t':
				if (size == 0)
					return -1;

				size--;		
				*buffer++ = '\t';
				break;
			case '\n':
				if (size == 0)
					return -1;

				size--;		
				*buffer++ = '\n';
				break;
			case '\f':
				if (size == 0)
					return -1;

				size--;		
				*buffer++ = '\f';
				break;
			case '\r':
				*buffer++ = '\r';
				break;
			default:
				if ((key->value.str_val[i] > 0)&&(ch < ' ')) {
					if (size < 7)
						return -1;
						
					size -= 7;		
					buffer += sprintf((char *)buffer, "\\u%04X", ch & 0xFF);
				} else {
					if (size == 0)
						return -1;

					size--;		
					*buffer++ = ch;
				}
			}
		}

		if (size == 0)
			return -1;

		size--;

		*buffer++ = '\"';

		break;

	case JPARSE_VALUE_TYPE_NUMBER:
		if (size < 14)
			return -1;
		l = sprintf((char *)buffer, "%d", key->value.int_val);
		buffer += l;
		size -= l;
		break;
	case JPARSE_VALUE_TYPE_FLOAT:
		if (size < 14)
			return -1;
		l = sprintf((char *)buffer, "%f", key->value.float_val);
		buffer += l;
		size -= l;
		break;
	case JPARSE_VALUE_TYPE_TRUE:
		if (size < 4)
			return -1;
		size -= 4;
		*buffer++ = 't';
		*buffer++ = 'r';
		*buffer++ = 'u';
		*buffer++ = 'e';
		break;
	case JPARSE_VALUE_TYPE_FALSE:
		if (size < 5)
			return -1;
		size -= 5;
		*buffer++ = 'f';
		*buffer++ = 'a';
		*buffer++ = 'l';
		*buffer++ = 's';
		*buffer++ = 'e';
		break;
	case JPARSE_VALUE_TYPE_NULL:
		if (size < 4)
			return -1;
		size -= 4;
		*buffer++ = 'n';
		*buffer++ = 'u';
		*buffer++ = 'l';
		*buffer++ = 'l';
		break;
	}

	*b = buffer;
	*buffer_size = size;

	return 0;
}

int jparse_build_json(JKEY *key, unsigned char *buffer, size_t *buffer_size)
{
	*error = '\0';

	size_t s = *buffer_size;

	if (jparse_build_key(key, &buffer, &s))
		return -1;

	*buffer_size -= s;

	return 0;
}

int jparse_set_key(JKEY *parent_key, JKEY *child_key)
{
	*error = '\0';

	if (parent_key->value_type != JPARSE_VALUE_TYPE_OBJECT)
		return -1;

	JKEY *prev_key = NULL;
	JKEY *key = parent_key->value.object_val.first_key;

	while (key) {
		if ((key->key_len == child_key->key_len)&&(memcmp(key->key, child_key->key, key->key_len) == 0)) {
			child_key->next_key = key->next_key;
			if (prev_key == NULL) {
				parent_key->value.object_val.first_key = child_key;
			}
			else {
				prev_key->next_key = child_key;
			}
			if (key == parent_key->value.object_val.last_key)
				parent_key->value.object_val.last_key = child_key;
			return 0;
		}
		prev_key = key;
		key = key->next_key;
	}

	if (parent_key->value.object_val.first_key == NULL) {
		parent_key->value.object_val.first_key = child_key;
	}
	else {
		parent_key->value.object_val.last_key->next_key = child_key;
	}
		
	parent_key->value.object_val.last_key = child_key;

	child_key->next_key = NULL;

	return 0;
}

int jparse_validate_object(JOBJECT *object, JKEY_VALIDATOR *validators, size_t validators_count)
{
	unsigned char *ptr = error;

	for (size_t i = 0; i < validators_count; i++) {
		
		validators[i].jkey = NULL;

		JKEY *key = object->first_key;
		
		while (key) {

			if ((key->key_len == validators[i].key_len)&&(memcmp(key->key, validators[i].key, key->key_len) == 0)) {

				validators[i].jkey = key;

				if (validators[i].value_type != key->value_type) {
					
					if ((validators[i].value_type != JPARSE_VALUE_TYPE_BOOLEAN)&&((key->value_type != JPARSE_VALUE_TYPE_TRUE)&&(key->value_type != JPARSE_VALUE_TYPE_FALSE))) {
					
						if ((validators[i].value_type != JPARSE_VALUE_TYPE_NUMBER)&&((key->value_type != JPARSE_VALUE_TYPE_FLOAT))) {

							if ((validators[i].value_type != JPARSE_VALUE_TYPE_FLOAT)&&((key->value_type != JPARSE_VALUE_TYPE_NUMBER))) {

								memcpy(ptr, "Key ", 4);
								ptr += 4;

								memcpy(ptr, key->key, key->key_len);
								ptr += key->key_len;

								memcpy(ptr, " has invalid type", 17);
								ptr += 17;

								*ptr = '\0';

								return -1;
							}
						}
					}
				}

				switch (key->value_type) {

				case JPARSE_VALUE_TYPE_STRING:

					if (key->str_len < validators[i].min_value) {
						memcpy(ptr, "Key ", 4);
						ptr += 4;

						memcpy(ptr, key->key, key->key_len);
						ptr += key->key_len;

						memcpy(ptr, " is too short", 13);
						ptr += 13;

						*ptr = '\0';

						return -1;
					}

					if (key->str_len > validators[i].max_value) {
						memcpy(ptr, "Key ", 4);
						ptr += 4;

						memcpy(ptr, key->key, key->key_len);
						ptr += key->key_len;

						memcpy(ptr, " is too long", 12);
						ptr += 12;

						*ptr = '\0';
						return -1;

					}

					break;

				case JPARSE_VALUE_TYPE_NUMBER:

					if (key->value.int_val < validators[i].min_value) {
						memcpy(ptr, "Key ", 4);
						ptr += 4;

						memcpy(ptr, key->key, key->key_len);
						ptr += key->key_len;

						memcpy(ptr, " is too small", 13);
						ptr += 13;

						*ptr = '\0';

						return -1;
					}

					if ((validators[i].max_value != 0)&&(key->value.int_val > validators[i].max_value)) {

						memcpy(ptr, "Key ", 4);
						ptr += 4;

						memcpy(ptr, key->key, key->key_len);
						ptr += key->key_len;

						memcpy(ptr, " is too big", 11);
						ptr += 11;

						*ptr = '\0';

						return -1;
					}

					break;
				}

				break;
			}

			key = key->next_key;
		}

		if ((validators[i].required)&&(validators[i].jkey == NULL)) {

			memcpy(ptr, "Key ", 4);
			ptr += 4;

			memcpy(ptr, validators[i].key, validators[i].key_len);
			ptr += validators[i].key_len;

			memcpy(ptr, " is required", 12);
			ptr += 12;

			*ptr = '\0';

			return -1;
		}
	}

	return 0;
}

void jparse_minify_object(JOBJECT *object, JKEY_VALIDATOR *validators, size_t validators_count)
{
	JKEY *key = object->first_key;
		
	while (key) {

		size_t i;

		for (i = 0; i < validators_count; i++) {

			if ((key->key_len == validators[i].key_len)&&(memcmp(key->key, validators[i].key, key->key_len) == 0)&&(validators[i].value_type != JPARSE_VALUE_TYPE_UNKNOWN))
				break;
		}

		if (i == validators_count)
			key->value_type = JPARSE_VALUE_TYPE_UNKNOWN;

		key = key->next_key;
	}
}

unsigned char *jparse_get_error(const unsigned char *json, size_t json_len)
{
	if (*error)
		return error;

	return yajl_get_error(parser, 1, json, json_len);
}

void jparse_free_error(unsigned char *err)
{
	if (err != error)
		yajl_free_error(parser, err);	
}

void jparse_init()
{
	parser = yajl_alloc(&callbacks, NULL, &context);  
	yajl_config(parser, yajl_allow_comments, 1);  
	extractor = yajl_alloc(&extractor_callbacks, NULL, &context);  
	yajl_config(extractor, yajl_allow_comments, 1);  
}

void jparse_destroy()
{
	yajl_free(parser);
	yajl_free(extractor);
}

// End
