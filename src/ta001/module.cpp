//******************************************************************************
//
// File Name : module.cpp
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <string>

#include "module.h"

#include "../core/almalloc.h"
#include "../core/jparse.h"

#include "api.h"
#include "config.h"
#include "common.h"
#include "data.h"
#include "events.h"

static unsigned char err_unable_to_allocate_id_buffer[] = "Unable to allocate id buffer";

static MODULE_FAMILY family			= MODULE_FAMILY_DEVICE;
static const char *name				= "TA-001 module, v1.0";

static const char *terminal_name	= "TA001";
static const int   terminal_type	= OBJECT_TYPE;

static std::string host				= "*";
static std::string port				= "45595";

unsigned char *get_error(size_t *len)
{
	*len = error_len;
	return error_ptr;
}

int get_info(DB_OBJECT *object, unsigned char *s, size_t len)
{
	return 0;
}

int get_var(int var_type, void **p)
{
	switch (var_type) {

	case MODULE_VAR_FAMILY:
		*p = (void *)family;
		break;

	case MODULE_VAR_NAME:
		*p = (void *)name;
		break;

	case MODULE_VAR_TERMINAL_TYPE:
		*p = (void *)&terminal_type;
		break;

	case MODULE_VAR_TERMINAL_NAME:
		*p = (void *)terminal_name;
		break;

	case MODULE_VAR_FUNC_CONFIG_GET_JSON:
		*p = (void *)config_get_json;
		break;

	case MODULE_VAR_FUNC_CONFIG_PUT_JSON:
		*p = (void *)config_put_json;
		break;

	case MODULE_VAR_FUNC_GET_ERROR:
		*p = (void *)get_error;
		break;
	case MODULE_VAR_FUNC_GET_DEVICE_CAPS:
		*p = (void *)config_get_device_caps;
		break;

	case MODULE_VAR_FUNC_ON_TIMER:
		*p = (void *)on_timer;
		break;

	case MODULE_VAR_FUNC_ON_OBJECT_CREATE:
		*p = (void *)on_object_create;
		break;

	case MODULE_VAR_FUNC_ON_OBJECT_REMOVE:
		*p = (void *)on_object_remove;
		break;

	case MODULE_VAR_FUNC_ON_OBJECT_UPDATE:
		*p = (void *)on_object_update;
		break;
		break;
	case MODULE_VAR_FUNC_GET_INFO:
		*p = (void *)get_info;
		break;
	default:
		*p = NULL;
		return -1;
	}

	return 0;
}

int set_var(int var_type, void *p)
{
	CONFIG_OPTION *op = (CONFIG_OPTION *)p;

	switch(var_type) {

	case MODULE_VAR_FUNC_LOG:
		api_log_printf = (LOG_PRINTF)p;
		break;
	case MODULE_VAR_FUNC_LISTEN_TCP:
		api_listen_tcp = (LISTEN_TCP)p;
		break;
	case MODULE_VAR_FUNC_SEND_TCP:
		api_send_tcp = (SEND_TCP)p;
		break;
	case MODULE_VAR_FUNC_CLOSE_TCP:
		api_close_tcp = (CLOSE_TCP)p;
		break;
	case MODULE_VAR_OPTION:
		if (strcmp(op->name, "host") == 0) {
			host = op->value;
		}
		else
		if (strcmp(op->name, "port") == 0) {
			port = op->value;
		}
		else {
			api_log_printf("[TA001] Warning: unknown config option '%s'\r\n", op->name);
		}
		break;

	case MODULE_VAR_FUNC_STORAGE_GET_STREAM_BY_ID:
		api_storage_get_stream_by_id = (STORAGE_GET_STREAM_BY_ID)p;
		break;
	case MODULE_VAR_FUNC_STORAGE_ADD_RECORD_TO_STREAM:
		api_storage_add_record_to_stream = (STORAGE_ADD_RECORD_TO_STREAM)p;
		break;
	case MODULE_VAR_FUNC_STORAGE_GET_STREAM_FIRST_RECORD:
		api_storage_get_stream_first_record = (STORAGE_GET_STREAM_FIRST_RECORD)p;
		break;
	case MODULE_VAR_FUNC_STORAGE_GET_STREAM_RECORDS_COUNT:
		api_storage_get_stream_records_count = (STORAGE_GET_STREAM_RECORDS_COUNT)p;
		break;
	case MODULE_VAR_FUNC_STORAGE_GET_STREAM_INFO:
		api_storage_get_stream_info = (STORAGE_GET_STREAM_INFO)p;
		break;
	case MODULE_VAR_FUNC_DB_UPDATE_OBJECT_BLOB:
		api_db_update_object_blob = (DB_UPDATE_OBJECT_BLOB)p;
		break;
	case MODULE_VAR_FUNC_DB_ENUM_OBJECTS:
		api_db_enum_objects = (DB_ENUM_OBJECTS)p;
		break;
	}
	return 0;
}

static int enum_callback(DB_OBJECT *object, void *c)
{
	if (object->type == OBJECT_TYPE)
		on_object_create(object);

	return 0;
}

static const SESSION_HANDLERS data_handlers = { 
	(SESSION_OPEN)data_session_open, 
	(SESSION_CLOSE)data_session_close, 
	(SESSION_DATA)data_session_data, 
	(SESSION_TIMER)data_session_timer 
};

int start()
{
	error_ptr = no_error;
	error_len = sizeof(no_error) - 1;

	jparse_init();

	api_db_enum_objects(enum_callback, NULL);

	api_listen_tcp(api_tag, host.c_str(), port.c_str(), &data_handlers);

	now = time(NULL);

	api_log_printf("[TA001] Started\r\n");

	return 0;
}

int stop()
{
	for (std::map<uint64_t, TERMINAL>::iterator it = terminals.begin(); it != terminals.end(); ++it) {
		if (it->second.session != NULL) {
			data_session_close(it->second.session);
		}
	}

	jparse_destroy();

	api_log_printf("[TA001] Stopped\r\n");

	return 0;
}

// End
