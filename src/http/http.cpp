//******************************************************************************
//
// File Name : http.cpp
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <stack>
#include <limits.h>

#include "../core/likely.h"
#include "../core/module.h"

#include "api.h"
#include "http_parser.h"
#include "http.h"
#include "sha2.h"
#include "rest.h"
#include "static.h"
#include "response.h"

static std::vector<HTTP_SESSION> sessions_vector;
static std::stack<HTTP_SESSION *> sessions_stack;

static size_t buffer_len;
static unsigned char *buffer;

/******************************************************************************/
/* HTTP headers parsing section                                               */
/******************************************************************************/

static void http_extract_params(char *ptr, std::map<std::string, std::string> *result)
{
	std::string key;
	std::string value;

	#define PARAM_SPACE 0
	#define PARAM_NAME	1
	#define PARAM_VALUE	2
	#define PARAM_END	3

	char state = PARAM_SPACE;

	while (*ptr != '\0') {
		switch (state) {
		case PARAM_SPACE:
			if (*ptr == ' ')
				break;
			state = PARAM_NAME;
		case PARAM_NAME:
			if (*ptr == '=') {
				state = PARAM_VALUE;
				break;
			}
			key += *ptr;
			break;
		case PARAM_VALUE:
			if (*ptr == '\r') {
				result->insert(std::pair<std::string,std::string>(key, value));
				state = PARAM_END;
				break;
			}
			value += *ptr;
			break;
		case PARAM_END:
			if (*ptr == ';')
				state = PARAM_SPACE;
			break;
		}
		ptr++;
	}
	result->insert(std::pair<std::string,std::string>(key, value));
}

static int http_handle_field(HTTP_SESSION *s)
{
	char hash[65];
	if (strcmp(s->http_header_field, "Authorization") == 0) {
		
		if (memcmp(s->http_header_value, "Basic ", 6) == 0) {

			SHA256_CTX ctx256;
			SHA256_Init(&ctx256);
			SHA256_Update(&ctx256, (unsigned char *)s->http_header_value + 6, s->zero_init.value_len - 6);
			SHA256_End(&ctx256, hash);
			char *ptr = hash;
			for (unsigned char i = 0; i < 32; i++) {
				s->zero_init.hash[i] = ((*ptr <= '9') ? (*ptr - '0') : (*ptr - 'a' + 10)) << 4;
				ptr++;
				s->zero_init.hash[i] |= (*ptr <= '9') ? (*ptr - '0') : (*ptr - 'a' + 10);
				ptr++;
			}
		}
	}
	else
	if (strcmp(s->http_header_field, "Content-Type") == 0) {
		if (memcmp(s->http_header_value, "application/json", 16) == 0)
			s->zero_init.content_type = 1;
		else
		if (memcmp(s->http_header_value, "multipart/form-data", 19) == 0)
			s->zero_init.content_type = 2;

		char *ptr = strchr(s->http_header_value, ';');
		if (ptr != NULL) {
			http_extract_params(ptr + 1, &s->ct_params);
		}
	}

	return 0;
}

static int http_on_url(http_parser *parser, const char *at, size_t len)
{
	HTTP_SESSION *s = (HTTP_SESSION *)parser->data;

	unsigned int nMaxBytesToAppend = sizeof(s->http_url) - s->zero_init.url_len - 1;

	unsigned int nBytesToAppend;

	if (len > nMaxBytesToAppend) {
		nBytesToAppend = nMaxBytesToAppend;
		s->zero_init.url_too_long = 1;
	}
	else
		nBytesToAppend =  len;

	memcpy(s->http_url, at, nBytesToAppend);
	s->zero_init.url_len += nBytesToAppend;

	s->http_url[s->zero_init.url_len] = '\0';

	s->ct_params.clear();
	
	return 0;
}

static int http_on_header_field(http_parser *parser, const char *at, size_t len)
{
	HTTP_SESSION *s = (HTTP_SESSION *)parser->data;

	if (likely(s->zero_init.value_len != 0)) {
		http_handle_field(s);
		s->zero_init.value_len = 0;
	}

	unsigned int nMaxBytesToAppend = sizeof(s->http_header_field) - s->zero_init.field_len - 1;
	unsigned int nBytesToAppend;
	
	if (len > nMaxBytesToAppend) {
		nBytesToAppend = nMaxBytesToAppend;
		s->zero_init.field_too_long = 1;
	}
	else
		nBytesToAppend = len;

	memcpy(s->http_header_field, at, nBytesToAppend);
	s->zero_init.field_len += nBytesToAppend;

	s->http_header_field[s->zero_init.field_len] = '\0';

	return 0;
}
 
static int http_on_header_value(http_parser *parser, const char *at, size_t len)
{
	HTTP_SESSION *s = (HTTP_SESSION *)parser->data;

	unsigned int nMaxBytesToAppend = sizeof(s->http_header_value) - s->zero_init.value_len - 1;
	unsigned int nBytesToAppend = (len > nMaxBytesToAppend) ? nMaxBytesToAppend : len;

	memcpy(s->http_header_value, at, nBytesToAppend);
	s->zero_init.value_len += nBytesToAppend;

	s->http_header_value[s->zero_init.value_len] = '\0';

	s->zero_init.field_len = 0;
	     
	return 0;
}

static int http_on_headers_complete(http_parser *parser)
{
	HTTP_SESSION *s = (HTTP_SESSION *)parser->data;

	if (likely(s->zero_init.value_len != 0))
		http_handle_field(s);

	return 0;
}

static int http_on_message_complete(http_parser *parser)
{
	HTTP_SESSION *s = (HTTP_SESSION *)parser->data;
	
	s->zero_init.keep_alive = http_should_keep_alive(parser);
	s->zero_init.complete = 1;

	return 0;
}

static int http_on_body(http_parser *parser, const char *at, size_t len)
{
	HTTP_SESSION *s = (HTTP_SESSION *)parser->data;

	unsigned int nMaxBytesToAppend = sizeof(s->http_body) - s->zero_init.body_len - 1;

	unsigned int nBytesToAppend;

	if (len > nMaxBytesToAppend) {
		nBytesToAppend = nMaxBytesToAppend;
		s->zero_init.body_too_long = 1;
	}
	else
		nBytesToAppend =  len;

	memcpy(s->http_body + s->zero_init.body_len, at, nBytesToAppend);
	s->zero_init.body_len += nBytesToAppend;

	s->http_body[s->zero_init.body_len] = '\0';

	return 0;
}

static http_parser_settings settings = { NULL, http_on_url, NULL, http_on_header_field, http_on_header_value, http_on_headers_complete, http_on_body, http_on_message_complete };

HTTP_SESSION *http_session_open()
{
	if (sessions_stack.empty())
		return NULL;

	HTTP_SESSION *s = sessions_stack.top();
	sessions_stack.pop();

	memset(&s->zero_init, 0, sizeof(s->zero_init));

	http_parser_init(&s->parser, HTTP_REQUEST);

	s->parser.data = s;
	s->active = 1;

	return s;
}

void http_session_close(HTTP_SESSION *s)
{
	if (s->active == 1) {
		sessions_stack.push(s);
		s->active = 0;
	}
}

int http_session_timer(HTTP_SESSION *s, char **p, size_t *l)
{
	if (s->active == 1) {
		sessions_stack.push(s);
		s->active = 0;
	}

	*l = 0;

	return HTTP_COMPLETE;
}

int http_session_data(HTTP_SESSION *s, unsigned char **p, size_t *l)
{
	if (unlikely(http_parser_execute(&s->parser, &settings, (char *)*p, *l) != *l)) {

		api_log_printf("[HTTP] Parse error\r\n");

		*p = response_fail_400_close;
		*l = response_fail_400_close_length;

		http_session_close(s);

		return HTTP_COMPLETE;
	}

	if (likely(s->zero_init.complete)) {

		if (unlikely(s->zero_init.url_too_long)) {

			api_log_printf("[HTTP] URL too long\r\n");

			*p = response_fail_414_close;
			*l = response_fail_414_close_length;

			http_session_close(s);

			return HTTP_COMPLETE;
		}

		if (unlikely(s->zero_init.field_too_long)) {

			api_log_printf("[HTTP] Header field too long\r\n");

			*p = response_fail_431_close;
			*l = response_fail_431_close_length;

			http_session_close(s);
		
			return HTTP_COMPLETE;
		}

		if (unlikely(s->zero_init.body_too_long)) {

			api_log_printf("[HTTP] Body too long\r\n");

			*p = response_fail_413_close;
			*l = response_fail_413_close_length;

			http_session_close(s);

			return HTTP_COMPLETE;
		}

		if (s->zero_init.content_type == 2) {
		}

		api_log_printf("%s\r\n", s->http_url);
		if (memcmp(s->http_url, "/static/", 8) == 0) {
			
			static_handle_request(s, p, l);

			if (s->zero_init.keep_alive == 0) {
				http_session_close(s);
				return HTTP_COMPLETE;
			}
			else {
				memset(&s->zero_init, 0, sizeof(s->zero_init));

				http_parser_init(&s->parser, HTTP_REQUEST);

				s->parser.data = s;

				return 60;
			}
		}
		else
		if ((s->http_url[1] == '\0')&&(s->http_url[0] == '/')) {

			if (s->zero_init.keep_alive == 0) {

				*p = response_redirect_close;
				*l = response_redirect_close_length;

				http_session_close(s);

				return HTTP_COMPLETE;
			}
			else {
				*p = response_redirect_ka;
				*l = response_redirect_ka_length;

				memset(&s->zero_init, 0, sizeof(s->zero_init));

				http_parser_init(&s->parser, HTTP_REQUEST);

				s->parser.data = s;

				return 60;
			}
		}
		else {

			*p = s->session_data;
			*l = s->session_data_len;

			if (rest_handle_request(s, p, l) == 0) {

				if (s->zero_init.keep_alive == 0) {
					http_session_close(s);
					return HTTP_COMPLETE;
				}
				else {
					memset(&s->zero_init, 0, sizeof(s->zero_init));

					http_parser_init(&s->parser, HTTP_REQUEST);

					s->parser.data = s;

					return 60;
				}
			}
			
			*l = 0;

			return 120;
		}
	}

	*l = 0;

	api_log_printf("[HTTP] Continue\r\n");

	return 60;
}

int http_start()
{
	buffer_len = 10 * 1024 * 1024;
	buffer = (unsigned char *)malloc(buffer_len);

	sessions_vector.reserve(100);

	for (size_t i = 0; i < 100; i++) {
		
		HTTP_SESSION s;
		
		s.session_data = buffer;
		s.session_data_len = buffer_len;
		s.active = 0;

		sessions_vector.push_back(s);
	}

	for (size_t i = 0; i < sessions_vector.size(); i++)
		sessions_stack.push(&sessions_vector[i]);

	return 0;
}

int http_stop()
{
	if (buffer != NULL) {
		free(buffer);
		buffer = NULL;
	}

	while (!sessions_stack.empty())
		sessions_stack.pop();

	sessions_vector.clear();  

	return 0;
}

// End
