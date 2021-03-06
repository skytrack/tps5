//******************************************************************************
//
// File Name : json.cpp
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include "json.h"
#include "../lib/yajl/src/api/yajl_parse.h"

static JSON_INTERESTED_KEY *keys;
static size_t keys_len;

static yajl_handle parser;

static int current_key, current_level;
static char my_error[1024];

static int parse_null(void *ctx)  
{
	if (current_key != -1) {
		keys[current_key].is_null = 1;
		keys[current_key].is_found = 1;
		current_key = -1;
	}

	return 1;
}  
  
static int parse_boolean(void * ctx, int boolean)  
{  
	if (current_key != -1) {
		keys[current_key].bool_val = boolean > 0;
		keys[current_key].actual_type = VALUE_TYPE_BOOL;
		keys[current_key].is_found = 1;
		current_key = -1;
	}

	return 1;
}  
  
static int parse_number(void * ctx, const char * s, size_t l)  
{  
	if (current_key != -1) {
		char *ptr;
		
		keys[current_key].int_val = strtoul(s, &ptr, 10);

		if (*ptr != '.') {
			keys[current_key].actual_type = VALUE_TYPE_NUMERIC;
		}
		else {
			keys[current_key].float_val = (float)strtod(s, &ptr);
			keys[current_key].actual_type = VALUE_TYPE_FLOAT;
		}
		keys[current_key].is_found = 1;
		current_key = -1;
	}

	return 1;
}  
  
static int parse_string(void * ctx, const unsigned char * stringVal, size_t stringLen)  
{  
	if (current_key != -1) {
		keys[current_key].string_val.clear();
		keys[current_key].string_val.insert(0, (const char *)stringVal, stringLen);
		keys[current_key].actual_type = VALUE_TYPE_STRING;
		keys[current_key].is_found = 1;
		current_key = -1;
	}
	return 1;
}  
  
static int parse_map_key(void * ctx, const unsigned char * stringVal, size_t stringLen)  
{
	if (current_level == 1) {

		for (size_t i = 0; i < keys_len; i++) {

			if ((stringLen == keys[i].key_len)&&(memcmp(keys[i].key, stringVal, stringLen) == 0)) {
				current_key = i;
				break;
			}
		}
	}

	return 1;
}  
  
static int parse_start_map(void * ctx)  
{
	current_level++;

	if (current_key != -1) {
		keys[current_key].actual_type = VALUE_TYPE_OBJECT;
		keys[current_key].is_found = 1;
		current_key = -1;
	}

	return 1;
}  
   
static int parse_end_map(void * ctx)  
{
	current_level--;
	
	return 1;
}  
  
static int parse_start_array(void * ctx)  
{  
	if (current_key != -1) {
		keys[current_key].actual_type = VALUE_TYPE_ARRAY;
		keys[current_key].is_found = 1;
		current_key = -1;
	}

	return 1;
}  
  
static int parse_end_array(void * ctx)  
{  	
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
    
int json_parse(const unsigned char *data_utf8, size_t data_len, JSON_INTERESTED_KEY *_keys, size_t _keys_len)
{
	yajl_status stat; 
 
	keys = _keys;
	keys_len = _keys_len;
	current_level = 0;
	current_key = -1;
	
	*my_error = '\0';

	for (size_t i = 0; i < keys_len; i++) {
		keys[i].is_null = 0;
		keys[i].is_found = 0;
	}

	yajl_clear(parser);

	stat = yajl_parse(parser, data_utf8, data_len);  
    
	if (stat == yajl_status_ok) {

		stat = yajl_complete_parse(parser);  

		if (stat == yajl_status_ok) {

			for (size_t i = 0; i < keys_len; i++) {
		
				if ((keys[i].is_required != 0)&&(keys[i].is_found == 0)) {
			
					sprintf(my_error, "%s is a mandatory key", keys[i].key);
			
					return -1;
				}

				if ((keys[i].is_required != 0)&&(keys[i].is_null != 0)) {
			
					sprintf(my_error, "%s can't be null", keys[i].key);
			
					return -1;
				}

				if (keys[i].is_found != 0) {
			
					if ((keys[i].required_type == VALUE_TYPE_FLOAT) && (keys[i].actual_type == VALUE_TYPE_NUMERIC)) {
						keys[i].actual_type = VALUE_TYPE_FLOAT;
						keys[i].float_val = (float)keys[i].int_val;
					}

					if (keys[i].required_type != keys[i].actual_type) {
			
						sprintf(my_error, "Key %s has wrong type", keys[i].key);
			
						return -1;
					}

					if (keys[i].required_type == VALUE_TYPE_STRING) {

						if ((keys[i].min_string_len == keys[i].max_string_len)&&(keys[i].string_val.length() != keys[i].min_string_len)) {
							sprintf(my_error, "Key %s must be exactly %d characters length", keys[i].key, keys[i].min_string_len);
							return -1;
						}
						else
						if ((keys[i].string_val.length() < keys[i].min_string_len)||(keys[i].string_val.length() > keys[i].max_string_len)) {
							sprintf(my_error, "Key %s length must be in [%u, %u] interval", keys[i].key, keys[i].min_string_len, keys[i].max_string_len);			
							return -1;
						}
					}
					else
					if (keys[i].required_type == VALUE_TYPE_NUMERIC) {

						if (keys[i].int_val < (int)keys[i].min_string_len) {
							sprintf(my_error, "Key %s must be greater or equal to %d", keys[i].key, keys[i].min_string_len);
							return -1;
						}
						else
						if ((keys[i].max_string_len != 0)&&(keys[i].int_val > (int)keys[i].max_string_len)) {
							sprintf(my_error, "Key %s must be less or equal to %d", keys[i].key, keys[i].max_string_len);
							return -1;
						}
					}
				}
			}

			return 0;
		}
	}
  
	return -1; 
}

unsigned char *json_get_error(const unsigned char *data_utf8, size_t len)
{
	if (*my_error != '\0') 
		return (unsigned char *)my_error;

	return yajl_get_error(parser, 1, data_utf8, len);
}

void json_free_error(unsigned char *err)
{
	if (err == (unsigned char *)my_error)
		return;

	yajl_free_error(parser, err);	
}

void json_init()
{
	parser = yajl_alloc(&callbacks, NULL, NULL);  
	yajl_config(parser, yajl_allow_comments, 1);  
	*my_error = 0;
}

void json_destroy()
{
	yajl_free(parser);
}

// End
