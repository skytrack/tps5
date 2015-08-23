//******************************************************************************
//
// File Name : online.cpp
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#include <vector>
#include <string.h>
#include <stdio.h>
#include "http.h"
#include "api.h"
#include "json.h"
#include "response.h"
#include "../core/jparse.h"
#include "../core/db.h"
#include "../core/storage.h"
#include "rest.h"
#include <zlib.h>

static size_t add_user_children_to_online_data(DB_OBJECT *object, unsigned char *ptr, size_t bytes_left)
{
	unsigned char *initial_ptr = ptr;

	size_t children_count = object->sub_objects.size();

	for (size_t i = 0; i < children_count; i++) {

		DB_OBJECT *child = object->sub_objects[i];

		if ((child->type == OBJECT_USER)||(child->type == OBJECT_GROUP)) {
			size_t len = add_user_children_to_online_data(child, ptr, bytes_left);
			bytes_left -= len;
			ptr += len;
		}
		else
		if (child->type >= OBJECT_TERMINAL_MIN) {

			if (child->stream != NULL) {
		
				STREAM_INFO *stream_info = api_storage_get_stream_info(child->stream);

				unsigned char online_pattern[] = "{\"nav_time\":         0,\"lat\":          0,\"lng\":          0,\"cog\":     0,\"alt\":      0,\"flags1\":     0,\"flags2\":     0,\"online\":0,\"id\":    0,\"speed\":    0,\"flags_time\":         0,\"adc1\":    0,\"adc2\":    0,\"adc3\":    0,\"frequency1\":    0,\"frequency2\":    0,\"frequency3\":    0,\"frequency4\":    0,\"rs485_1\":    0,\"rs485_2\":    0},";

				if (bytes_left < sizeof(online_pattern) - 1)
					continue;

				memcpy(ptr, online_pattern, sizeof(online_pattern) - 1);

				unsigned int int_value;
				unsigned short short_value;

				unsigned char *time		= ptr + 22;
				unsigned char *lat		= ptr + 40;
				unsigned char *lng		= ptr + 58;
				unsigned char *cog		= ptr + 71;
				unsigned char *alt		= ptr + 85;
				unsigned char *flags1	= ptr + 101;
				unsigned char *flags2	= ptr + 117;
				unsigned char *online	= ptr + 127;
				unsigned char *id		= ptr + 139;
				unsigned char *speed	= ptr + 153;
				unsigned char *ftime	= ptr + 177;
				unsigned char *adc1		= ptr + 190;
				unsigned char *adc2		= ptr + 203;
				unsigned char *adc3		= ptr + 216;
				unsigned char *frequency1	= ptr + 235;
				unsigned char *frequency2	= ptr + 254;
				unsigned char *frequency3	= ptr + 273;
				unsigned char *frequency4	= ptr + 292;
				unsigned char *rs485_1		= ptr + 308;
				unsigned char *rs485_2		= ptr + 324;

				int_value = stream_info->last_nav_time;

				do {
					*--time = '0' + int_value % 10;
					int_value /= 10;
				} while (int_value != 0);

				int_value = stream_info->last_latitude;

				do {
					*--lat = '0' + int_value % 10;
					int_value /= 10;
				} while (int_value != 0);

				int_value = stream_info->last_longitude;

				do {
					*--lng = '0' + int_value % 10;
					int_value /= 10;
				} while (int_value != 0);

				short_value = stream_info->last_cog;

				do {
					*--cog = '0' + short_value % 10;
					short_value /= 10;
				} while (short_value != 0);

				short_value = stream_info->last_altitude;

				do {
					*--alt = '0' + short_value % 10;
					short_value /= 10;
				} while (short_value != 0);

				short_value = stream_info->last_flags1;

				do {
					*--flags1 = '0' + short_value % 10;
					short_value /= 10;
				} while (short_value != 0);

				short_value = stream_info->last_flags2;

				do {
					*--flags2 = '0' + short_value % 10;
					short_value /= 10;
				} while (short_value != 0);

				if (stream_info->online)
					*online = '1';

				short_value = child->id;

				do {
					*--id = '0' + short_value % 10;
					short_value /= 10;
				} while (short_value != 0);

				short_value = stream_info->last_speed;

				do {
					*--speed = '0' + short_value % 10;
					short_value /= 10;
				} while (short_value != 0);

				int_value = stream_info->last_flags_time;

				do {
					*--ftime = '0' + int_value % 10;
					int_value /= 10;
				} while (int_value != 0);

				short_value = stream_info->last_adc1;

				do {
					*--adc1 = '0' + short_value % 10;
					short_value /= 10;
				} while (short_value != 0);
			
				short_value = stream_info->last_adc2;

				do {
					*--adc2 = '0' + short_value % 10;
					short_value /= 10;
				} while (short_value != 0);

				short_value = stream_info->last_adc3;

				do {
					*--adc3 = '0' + short_value % 10;
					short_value /= 10;
				} while (short_value != 0);

				short_value = stream_info->last_freq1;

				do {
					*--frequency1 = '0' + short_value % 10;
					short_value /= 10;
				} while (short_value != 0);

				short_value = stream_info->last_freq2;

				do {
					*--frequency2 = '0' + short_value % 10;
					short_value /= 10;
				} while (short_value != 0);

				short_value = stream_info->last_freq3;

				do {
					*--frequency3 = '0' + short_value % 10;
					short_value /= 10;
				} while (short_value != 0);

				short_value = stream_info->last_freq4;

				do {
					*--frequency4 = '0' + short_value % 10;
					short_value /= 10;
				} while (short_value != 0);

				short_value = stream_info->last_rs485_1;

				do {
					*--rs485_1 = '0' + short_value % 10;
					short_value /= 10;
				} while (short_value != 0);

				short_value = stream_info->last_rs485_2;

				do {
					*--rs485_2 = '0' + short_value % 10;
					short_value /= 10;
				} while (short_value != 0);

				ptr += sizeof(online_pattern) - 1;
				bytes_left -= sizeof(online_pattern) - 1;
			}
		}
	}

	return ptr - initial_ptr;
}

int online_get(HTTP_SESSION *s, unsigned int id, unsigned char **d, size_t *l)
{
	REST_USER *user = rest_find_user(s);
	if (user == NULL)
		return 401;

	DB_OBJECT *object;
	object = api_db_get_object(id);

	if (object == NULL)
		return 404;

	unsigned char *content_length_ptr;
	size_t content_length;

	unsigned char *ptr = response_success_object(*d, l, s->zero_init.keep_alive > 0, &content_length_ptr, &content_length);

	if (ptr == NULL)
		return 500;

	unsigned char *o_start = ptr - content_length;

	unsigned char *object_start = ptr;

	size_t bytes_left = *l - (ptr - *d);

	if (bytes_left < 1)
		return 500;

	*ptr++ = '[';

	size_t len = add_user_children_to_online_data(object, ptr, bytes_left);

	bytes_left -= len;
	ptr += len;

	if (*(ptr - 1) == ',') {
		ptr--;
		len--;
		bytes_left++;
	}

	if (bytes_left < 2)
		return 500;

	*ptr++ = ']';
	*ptr++ = '}';

	content_length += ptr - object_start;

	if (content_length < 32768) {
		do {
			*--content_length_ptr = '0' + content_length % 10;
			content_length /= 10;
		} while (content_length != 0);

		*l = ptr - *d;

		return 0;
	}

	content_length = ptr - o_start;

	bytes_left -= 2;
	
	const char gzip_headers_ka[] = "HTTP/1.1 200 OK\r\n"
					"Content-Type: application/json; charset=utf-8\r\n"
					"Server: attiny2313\r\n"
					"Connection: Keep-alive\r\n"					
					"Content-Encoding: gzip\r\n"
					"Content-Length:              \r\n\r\n"
					"\37\213\10\0\0\0\0\0\0\377";

	const char gzip_headers_close[] = "HTTP/1.1 200 OK\r\n"
					"Content-Type: application/json; charset=utf-8\r\n"
					"Server: attiny2313\r\n"
					"Connection: Close\r\n"					
					"Content-Encoding: gzip\r\n"
					"Content-Length:              \r\n\r\n"
					"\37\213\10\0\0\0\0\0\0\377";

	unsigned char *response_begin = ptr;

	if (s->zero_init.keep_alive) {

		if (bytes_left < sizeof(gzip_headers_ka) - 1)
			return 500;

		memcpy(ptr, gzip_headers_ka, sizeof(gzip_headers_ka) - 1);

		content_length_ptr = ptr + sizeof(gzip_headers_ka) - 15;

		ptr += sizeof(gzip_headers_ka) - 1;

		bytes_left -= sizeof(gzip_headers_ka) - 1;
	}
	else {

		if (bytes_left < sizeof(gzip_headers_close) - 1)
			return 500;

		memcpy(ptr, gzip_headers_close, sizeof(gzip_headers_close) - 1);

		content_length_ptr = ptr + sizeof(gzip_headers_close) - 15;

		ptr += sizeof(gzip_headers_close) - 1;

		bytes_left -= sizeof(gzip_headers_close) - 1;
	}

	unsigned char *gzip_begin = ptr;

	z_stream zs;

	zs.zalloc    = (alloc_func)0;
	zs.zfree     = (free_func)0;
	zs.opaque    = (voidpf)0;

	zs.next_in   = (Byte*)o_start;
	zs.avail_in  = content_length;

	zs.next_out  = (Byte*)gzip_begin;
	zs.avail_out = bytes_left;

	if (deflateInit2(&zs, 9, Z_DEFLATED, -MAX_WBITS, 8, Z_DEFAULT_STRATEGY) != Z_OK) {
		api_log_printf("[HTTP] deflateInit2 error\r\n");
		deflateEnd(&zs);
		return 500;
	}

	if (deflate(&zs, Z_FINISH) != Z_STREAM_END) {
		api_log_printf("[HTTP] deflate error\r\n");
		deflateEnd(&zs);
		return 500;
	}

	deflateEnd(&zs);

	ptr += zs.total_out;

	bytes_left -= zs.total_out;

	if (bytes_left < 8)
		return 500;

	*(unsigned int *)ptr = crc32(crc32(0, NULL, 0), o_start, content_length);
	ptr += 4;
	*(unsigned int *)ptr = content_length;
	ptr += 4;

	content_length = ptr - gzip_begin + 10;

    do {
        *--content_length_ptr = '0' + content_length % 10;
        content_length /= 10;
    } while (content_length != 0);

	*d = response_begin;
	*l = ptr - response_begin;

	return 0;
}