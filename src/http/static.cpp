//******************************************************************************
//
// File Name : static.cpp
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#include <stdio.h>
#include <string.h>
#include "http.h"
#include "static.h"
#include "response.h"

static char filename[2048];
static size_t rootdir_len = 0;

void static_set_rootdir(const char *rootdir)
{
	rootdir_len = strlen(rootdir);
	memcpy(filename, rootdir, rootdir_len);
	if ((filename[rootdir_len] != '/')&&(filename[rootdir_len] != '\\')) {
		filename[rootdir_len] = '/';
		rootdir_len++;
	}
}

int static_handle_request(HTTP_SESSION *s, unsigned char **d, size_t *l)
{
	const char *ptr = s->http_url + 8;
	char *dst = filename + rootdir_len;
	const char *lastDot = NULL;

	if (*ptr == '\0') {
		strcpy(dst, "index.html"); 
		lastDot = dst + 5;
	}
	else {

		while ((*ptr != '\0')&&(*ptr != '?')) {
			if (*ptr == '.')
				lastDot = ptr;
			*dst++ = *ptr++;
		}

		*dst = '\0';
	}

	FILE *f = fopen(filename, "rb");

	if (f == NULL) {
		*d = response_fail_404_ka;
		*l = response_fail_404_ka_length;
		return 0;
	}

	fseek(f, 0, SEEK_END);
	size_t size = ftell(f);
	fseek(f, 0, SEEK_SET);

	if (memcmp(lastDot, ".html", 5) == 0) {
		const char *http_headers =	"HTTP/1.1 200 OK\r\n"
					"Content-Type: text/html; charset=utf-8\r\n"
					"Server: attiny2313\r\n"
					"Connection: Keep-alive\r\n"
					"Content-Length: %u\r\n\r\n";

		int headers_len = sprintf((char *)*d, http_headers, size);

		fread(*d + headers_len, 1, size, f);

		*l = size + headers_len;
	}
	else
	if (memcmp(lastDot, ".js", 3) == 0) {
		const char *http_headers =	"HTTP/1.1 200 OK\r\n"
					"Content-Type: application/x-javascript; charset=utf-8\r\n"
					"Server: attiny2313\r\n"
					"Connection: Keep-alive\r\n"
					"Content-Length: %u\r\n\r\n";

		int headers_len = sprintf((char *)*d, http_headers, size);

		fread(*d + headers_len, 1, size, f);

		*l = size + headers_len;
	}	
	else
	if (memcmp(lastDot, ".css", 4) == 0) {
		const char *http_headers =	"HTTP/1.1 200 OK\r\n"
					"Content-Type: text/css; charset=utf-8\r\n"
					"Server: attiny2313\r\n"
					"Connection: Keep-alive\r\n"
					"Content-Length: %u\r\n\r\n";

		int headers_len = sprintf((char *)*d, http_headers, size);

		fread(*d + headers_len, 1, size, f);

		*l = size + headers_len;
	}	
	else
	if (memcmp(lastDot, ".gif", 4) == 0) {
		const char *http_headers =	"HTTP/1.1 200 OK\r\n"
					"Content-Type: image/gif\r\n"
					"Server: attiny2313\r\n"
					"Connection: Keep-alive\r\n"
					"Content-Length: %u\r\n\r\n";

		int headers_len = sprintf((char *)*d, http_headers, size);

		fread(*d + headers_len, 1, size, f);

		*l = size + headers_len;
	}		
	else
	if (memcmp(lastDot, ".png", 4) == 0) {
		const char *http_headers =	"HTTP/1.1 200 OK\r\n"
					"Content-Type: image/png\r\n"
					"Server: attiny2313\r\n"
					"Connection: Keep-alive\r\n"
					"Content-Length: %u\r\n\r\n";

		int headers_len = sprintf((char *)*d, http_headers, size);

		fread(*d + headers_len, 1, size, f);

		*l = size + headers_len;
	}	
	else
	if (memcmp(lastDot, ".ico", 4) == 0) {
		const char *http_headers =	"HTTP/1.1 200 OK\r\n"
					"Content-Type: image/image/x-icon\r\n"
					"Server: attiny2313\r\n"
					"Connection: Keep-alive\r\n"
					"Content-Length: %u\r\n\r\n";

		int headers_len = sprintf((char *)*d, http_headers, size);

		fread(*d + headers_len, 1, size, f);

		*l = size + headers_len;
	}	

	fclose(f);

	return 0;
}


// End
