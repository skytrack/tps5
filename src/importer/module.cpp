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
#include "common.h"
#include "data.h"
#include "events.h"

static unsigned char err_unable_to_allocate_id_buffer[] = "Unable to allocate id buffer";

static MODULE_FAMILY family			= MODULE_FAMILY_API;
static const char *name				= "Importer module, v1.0";

static std::string host				= "*";
static std::string port				= "45590";

unsigned char *get_error(size_t *len)
{
	*len = error_len;
	return error_ptr;
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

	case MODULE_VAR_FUNC_GET_ERROR:
		*p = (void *)get_error;
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
	case MODULE_VAR_OPTION:
		if (strcmp(op->name, "host") == 0) {
			host = op->value;
		}
		else
		if (strcmp(op->name, "port") == 0) {
			port = op->value;
		}
		else {
			api_log_printf("[Importer] Warning: unknown config option '%s'\r\n", op->name);
		}
		break;

	case MODULE_VAR_FUNC_STORAGE_GET_STREAM_BY_ID:
		api_storage_get_stream_by_id = (STORAGE_GET_STREAM_BY_ID)p;
		break;
	case MODULE_VAR_FUNC_STORAGE_ADD_RECORD_TO_STREAM:
		api_storage_add_record_to_stream = (STORAGE_ADD_RECORD_TO_STREAM)p;
		break;
	case MODULE_VAR_FUNC_DB_ENUM_OBJECTS:
		api_db_enum_objects = (DB_ENUM_OBJECTS)p;
		break;
	}
	return 0;
}

static int enum_callback(DB_OBJECT *object, void *c)
{
	if (object->type >= OBJECT_TERMINAL_MIN)
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

	dev_id = (unsigned char *)aligned_malloc(32, SHRT_MAX * 8);

	if (dev_id == NULL) {
		api_log_printf("[Importer] Unable to allocate dev_id buffer\r\n");
		error_ptr = err_unable_to_allocate_id_buffer;
		error_len = sizeof(err_unable_to_allocate_id_buffer) - 1;
		return -1;
	}

	api_db_enum_objects(enum_callback, NULL);

	api_listen_tcp(api_tag, host.c_str(), port.c_str(), &data_handlers);

	api_log_printf("[Importer] Started\r\n");

	return 0;
}

int stop()
{
	if (dev_id != NULL)
		aligned_free(dev_id);

	jparse_destroy();

	api_log_printf("[Importer] Stopped\r\n");

	return 0;
}

// End
