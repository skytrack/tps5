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

	size_t size = terminals.size();

	if (size > 0) {

		TERMINAL *terminal = &terminals[0];

		for (size_t i = 0; i < size; i++, terminal++) {

			if (terminal->id == object->id) {

				unsigned char *ptr = id + i * 8;

				*ptr++ = ((key.value.str_val[0]  - '0') << 4) | (key.value.str_val[1]  - '0');
				*ptr++ = ((key.value.str_val[2]  - '0') << 4) | (key.value.str_val[3]  - '0');
				*ptr++ = ((key.value.str_val[4]  - '0') << 4) | (key.value.str_val[5]  - '0');
				*ptr++ = ((key.value.str_val[6]  - '0') << 4) | (key.value.str_val[7]  - '0');
				*ptr++ = ((key.value.str_val[8]  - '0') << 4) | (key.value.str_val[9]  - '0');
				*ptr++ = ((key.value.str_val[10] - '0') << 4) | (key.value.str_val[11] - '0');
				*ptr++ = ((key.value.str_val[12] - '0') << 4) | (key.value.str_val[13] - '0');
				*ptr++ = ((key.value.str_val[14] - '0') << 4);

				memcpy(terminal->tx_cmd.header.imei, ptr - 8, 8);

				return 0;
			}
		}
	}

	error_ptr = err_object_not_found;
	error_len = sizeof(err_object_not_found) - 1;

	return -1;
}

int on_object_remove(DB_OBJECT *object)
{
	for (size_t i = 0; i < terminals.size(); i++) {

		if (terminals[i].id == object->id) {

			TERMINAL *terminal = &terminals[i];

			if (terminal->online)
				close_connection(terminal);
		
			unsigned char *ptr = id + i * 8;
			memcpy(ptr, ptr + 8, (terminals.size() * 8) - (ptr - id) - 8);

			terminals.erase(terminals.begin() + i);

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

	if ((object->module_data != NULL)&&(object->module_data_size < sizeof(BLOB_RECORD))) {

		BLOB_RECORD *config = (BLOB_RECORD *)realloc(object->module_data, sizeof(BLOB_RECORD));

		if (config == NULL) {
			error_ptr = err_unable_to_allocate_config;
			error_len = sizeof(err_unable_to_allocate_config) - 1;
			return -1;
		}

		memset((char *)config + object->module_data_size, 0, sizeof(BLOB_RECORD) - object->module_data_size);

		object->module_data = (unsigned char *)config;
		object->module_data_size = sizeof(BLOB_RECORD);
	}

	if ((object->module_data == NULL)||(object->module_data_size != sizeof(BLOB_RECORD))) {

		BLOB_RECORD *config = (BLOB_RECORD *)realloc(object->module_data, sizeof(BLOB_RECORD));

		if (config == NULL) {
			error_ptr = err_unable_to_allocate_config;
			error_len = sizeof(err_unable_to_allocate_config) - 1;
			return -1;
		}

		memcpy(config, &default_config, sizeof(BLOB_RECORD));

		config->requested_fw_ver = 0;
		config->actual_fw_ver = 0;
		config->need_profile = true;
		config->timestamp = now;

		object->module_data = (unsigned char *)config;
		object->module_data_size = sizeof(BLOB_RECORD);

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

	unsigned char *ptr = id + terminals.size() * 8;

	*ptr++ = ((key.value.str_val[0]  - '0') << 4) | (key.value.str_val[1]  - '0');
	*ptr++ = ((key.value.str_val[2]  - '0') << 4) | (key.value.str_val[3]  - '0');
	*ptr++ = ((key.value.str_val[4]  - '0') << 4) | (key.value.str_val[5]  - '0');
	*ptr++ = ((key.value.str_val[6]  - '0') << 4) | (key.value.str_val[7]  - '0');
	*ptr++ = ((key.value.str_val[8]  - '0') << 4) | (key.value.str_val[9]  - '0');
	*ptr++ = ((key.value.str_val[10] - '0') << 4) | (key.value.str_val[11] - '0');
	*ptr++ = ((key.value.str_val[12] - '0') << 4) | (key.value.str_val[13] - '0');
	*ptr++ = ((key.value.str_val[14] - '0') << 4);

	memcpy(terminal.tx_cmd.header.imei, ptr - 8, 8);

	terminals.push_back(terminal);

	return 0;
}

int on_timer()
{
	now = time(NULL);

	size_t size = terminals.size();

	if (size > 0) {

		TERMINAL *terminal = &terminals[0];
		
		for (size_t i = 0; i < size; i++, terminal++) {

			if (terminal->online == false)
				continue;

			if (terminal->session_timeout <= now) {
				close_connection(terminal);
				continue;
			}

			if (terminal->ack_timeout <= now) {

				terminal->send_attempt++;

				if (terminal->send_attempt == MAX_SEND_ATTEMPT) {
					close_connection(terminal);
				}
				else {
					terminal->ack_timeout = now + send_attempt_intervals[terminal->send_attempt];

					if (terminal->f == NULL) {
						api_log_printf("[AK306] Resend command #%u, seq_no #%u, terminal_id=%u\r\n", terminal->tx_cmd.header.command_id, terminal->tx_cmd.header.seqNo, terminal->id);
						api_send_udp((unsigned char *)&terminal->tx_cmd, terminal->tx_cmd.header.size, terminal->context);
					}
					else {
						api_log_printf("[AK306] Resend frame #%u, size #%u, terminal_id=%u\r\n", terminal->tx_cmd.payload.buffer[3] & 0xFF, terminal->tx_cmd.header.size, terminal->id);
						api_send_udp((unsigned char *)&terminal->tx_cmd.payload.buffer, terminal->tx_cmd.header.size, terminal->context);
					}
				}

				continue;
			}

			BLOB_RECORD *config = (BLOB_RECORD *)terminal->object->module_data;

			if (terminal->ack_timeout == UINT_MAX) {
				if (config->need_profile) {
					construct_profile(terminal);
					api_send_udp((unsigned char *)&terminal->tx_cmd, terminal->tx_cmd.header.size, terminal->context);
				}
				else
				if ((config->requested_fw_ver != 0)&&(config->requested_fw_ver != config->actual_fw_ver)) {
					terminal->tx_cmd.header.command_id = AK306_CMD_FIRMWARE;
					construct_command(terminal, 0);
					api_send_udp((unsigned char *)&terminal->tx_cmd, terminal->tx_cmd.header.size, terminal->context);
				}
			}
		}
	}

	return 0;
}
