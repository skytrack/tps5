//******************************************************************************
//
// File Name : module.cpp
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#include <stdlib.h>
#include <string>
#include <string.h>

#include "module.h"
#include "../core/module.h"
#include "../core/likely.h"
#include "api.h"
#include "thread_pool.h"
#include "cpu.h"
#include "http.h"
#include "rest.h"
#include "static.h"
#include "../core/jparse.h"
#include "device.h"
#include "isearch.h"
#include "billing.h"

#ifdef _WIN32
#include <windows.h>
#elif MACOS
#include <sys/param.h>
#include <sys/sysctl.h>
#else
#include <unistd.h>
#endif

void *searcher;

static MODULE_FAMILY family = MODULE_FAMILY_API;

static const char *name = "HTTP module, v1.0";

static std::string host = "*";
static std::string port = "8080";
static std::string search_file = "";
std::string font_file_name = "";
std::string billing = "";

int get_var(int var_type, void **p)
{
	switch (var_type) {

	case MODULE_VAR_FAMILY:
		*p = (void *)&family;
		break;

	case MODULE_VAR_NAME:
		*p = (void *)name;
		break;

	case MODULE_VAR_TAG:
		*p = api_tag;
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
		else
		if (strcmp(op->name, "rootdir") == 0) {
			static_set_rootdir(op->value);
		}
		else
		if (strcmp(op->name, "pdf_font") == 0) {
			font_file_name = op->value;
		}
		else
		if (strcmp(op->name, "search") == 0) {
			search_file = op->value;
		}
		else
		if (strcmp(op->name, "billing") == 0) {
			billing = op->value;
		}
		else {
			api_log_printf("[HTTP] Unknown config option '%s'\r\n", op->name);
		}
		break;

	case MODULE_VAR_TAG:
		api_tag = p;
		break;

	case MODULE_VAR_FUNC_STORAGE_CREATE_STREAM:
		api_storage_create_stream = (STORAGE_CREATE_STREAM)p;
		break;
	case MODULE_VAR_FUNC_STORAGE_DESTROY_STREAM:
		api_storage_destroy_stream = (STORAGE_DESTROY_STREAM)p;
		break;
	case MODULE_VAR_FUNC_STORAGE_GET_STREAM_BY_ID:
		api_storage_get_stream_by_id = (STORAGE_GET_STREAM_BY_ID)p;
		break;
	case MODULE_VAR_FUNC_STORAGE_LOCK_STREAM:
		api_storage_lock_stream = (STORAGE_LOCK_STREAM)p;
		break;
	case MODULE_VAR_FUNC_STORAGE_UNLOCK_STREAM:
		api_storage_unlock_stream = (STORAGE_UNLOCK_STREAM)p;
		break;
	case MODULE_VAR_FUNC_STORAGE_ADD_RECORD_TO_STREAM:
		api_storage_add_record_to_stream = (STORAGE_ADD_RECORD_TO_STREAM)p;
		break;
	case MODULE_VAR_FUNC_STORAGE_UPDATE_RECORD:
		api_storage_update_record = (STORAGE_UPDATE_RECORD)p;
		break;
	case MODULE_VAR_FUNC_STORAGE_GET_STREAM_FIRST_RECORD:
		api_storage_get_stream_first_record = (STORAGE_GET_STREAM_FIRST_RECORD)p;
		break;
	case MODULE_VAR_FUNC_STORAGE_GET_STREAM_RECORDS_COUNT:
		api_storage_get_stream_records_count = (STORAGE_GET_STREAM_RECORDS_COUNT)p;
		break;
	case MODULE_VAR_FUNC_STORAGE_SORT_STREAM:
		api_storage_sort_stream = (STORAGE_SORT_STREAM)p;
		break;
	case MODULE_VAR_FUNC_STORAGE_TRIM_STREAM:
		api_storage_trim_stream = (STORAGE_TRIM_STREAM)p;
		break;
	case MODULE_VAR_FUNC_STORAGE_GET_STREAM_INFO:
		api_storage_get_stream_info = (STORAGE_GET_STREAM_INFO)p;
		break;
	case MODULE_VAR_FUNC_DB_GET_ERROR:
		api_db_get_error = (DB_GET_ERROR)p;
		break;
	case MODULE_VAR_FUNC_DB_GET_OBJECT:
		api_db_get_object = (DB_GET_OBJECT)p;
		break;
	case MODULE_VAR_FUNC_DB_PUT_OBJECT:
		api_db_put_object = (DB_PUT_OBJECT)p;
		break;
	case MODULE_VAR_FUNC_DB_UPDATE_OBJECT:
		api_db_update_object = (DB_UPDATE_OBJECT)p;
		break;
	case MODULE_VAR_FUNC_DB_DELETE_OBJECT:
		api_db_delete_object = (DB_DELETE_OBJECT)p;
		break;
	case MODULE_VAR_FUNC_DB_CHANGE_OBJECT_PARENT:
		api_db_change_object_parent = (DB_CHANGE_OBJECT_PARENT)p;
		break;
	case MODULE_VAR_FUNC_DB_MOVE_OBJECT:
		api_db_move_object = (DB_MOVE_OBJECT)p;
		break;
	case MODULE_VAR_FUNC_DB_ENUM_OBJECTS:
		api_db_enum_objects = (DB_ENUM_OBJECTS)p;
		break;
	case MODULE_VAR_FUNC_ENUM_MODULES:
		api_enum_modules = (ENUM_MODULES)p;
		break;
	case MODULE_VAR_FUNC_GET_DEVICE_MODULE:
		api_get_device_module = (GET_DEVICE_MODULE)p;
		break;
	case MODULE_VAR_FUNC_FUEL_PROCESS:
		api_fuel_process = (FUEL_PROCESS)p;
		break;
	}
	return 0;
}

static const SESSION_HANDLERS handlers = { 
	(SESSION_OPEN)http_session_open, 
	(SESSION_CLOSE)http_session_close, 
	(SESSION_DATA)http_session_data, 
	(SESSION_TIMER)http_session_timer 
};

int start()
{
	jparse_init();
	rest_init();
	http_start();
	device_init();
#ifdef _DEBUG
	billing_init();
#endif
#ifndef _MSC_VER
	const char *initrc = initSearch(search_file.c_str());
	if (initrc && initrc[0]) {
		api_log_printf("%s\r\n", initrc);
		searcher = NULL;
	}
	else {
		searcher = makeSearcher();
	}
#endif

	api_listen_tcp(api_tag, host.c_str(), port.c_str(), &handlers);

	size_t num_cores;

	#ifdef WIN32
		SYSTEM_INFO sysinfo;
		GetSystemInfo(&sysinfo);
		num_cores = sysinfo.dwNumberOfProcessors;
	#elif MACOS
	    int nm[2];
		size_t len = 4;
		uint32_t count;

		nm[0] = CTL_HW; nm[1] = HW_AVAILCPU;
	    sysctl(nm, 2, &count, &len, NULL, 0);

	    if(count < 1) {
		    nm[1] = HW_NCPU;
			sysctl(nm, 2, &count, &len, NULL, 0);
	        if(count < 1) { count = 1; }
		}
	    num_cores = count;
	#else
		num_cores = sysconf(_SC_NPROCESSORS_ONLN);
	#endif

	CPUINFO *cpu_info = cpu_get_info();
	api_log_printf("[HTTP] CPU INFO: Brand name: %s, cores count: %u, Hyper threads: %s\r\n",
		cpu_info->vendor.c_str(), num_cores, (cpu_info->hyper_threads) ? "yes" : "no");

	thread_pool_init(num_cores - 1);

	api_log_printf("[HTTP] Started\r\n");

	return 0;
}

int stop()
{
	http_stop();
	thread_pool_destroy();
	rest_destroy();
	jparse_destroy();

#ifdef _DEBUG
	billing_destroy();
#endif

	api_log_printf("[HTTP] Stopped\r\n");

	return 0;
}

// End
