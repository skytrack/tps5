//******************************************************************************
//
// File Name : events.h
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#include <string.h>
#include <limits.h>
#include "api.h"
#include "config.h"
#include "common.h"
#include "data.h"
#include "../core/jparse.h"
#include "../core/record.h"

static unsigned char err_invalid_dev_id[] = "Invalid device id";
static unsigned char err_object_not_found[] = "Object not found";
static unsigned char err_unable_to_allocate_config[] = "Unable to allocate config buffer";
static unsigned char err_unable_to_locate_data_stream[] = "Unable to locate data stream";

int on_object_update(DB_OBJECT *object)
{
	JKEY key;

	jparse_extract_key((unsigned char *)"dev_id", 6, (unsigned char *)object->core_data, object->core_data_size, &key);

	if (key.value_type != JPARSE_VALUE_TYPE_STRING) {
		error_ptr = err_invalid_dev_id;
		error_len = sizeof(err_invalid_dev_id) - 1;
		return -1;
	}

	unsigned char dev_id[8];

	dev_id[0] = ((key.value.str_val[0]  - '0') << 4) | (key.value.str_val[1]  - '0');
	dev_id[1] = ((key.value.str_val[2]  - '0') << 4) | (key.value.str_val[3]  - '0');
	dev_id[2] = ((key.value.str_val[4]  - '0') << 4) | (key.value.str_val[5]  - '0');
	dev_id[3] = ((key.value.str_val[6]  - '0') << 4) | (key.value.str_val[7]  - '0');
	dev_id[4] = ((key.value.str_val[8]  - '0') << 4) | (key.value.str_val[9]  - '0');
	dev_id[5] = ((key.value.str_val[10] - '0') << 4) | (key.value.str_val[11] - '0');
	dev_id[6] = ((key.value.str_val[12] - '0') << 4) | (key.value.str_val[13] - '0');
	dev_id[7] = ((key.value.str_val[14] - '0') << 4);

	for (std::map<uint64_t, TERMINAL>::iterator it = terminals.begin(); it != terminals.end(); ++it) {

		if (it->second.id == object->id) {

			uint64_t old_dev_id = it->first;
			uint64_t new_dev_id = *(uint64_t *)dev_id;

			if (old_dev_id != new_dev_id) {
				std::swap(terminals[new_dev_id], it->second);
				terminals.erase(it);
			}

			return 0;
		}
	}

	error_ptr = err_object_not_found;
	error_len = sizeof(err_object_not_found) - 1;

	return -1;
}

int on_object_remove(DB_OBJECT *object)
{
	for (std::map<uint64_t, TERMINAL>::iterator it = terminals.begin(); it != terminals.end(); ++it) {

		if (it->second.id == object->id) {

			if (it->second.session != NULL)
				data_session_close(it->second.session);
		
			terminals.erase(it);

			return 0;
		}
	}

	error_ptr = err_object_not_found;
	error_len = sizeof(err_object_not_found) - 1;

	return -1;
}

int on_object_create(DB_OBJECT *object)
{
	JKEY key;

	jparse_extract_key((unsigned char *)"dev_id", 6, (unsigned char *)object->core_data, object->core_data_size, &key);

	if (key.value_type != JPARSE_VALUE_TYPE_STRING) {
		error_ptr = err_invalid_dev_id;
		error_len = sizeof(err_invalid_dev_id) - 1;
		return -1;
	}

	unsigned char dev_id[8];

	dev_id[0] = ((key.value.str_val[0]  - '0') << 4) | (key.value.str_val[1]  - '0');
	dev_id[1] = ((key.value.str_val[2]  - '0') << 4) | (key.value.str_val[3]  - '0');
	dev_id[2] = ((key.value.str_val[4]  - '0') << 4) | (key.value.str_val[5]  - '0');
	dev_id[3] = ((key.value.str_val[6]  - '0') << 4) | (key.value.str_val[7]  - '0');
	dev_id[4] = ((key.value.str_val[8]  - '0') << 4) | (key.value.str_val[9]  - '0');
	dev_id[5] = ((key.value.str_val[10] - '0') << 4) | (key.value.str_val[11] - '0');
	dev_id[6] = ((key.value.str_val[12] - '0') << 4) | (key.value.str_val[13] - '0');
	dev_id[7] = ((key.value.str_val[14] - '0') << 4);

	if ((object->module_data != NULL)&&(object->module_data_size < sizeof(BLOB_RECORD_305))) {

		BLOB_RECORD_305 *config = (BLOB_RECORD_305 *)realloc(object->module_data, sizeof(BLOB_RECORD_305));

		if (config == NULL) {
			error_ptr = err_unable_to_allocate_config;
			error_len = sizeof(err_unable_to_allocate_config) - 1;
			return -1;
		}

		memset((char *)config + object->module_data_size, 0, sizeof(BLOB_RECORD_305) - object->module_data_size);

		object->module_data = (unsigned char *)config;
		object->module_data_size = sizeof(BLOB_RECORD_305);
	}

	if ((object->module_data == NULL)||(object->module_data_size != sizeof(BLOB_RECORD_305))) {

		BLOB_RECORD_305 *config = (BLOB_RECORD_305 *)realloc(object->module_data, sizeof(BLOB_RECORD_305));

		if (config == NULL) {
			error_ptr = err_unable_to_allocate_config;
			error_len = sizeof(err_unable_to_allocate_config) - 1;
			return -1;
		}

		memcpy(config, &default_config, sizeof(BLOB_RECORD_305));

		config->requested_fw_ver	= 0;
		config->actual_fw_ver		= 0;
		config->need_profile		= true;
		config->timestamp			= now;

		object->module_data			= (unsigned char *)config;
		object->module_data_size	= sizeof(BLOB_RECORD_305);

		api_db_update_object_blob(object);
	}

	if (object->stream == NULL)
		return -1;

	STREAM_INFO *si = api_storage_get_stream_info(object->stream);

	TERMINAL terminal;

	memset(&terminal, 0, sizeof(terminal));

	terminal.id					= object->id;
	terminal.object				= object;	
	terminal.last_command_time	= si->last_flags_time;
	terminal.flags1				= si->last_flags1;
	terminal.flags2				= si->last_flags2;

	if (si->last_nav_time == si->last_flags_time)
		terminal.flags1 |= RECORD_FLAG1_COG_9;

	terminals[*(uint64_t*)dev_id] = terminal;

	return 0;
}

int on_timer()
{
	now = time(NULL);

	return 0;
}
