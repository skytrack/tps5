//******************************************************************************
//
// File Name : rest.h
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#ifndef _REST_H

#define _REST_H

#include "http.h"

#define REST_SUCCESS			1
#define REST_FAIL				2
#define REST_FAIL_WITH_MESSAGE	3

typedef unsigned int (*REST_URI_EXTRACT_ID)(HTTP_SESSION *s, const char *url);
typedef int (*REST_URI_HANDLER)(HTTP_SESSION *s, unsigned int id, unsigned char **d, size_t *l);

typedef struct rest_uri
{
	const char *uri;

	REST_URI_EXTRACT_ID extract_id;

	REST_URI_HANDLER get;
	REST_URI_HANDLER put;
	REST_URI_HANDLER post;
	REST_URI_HANDLER del;

} REST_URI;

typedef struct _user {
	unsigned short user_id;
	std::vector<unsigned short> objects;
} REST_USER;

extern char rest_success_ka[];
extern size_t rest_success_ka_size;
extern char rest_success_close[];
extern size_t rest_success_close_size;

int rest_init();
int rest_destroy();

int rest_handle_request(HTTP_SESSION *s, unsigned char **d, size_t *l);

unsigned int rest_get_self_id(HTTP_SESSION *s, const char *uri);
unsigned int rest_extract_id(HTTP_SESSION *s, const char *uri);

int on_object_create(DB_OBJECT *object);
int on_object_update(DB_OBJECT *object);
int on_object_remove(DB_OBJECT *object);
int on_object_change_parent(DB_OBJECT *object);

REST_USER *rest_find_user(HTTP_SESSION *s);
bool rest_check_access_to_object(REST_USER *user, DB_OBJECT *object);

#endif

// End
