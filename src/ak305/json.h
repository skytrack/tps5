//******************************************************************************
//
// File Name : json.h
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#ifndef _JSON_H

#define _JSON_H

#include <string>

typedef enum { VALUE_TYPE_STRING = 0, VALUE_TYPE_NUMERIC, VALUE_TYPE_FLOAT, VALUE_TYPE_BOOL, VALUE_TYPE_NULL, VALUE_TYPE_OBJECT, VALUE_TYPE_ARRAY } JSON_VALUE_TYPE;

typedef struct json_interested_key
{
	char			key[32];
	size_t			key_len;
	JSON_VALUE_TYPE	required_type;
	char			is_required;
	size_t			min_string_len;
	size_t			max_string_len;
	char			store_to_db;
	JSON_VALUE_TYPE	actual_type;
	std::string		string_val;
	int				int_val;
	float			float_val;
	bool			bool_val;
	char			is_null;
	char			is_found;
} JSON_INTERESTED_KEY;

void json_init();
void json_destroy();

int json_parse(const unsigned char *data_utf8, size_t data_len, JSON_INTERESTED_KEY *keys, size_t keys_len);

unsigned char *json_get_error(const unsigned char *data_utf8, size_t len);
void json_free_error(unsigned char *err);

const char *json_construct_error(const char *s, size_t *len);
const char *json_construct_success_with_id(unsigned int id, size_t *len);

#endif

// End
