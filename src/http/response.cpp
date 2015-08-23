//******************************************************************************
//
// File Name : response.cpp
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#include <string>
#include <string.h>
#include <stdio.h>
#include "response.h"

#define RESPONSE_STATUS				"HTTP/1.0 200 OK\r\n"
#define CONTENT_TYPE				"Content-Type: application/json; charset=UTF-8\r\n"
#define SERVER						"Server: attiny2313\r\n"
#define CONNECTION_CLOSE			"Connection: close\r\n"
#define CONNECTION_KEEPALIVE		"Connection: keep-alive\r\n"
#define CONTENT_LENGTH				"Content-Length:              \r\n\r\n"

unsigned char response_redirect_ka[] =	"HTTP/1.0 301 Moved Permanently\r\n"
									CONTENT_TYPE
									SERVER
									CONNECTION_KEEPALIVE
									"Location: /static/index.html\r\n"
									"Content-Length: 0\r\n\r\n";

const size_t response_redirect_ka_length = sizeof(response_redirect_ka) - 1;

unsigned char response_redirect_close[] =	"HTTP/1.0 301 Moved Permanently\r\n"
									CONTENT_TYPE
									SERVER
									CONNECTION_CLOSE
									"Location: /static/index.html\r\n"
									"Content-Length: 0\r\n\r\n";

const size_t response_redirect_close_length = sizeof(response_redirect_close) - 1;


unsigned char response_success_ka[] =		RESPONSE_STATUS
									CONTENT_TYPE
									SERVER
									CONNECTION_KEEPALIVE
									"Content-Length: 16\r\n\r\n"
									"{\"success\":true}";

const size_t response_success_ka_length = sizeof(response_success_ka) - 1;

unsigned char response_success_close[] =		RESPONSE_STATUS
									CONTENT_TYPE
									SERVER
									CONNECTION_CLOSE
									"Content-Length: 16\r\n\r\n"
									"{\"success\":true}";

const size_t response_success_close_length = sizeof(response_success_close) - 1;

unsigned char response_success_empty_object_ka[] =	RESPONSE_STATUS
													CONTENT_TYPE
													SERVER
													CONNECTION_KEEPALIVE
													"Content-Length:29\r\n\r\n"
													"{\"success\":true,\"object\": {}}";

const size_t response_success_empty_object_ka_length = sizeof(response_success_empty_object_ka) - 1;

unsigned char response_success_empty_object_close[] =	RESPONSE_STATUS
													CONTENT_TYPE
													SERVER
													CONNECTION_KEEPALIVE
													"Content-Length:29\r\n\r\n"
													"{\"success\":true,\"object\": {}}";

const size_t response_success_empty_object_close_length = sizeof(response_success_empty_object_close) - 1;

unsigned char response_success_empty_array_ka[] =	RESPONSE_STATUS
													CONTENT_TYPE
													SERVER
													CONNECTION_KEEPALIVE
													"Content-Length:29\r\n\r\n"
													"{\"success\":true,\"object\": []}";

const size_t response_success_empty_array_ka_length = sizeof(response_success_empty_array_ka) - 1;

unsigned char response_success_empty_array_close[] =	RESPONSE_STATUS
													CONTENT_TYPE
													SERVER
													CONNECTION_KEEPALIVE
													"Content-Length:29\r\n\r\n"
													"{\"success\":true,\"object\": []}";

const size_t response_success_empty_array_close_length = sizeof(response_success_empty_array_close) - 1;

unsigned char response_fail_ka[] =			RESPONSE_STATUS
									CONTENT_TYPE
									SERVER
									CONNECTION_KEEPALIVE
									"Content-Length: 17\r\n\r\n"
									"{\"success\":false}";

const size_t response_fail_ka_length = sizeof(response_fail_ka) - 1;

unsigned char response_fail_close[] =		RESPONSE_STATUS
									CONTENT_TYPE
									SERVER
									CONNECTION_CLOSE
									"Content-Length: 17\r\n\r\n"
									"{\"success\":false}";

const size_t response_fail_close_length = sizeof(response_fail_close) - 1;

unsigned char response_fail_msg_ka[] =		RESPONSE_STATUS
									CONTENT_TYPE
									SERVER
									CONNECTION_KEEPALIVE
									CONTENT_LENGTH
									"{\"success\":false,\"message\":\"";

const size_t rresponse_fail_msg_ka_length = sizeof(response_fail_msg_ka) - 1;

unsigned char response_fail_msg_close[] =	RESPONSE_STATUS
									CONTENT_TYPE
									SERVER
									CONNECTION_CLOSE
									CONTENT_LENGTH
									"{\"success\":false,\"message\":\"";

const size_t response_fail_msg_close_length = sizeof(response_fail_msg_close) - 1;

unsigned char response_fail_400_ka[] =		"HTTP/1.0 400 Bad Request\r\n"
									CONTENT_TYPE
								    SERVER
									CONNECTION_KEEPALIVE
									"Content-Length: 28\r\n\r\n"
									"{\"success\":false,\"code\":400}";

const size_t response_fail_400_ka_length = sizeof(response_fail_400_ka) - 1;

unsigned char response_fail_400_close[] =	"HTTP/1.0 400 Bad Request\r\n"
									CONTENT_TYPE
								    SERVER
									CONNECTION_CLOSE
									"Content-Length: 28\r\n\r\n"
									"{\"success\":false,\"code\":400}";

const size_t response_fail_400_close_length = sizeof(response_fail_400_close) - 1;

unsigned char response_fail_401_ka[] =		"HTTP/1.0 401 Unauthorized\r\n"
									"WWW-Authenticate: Basic realm=\"TPS Realm\"\r\n"
									CONTENT_TYPE
								    SERVER
									CONNECTION_KEEPALIVE
									"Content-Length: 28\r\n\r\n"
									"{\"success\":false,\"code\":401}";

const size_t response_fail_401_ka_length = sizeof(response_fail_401_ka) - 1;

unsigned char response_fail_401_close[] =	"HTTP/1.0 401 Unauthorized\r\n"
									"WWW-Authenticate: Basic realm=\"TPS Realm\"\r\n"
									CONTENT_TYPE
								    SERVER
									CONNECTION_CLOSE
									"Content-Length: 28\r\n\r\n"
									"{\"success\":false,\"code\":401}";

const size_t response_fail_401_close_length = sizeof(response_fail_401_close) - 1;

unsigned char response_fail_403_ka[] =		"HTTP/1.0 403 Forbidden\r\n"
									CONTENT_TYPE
								    SERVER
									CONNECTION_KEEPALIVE
									"Content-Length: 28\r\n\r\n"
									"{\"success\":false,\"code\":403}";

const size_t response_fail_403_ka_length = sizeof(response_fail_403_ka) - 1;

unsigned char response_fail_403_close[] =	"HTTP/1.0 403 Forbidden\r\n"
									CONTENT_TYPE
								    SERVER
									CONNECTION_CLOSE
									"Content-Length: 28\r\n\r\n"
									"{\"success\":false,\"code\":403}";

const size_t response_fail_403_close_length = sizeof(response_fail_403_close) - 1;

unsigned char response_fail_404_ka[] =		"HTTP/1.0 404 Not Found\r\n"
									CONTENT_TYPE
								    SERVER
									CONNECTION_KEEPALIVE
									"Content-Length: 28\r\n\r\n"
									"{\"success\":false,\"code\":404}";

const size_t response_fail_404_ka_length = sizeof(response_fail_404_ka) - 1;

unsigned char response_fail_404_close[] =	"HTTP/1.0 404 Not Found\r\n"
									CONTENT_TYPE
								    SERVER
									CONNECTION_CLOSE
									"Content-Length: 28\r\n\r\n"
									"{\"success\":false,\"code\":404}";

const size_t response_fail_404_close_length = sizeof(response_fail_404_close) - 1;

unsigned char response_fail_405_ka[] =		"HTTP/1.0 405 Method Not Allowed\r\n"
									CONTENT_TYPE
								    SERVER
									CONNECTION_KEEPALIVE
									"Content-Length: 28\r\n\r\n"
									"{\"success\":false,\"code\":405}";

const size_t response_fail_405_ka_length = sizeof(response_fail_405_ka) - 1;

unsigned char response_fail_405_close[] =	"HTTP/1.0 405 Method Not Allowed\r\n"
									CONTENT_TYPE
								    SERVER
									CONNECTION_CLOSE
									"Content-Length: 28\r\n\r\n"
									"{\"success\":false,\"code\":405}";

const size_t response_fail_405_close_length = sizeof(response_fail_405_close) - 1;

unsigned char response_fail_413_ka[] =		"HTTP/1.0 413 Request Entity Too Large\r\n"
									CONTENT_TYPE
								    SERVER
									CONNECTION_KEEPALIVE
									"Content-Length: 28\r\n\r\n"
									"{\"success\":false,\"code\":413}";

const size_t response_fail_413_ka_length = sizeof(response_fail_413_ka) - 1;

unsigned char response_fail_413_close[] =	"HTTP/1.0 413 Request Entity Too Large\r\n"
									CONTENT_TYPE
								    SERVER
									CONNECTION_CLOSE
									"Content-Length: 28\r\n\r\n"
									"{\"success\":false,\"code\":413}";

const size_t response_fail_413_close_length = sizeof(response_fail_413_close) - 1;

unsigned char response_fail_414_ka[] =		"HTTP/1.0 414 Request-URI Too Large\r\n"
									CONTENT_TYPE
								    SERVER
									CONNECTION_KEEPALIVE
									"Content-Length: 28\r\n\r\n"
									"{\"success\":false,\"code\":414}";

const size_t response_fail_414_ka_length = sizeof(response_fail_414_ka) - 1;

unsigned char response_fail_414_close[] =	"HTTP/1.0 414 Request-URI Too Large\r\n"
									CONTENT_TYPE
								    SERVER
									CONNECTION_CLOSE
									"Content-Length: 28\r\n\r\n"
									"{\"success\":false,\"code\":414}";

const size_t response_fail_414_close_length = sizeof(response_fail_414_close) - 1;

unsigned char response_fail_415_ka[] =		"HTTP/1.0 415 Unsupported Media Type\r\n"
									CONTENT_TYPE
								    SERVER
									CONNECTION_KEEPALIVE
									"Content-Length: 28\r\n\r\n"
									"{\"success\":false,\"code\":415}";

const size_t response_fail_415_ka_length = sizeof(response_fail_415_ka) - 1;

unsigned char response_fail_415_close[] =	"HTTP/1.0 415 Unsupported Media Type\r\n"
									CONTENT_TYPE
								    SERVER
									CONNECTION_CLOSE
									"Content-Length: 28\r\n\r\n"
									"{\"success\":false,\"code\":415}";

const size_t response_fail_415_close_length = sizeof(response_fail_415_close) - 1;

unsigned char response_fail_422_ka[] =		"HTTP/1.0 422 Unprocessable Entity \r\n"
									CONTENT_TYPE
								    SERVER
									CONNECTION_KEEPALIVE
									"Content-Length: 28\r\n\r\n"
									"{\"success\":false,\"code\":422}";

const size_t response_fail_422_ka_length = sizeof(response_fail_422_ka) - 1;

unsigned char response_fail_422_close[] =	"HTTP/1.0 422 Unprocessable Entity \r\n"
									CONTENT_TYPE
								    SERVER
									CONNECTION_CLOSE
									"Content-Length: 28\r\n\r\n"
									"{\"success\":false,\"code\":422}";

const size_t response_fail_422_close_length = sizeof(response_fail_422_close) - 1;

unsigned char response_fail_431_ka[] =		"HTTP/1.0 431 Request Header Fields Too Large\r\n"
									CONTENT_TYPE
								    SERVER
									CONNECTION_KEEPALIVE
									"Content-Length: 28\r\n\r\n"
									"{\"success\":false,\"code\":431}";

const size_t response_fail_431_ka_length = sizeof(response_fail_431_ka) - 1;

unsigned char response_fail_431_close[] =	"HTTP/1.0 431 Request Header Fields Too Large\r\n"
									CONTENT_TYPE
								    SERVER
									CONNECTION_KEEPALIVE
									"Content-Length: 28\r\n\r\n"
									"{\"success\":false,\"code\":431}";

const size_t response_fail_431_close_length = sizeof(response_fail_431_close) - 1;

unsigned char response_fail_500_ka[] =		"HTTP/1.0 500 Internal Server Error\r\n"
									CONTENT_TYPE
								    SERVER
									CONNECTION_KEEPALIVE
									"Content-Length: 28\r\n\r\n"
									"{\"success\":false,\"code\":500}";

const size_t response_fail_500_ka_length = sizeof(response_fail_500_ka) - 1;

unsigned char response_fail_500_close[] =	"HTTP/1.0 500 Internal Server Error\r\n"
									CONTENT_TYPE
								    SERVER
									CONNECTION_CLOSE
									"Content-Length: 28\r\n\r\n"
									"{\"success\":false,\"code\":500}";

const size_t response_fail_500_close_length = sizeof(response_fail_500_close) - 1;

int response_fail_with_message(unsigned char *dst, size_t *dst_size, const unsigned char *msg, const size_t msg_len, const bool keep_alive)
{
	unsigned char *ptr;
	unsigned char *content_start;
	unsigned char *content_length;

	if (keep_alive) {

		if (*dst_size < sizeof(response_fail_msg_ka) - 1)
			return -1;

		memcpy(dst, response_fail_msg_ka, sizeof(response_fail_msg_ka) - 1);

		ptr = dst + sizeof(response_fail_msg_ka) - 1;
	}
	else {

		if (*dst_size < sizeof(response_fail_msg_close) - 1)
			return -1;

		memcpy(dst, response_fail_msg_close, sizeof(response_fail_msg_close) - 1);

		ptr = dst + sizeof(response_fail_msg_close) - 1;
	}

	content_start = ptr - 28;
	content_length = ptr - 32;

	while (*msg) {
	
		switch (*msg) {
		case '\\':
		case '"':
			*ptr++ = '\\';
			*ptr++ = *msg;
			break;
		case '\b':
			*ptr++ = '\\';
			*ptr++ = 'b';
			break;
		case '\t':
			*ptr++ = '\\';
			*ptr++ = 't';
			break;
		case '\n':
			*ptr++ = '\\';
			*ptr++ = 'n';
			break;
		case '\f':
			*ptr++ = '\\';
			*ptr++ = 'f';
			break;
		case '\r':
			*ptr++ = '\\';
			*ptr++ = 'r';
			break;
		default:
			if (*msg >= ' ')
				*ptr++ = *msg;
			else {
				sprintf((char *)ptr, "\\u%04X", *msg & 0xFF);
				ptr += 6;
			}
		}
		msg++;
	}

	*ptr++ = '\"';
	*ptr++ = '}';

	*dst_size = ptr - dst;

	int cl = ptr - content_start;

    do {
        *--content_length = '0' + cl % 10;
        cl /= 10;
    } while (cl != 0);

	return 0;
}

unsigned char *response_success_object(unsigned char *dst, size_t *dst_size, const bool keep_alive, unsigned char **content_length, size_t *initial_content_length)
{
	static const unsigned char ka[] =	RESPONSE_STATUS
										CONTENT_TYPE
										SERVER
										CONNECTION_KEEPALIVE
										"Content-Length:              \r\n\r\n"
										"{\"success\":true,\"object\":";

	static const unsigned char cl[] =	RESPONSE_STATUS
										CONTENT_TYPE
										SERVER
										CONNECTION_KEEPALIVE
										"Content-Length:              \r\n\r\n"
										"{\"success\":true,\"object\":";

	*initial_content_length = 25;

	if (keep_alive) {

		if (*dst_size < (sizeof(ka) - 1))
			return NULL;

		memcpy(dst, ka, sizeof(ka) - 1);

		*content_length = dst + sizeof(ka) - 30;
		*dst_size -= sizeof(ka) - 1;

		return dst + sizeof(ka) - 1;
	}
	else {

		if (*dst_size < sizeof(cl) - 1)
			return NULL;

		memcpy(dst, cl, sizeof(cl) - 1);

		*content_length = dst + sizeof(cl) - 30;
		*dst_size -= sizeof(cl) - 1;

		return dst + sizeof(cl) - 1;
	}
}

int response_success_id(unsigned char *dst, size_t *dst_size, const bool keep_alive, unsigned int id)
{
	static const unsigned char ka[] =	RESPONSE_STATUS
										CONTENT_TYPE
										SERVER
										CONNECTION_KEEPALIVE
										"Content-Length: 35\r\n\r\n"
										"{\"success\":true,\"id\":             }";

	static const unsigned char cl[] =	RESPONSE_STATUS
										CONTENT_TYPE
										SERVER
										CONNECTION_KEEPALIVE
										"Content-Length: 35\r\n\r\n"
										"{\"success\":true,\"id\":             }";

	if (keep_alive) {

		if (*dst_size < sizeof(ka) - 1)
			return -1;

		memcpy(dst, ka, sizeof(ka) - 1);

		*dst_size = sizeof(ka) - 1;
	}
	else {

		if (*dst_size < sizeof(cl) - 1)
			return -1;

		memcpy(dst, cl, sizeof(cl) - 1);

		*dst_size = sizeof(cl) - 1;
	}

	unsigned char *ptr = dst + *dst_size - 1;

    do {
        *--ptr = '0' + id % 10;
        id /= 10;
    } while (id != 0);

	return 0;
}
/*
unsigned char *response_success_empty_object(unsigned char *dst, size_t dst_size, const bool keep_alive)
{
	static const unsigned char ka[] =	RESPONSE_STATUS
										CONTENT_TYPE
										SERVER
										CONNECTION_KEEPALIVE
										"Content-Length:29\r\n\r\n"
										"{\"success\":true,\"object\": {}}";

	static const unsigned char cl[] =	RESPONSE_STATUS
										CONTENT_TYPE
										SERVER
										CONNECTION_KEEPALIVE
										"Content-Length: 29\r\n\r\n"
										"{\"success\":true,\"object\": {}}";

	if (keep_alive) {

		if (dst_size < sizeof(ka) - 1)
			return NULL;

		memcpy(dst, ka, sizeof(ka) - 1);

		return dst + sizeof(ka) - 1;
	}
	else {

		if (dst_size < sizeof(cl) - 1)
			return NULL;

		memcpy(dst, cl, sizeof(cl) - 1);

		return dst + sizeof(cl) - 1;
	}
}
*/