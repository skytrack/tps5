//******************************************************************************
//
// File Name : jparse.h
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#ifndef _JPARSE_H

#define _JPARSE_H

#define JPARSE_VALUE_TYPE_UNKNOWN	0
#define JPARSE_VALUE_TYPE_OBJECT	1
#define JPARSE_VALUE_TYPE_ARRAY		2
#define JPARSE_VALUE_TYPE_STRING	3
#define JPARSE_VALUE_TYPE_NUMBER	4
#define JPARSE_VALUE_TYPE_TRUE		5
#define JPARSE_VALUE_TYPE_FALSE		6
#define JPARSE_VALUE_TYPE_NULL		7
#define JPARSE_VALUE_TYPE_BOOLEAN	8
#define JPARSE_VALUE_TYPE_FLOAT		9

typedef struct jkey JKEY;

typedef struct jobject
{
	JKEY *first_key;
	JKEY *last_key;
	struct jobject *next_object;
} JOBJECT;

typedef struct jarray
{
	JOBJECT *first_object;
	JOBJECT *last_object;
} JARRAY;

typedef struct jkey
{
	char			key[32];
	unsigned char	key_len;

	unsigned char	value_type;

	union {
		unsigned char str_val[255];
		int			int_val;
		float		float_val;
		JOBJECT		object_val;
		JARRAY		array_val;
	} value;

	unsigned char str_len;

	struct jkey *parent_key;
	struct jkey *next_key;

} JKEY;

typedef struct jkey_validator
{
	char			key[32];
	unsigned char	key_len;
	unsigned char	value_type;
	bool			required;
	int				min_value;
	int				max_value;
	JKEY			*jkey;
} JKEY_VALIDATOR;

void jparse_init();
void jparse_destroy();
int jparse_parse(unsigned char *json, size_t json_len, unsigned char *buffer, size_t buffer_size, JKEY *result);
int jparse_extract_key(unsigned char *key, size_t key_len, unsigned char *json, size_t json_len, JKEY *result);
unsigned char *jparse_get_error(const unsigned char *data_utf8, size_t len);
void jparse_free_error(unsigned char *err);
int jparse_build_json(JKEY *key, unsigned char *buffer, size_t *buffer_size);
int jparse_set_key(JKEY *parent_key, JKEY *child_key);
int jparse_validate_object(JOBJECT *object, JKEY_VALIDATOR *validators, size_t validators_count);
void jparse_minify_object(JOBJECT *object, JKEY_VALIDATOR *validators, size_t validators_count);

#endif

// End
