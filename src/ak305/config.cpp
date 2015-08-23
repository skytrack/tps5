//******************************************************************************
//
// File Name : config.cpp
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <string>
#include "json.h"
#include "form.h"
#include "api.h"
#include "ak305.h"
#include "config.h"
#include "dirscan.h"
#include "common.h"

#ifndef offsetof

#define offsetof(type, member)  __builtin_offsetof (type, member)

#endif

static unsigned char not_enough_memory[] = "Not enough memory";

#define KEYS_COUNT 34

static JSON_INTERESTED_KEY keys[KEYS_COUNT] = {

	{ "fw",							0,	VALUE_TYPE_STRING,	1,	0,	32,		1},
	{ "active_interval",			0,	VALUE_TYPE_NUMERIC,	1,	1,	3600,	1},
	{ "active_ppp",					0,	VALUE_TYPE_NUMERIC,	1,	1,	4,		1},
	{ "disable_static",				0,	VALUE_TYPE_BOOL,	1,	0,	0,		1},
	{ "park_interval",				0,	VALUE_TYPE_NUMERIC,	1,	1,	1440,	1},
	{ "park_ppp",					0,	VALUE_TYPE_NUMERIC,	1,	1,	4,		1},
	{ "input1",						0,	VALUE_TYPE_BOOL,	1,	0,	0,		1},
	{ "input1_type",				0,	VALUE_TYPE_NUMERIC,	1,	0,	4,		1},
	{ "input1_sms_on_active",		0,	VALUE_TYPE_STRING,	1,	0,	15,		1},
	{ "input2",						0,	VALUE_TYPE_BOOL,	1,	0,	0,		1},
	{ "input2_type",				0,	VALUE_TYPE_NUMERIC,	1,	0,	3,		1},
	{ "input2_sms_on_active",		0,	VALUE_TYPE_STRING,	1,	0,	15,		1},
	{ "input3",						0,	VALUE_TYPE_BOOL,	1,	0,	0,		1},
	{ "input3_type",				0,	VALUE_TYPE_NUMERIC,	1,	0,	3,		1},
	{ "input3_sms_on_active",		0,	VALUE_TYPE_STRING,	1,	0,	15,		1},
	{ "input4",						0,	VALUE_TYPE_BOOL,	1,	0,	0,		1},
	{ "input4_type",				0,	VALUE_TYPE_NUMERIC,	1,	0,	3,		1},
	{ "input4_sms_on_active",		0,	VALUE_TYPE_STRING,	1,	0,	15,		1},
	{ "sms_a1",						0,	VALUE_TYPE_STRING,	1,	0,	15,		1},
	{ "sms_d1",						0,	VALUE_TYPE_STRING,	1,	0,	15,		1},
	{ "sms_a2",						0,	VALUE_TYPE_STRING,	1,	0,	15,		1},
	{ "sms_d2",						0,	VALUE_TYPE_STRING,	1,	0,	15,		1},
	{ "sms_a3",						0,	VALUE_TYPE_STRING,	1,	0,	15,		1},
	{ "sms_d3",						0,	VALUE_TYPE_STRING,	1,	0,	15,		1},
	{ "sms_a4",						0,	VALUE_TYPE_STRING,	1,	0,	15,		1},
	{ "sms_d4",						0,	VALUE_TYPE_STRING,	1,	0,	15,		1},
	{ "phone",						0,	VALUE_TYPE_STRING,	1,	0,	32,		1},
	{ "audio",						0,	VALUE_TYPE_BOOL,	1,	0,	0,		1},
	{ "volume",						0,	VALUE_TYPE_NUMERIC,	1,	1,	100,	1},
	{ "mic",						0,	VALUE_TYPE_NUMERIC,	1,	1,	15,		1},
	{ "echo_model",					0,	VALUE_TYPE_NUMERIC,	1,	0,	0,		1},
	{ "echo_level",					0,	VALUE_TYPE_NUMERIC,	1,	0,	0,		1},
	{ "echo_patterns",				0,	VALUE_TYPE_NUMERIC,	1,	0,	0,		1},
	{ "autoanswer",					0,	VALUE_TYPE_BOOL,	1,	0,	0,		1},
};

BLOB_RECORD_305 default_config;

typedef struct config_field 
{
	const char *tag;
	size_t len;
	JSON_VALUE_TYPE type;
	size_t offset;
	size_t size;
} CONFIG_FIELD;

static CONFIG_FIELD fields[KEYS_COUNT] = 
{
	{ "\"fw_value***********************\"",	0, VALUE_TYPE_STRING,  offsetof(BLOB_RECORD_305, fw), sizeof(default_config.fw) },
	{ "\"active_interval_value\"",				0, VALUE_TYPE_NUMERIC, offsetof(BLOB_RECORD_305, DriveInterval), sizeof(default_config.DriveInterval) },
	{ "\"active_ppp_value\"",					0, VALUE_TYPE_NUMERIC, offsetof(BLOB_RECORD_305, DrivePPP), sizeof(default_config.DrivePPP) },
	{ "\"disable_static_checked\"",				0, VALUE_TYPE_BOOL,	   offsetof(BLOB_RECORD_305, bDisableStatic), sizeof(default_config.bDisableStatic) },
	{ "\"park_interval_value\"",				0, VALUE_TYPE_NUMERIC, offsetof(BLOB_RECORD_305, ParkInterval), sizeof(default_config.ParkInterval) },
	{ "\"park_ppp_value\"",						0, VALUE_TYPE_NUMERIC, offsetof(BLOB_RECORD_305, ParkPPP), sizeof(default_config.ParkPPP) },
	{ "\"input1_expanded\"",					0, VALUE_TYPE_BOOL,    offsetof(BLOB_RECORD_305, input1_expanded), sizeof(default_config.input1_expanded) },
	{ "\"input1_type_value\"",					0, VALUE_TYPE_NUMERIC, offsetof(BLOB_RECORD_305, Input1Mode), sizeof(default_config.Input1Mode) },
	{ "\"input1_sms_on_active_value\"",			0, VALUE_TYPE_STRING,  offsetof(BLOB_RECORD_305, Input1SMSOnActive), sizeof(default_config.Input1SMSOnActive) },
	{ "\"input2_expanded\"",					0, VALUE_TYPE_BOOL,    offsetof(BLOB_RECORD_305, input2_expanded), sizeof(default_config.input2_expanded) },
	{ "\"input2_type_value\"",					0, VALUE_TYPE_NUMERIC, offsetof(BLOB_RECORD_305, Input2Mode), sizeof(default_config.Input2Mode) },
	{ "\"input2_sms_on_active_value\"",			0, VALUE_TYPE_STRING,  offsetof(BLOB_RECORD_305, Input2SMSOnActive), sizeof(default_config.Input2SMSOnActive) },
	{ "\"input3_expanded\"",					0, VALUE_TYPE_BOOL,    offsetof(BLOB_RECORD_305, input3_expanded), sizeof(default_config.input3_expanded) },
	{ "\"input3_type_value\"",					0, VALUE_TYPE_NUMERIC, offsetof(BLOB_RECORD_305, Input3Mode), sizeof(default_config.Input3Mode) },
	{ "\"input3_sms_on_active_value\"",			0, VALUE_TYPE_STRING,  offsetof(BLOB_RECORD_305, Input3SMSOnActive), sizeof(default_config.Input3SMSOnActive) },
	{ "\"input4_expanded\"",					0, VALUE_TYPE_BOOL,    offsetof(BLOB_RECORD_305, input4_expanded), sizeof(default_config.input4_expanded) },
	{ "\"input4_type_value\"",					0, VALUE_TYPE_NUMERIC, offsetof(BLOB_RECORD_305, Input4Mode), sizeof(default_config.Input4Mode) },
	{ "\"input4_sms_on_active_value\"",			0, VALUE_TYPE_STRING,  offsetof(BLOB_RECORD_305, Input4SMSOnActive), sizeof(default_config.Input4SMSOnActive) },
	{ "\"sms_a1_value***\"",					0, VALUE_TYPE_STRING,  offsetof(BLOB_RECORD_305, sms_a1), sizeof(default_config.sms_a1) },
	{ "\"sms_d1_value***\"",					0, VALUE_TYPE_STRING,  offsetof(BLOB_RECORD_305, sms_d1), sizeof(default_config.sms_d1) },
	{ "\"sms_a2_value***\"",					0, VALUE_TYPE_STRING,  offsetof(BLOB_RECORD_305, sms_a2), sizeof(default_config.sms_a2) },
	{ "\"sms_d2_value***\"",					0, VALUE_TYPE_STRING,  offsetof(BLOB_RECORD_305, sms_d2), sizeof(default_config.sms_d2) },
	{ "\"sms_a3_value***\"",					0, VALUE_TYPE_STRING,  offsetof(BLOB_RECORD_305, sms_a3), sizeof(default_config.sms_a3) },
	{ "\"sms_d3_value***\"",					0, VALUE_TYPE_STRING,  offsetof(BLOB_RECORD_305, sms_d3), sizeof(default_config.sms_d3) },
	{ "\"sms_a4_value***\"",					0, VALUE_TYPE_STRING,  offsetof(BLOB_RECORD_305, sms_a4), sizeof(default_config.sms_a4) },
	{ "\"sms_d4_value***\"",					0, VALUE_TYPE_STRING,  offsetof(BLOB_RECORD_305, sms_d4), sizeof(default_config.sms_d4) },
	{ "\"phone_value********************\"",	0, VALUE_TYPE_STRING,  offsetof(BLOB_RECORD_305, Phone), sizeof(default_config.Phone) },
	{ "\"audio_expanded\"",						0, VALUE_TYPE_BOOL,    offsetof(BLOB_RECORD_305, audio_expanded), sizeof(default_config.audio_expanded) },
	{ "\"volume_value\"",						0, VALUE_TYPE_NUMERIC, offsetof(BLOB_RECORD_305, nCLVL), sizeof(default_config.nCLVL) },
	{ "\"mic_value\"",							0, VALUE_TYPE_NUMERIC, offsetof(BLOB_RECORD_305, nCMIC), sizeof(default_config.nCMIC) },
	{ "\"echo_model_value\"",					0, VALUE_TYPE_NUMERIC, offsetof(BLOB_RECORD_305, EchoModel), sizeof(default_config.EchoModel) },
	{ "\"echo_level_value\"",					0, VALUE_TYPE_NUMERIC, offsetof(BLOB_RECORD_305, EchoLevel), sizeof(default_config.EchoLevel) },
	{ "\"echo_patterns_value\"",				0, VALUE_TYPE_NUMERIC, offsetof(BLOB_RECORD_305, EchoPatterns), sizeof(default_config.EchoPatterns) },
	{ "\"autoanswer_checked\"",					0, VALUE_TYPE_BOOL,    offsetof(BLOB_RECORD_305, bAutoAnswer), sizeof(default_config.bAutoAnswer) }
};

std::string fw_dir;

typedef struct scan_ctx
{
	size_t bytes_left;
	size_t item_no;
	unsigned char *ptr;
} SCAN_CTX;

int config_put_json(DB_OBJECT *object, unsigned char *json, size_t len)
{
	char *ptr;

	if (json_parse(json, len, keys, KEYS_COUNT)) {

		unsigned char *err = json_get_error(json, len);

		error_buf[sizeof(error_buf) - 1] = '\0';
		strncpy((char *)error_buf, (char *)err, sizeof(error_buf) - 1);
		error_len = strlen((char *)error_buf);
		error_ptr = error_buf;

		json_free_error(err);

		return -1;
	}

	BLOB_RECORD_305 *config = (BLOB_RECORD_305 *)object->module_data;

	if ((config == NULL)||(object->module_data_size != sizeof(BLOB_RECORD_305))) {
		strncpy((char *)error_buf, "Invalid config blob", sizeof(error_buf) - 1);
		error_ptr = error_buf;
		error_len = 19;
		return -1;
	}

	for (int i = 0; i < KEYS_COUNT; i++) {
		switch (fields[i].type) {

		case VALUE_TYPE_NUMERIC:

			switch (fields[i].size) {
			case sizeof(unsigned char):
				*(unsigned char *)((unsigned char*)config + fields[i].offset) = keys[i].int_val;
				break;
			case sizeof(unsigned short):
				*(unsigned short *)((unsigned char*)config + fields[i].offset) = keys[i].int_val;
				break;
			case sizeof(unsigned int):
				*(unsigned int *)((unsigned char*)config + fields[i].offset) = keys[i].int_val;
				break;
			}

			break;

		case VALUE_TYPE_FLOAT:

			*(float *)((unsigned char*)config + fields[i].offset) = keys[i].float_val;

			break;

		case VALUE_TYPE_BOOL:

			*(bool *)((unsigned char*)config + fields[i].offset) = keys[i].bool_val;

			break;

		case VALUE_TYPE_STRING:

			ptr = (char*)config + fields[i].offset;

			ptr[fields[i].size - 1] = '\0';

			strncpy(ptr, keys[i].string_val.c_str(), fields[i].size - 1);

			break;

		case VALUE_TYPE_NULL:
		case VALUE_TYPE_OBJECT:
		case VALUE_TYPE_ARRAY:
			break;
		}
	}

	config->need_profile = true;
	config->timestamp = time(NULL);

	if (memcmp(config->fw, "default", 8)) {
		char *p = config->fw;
		while ((*p != '\0')&&((*p < '0')||(*p > '9')))
			p++;
		config->requested_fw_ver = strtoul(p, NULL, 10);
	}
	else
		config->requested_fw_ver = 0;

	if (api_db_update_object_blob(object) != 0) {

		error_buf[sizeof(error_buf) - 1] = '\0';
		strncpy((char *)error_buf, (char *)api_db_get_error(), sizeof(error_buf) - 1);
		error_len = strlen((char *)error_buf);
		error_ptr = error_buf;
				
		return -1;
	}

	return 0;
}

static int config_set_bool_value(unsigned char *s, const char *key, int key_len, bool value)
{
	char *ptr = strstr((char *)s, (char *)key);

	if (ptr == NULL)
		return -1;

	memset(ptr, ' ', key_len);

	if (value)
		memcpy(ptr, "true", 4);
	else
		memcpy(ptr, "false", 5);

	return 0;
}

int dirscan_callback(const char *file_name, void *c)
{
	SCAN_CTX *ctx = (SCAN_CTX*)c;

	size_t len = strlen(file_name);

	size_t item_len = 11 + len + 2 + 6 + len + 2;

	if (item_len > ctx->bytes_left)
		return -1;

	ctx->item_no++;

	memcpy(ctx->ptr, ",{\"label\":\"", 11);
	ctx->ptr += 11;
	memcpy(ctx->ptr, file_name, len);
	ctx->ptr += len;
	memcpy(ctx->ptr, "\",\"id\":\"", 8);
	ctx->ptr += 8;
	memcpy(ctx->ptr, file_name, len);
	ctx->ptr += len;
	*ctx->ptr++ = '\"';
	*ctx->ptr++ = '}';

	return 0;
}

int config_get_json(DB_OBJECT *object, unsigned char *s, size_t *len)
{
	unsigned int int_val;
	char *ptr = (char *)s;
	int k;
	SCAN_CTX ctx;

	if ((sizeof(form) - 1) > *len) {
		error_ptr = not_enough_memory;
		error_len = sizeof(not_enough_memory) - 1;
		return -1;
	}

	BLOB_RECORD_305 *config = (BLOB_RECORD_305 *)object->module_data;
	if ((config == NULL)||(object->module_data_size != sizeof(BLOB_RECORD_305)))
		config = &default_config;

	unsigned char *fw = (unsigned char *)strstr((char *)form, "{\"FIRMWARE\":0}");

	if (fw) {
		
		size_t bytes_left = *len;

		unsigned char *fw_end = fw + 14;

		memcpy(s, form, fw - form);

		bytes_left -= fw - form;

		fw = s + (fw - form);

		if (bytes_left < 36) {
			error_ptr = not_enough_memory;
			error_len = sizeof(not_enough_memory) - 1;
			return -1;
		}

		memcpy(fw, "{\"label\": \"default\", \"id\":\"default\"}", 36);
		fw += 36;

		bytes_left -= 36;

		ctx.bytes_left = bytes_left;
		ctx.item_no = 0;
		ctx.ptr = fw;

		dirscan(fw_dir.c_str(), dirscan_callback, &ctx);

		size_t footer_len = form + sizeof(form) - fw_end - 1;

		if (footer_len > ctx.bytes_left) {
			error_ptr = not_enough_memory;
			error_len = sizeof(not_enough_memory) - 1;
			return -1;
		}

		memcpy(ctx.ptr, fw_end, footer_len);

		*len = (ctx.ptr - s) + footer_len;
	}
	else {
		memcpy(s, form, sizeof(form) - 1);
		*len = sizeof(form) - 1;
	}

	for (int i = 0; i < KEYS_COUNT; i++) {
		switch (fields[i].type) {

		case VALUE_TYPE_NUMERIC:

			switch (fields[i].size) {
			case sizeof(unsigned char):
				int_val = *(unsigned char *)((unsigned char*)config + fields[i].offset);
				break;
			case sizeof(unsigned short):
				int_val = *(unsigned short *)((unsigned char*)config + fields[i].offset);
				break;
			case sizeof(unsigned int):
				int_val = *(unsigned int *)((unsigned char*)config + fields[i].offset);
				break;
			default:
				return -1;
			}

			ptr = strstr(ptr, fields[i].tag);

			if (ptr == NULL)
				return -1;

			memset(ptr, ' ', fields[i].len);
			ptr += fields[i].len - 1;

		    do {
		        *--ptr = '0' + int_val % 10;
				int_val /= 10;
			} while (int_val != 0);

			break;

		case VALUE_TYPE_FLOAT:

			ptr = strstr(ptr, fields[i].tag);

			if (ptr == NULL)
				return -1;

			memset(ptr, ' ', fields[i].len);
			ptr[sprintf(ptr, "%f", *(float *)((unsigned char*)config + fields[i].offset))] = ' ';

			break;

		case VALUE_TYPE_BOOL:

			ptr = strstr(ptr, fields[i].tag);

			if (ptr == NULL)
				return -1;

			memset(ptr, ' ', fields[i].len);

			if (*(char *)((unsigned char*)config + fields[i].offset) > 0)
				memcpy(ptr, "true", 4);
			else
				memcpy(ptr, "false", 5);

			break;

		case VALUE_TYPE_STRING:

			ptr = strstr(ptr, fields[i].tag);

			if (ptr == NULL)
				return -1;

			memset(ptr, ' ', fields[i].len);

			*ptr++ = '\"';

			k = 0;
			while (char ch = *((char*)config + fields[i].offset + k)) {
				*ptr++ = ch;
				k++;
				if (k == fields[i].len - 2)
					break;
			}

			*ptr++ = '\"';

			break;
		default:
			return -1;
		}
	}

	return 0;
}

int config_get_device_caps(DB_OBJECT *object, unsigned long long *caps)
{
	*caps = 0;

	BLOB_RECORD_305 *config = (BLOB_RECORD_305 *)object->module_data;
	if ((config == NULL)||(object->module_data_size != sizeof(BLOB_RECORD_305)))
		config = &default_config;

	*caps = DEVICE_CAPS_HARDWARE_SETTINGS |DEVICE_CAPS_NAV_ALTITUDE | DEVICE_CAPS_ANALOG_INPUT1 | DEVICE_CAPS_ANALOG_INPUT2;

	if (config->input1_expanded) {
		switch (config->Input1Mode) {
		case 1:
			*caps |= DEVICE_CAPS_IGNITION;
			break;
		case 2:
			*caps |= DEVICE_CAPS_DISCRETE_INPUT1;
			break;
		case 3:
			*caps |= DEVICE_CAPS_COUNTER_INPUT1;
			break;
		case 4:
			*caps |= DEVICE_CAPS_INJECTOR;
			break;
		}
	}

	if (config->input2_expanded) {
		switch (config->Input2Mode) {
		case 1:
			*caps |= DEVICE_CAPS_IGNITION;
			break;
		case 2:
			*caps |= DEVICE_CAPS_DISCRETE_INPUT2;
			break;
		case 3:
			*caps |= DEVICE_CAPS_COUNTER_INPUT2;
			break;
		}
	}

	if (config->input3_expanded) {
		switch (config->Input3Mode) {
		case 1:
			*caps |= DEVICE_CAPS_IGNITION;
			break;
		case 2:
			*caps |= DEVICE_CAPS_DISCRETE_INPUT3;
			break;
		case 3:
			*caps |= DEVICE_CAPS_COUNTER_INPUT3;
			break;
		}
	}

	if (config->input4_expanded) {
		switch (config->Input4Mode) {
		case 1:
			*caps |= DEVICE_CAPS_IGNITION;
			break;
		case 2:
			*caps |= DEVICE_CAPS_DISCRETE_INPUT4;
			break;
		case 3:
			*caps |= DEVICE_CAPS_COUNTER_INPUT4;
			break;
		}
	}

	if (config->audio_expanded)
		*caps |= DEVICE_CAPS_VOICE;

	return 0;
}

int config_init()
{
	for (size_t i = 0; i < KEYS_COUNT; i++) {
		keys[i].key_len = strlen((char *)keys[i].key);
		fields[i].len = strlen((char *)fields[i].tag);
	}

	default_config.Phone[0] = '\0';

	default_config.fw[sizeof(default_config.fw) - 1] = '\0';
	strncpy(default_config.fw, "default", sizeof(default_config.fw) - 1);
	
	default_config.DriveInterval = 15;
	default_config.DrivePPP = 4;

	default_config.ParkInterval = 30;
	default_config.ParkPPP = 1;

	default_config.bDisableStatic = true;

	default_config.input1_expanded = false;
	default_config.Input1Mode = 0;
	default_config.Input1SMSOnActive[0] = '\0';

	default_config.input2_expanded = true;
	default_config.Input2Mode = 1;
	default_config.Input2SMSOnActive[0] = '\0';

	default_config.input3_expanded = false;
	default_config.Input3Mode = 0;
	default_config.Input3SMSOnActive[0] = '\0';

	default_config.input4_expanded = false;
	default_config.Input4Mode = 0;
	default_config.Input4SMSOnActive[0] = '\0';

	default_config.sms_a1[0] = '\0';
	default_config.sms_a2[0] = '\0';
	default_config.sms_a3[0] = '\0';
	default_config.sms_a4[0] = '\0';
	default_config.sms_d1[0] = '\0';
	default_config.sms_d2[0] = '\0';
	default_config.sms_d3[0] = '\0';
	default_config.sms_d4[0] = '\0';

	default_config.audio_expanded = false;
	default_config.nCMIC = 12;
	default_config.nCLVL = 85;
	default_config.EchoModel = 4000;
	default_config.EchoLevel = 20;
	default_config.EchoPatterns = 4;

	default_config.requested_fw_ver = 0;
	default_config.actual_fw_ver = 0;
	default_config.need_profile = true;

	default_config.timestamp = time(NULL);

	json_init();

	return 0;
}

int config_destroy()
{
	json_destroy();
	return 0;
}
