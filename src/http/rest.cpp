//******************************************************************************
//
// File Name : rest.cpp
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <map>
#include <algorithm>
#include "api.h"
#include "json.h"
#include "../core/jparse.h"
#include "../core/almalloc.h"
#include "../core/fuel.h"
#include "rest.h"
#include "user.h"
#include "device.h"
#include "object.h"
#include "response.h"
#include "group.h"
#include "thread_pool.h"
#include "online.h"
#include "retranslator.h"

#define REST_ITEMS_COUNT 20

static const REST_URI rest_table[REST_ITEMS_COUNT] = 
{
	{ "/online",					rest_get_self_id,		online_get,				NULL,					NULL,						NULL},
	{ "/tree",						rest_get_self_id,		object_get_tree,		NULL,					NULL,						NULL},
	{ "/objects",					rest_get_self_id,		object_get,				NULL,					NULL,						NULL},
	{ "/objects/*",					rest_extract_id,		object_get,				NULL,					object_post,				object_delete},
	{ "/objects/*/users",			rest_extract_id,		NULL,					user_put,				NULL,						NULL},
	{ "/objects/*/devices",			rest_extract_id,		NULL,					device_put,				NULL,						NULL},
	{ "/objects/*/groups",			rest_extract_id,		NULL,					group_put,				NULL,						NULL},
	{ "/objects/*/settings",		rest_extract_id,		device_get_settings,	NULL,					device_post_settings,		NULL},
	{ "/objects/*/sensors",			rest_extract_id,		NULL,					NULL,					device_post_sensors,		NULL},
	{ "/objects/*/llsleft",			rest_extract_id,		NULL,					NULL,					device_post_llsleft,		NULL},
	{ "/objects/*/llsright",		rest_extract_id,		NULL,					NULL,					device_post_llsright,		NULL},
	{ "/objects/*/retranslator",	rest_extract_id,		NULL,					NULL,					device_post_retranslator,	NULL},
	{ "/objects/*/fuel",			rest_extract_id,		NULL,					NULL,					device_post_fuel,			NULL},
	{ "/objects/*/modify",			rest_extract_id,		NULL,					NULL,					device_post_modify,			NULL},
	{ "/objects/*/parent",			rest_extract_id,		NULL,					NULL,					object_parent,				NULL},
	{ "/objects/*/moveappend",		rest_extract_id,		NULL,					NULL,					object_moveappend,			NULL},
	{ "/objects/*/movebefore",		rest_extract_id,		NULL,					NULL,					object_movebefore,			NULL},
	{ "/objects/*/moveafter",		rest_extract_id,		NULL,					NULL,					object_moveafter,			NULL},
	{ "/devices",					rest_get_self_id,		devices,				NULL,					NULL,						NULL},
	{ "/retranslators",				rest_get_self_id,		retranslators,			NULL,					NULL,						NULL},
};

static JSON_INTERESTED_KEY keys[1] = {

	{ "hash",					4,	VALUE_TYPE_STRING,	1,	64,	64}
};
static std::string pattern;

_ALIGNED(unsigned char *) hash;

std::vector<REST_USER> users;

static void construct_object_access_tree(DB_OBJECT *object, unsigned short id)
{
	if (object == NULL)
		return;

	if (object->type == OBJECT_USER) {

		size_t size = users.size();

		if (size > 0) {

			REST_USER *user = &users[0];

			for (size_t i = 0; i < size; i++, user++) {

				if (user->user_id == object->id) {
					user->objects.push_back(id);
					break;
				}
			}
		}
	}

	if (object->id != 0)
		construct_object_access_tree(api_db_get_object(object->parent_id), id);
}

int on_object_create(DB_OBJECT *object)
{
	if (object->type == OBJECT_USER) {

		JKEY hash_key;

		jparse_extract_key((unsigned char *)"hash", 4, object->core_data, object->core_data_size, &hash_key);

		if ((hash_key.value_type == JPARSE_VALUE_TYPE_STRING)&&(hash_key.str_len == 64)) {

			REST_USER user;
			user.user_id = object->id;
			user.objects.push_back(object->id);

			users.push_back(user);

			unsigned char *ptr = hash_key.value.str_val;
			unsigned char *dst = hash + (users.size() - 1) * 32;

			for (unsigned char i = 0; i < 32; i++) {
				*dst = ((*ptr <= '9') ? (*ptr - '0') : (*ptr - 'a' + 10)) << 4;
				ptr++;
				*dst++ |= (*ptr <= '9') ? (*ptr - '0') : (*ptr - 'a' + 10);
				ptr++;
			}
		}
	}

 	if (object->id != 0)
		construct_object_access_tree(api_db_get_object(object->parent_id), object->id);

	return 0;
}

int on_object_update(DB_OBJECT *object)
{
	if (object->type == OBJECT_USER) {

		JKEY hash_key;

		jparse_extract_key((unsigned char *)"hash", 4, object->core_data, object->core_data_size, &hash_key);

		if ((hash_key.value_type == JPARSE_VALUE_TYPE_STRING)&&(hash_key.str_len == 64)) {

			size_t size = users.size();

			if (size > 0) {

				REST_USER *user = &users[0];

				for (size_t i = 0; i < size; i++, user++) {

					if (user->user_id == object->id) {

						unsigned char *ptr = hash_key.value.str_val;
						unsigned char *dst = hash + i * 32;

						for (unsigned char j = 0; j < 32; j++) {
							*dst = ((*ptr <= '9') ? (*ptr - '0') : (*ptr - 'a' + 10)) << 4;
							ptr++;
							*dst++ |= (*ptr <= '9') ? (*ptr - '0') : (*ptr - 'a' + 10);
							ptr++;
						}

						break;
					}
				}
			}
		}
	}

	return 0;
}

static void remove_object_from_access_tree(DB_OBJECT *object, unsigned short id)
{
	if (object->type == OBJECT_USER) {

		size_t size = users.size();

		if (size > 0) {

			REST_USER *user = &users[0];

			for (size_t i = 0; i < size; i++, user++) {

				if (user->user_id == object->id) {
					
					for (size_t j = 0; j < user->objects.size(); j++) {

						if (user->objects[j] == id) {
							user->objects.erase(user->objects.begin() + j);
							break;
						}
					}

					break;
				}
			}
		}
	}

	if (object->id != 0)
		remove_object_from_access_tree(api_db_get_object(object->parent_id), id);
}

int on_object_remove(DB_OBJECT *object)
{
	if (object->type == OBJECT_USER) {

		size_t size = users.size();

		if (size > 0) {

			REST_USER *user = &users[0];
				
			for (size_t i = 0; i < size; i++, user++) {

				if (user->user_id == object->id) {

					unsigned char *ptr = hash + i * 32;
					memcpy(ptr, ptr + 32, (users.size() * 32) - (ptr - hash) - 32);

					users.erase(users.begin() + i);

					break;
				}
			}
		}
	}
	else
	if (object->type >= OBJECT_TERMINAL_MIN) {

		MODULE *module = api_get_device_module(object->type);
		if (module != NULL)
			module->on_object_remove(object);
	}

	if (object->id != 0)
		remove_object_from_access_tree(api_db_get_object(object->parent_id), object->id);

	return 0;
}

int on_object_change_parent(DB_OBJECT *object)
{
	size_t size = users.size();

	if (size > 0) {

		REST_USER *user = &users[0];

		for (size_t i = 0; i < size; i++, user++) {

			for (size_t j = 0; j < user->objects.size(); j++) {

				if (user->objects[j] == object->id) {
					user->objects.erase(user->objects.begin() + j);
					break;
				}
			}
		}
	}

	construct_object_access_tree(api_db_get_object(object->parent_id), object->id);

	return 0;
}

unsigned int rest_extract_id(HTTP_SESSION *s, const char *uri)
{
	while ((*uri != '\0')&&((*uri < '0')||(*uri > '9')))
		uri++;

	unsigned long id = strtoul(uri, NULL, 10);

	if (id > UINT_MAX)
		return UINT_MAX;

	return (unsigned int)id;
}

unsigned int rest_get_self_id(HTTP_SESSION *s, const char *uri)
{
	size_t size = users.size();

	if (size > 0) {

		unsigned char *ptr = hash;

		for (size_t i = 0; i < size; i++, ptr += 32) {

			if (memcmp(ptr, s->zero_init.hash, 32) == 0)
				return users[i].user_id;
		}
	}

	return -1;
}

REST_USER *rest_find_user(HTTP_SESSION *s)
{
	size_t size = users.size();

	if (size > 0) {

		unsigned char *ptr = hash;

		for (size_t i = 0; i < size; i++, ptr += 32) {

			if (memcmp(ptr, s->zero_init.hash, 32) == 0)
				return &users[i];
		}
	}

	return NULL;
}

bool rest_check_access_to_object(REST_USER *user, DB_OBJECT *object)
{
	size_t size = user->objects.size();

	for (size_t i = 0; i < size; i++)
		if (user->objects[i] == object->id)
			return true;

	return false;
}

static int users_callback(DB_OBJECT *object, void *c)
{
	if (object->type == OBJECT_USER)
		on_object_create(object);

	return 0;
}

static int objects_callback(DB_OBJECT *object, void *c)
{
	if (object->type != OBJECT_USER) {
		on_object_create(object);
	}

	return 0;
}

int rest_init()
{
	unsigned short users_count = 0;

	for (size_t i = 0; i < REST_ITEMS_COUNT; i++) {
		if (i > 0)
			pattern += "\t";
		pattern += rest_table[i].uri;
	}

	hash = (unsigned char *)aligned_malloc(32, SHRT_MAX * 32);

	if (hash == NULL) {
		api_log_printf("[HTTP] Unable to allocate hash buffer\r\n");
		return -1;
	}

	api_db_enum_objects(users_callback, NULL);
	api_db_enum_objects(objects_callback, NULL);

	return 0;
}

int rest_destroy()
{
	pattern.clear();

	if (hash != NULL)
		aligned_free(hash);

	return 0;
}

inline int rest_find_uri(const char *_uri)
{
	int i = 0;

	const char *uri = _uri;
	const char *ptr = pattern.c_str();

	while (*ptr != '\0') {

		if ((*ptr == '\t')&&(*uri == '\0'))
			break;

		if (*ptr == *uri) {
			ptr++;
			uri++;
			continue;
		}

		if (*uri == '?')
			return i;

		if (*ptr != '*') {
			while (*ptr != '\t') {
				if (*ptr == '\0')
					return -1;
				ptr++;
			}

			uri = _uri;
			ptr++;
			i++;
			continue;
		}

		ptr++;
		while ((*uri >= '0')&&(*uri <= '9'))
			uri++;
	}

	if ((*uri != '\0')&&(*uri != '?'))
		return -1;

	return i;
}

int rest_handle_request(HTTP_SESSION *s, unsigned char **d, size_t *l)
{
	REST_URI_HANDLER handler = NULL;

	if (memcmp(s->http_url, "/history/", 9) == 0) {
		char *ptr;
		unsigned int object_id = strtoul(&s->http_url[9], &ptr, 10);
		if (*ptr == '/') {
			unsigned int t_from = strtoul(ptr + 1, &ptr, 10);
			if (*ptr == '/') {
				unsigned int t_to = strtoul(ptr + 1, &ptr, 10);
				if ((memcmp(ptr, ".png", 4) == 0)||(memcmp(ptr, ".bin", 4) == 0)) {
					
					DB_OBJECT *object;
					object = api_db_get_object(object_id);

					if (object == NULL) {
					
						if (s->zero_init.keep_alive) {
							*d = response_fail_404_ka;
							*l = response_fail_404_ka_length;
						}
						else {
							*d = response_fail_404_close;
							*l = response_fail_404_close_length;
						}

						return 0;
					}

					REST_USER *user = rest_find_user(s);
					if ((user == NULL)||(!rest_check_access_to_object(user, object))) {
						if (s->zero_init.keep_alive) {
							*d = response_fail_401_ka;
							*l = response_fail_401_ka_length;
						}
						else {
							*d = response_fail_401_close;
							*l = response_fail_401_close_length;
						}
					}

					THREAD_POOL_JOB job;

					job.terminal_id = object_id;
					job.tFrom = t_from;
					job.tTo = t_to;
					job.session = s;
					job.code = (memcmp(ptr, ".bin", 4)) ? JOB_CODE_HISTORY_PNG : JOB_CODE_HISTORY_BIN;

					thread_pool_add_job(&job);

					return 1;
				}
			}
		}
	}

	if (memcmp(s->http_url, "/report/", 8) == 0) {
		char *ptr;
		unsigned int report_id = strtoul(&s->http_url[8], &ptr, 10);
		if (*ptr == '/') {
			unsigned int object_id = strtoul(ptr + 1, &ptr, 10);
			if (*ptr == '/') {
				unsigned int t_from = strtoul(ptr + 1, &ptr, 10);
				if (*ptr == '/') {
					unsigned int t_to = strtoul(ptr + 1, &ptr, 10);
					if (memcmp(ptr, ".json", 5) == 0) {
					
						DB_OBJECT *object;
						object = api_db_get_object(object_id);

						if ((object == NULL)||(report_id < 1)||(report_id > 16)) {
					
							if (s->zero_init.keep_alive) {
								*d = response_fail_404_ka;
								*l = response_fail_404_ka_length;
							}
							else {
								*d = response_fail_404_close;
								*l = response_fail_404_close_length;
							}

							return 0;
						}

						REST_USER *user = rest_find_user(s);
						if ((user == NULL)||(!rest_check_access_to_object(user, object))) {
							if (s->zero_init.keep_alive) {
								*d = response_fail_401_ka;
								*l = response_fail_401_ka_length;
							}
							else {
								*d = response_fail_401_close;
								*l = response_fail_401_close_length;
							}
						}
						
						if (t_from > t_to) {
							response_fail_with_message(*d, l, (unsigned char *)"Invalid argument", 16, s->zero_init.keep_alive > 0);
							return 0;
						}

						THREAD_POOL_JOB job;

						JKEY key;
						if ((jparse_parse(object->core_data, object->core_data_size, s->session_data, s->session_data_len, &key) == 0)&&(key.value_type == JPARSE_VALUE_TYPE_OBJECT)) {
						
							job.ignition = false;
							job.engine = false;
							job.move = false;

							job.ignition_source = 0;
							job.engine_source = 0;
							job.move_source = 0;

							job.lls_table_left.reserve(100);
							job.lls_table_right.reserve(100);

							job.lls_left_source = 0;
							job.lls_right_source = 0;

							bool A[8] = {false, false, false, false, false, false, false, false};
							bool F[8] = {false, false, false, false, false, false, false, false};
							bool D[8] = {false, false, false, false, false, false, false, false};
							bool RS485 = false;
							bool RS232[2] = {false, false};
							bool injector = false;

							JKEY *k = key.value.object_val.first_key;

							while (k) {
								if ((k->key_len == 4)&&(memcmp(k->key, "caps", 4) == 0)) {
									if (k->value_type == JPARSE_VALUE_TYPE_OBJECT) {
										JKEY *l = k->value.object_val.first_key;
										while (l) {
											if ((l->key_len == 8)&&(memcmp(l->key, "ignition", 8) == 0)&&(l->value_type == JPARSE_VALUE_TYPE_TRUE)) {
												job.ignition = true;
											}
											else
											if ((l->key_len == 8)&&(memcmp(l->key, "engine", 6) == 0)&&(l->value_type == JPARSE_VALUE_TYPE_TRUE)) {
												job.engine = true;
											}
											else
											if ((l->key_len == 4)&&(memcmp(l->key, "move", 4) == 0)&&(l->value_type == JPARSE_VALUE_TYPE_TRUE)) {
												job.move = true;
											}
											else
											if ((l->key_len == 5)&&(memcmp(l->key, "rs485", 5) == 0)&&(l->value_type == JPARSE_VALUE_TYPE_TRUE)) {
												RS485 = true;
											}
											else
											if ((l->key_len == 7)&&(memcmp(l->key, "rs232_1", 7) == 0)&&(l->value_type == JPARSE_VALUE_TYPE_TRUE)) {
												RS232[0] = true;
											}
											else
											if ((l->key_len == 7)&&(memcmp(l->key, "rs232_2", 7) == 0)&&(l->value_type == JPARSE_VALUE_TYPE_TRUE)) {
												RS232[1] = true;
											}
											else
											if ((l->key_len == 8)&&(memcmp(l->key, "injector", 8) == 0)&&(l->value_type == JPARSE_VALUE_TYPE_TRUE)) {
												injector = true;
											}
											else
											if (l->key_len == 7) {
												if ((memcmp(l->key, "analog1", 7) == 0)&&(l->value_type == JPARSE_VALUE_TYPE_TRUE))
													A[0] = true;
												else
												if ((memcmp(l->key, "analog2", 7) == 0)&&(l->value_type == JPARSE_VALUE_TYPE_TRUE))
													A[1] = true;
												else
												if ((memcmp(l->key, "analog3", 7) == 0)&&(l->value_type == JPARSE_VALUE_TYPE_TRUE))
													A[2] = true;
												else
												if ((memcmp(l->key, "analog4", 7) == 0)&&(l->value_type == JPARSE_VALUE_TYPE_TRUE))
													A[3] = true;
												else
												if ((memcmp(l->key, "analog5", 7) == 0)&&(l->value_type == JPARSE_VALUE_TYPE_TRUE))
													A[4] = true;
												else
												if ((memcmp(l->key, "analog6", 7) == 0)&&(l->value_type == JPARSE_VALUE_TYPE_TRUE))
													A[5] = true;
												else
												if ((memcmp(l->key, "analog7", 7) == 0)&&(l->value_type == JPARSE_VALUE_TYPE_TRUE))
													A[6] = true;
												else
												if ((memcmp(l->key, "analog8", 7) == 0)&&(l->value_type == JPARSE_VALUE_TYPE_TRUE))
													A[7] = true;
											}
											else
											if (l->key_len == 10) {
												if ((memcmp(l->key, "frequency1", 10) == 0)&&(l->value_type == JPARSE_VALUE_TYPE_TRUE))
													F[0] = true;
												else
												if ((memcmp(l->key, "frequency2", 10) == 0)&&(l->value_type == JPARSE_VALUE_TYPE_TRUE))
													F[1] = true;
												else
												if ((memcmp(l->key, "frequency3", 10) == 0)&&(l->value_type == JPARSE_VALUE_TYPE_TRUE))
													F[2] = true;
												else
												if ((memcmp(l->key, "frequency4", 10) == 0)&&(l->value_type == JPARSE_VALUE_TYPE_TRUE))
													F[3] = true;
												else
												if ((memcmp(l->key, "frequency5", 10) == 0)&&(l->value_type == JPARSE_VALUE_TYPE_TRUE))
													F[4] = true;
												else
												if ((memcmp(l->key, "frequency6", 10) == 0)&&(l->value_type == JPARSE_VALUE_TYPE_TRUE))
													F[5] = true;
												else
												if ((memcmp(l->key, "frequency7", 10) == 0)&&(l->value_type == JPARSE_VALUE_TYPE_TRUE))
													F[6] = true;
												else
												if ((memcmp(l->key, "frequency8", 10) == 0)&&(l->value_type == JPARSE_VALUE_TYPE_TRUE))
													F[7] = true;
											}
											else
											if (l->key_len == 9) {
												if ((memcmp(l->key, "discrete1", 9) == 0)&&(l->value_type == JPARSE_VALUE_TYPE_TRUE))
													D[0] = true;
												else
												if ((memcmp(l->key, "discrete2", 9) == 0)&&(l->value_type == JPARSE_VALUE_TYPE_TRUE))
													D[1] = true;
												else
												if ((memcmp(l->key, "discrete3", 9) == 0)&&(l->value_type == JPARSE_VALUE_TYPE_TRUE))
													D[2] = true;
												else
												if ((memcmp(l->key, "discrete4", 9) == 0)&&(l->value_type == JPARSE_VALUE_TYPE_TRUE))
													D[3] = true;
												else
												if ((memcmp(l->key, "discrete5", 9) == 0)&&(l->value_type == JPARSE_VALUE_TYPE_TRUE))
													D[4] = true;
												else
												if ((memcmp(l->key, "discrete6", 9) == 0)&&(l->value_type == JPARSE_VALUE_TYPE_TRUE))
													D[5] = true;
												else
												if ((memcmp(l->key, "discrete7", 9) == 0)&&(l->value_type == JPARSE_VALUE_TYPE_TRUE))
													D[6] = true;
												else
												if ((memcmp(l->key, "discrete8", 9) == 0)&&(l->value_type == JPARSE_VALUE_TYPE_TRUE))
													D[7] = true;
											}
											l = l->next_key;
										}
									}
								}
								k = k->next_key;
							}

							k = key.value.object_val.first_key;

							while (k) {
								if ((k->key_len == 7)&&(memcmp(k->key, "sensors", 7) == 0)) {
									if (k->value_type == JPARSE_VALUE_TYPE_OBJECT) {
										JKEY *l = k->value.object_val.first_key;
										while (l) {

											char discrete[]		= "discrete1_type";

											for (int i = 1; i <= 8; i++) {
												
												discrete[8]		= i + '0';

												if ((l->key_len == 14)&&(memcmp(l->key, discrete, 14) == 0)&&(l->value_type == JPARSE_VALUE_TYPE_NUMBER)) {
													if ((l->value.int_val == 1)&&(D[i - 1]))
														job.ignition_source = i;
													else
													if ((l->value.int_val == 2)&&(D[i - 1]))
														job.engine_source = i;
													else
													if ((l->value.int_val == 3)&&(D[i - 1]))
														job.move_source = i;
												}
											}
											if ((l->key_len == 15)&&(memcmp(l->key, "injector_factor", 15) == 0)&&(l->value_type == JPARSE_VALUE_TYPE_NUMBER)) {
												job.injector_factor = l->value.int_val;
											}
											l = l->next_key;
										}
									}
									break;
								}
								k = k->next_key;
							}

							if (report_id == JOB_CODE_REPORT_FUEL) {

								job.filter_level = 2;
								job.fill_threshold = 25;
								job.drain_threshold = 10000;
								job.max_consumption = 10000;
								job.injector = injector;

								while (k) {
									if ((k->key_len == 7)&&(memcmp(k->key, "sensors", 7) == 0)) {
										if (k->value_type == JPARSE_VALUE_TYPE_OBJECT) {
											JKEY *l = k->value.object_val.first_key;
											while (l) {
												char analog[]		= "analog1_type";
												char frequency[]	= "frequency1_type";

												for (int i = 1; i <= 8; i++) {
												
													analog[6]		= i + '0';
													frequency[9]	= i + '0';

													if ((l->key_len == 12)&&(memcmp(l->key, analog, 12) == 0)&&(l->value_type == JPARSE_VALUE_TYPE_NUMBER)) {
														if ((l->value.int_val == 2)&&(A[i - 1]))
															job.lls_left_source = 0x10 | i;
														else
														if ((l->value.int_val == 3)&&(A[i - 1]))
															job.lls_right_source = 0x10 | i;
													}
													else
													if ((l->key_len == 15)&&(memcmp(l->key, frequency, 15) == 0)&&(l->value_type == JPARSE_VALUE_TYPE_NUMBER)) {
														if ((l->value.int_val == 2)&&(F[i - 1]))
															job.lls_left_source = i;
														else
														if ((l->value.int_val == 3)&&(F[i - 1]))
															job.lls_right_source = i;
													}
												}

												if ((RS485 == true)&&(l->key_len == 14)&&(memcmp(l->key, "rs485_lls_left", 14) == 0)&&(l->value_type == JPARSE_VALUE_TYPE_TRUE)) {
													job.lls_left_source = FUEL_SOURCE_RS485_1;
												}

												if ((RS485 == true)&&(l->key_len == 15)&&(memcmp(l->key, "rs485_lls_right", 15) == 0)&&(l->value_type == JPARSE_VALUE_TYPE_TRUE)) {
													job.lls_right_source = FUEL_SOURCE_RS485_2;
												}

												l = l->next_key;
											}
										}
									}
									else
									if ((k->key_len == 4)&&(memcmp(k->key, "fuel", 4) == 0)) {
										if (k->value_type == JPARSE_VALUE_TYPE_OBJECT) {
											JKEY *l = k->value.object_val.first_key;
											while (l) {
												if ((l->key_len == 12)&&(memcmp(l->key, "filter_level", 12) == 0)&&(l->value_type == JPARSE_VALUE_TYPE_NUMBER)) {
													job.filter_level = l->value.int_val;
												}
												else
												if ((l->key_len == 14)&&(memcmp(l->key, "fill_threshold", 14) == 0)&&(l->value_type == JPARSE_VALUE_TYPE_NUMBER)) {
													job.fill_threshold = l->value.int_val;
												}
												else
												if ((l->key_len == 15)&&(memcmp(l->key, "drain_threshold", 15) == 0)&&(l->value_type == JPARSE_VALUE_TYPE_NUMBER)) {
													job.drain_threshold = l->value.int_val;
												}
												else
												if ((l->key_len == 15)&&(memcmp(l->key, "max_consumption", 15) == 0)&&(l->value_type == JPARSE_VALUE_TYPE_NUMBER)) {
													job.max_consumption = l->value.int_val;
												}
												l = l->next_key;
											}
										}
									}
									else
									if ((k->key_len == 7)&&(memcmp(k->key, "llsleft", 7) == 0)) {
										if (k->value_type == JPARSE_VALUE_TYPE_OBJECT) {
											JKEY *l = k->value.object_val.first_key;
											while (l) {
												if ((l->key_len == 5)&&(memcmp(l->key, "table", 5) == 0)) {
													if (l->value_type == JPARSE_VALUE_TYPE_ARRAY) {
														JOBJECT *o = l->value.array_val.first_object;
														while (o) {

															FUEL_TABLE_RECORD ftr;
															ftr.fuel_value = -1;
															ftr.sensor_value = -1;

															JKEY *tk = o->first_key;
															while (tk) {
																if ((tk->key_len == 12)&&(memcmp(tk->key, "sensor_value", 12) == 0)) {
																	if (tk->value_type == JPARSE_VALUE_TYPE_FLOAT)
																		ftr.sensor_value = tk->value.float_val;
																	else
																	if (tk->value_type == JPARSE_VALUE_TYPE_NUMBER)
																		ftr.sensor_value = (float)tk->value.int_val;
																}
																else
																if ((tk->key_len == 10)&&(memcmp(tk->key, "fuel_value", 10) == 0)) {
																	if (tk->value_type == JPARSE_VALUE_TYPE_FLOAT)
																		ftr.fuel_value = tk->value.float_val;
																	else
																	if (tk->value_type == JPARSE_VALUE_TYPE_NUMBER)
																		ftr.fuel_value = (float)tk->value.int_val;
																}
																tk = tk->next_key;
															}

															if ((ftr.fuel_value != -1)&&(ftr.sensor_value != -1))
																job.lls_table_left.push_back(ftr);
														
															o = o->next_object;
														}

														std::sort(job.lls_table_left.begin(), job.lls_table_left.end());
													}
													break;
												}
												l = l->next_key;
											}
										}
									}
									else
									if ((k->key_len == 8)&&(memcmp(k->key, "llsright", 8) == 0)) {
										if (k->value_type == JPARSE_VALUE_TYPE_OBJECT) {
											JKEY *l = k->value.object_val.first_key;
											while (l) {
												if ((l->key_len == 5)&&(memcmp(l->key, "table", 5) == 0)) {
													if (l->value_type == JPARSE_VALUE_TYPE_ARRAY) {
														JOBJECT *o = l->value.array_val.first_object;
														while (o) {

															FUEL_TABLE_RECORD ftr;
															ftr.fuel_value = -1;
															ftr.sensor_value = -1;

															JKEY *tk = o->first_key;
															while (tk) {
																if ((tk->key_len == 12)&&(memcmp(tk->key, "sensor_value", 12) == 0)) {
																	if (tk->value_type == JPARSE_VALUE_TYPE_FLOAT)
																		ftr.sensor_value = tk->value.float_val;
																	else
																	if (tk->value_type == JPARSE_VALUE_TYPE_NUMBER)
																		ftr.sensor_value = (float)tk->value.int_val;
																}
																else
																if ((tk->key_len == 10)&&(memcmp(tk->key, "fuel_value", 10) == 0)) {
																	if (tk->value_type == JPARSE_VALUE_TYPE_FLOAT)
																		ftr.fuel_value = tk->value.float_val;
																	else
																	if (tk->value_type == JPARSE_VALUE_TYPE_NUMBER)
																		ftr.fuel_value = (float)tk->value.int_val;
																}
																tk = tk->next_key;
															}

															if ((ftr.fuel_value != -1)&&(ftr.sensor_value != -1))
																job.lls_table_right.push_back(ftr);
														
															o = o->next_object;
														}

														std::sort(job.lls_table_right.begin(), job.lls_table_right.end());
													}
													break;
												}
												l = l->next_key;
											}
										}
									}

									k = k->next_key;
								}

								job.terminal_id = object_id;
								job.tFrom = t_from;
								job.tTo = t_to;
								job.session = s;
								job.code = report_id;

								thread_pool_add_job(&job);

								return 1;
							}
							else {
								job.terminal_id = object_id;
								job.tFrom = t_from;
								job.tTo = t_to;
								job.session = s;
								job.code = report_id;

								thread_pool_add_job(&job);

								return 1;
							}
						}
					}
				}
			}
		}
	}

	if (memcmp(s->http_url, "/report.pdf", 11) == 0) {

		THREAD_POOL_JOB job;

		job.session = s;
		job.code = JOB_CODE_PDF;
		job.buffer = (unsigned char *)malloc(s->zero_init.body_len);
		
		if (job.buffer == NULL) {
			*d = response_fail_500_ka;
			*l = response_fail_500_ka_length;
			return 0;
		}
		job.buffer_size = s->zero_init.body_len;
		memcpy(job.buffer, s->http_body, s->zero_init.body_len);

		thread_pool_add_job(&job);

		return 1;
	}

	int i = rest_find_uri(s->http_url);

	if (s->zero_init.keep_alive) {

		if (i == -1) {
			*d = response_fail_404_ka;
			*l = response_fail_404_ka_length;
			return 0;
		}

		switch (s->parser.method) {

		case HTTP_GET:
			handler = rest_table[i].get;
			break;

		case HTTP_PUT:
			handler = rest_table[i].put;
			break;

		case HTTP_POST:
			handler = rest_table[i].post;
			break;

		case HTTP_DELETE:
			handler = rest_table[i].del;
			break;
		}

		if (handler == NULL) {
			*d = response_fail_405_ka;
			*l = response_fail_405_ka_length;
			return 0;
		}

		unsigned int id = rest_table[i].extract_id(s, s->http_url);

		if (id == UINT_MAX) {
			if (rest_table[i].extract_id == rest_get_self_id) {
				*d = response_fail_401_ka;
				*l = response_fail_401_ka_length;
			}
			else {
				*d = response_fail_404_ka;
				*l = response_fail_404_ka_length;
			}
			return 0;
		}

		int res = handler(s, id, d, l);

		switch (res) {
		case 0:
			break;
		default:
		case 500:
			*d = response_fail_500_ka;
			*l = response_fail_500_ka_length;
			break;
		case 401:
			*d = response_fail_401_ka;
			*l = response_fail_401_ka_length;
			break;
		case 404:
			*d = response_fail_404_ka;
			*l = response_fail_404_ka_length;
			break;
		case REST_SUCCESS:
			*d = response_success_ka;
			*l = response_success_ka_length;
			break;
		case REST_FAIL:
			*d = response_fail_ka;
			*l = response_fail_ka_length;
			break;
		}
	}

	return 0;
}

// End
