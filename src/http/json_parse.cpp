//******************************************************************************
//
// File Name : json_parse.cpp
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include "json_parse.h"
#include "../lib/yajl/src/api/yajl_parse.h"

typedef struct ctx
{
	unsigned char	*ptr;
	size_t			bytes_left;
	JVALUE			*value;
	JVALUE			*next_value;
	yajl_handle		parser;
	unsigned char	*perror;
	unsigned char	error[1024];
} CTX;

static int add_next_value(CTX *context)  
{	
	JVALUE *this_value = context->next_value;

	context->bytes_left -= sizeof(JVALUE);

	if (context->bytes_left < sizeof(JVALUE)) {

		strcpy((char *)context->error, "Not enough memory");

		return 0;
	}

	context->next_value = (JVALUE *)context->ptr;
	context->next_value->key_len = 0;
	context->ptr += sizeof(JVALUE);

	this_value->parent = context->value;
	this_value->next = NULL;

	if (context->value != NULL) {

		if ((context->value->type == JSON_PARSE_VALUE_TYPE_OBJECT)||(context->value->type == JSON_PARSE_VALUE_TYPE_ARRAY)) {
			JLIST *list = &context->value->value.list_val;
			if (list->first_value == NULL) {
				list->first_value = this_value;
			}
			else {
				list->last_value->next = this_value;
			}
			list->last_value = this_value;
		}
	}

	if ((this_value->type == JSON_PARSE_VALUE_TYPE_OBJECT)||(this_value->type == JSON_PARSE_VALUE_TYPE_ARRAY))
		context->value = this_value;

	return 1;
}  

static int parse_null(void *ctx)  
{
	CTX *context = (CTX *)ctx;

	context->next_value->type		= JSON_PARSE_VALUE_TYPE_NULL;
	context->next_value->sub_type	= JSON_PARSE_VALUE_SUBTYPE_NONE;

	return add_next_value(context);
}  
  
static int parse_boolean(void * ctx, int boolean)  
{  
	CTX *context = (CTX *)ctx;

	context->next_value->type			= JSON_PARSE_VALUE_TYPE_BOOLEAN;
	context->next_value->sub_type		= JSON_PARSE_VALUE_SUBTYPE_NONE;
	context->next_value->value.bool_val	= (boolean > 0);

	return add_next_value(context);
}

static int parse_number(void * ctx, const char * s, size_t l)  
{  
	CTX *context = (CTX *)ctx;

	char *ptr;

	context->next_value->type = JSON_PARSE_VALUE_TYPE_NUMBER;
	
	context->next_value->value.int_val = strtol(s, &ptr, 10);

	if (*ptr == '.') {
		context->next_value->value.float_val = (float)strtod(s, &ptr);
		context->next_value->sub_type = JSON_PARSE_VALUE_SUBTYPE_FLOAT;
	}
	else {
		context->next_value->sub_type = JSON_PARSE_VALUE_SUBTYPE_NONE;
	}

	return add_next_value(context);
}  
  
static int parse_string(void * ctx, const unsigned char * stringVal, size_t stringLen)  
{  
	CTX *context = (CTX *)ctx;

	if (stringLen > sizeof(context->next_value->value.str_val)) {
		context->next_value->str_len = sizeof(context->next_value->value.str_val);
	}
	else {
		context->next_value->str_len = stringLen;
	}

	memcpy(context->next_value->value.str_val, stringVal, context->next_value->str_len);

	context->next_value->type		= JSON_PARSE_VALUE_TYPE_STRING;
	context->next_value->sub_type	= JSON_PARSE_VALUE_SUBTYPE_NONE;

	return add_next_value(context);
}  
  
static int parse_map_key(void * ctx, const unsigned char * stringVal, size_t stringLen)  
{
	CTX *context = (CTX *)ctx;

	if (stringLen > sizeof(context->next_value->key)) {
		context->next_value->key_len = sizeof(context->next_value->key);
	}
	else {
		context->next_value->key_len = stringLen;
	}
	
	memcpy(context->next_value->key, stringVal, context->next_value->key_len);

	return 1;
}  
  
static int parse_start_map(void * ctx)  
{
	CTX *context = (CTX *)ctx;

	context->next_value->type		= JSON_PARSE_VALUE_TYPE_OBJECT;
	context->next_value->sub_type	= JSON_PARSE_VALUE_SUBTYPE_NONE;

	context->next_value->value.list_val.first_value = NULL;

	return add_next_value(context);
}  
   
static int parse_end_map(void * ctx)  
{
	CTX *context = (CTX *)ctx;
	
	while (context->value->type != JSON_PARSE_VALUE_TYPE_OBJECT)
		context->value = context->value->parent;

	context->value = context->value->parent;

	return 1;
}  
  
static int parse_start_array(void * ctx)  
{
	CTX *context = (CTX *)ctx;

	context->next_value->type		= JSON_PARSE_VALUE_TYPE_ARRAY;
	context->next_value->sub_type	= JSON_PARSE_VALUE_SUBTYPE_NONE;

	context->next_value->value.list_val.first_value = NULL;

	return add_next_value(context);
}  
  
static int parse_end_array(void * ctx)  
{  	
	CTX *context = (CTX *)ctx;
	
	while (context->value->type != JSON_PARSE_VALUE_TYPE_ARRAY)
		context->value = context->value->parent;

	context->value = context->value->parent;

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
    
JVALUE *json_parse_parse(void *ctx, unsigned char *json, size_t json_len, unsigned char *buffer, size_t *buffer_size)
{
	CTX *context = (CTX *)ctx;

	yajl_status stat; 
 
	yajl_clear(context->parser);

	if (*buffer_size < sizeof(JVALUE)) {

		strcpy((char *)context->error, "Not enough memory");

		return NULL;
	}

	context->value		= NULL;
	context->error[0]	= '\0';
	context->bytes_left	= *buffer_size - sizeof(JVALUE);
	context->ptr		= buffer + sizeof(JVALUE);
	context->next_value	= (JVALUE *)buffer;
	context->next_value->key_len = 0;

	stat = yajl_parse(context->parser, json, json_len);  
    
	if (stat == yajl_status_ok) {

		stat = yajl_complete_parse(context->parser);  

		if (stat == yajl_status_ok) {

			*buffer_size = context->ptr - buffer;

			return (JVALUE *)buffer;
		}
	}
  
	return NULL; 
}

unsigned char *json_parse_get_error(void *ctx, const unsigned char *json, size_t json_len)
{
	CTX *context = (CTX *)ctx;

	if (context->error[0])
		return context->error;

	if (context->perror != NULL)
		yajl_free_error(context->parser, context->perror);	

	context->perror = yajl_get_error(context->parser, 1, json, json_len);

	return context->perror;
}

void json_parse_free_error(void *ctx)
{
	CTX *context = (CTX *)ctx;

	if (context->perror != NULL) {
		yajl_free_error(context->parser, context->perror);	
		context->perror = NULL;
	}
}

void *json_parse_init()
{
	CTX *context = (CTX *)malloc(sizeof(CTX));
	
	if (context == NULL)
		return NULL;

	context->perror		= NULL;
	context->error[0]	= 0;
	context->parser		= yajl_alloc(&callbacks, NULL, context);  
	
	yajl_config(context->parser, yajl_allow_comments, 1);  

	return context;
}

void json_parse_destroy(void *ctx)
{
	CTX *context = (CTX *)ctx;

	if (context->perror != NULL)
		yajl_free_error(context->parser, context->perror);	

	yajl_free(context->parser);

	free(context);
}

// End
