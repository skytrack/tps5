//******************************************************************************
//
// File Name : json_parse.h
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#ifndef _JSON_PARSE_H

#define _JSON_PARSE_H

#define JSON_PARSE_VALUE_TYPE_UNKNOWN	0
#define JSON_PARSE_VALUE_TYPE_NUMBER	1
#define JSON_PARSE_VALUE_TYPE_STRING	2
#define JSON_PARSE_VALUE_TYPE_OBJECT	3
#define JSON_PARSE_VALUE_TYPE_ARRAY	4
#define JSON_PARSE_VALUE_TYPE_BOOLEAN	5
#define JSON_PARSE_VALUE_TYPE_NULL		6

#define JSON_PARSE_VALUE_SUBTYPE_NONE	0
#define JSON_PARSE_VALUE_SUBTYPE_FLOAT	1

typedef struct jvalue JVALUE;

typedef struct jlist
{
	JVALUE *first_value;
	JVALUE *last_value;
} JLIST;

typedef struct jvalue
{
	unsigned char		key[32];
	unsigned char		key_len;
	unsigned char		type;
	unsigned char		sub_type;
	unsigned char		str_len;

	union {
		unsigned char	str_val[255];
		int				int_val;
		float			float_val;
		JLIST			list_val;
		bool			bool_val;
	} value;

	struct jvalue		*next;
	struct jvalue		*parent;

} JVALUE;

void *json_parse_init();
void json_parse_destroy(void *ctx);
JVALUE *json_parse_parse(void *ctx, unsigned char *json, size_t json_len, unsigned char *buffer, size_t *buffer_size);
unsigned char *json_parse_get_error(void *ctx, const unsigned char *data_utf8, size_t len);
void json_parse_free_error(void *ctx);

#endif

// End
