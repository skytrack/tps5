//******************************************************************************
//
// File Name : object.cpp
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#include <stdio.h>
#include <string.h>
#include <zlib.h>
#include "api.h"
#include "rest.h"
#include "json.h"
#include "sha2.h"
#include "object.h"
#include "user.h"
#include "device.h"
#include "response.h"
#include "group.h"
#include "../core/jparse.h"

static int add_object_to_tree(DB_OBJECT *object, unsigned char **d, size_t *l)
{
	unsigned char nav_pattern[] = ",\"last_nav\":{\"time\":         0,\"lat\":          0,\"lng\":          0,\"speed\":    0,\"cog\":     0,\"alt\":      0}";
	
	unsigned char data_pattern[] = ",\"last_data\":{\"time\":         0,\"flags1\":     0,\"flags2\":     0,\"adc1\":    0,\"adc2\":    0,\"adc3\":    0,\"frequency1\":    0,\"frequency2\":    0,\"frequency3\":    0,\"frequency4\":    0,\"rs485_1\":    0,\"rs485_2\":    0}";

	unsigned char info_pattern[] = ",\"info\":";

	unsigned char online_pattern[] = ",\"online\":true";
	unsigned char offline_pattern[] = ",\"online\":false";

	if (*l < (object->core_data_size - 1))
		return -1;

	size_t children_count = object->sub_objects.size();

	memcpy(*d, object->core_data, object->core_data_size - 1);

	*d += object->core_data_size - 1;
	*l -= object->core_data_size - 1;

	if (object->type >= 1000) {

		STREAM_INFO *stream_info = api_storage_get_stream_info(object->stream);

		if (stream_info != NULL) {

			if ((stream_info->last_nav_time != 0)&&(*l > sizeof(nav_pattern))) {

				memcpy(*d, nav_pattern, sizeof(nav_pattern) - 1);

				unsigned int int_value;
				unsigned short short_value;

				unsigned char *time		= *d + 30;
				unsigned char *lat		= *d + 48;
				unsigned char *lng		= *d + 66;
				unsigned char *speed	= *d + 80;
				unsigned char *cog		= *d + 93;
				unsigned char *alt		= *d + 107;

				*d += sizeof(nav_pattern) - 1;
				*l -= sizeof(nav_pattern) - 1;

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

				short_value = stream_info->last_speed;

				do {
					*--speed = '0' + short_value % 10;
					short_value /= 10;
				} while (short_value != 0);

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
			}

			if ((stream_info->last_flags_time != 0)&&(*l > sizeof(data_pattern))) {

				memcpy(*d, data_pattern, sizeof(data_pattern) - 1);

				unsigned char data_pattern[] = "\"last_data\":{\"time\":         0,\"flags1\":     0,\"flags2\":     0,\"adc1\":    0,\"adc2\":    0,\"adc3\":    0,\"frequency1\":    0,\"frequency2\":    0,\"frequency3\":    0,\"frequency4\":    0,\"rs485_1\":    0,\"rs485_2\":    0},";

				unsigned char *time			= *d + 31;
				unsigned char *flags1		= *d + 47;
				unsigned char *flags2		= *d + 63;
				unsigned char *adc1			= *d + 76;
				unsigned char *adc2			= *d + 89;
				unsigned char *adc3			= *d + 102;
				unsigned char *frequency1	= *d + 121;
				unsigned char *frequency2	= *d + 140;
				unsigned char *frequency3	= *d + 159;
				unsigned char *frequency4	= *d + 178;
				unsigned char *rs485_1		= *d + 194;
				unsigned char *rs485_2		= *d + 210;

				unsigned int int_value;
				unsigned short short_value;

				*d += sizeof(data_pattern) - 1;
				*l -= sizeof(data_pattern) - 1;

				int_value = stream_info->last_flags_time;

				do {
					*--time = '0' + int_value % 10;
					int_value /= 10;
				} while (int_value != 0);

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
			}

			if (*l > (sizeof(info_pattern) + 513)) {

				memcpy(*d, info_pattern, sizeof(info_pattern) - 1);

				size_t info_len = 0;

				MODULE *module = api_get_device_module(object->type);
				if ((module != NULL)&&((info_len = module->get_info(object, *d + 8, 512)) > 0)) {
					*d += sizeof(info_pattern) - 1 + info_len;
					*l -= sizeof(info_pattern) - 1 + info_len;
				}
			}

			if ((stream_info->online)&&(*l > sizeof(online_pattern))) {

				memcpy(*d, online_pattern, sizeof(online_pattern) - 1);

				*d += sizeof(online_pattern) - 1;
				*l -= sizeof(online_pattern) - 1;
			}

 			if ((stream_info->online == 0)&&(*l > sizeof(offline_pattern))) {

				memcpy(*d, offline_pattern, sizeof(offline_pattern) - 1);

				*d += sizeof(offline_pattern) - 1;
				*l -= sizeof(offline_pattern) - 1;
			}
		}
	}

	if (children_count > 0) {

		if (*l < 13)
			return -1;

		memcpy(*d, ",\"children\":[", 13);
		*d += 13;
		*l -= 13;

		for (size_t i = 0; i < children_count; i++) {

			if (i > 0) {
				if (*l < 1)
					return -1;
				**d = ','; *d += 1; (*l)--;
			}

			if (add_object_to_tree(object->sub_objects[i], d, l))
				return -1;
		}
		
		if (*l < 1)
			return -1;

		**d = ']'; *d += 1; (*l)--;

		*l -= 1;
	}

	if (*l < 1)
		return -1;

	**d = '}'; *d += 1; (*l)--;

	return 0;
}

int object_get_tree(HTTP_SESSION *s, unsigned int id, unsigned char **d, size_t *l)
{
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

	DB_OBJECT *object;
	object = api_db_get_object(id);

	if (object == NULL)
		return 404;

	REST_USER *user = rest_find_user(s);
	if (user == NULL)
		return 401;

	if (!rest_check_access_to_object(user, object))
		return 401;

	unsigned char *ptr = *d;
	unsigned char *content_length_ptr;

	if (*l < 25)
		return 500;

	memcpy(ptr, "{\"success\":true,\"object\":", 25);
	ptr += 25;
	*l -= 25;

	if (add_object_to_tree(object, &ptr, l))
		return 500;

	if (*l < 1)
		return 500;

	*ptr++ = '}';
	(*l)--;

	size_t data_len = ptr - *d;

	unsigned char *response_begin = ptr;

	if (s->zero_init.keep_alive) {

		if (*l < sizeof(gzip_headers_ka) - 1)
			return 500;

		memcpy(ptr, gzip_headers_ka, sizeof(gzip_headers_ka) - 1);

		content_length_ptr = ptr + sizeof(gzip_headers_ka) - 15;

		ptr += sizeof(gzip_headers_ka) - 1;

		*l -= sizeof(gzip_headers_ka) - 1;
	}
	else {

		if (*l < sizeof(gzip_headers_close) - 1)
			return 500;

		memcpy(ptr, gzip_headers_close, sizeof(gzip_headers_close) - 1);

		content_length_ptr = ptr + sizeof(gzip_headers_close) - 15;

		ptr += sizeof(gzip_headers_close) - 1;

		*l -= sizeof(gzip_headers_close) - 1;
	}

	unsigned char *gzip_begin = ptr;

	z_stream zs;

	zs.zalloc    = (alloc_func)0;
	zs.zfree     = (free_func)0;
	zs.opaque    = (voidpf)0;

	zs.next_in   = (Byte*)*d;
	zs.avail_in  = data_len;

	zs.next_out  = (Byte*)gzip_begin;
	zs.avail_out = *l;

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

	*l -= zs.total_out;

	if (*l < 8)
		return 500;

	*(unsigned int *)ptr = crc32(crc32(0, NULL, 0), *d, data_len);
	ptr += 4;
	*(unsigned int *)ptr = data_len;
	ptr += 4;

	size_t content_length = ptr - gzip_begin + 10;

    do {
        *--content_length_ptr = '0' + content_length % 10;
        content_length /= 10;
    } while (content_length != 0);

	*d = response_begin;
	*l = ptr - response_begin;

	return 0;
}

int object_get(HTTP_SESSION *s, unsigned int id, unsigned char **d, size_t *l)
{
	DB_OBJECT *object;
	object = api_db_get_object(id);

	if (object == NULL)
		return 404;

	REST_USER *user = rest_find_user(s);
	if (user == NULL)
		return 401;

	if (!rest_check_access_to_object(user, object))
		return 401;

	unsigned char *content_length_ptr;
	size_t content_length;

	unsigned char *ptr = response_success_object(*d, l, s->zero_init.keep_alive > 0, &content_length_ptr, &content_length);

	if (ptr == NULL)
		return 500;

	unsigned char *object_start = ptr;

	size_t bytes_left = *l - (ptr - *d);

	if (bytes_left < object->core_data_size)
		return 500;

	size_t children_count = object->sub_objects.size();

	if (children_count == 0) {

		memcpy(ptr, object->core_data, object->core_data_size);

		ptr += object->core_data_size;
		bytes_left -= object->core_data_size;
	}
	else {
		memcpy(ptr, object->core_data, object->core_data_size);

		ptr += object->core_data_size - 1;
		bytes_left -= object->core_data_size - 1;

		if (bytes_left < 13)
			return 500;

		memcpy(ptr, ",\"children\":[", 13);
		ptr += 13;
		bytes_left -= 13;

		for (size_t i = 0; i < children_count; i++) {

			if (i > 0) {
				if (bytes_left < 1)
					return 500;
				*ptr++ = ',';
			}

			if (object->sub_objects[i]->sub_objects.size() == 0) {
				
				if (bytes_left < object->sub_objects[i]->core_data_size)
					return 500;

				memcpy(ptr, object->sub_objects[i]->core_data, object->sub_objects[i]->core_data_size);

				ptr += object->sub_objects[i]->core_data_size;
				bytes_left -= object->sub_objects[i]->core_data_size;
			}
			else {

				if (bytes_left < (object->sub_objects[i]->core_data_size + 12))
					return 500;

				memcpy(ptr, object->sub_objects[i]->core_data, object->sub_objects[i]->core_data_size - 1);
				ptr += object->sub_objects[i]->core_data_size - 1;
				memcpy(ptr, ",\"more\":true}", 13);
				ptr += 13;

				bytes_left -= object->sub_objects[i]->core_data_size + 12;
			}
		}
		
		if (bytes_left < 2)
			return 500;

		*ptr++ = ']'; 
		*ptr++ = '}';
	}

	if (bytes_left < 1)
		return 500;

	*ptr++ = '}';

	content_length += ptr - object_start;

    do {
        *--content_length_ptr = '0' + content_length % 10;
        content_length /= 10;
    } while (content_length != 0);

	*l = ptr - *d;

	return 0;
}

int object_report(HTTP_SESSION *s, DB_OBJECT *object, unsigned char **d, size_t *l)
{
	unsigned char *content_length_ptr;
	size_t content_length;

	unsigned char *ptr = response_success_object(*d, l, s->zero_init.keep_alive > 0, &content_length_ptr, &content_length);

	if (ptr == NULL)
		return 500;

	unsigned char *object_start = ptr;

	size_t bytes_left = *l - (ptr - *d);

	if (bytes_left < (object->core_data_size + 1))
		return 500;

	memcpy(ptr, object->core_data, object->core_data_size);
	ptr += object->core_data_size;

	*ptr++ = '}';

	content_length += ptr - object_start;

    do {
        *--content_length_ptr = '0' + content_length % 10;
        content_length /= 10;
    } while (content_length != 0);

	*l = ptr - *d;

	return 0;
}

int object_post(HTTP_SESSION *s, unsigned int id, unsigned char **d, size_t *l)
{
	int result = 500;

	DB_OBJECT *object;
	object = api_db_get_object(id);

	if (object == NULL)
		return 404;

	REST_USER *user = rest_find_user(s);
	if (user == NULL)
		return 401;

	if (!rest_check_access_to_object(user, object))
		return 401;

	if (object->type == OBJECT_USER) {
		result = user_post(s, object, d, l);
	}
	else
	if (object->type >= OBJECT_TERMINAL_MIN) {
		result = device_post(s, object, d, l);
	}
	else
	if (object->type == OBJECT_GROUP) {
		result = group_post(s, object, d, l);
	}
	
	on_object_update(object);

	return result;
}

int object_delete(HTTP_SESSION *s, unsigned int id, unsigned char **d, size_t *l)
{
	DB_OBJECT *object;
	object = api_db_get_object(id);

	if (object == NULL)
		return 404;

	REST_USER *user = rest_find_user(s);
	if (user == NULL)
		return 401;

	if (user->user_id == id)
		return 404;

	if (!rest_check_access_to_object(user, object))
		return 401;

	DB_OBJECT o = *object;
	o.core_data = NULL;
	o.core_data_size = 0;
	o.module_data = NULL;
	o.module_data_size = 0;
	o.stream = NULL;
	o.sub_objects.clear();

	int res = api_db_delete_object(object);

	if (res == DB_OK) {
		on_object_remove(&o);
		return REST_SUCCESS;
	}

	unsigned char *error = (unsigned char *)api_db_get_error();

	response_fail_with_message(*d, l, error, strlen((char *)error), s->zero_init.keep_alive > 0);

	return 0;
}

int object_parent(HTTP_SESSION *s, unsigned int id, unsigned char **d, size_t *l)
{
	DB_OBJECT *object;
	object = api_db_get_object(id);

	if (object == NULL)
		return 404;

	REST_USER *user = rest_find_user(s);
	if (user == NULL)
		return 401;

	if (user->user_id == id)
		return 404;

	if (!rest_check_access_to_object(user, object))
		return 401;

	unsigned char *existing_key_buffer = *d;
	const size_t existing_key_buffer_size = *l / 3;
	unsigned char *incoming_key_buffer = existing_key_buffer + existing_key_buffer_size;
	const size_t incoming_key_buffer_size = *l / 3;
	unsigned char *new_key_buffer = incoming_key_buffer + incoming_key_buffer_size;
	const size_t new_key_buffer_size = *l / 3;

	JKEY incoming_key;

	if (jparse_parse(s->http_body, s->zero_init.body_len, incoming_key_buffer, incoming_key_buffer_size, &incoming_key)) {

		unsigned char *error = jparse_get_error(s->http_body, s->zero_init.body_len);

		response_fail_with_message(*d, l, error, strlen((char *)error), s->zero_init.keep_alive > 0);

		jparse_free_error(error);

		return 0;
	}

	if (incoming_key.value_type != JPARSE_VALUE_TYPE_OBJECT)
		return 500;

	if ((incoming_key.value.object_val.first_key->key_len != 9)&&(memcmp(incoming_key.value.object_val.first_key->key, "parent_id", 9)))
		return 500;

	unsigned int parent_id = incoming_key.value.object_val.first_key->value.int_val;

	DB_OBJECT *parent_object = api_db_get_object(parent_id);

	if (parent_object == NULL)
		return 404;

	if (parent_object->type >= OBJECT_TERMINAL_MIN) {
		return REST_SUCCESS;
	}

	if (!rest_check_access_to_object(user, parent_object))
		return 401;

	unsigned int old_parent_id = object->parent_id;

	int res = api_db_change_object_parent(object, parent_object);

	if (res == DB_OK) {

		on_object_change_parent(object);

		return REST_SUCCESS;
	}

	unsigned char *error = (unsigned char *)api_db_get_error();

	response_fail_with_message(*d, l, error, strlen((char *)error), s->zero_init.keep_alive > 0);

	return 0;
}

int object_moveappend(HTTP_SESSION *s, unsigned int id, unsigned char **d, size_t *l)
{
	DB_OBJECT *object;
	object = api_db_get_object(id);

	if (object == NULL)
		return 404;

	REST_USER *user = rest_find_user(s);
	if (user == NULL)
		return 401;

	if (user->user_id == id)
		return 404;

	if (!rest_check_access_to_object(user, object))
		return 401;

	unsigned char *existing_key_buffer = *d;
	const size_t existing_key_buffer_size = *l / 3;
	unsigned char *incoming_key_buffer = existing_key_buffer + existing_key_buffer_size;
	const size_t incoming_key_buffer_size = *l / 3;
	unsigned char *new_key_buffer = incoming_key_buffer + incoming_key_buffer_size;
	const size_t new_key_buffer_size = *l / 3;

	JKEY incoming_key;

	if (jparse_parse(s->http_body, s->zero_init.body_len, incoming_key_buffer, incoming_key_buffer_size, &incoming_key)) {

		unsigned char *error = jparse_get_error(s->http_body, s->zero_init.body_len);

		response_fail_with_message(*d, l, error, strlen((char *)error), s->zero_init.keep_alive > 0);

		jparse_free_error(error);

		return 0;
	}

	if (incoming_key.value_type != JPARSE_VALUE_TYPE_OBJECT)
		return 500;

	if ((incoming_key.value.object_val.first_key->key_len != 9)&&(memcmp(incoming_key.value.object_val.first_key->key, "parent_id", 9)))
		return 500;

	unsigned int parent_id = incoming_key.value.object_val.first_key->value.int_val;

	DB_OBJECT *parent_object = api_db_get_object(parent_id);

	if (parent_object == NULL)
		return 404;

	if (!rest_check_access_to_object(user, parent_object))
		return 401;

	unsigned int old_parent_id = object->parent_id;
	
	int res;

	if (parent_object->type >= OBJECT_TERMINAL_MIN) {
		return REST_SUCCESS;
	}

	res = api_db_move_object(object, parent_object, NULL, NULL);

	if (res == DB_OK) {

		on_object_change_parent(object);

		return REST_SUCCESS;
	}

	unsigned char *error = (unsigned char *)api_db_get_error();

	response_fail_with_message(*d, l, error, strlen((char *)error), s->zero_init.keep_alive > 0);

	return 0;
}

int object_movebefore(HTTP_SESSION *s, unsigned int id, unsigned char **d, size_t *l)
{
	DB_OBJECT *object;
	object = api_db_get_object(id);

	if (object == NULL)
		return 404;

	REST_USER *user = rest_find_user(s);
	if (user == NULL)
		return 401;

	if (user->user_id == id)
		return 404;

	if (!rest_check_access_to_object(user, object))
		return 401;

	unsigned char *existing_key_buffer = *d;
	const size_t existing_key_buffer_size = *l / 3;
	unsigned char *incoming_key_buffer = existing_key_buffer + existing_key_buffer_size;
	const size_t incoming_key_buffer_size = *l / 3;
	unsigned char *new_key_buffer = incoming_key_buffer + incoming_key_buffer_size;
	const size_t new_key_buffer_size = *l / 3;

	JKEY incoming_key;

	if (jparse_parse(s->http_body, s->zero_init.body_len, incoming_key_buffer, incoming_key_buffer_size, &incoming_key)) {

		unsigned char *error = jparse_get_error(s->http_body, s->zero_init.body_len);

		response_fail_with_message(*d, l, error, strlen((char *)error), s->zero_init.keep_alive > 0);

		jparse_free_error(error);

		return 0;
	}

	if (incoming_key.value_type != JPARSE_VALUE_TYPE_OBJECT)
		return 500;

	if ((incoming_key.value.object_val.first_key->key_len != 9)&&(memcmp(incoming_key.value.object_val.first_key->key, "parent_id", 9)))
		return 500;

	unsigned int parent_id = incoming_key.value.object_val.first_key->value.int_val;

	DB_OBJECT *sibling_object = api_db_get_object(parent_id);

	if (sibling_object == NULL)
		return 404;

	DB_OBJECT *parent_object = api_db_get_object(sibling_object->parent_id);

	if (parent_object == NULL)
		return 404;

	if (!rest_check_access_to_object(user, parent_object))
		return 401;

	unsigned int old_parent_id = object->parent_id;
	
	int res;

	if (parent_object->type >= OBJECT_TERMINAL_MIN)
		return REST_SUCCESS;	

	res = api_db_move_object(object, parent_object, sibling_object, NULL);

	if (res == DB_OK) {

		on_object_change_parent(object);

		return REST_SUCCESS;
	}

	unsigned char *error = (unsigned char *)api_db_get_error();

	response_fail_with_message(*d, l, error, strlen((char *)error), s->zero_init.keep_alive > 0);

	return 0;
}

int object_moveafter(HTTP_SESSION *s, unsigned int id, unsigned char **d, size_t *l)
{
	DB_OBJECT *object;
	object = api_db_get_object(id);

	if (object == NULL)
		return 404;

	REST_USER *user = rest_find_user(s);
	if (user == NULL)
		return 401;

	if (user->user_id == id)
		return 404;

	if (!rest_check_access_to_object(user, object))
		return 401;

	unsigned char *existing_key_buffer = *d;
	const size_t existing_key_buffer_size = *l / 3;
	unsigned char *incoming_key_buffer = existing_key_buffer + existing_key_buffer_size;
	const size_t incoming_key_buffer_size = *l / 3;
	unsigned char *new_key_buffer = incoming_key_buffer + incoming_key_buffer_size;
	const size_t new_key_buffer_size = *l / 3;

	JKEY incoming_key;

	if (jparse_parse(s->http_body, s->zero_init.body_len, incoming_key_buffer, incoming_key_buffer_size, &incoming_key)) {

		unsigned char *error = jparse_get_error(s->http_body, s->zero_init.body_len);

		response_fail_with_message(*d, l, error, strlen((char *)error), s->zero_init.keep_alive > 0);

		jparse_free_error(error);

		return 0;
	}

	if (incoming_key.value_type != JPARSE_VALUE_TYPE_OBJECT)
		return 500;

	if ((incoming_key.value.object_val.first_key->key_len != 9)&&(memcmp(incoming_key.value.object_val.first_key->key, "parent_id", 9)))
		return 500;

	unsigned int parent_id = incoming_key.value.object_val.first_key->value.int_val;

	DB_OBJECT *sibling_object = api_db_get_object(parent_id);

	if (sibling_object == NULL)
		return 404;

	DB_OBJECT *parent_object = api_db_get_object(sibling_object->parent_id);

	if (parent_object == NULL)
		return 404;

	if (!rest_check_access_to_object(user, parent_object))
		return 401;

	unsigned int old_parent_id = object->parent_id;
	
	int res;

	if (parent_object->type >= OBJECT_TERMINAL_MIN)
		return REST_SUCCESS;	

	res = api_db_move_object(object, parent_object, NULL, sibling_object);

	if (res == DB_OK) {

		on_object_change_parent(object);

		return REST_SUCCESS;
	}

	unsigned char *error = (unsigned char *)api_db_get_error();

	response_fail_with_message(*d, l, error, strlen((char *)error), s->zero_init.keep_alive > 0);

	return 0;
}