//******************************************************************************
//
// File Name : http.h
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#ifndef _HTTP_H

#define _HTTP_H

#include "http_parser.h"
#include <map>

#define HTTP_COMPLETE 0

#define HTTP_CALLBACK_COMPLETE 0

typedef struct tagHTTP_SESSION
{
	char module_data[64];
	char active;

	struct {

		char complete;
		char keep_alive;
		char content_type;
	
		char body_too_long;
		char url_too_long;
		char field_too_long;

		int  url_len;
		int  body_len;

		char hash[32];

		int  field_len;
		int  value_len;
	} zero_init;

	http_parser parser;
	std::map<std::string, std::string> ct_params;

	char http_header_field[64];
	char http_header_value[128];
	char http_url[1024];

	unsigned char http_body[64 * 1024];
	unsigned char *session_data;
	size_t session_data_len;

} HTTP_SESSION;

typedef int (*HTTP_URI_HANDLER)(HTTP_SESSION *s, char **d, size_t *l);

int http_start();
int http_stop();

HTTP_SESSION *http_session_open();
int http_session_data(HTTP_SESSION *s, unsigned char **p, size_t *l);
void http_session_close(HTTP_SESSION *s);
int http_session_timer(HTTP_SESSION *s, char **p, size_t *l);

#endif

// End
