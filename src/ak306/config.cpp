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
#include "ak306.h"
#include "config.h"
#include "dirscan.h"
#include "common.h"

#ifndef offsetof

#define offsetof(type, member)  __builtin_offsetof (type, member)

#endif

static unsigned char not_enough_memory[] = "Not enough memory";

#define KEYS_COUNT 62

static JSON_INTERESTED_KEY keys[KEYS_COUNT] = {

	{ "fw",							0,	VALUE_TYPE_STRING,	1,	0,	32,		1},
	{ "move",						0,	VALUE_TYPE_NUMERIC,	1,	0,	5,		1},
	{ "park_time",					0,	VALUE_TYPE_NUMERIC,	1,	0,	0,		1},
	{ "impulse_count",				0,	VALUE_TYPE_NUMERIC,	1,	0,	0,		1},
	{ "active_interval",			0,	VALUE_TYPE_NUMERIC,	1,	1,	3600,	1},
	{ "active_ppp",					0,	VALUE_TYPE_NUMERIC,	1,	1,	4,		1},
	{ "wait_enable",				0,	VALUE_TYPE_BOOL,	1,	0,	0,		1},
	{ "wait_delay",					0,	VALUE_TYPE_NUMERIC,	1,	1,	0,		1},
	{ "wait_interval",				0,	VALUE_TYPE_NUMERIC,	1,	1,	1440,	1},
	{ "sleep_enable",				0,	VALUE_TYPE_BOOL,	1,	0,	0,		1},
	{ "sleep_delay",				0,	VALUE_TYPE_NUMERIC,	1,	1,	0,		1},
	{ "sleep_interval",				0,	VALUE_TYPE_NUMERIC,	1,	1,	24,		1},
	{ "sleep_max_nav_search",		0,	VALUE_TYPE_NUMERIC,	1,	0,	0,		1},
	{ "sleep_max_gsm_search",		0,	VALUE_TYPE_NUMERIC,	1,	0,	0,		1},
	{ "input1",						0,	VALUE_TYPE_BOOL,	1,	0,	0,		1},
	{ "input1_type",				0,	VALUE_TYPE_NUMERIC,	1,	0,	11,		1},
	{ "input1_min_active_time",		0,	VALUE_TYPE_NUMERIC,	1,	10,	0,		1},
	{ "input1_min_inactive_time",	0,	VALUE_TYPE_NUMERIC,	1,	10,	0,		1},
	{ "input1_sms_on_active",		0,	VALUE_TYPE_STRING,	1,	0,	15,		1},
	{ "input1_sms_on_inactive",		0,	VALUE_TYPE_STRING,	1,	0,	15,		1},
	{ "input2",						0,	VALUE_TYPE_BOOL,	1,	0,	0,		1},
	{ "input2_type",				0,	VALUE_TYPE_NUMERIC,	1,	0,	11,		1},
	{ "input2_min_active_time",		0,	VALUE_TYPE_NUMERIC,	1,	10,	0,		1},
	{ "input2_min_inactive_time",	0,	VALUE_TYPE_NUMERIC,	1,	10,	0,		1},
	{ "input2_sms_on_active",		0,	VALUE_TYPE_STRING,	1,	0,	15,		1},
	{ "input2_sms_on_inactive",		0,	VALUE_TYPE_STRING,	1,	0,	15,		1},
	{ "input3",						0,	VALUE_TYPE_BOOL,	1,	0,	0,		1},
	{ "input3_type",				0,	VALUE_TYPE_NUMERIC,	1,	0,	11,		1},
	{ "input3_min_active_time",		0,	VALUE_TYPE_NUMERIC,	1,	10,	0,		1},
	{ "input3_min_inactive_time",	0,	VALUE_TYPE_NUMERIC,	1,	10,	0,		1},
	{ "input3_sms_on_active",		0,	VALUE_TYPE_STRING,	1,	0,	15,		1},
	{ "input3_sms_on_inactive",		0,	VALUE_TYPE_STRING,	1,	0,	15,		1},
	{ "input4",						0,	VALUE_TYPE_BOOL,	1,	0,	0,		1},
	{ "input4_type",				0,	VALUE_TYPE_NUMERIC,	1,	0,	11,		1},
	{ "input4_min_active_time",		0,	VALUE_TYPE_NUMERIC,	1,	10,	0,		1},
	{ "input4_min_inactive_time",	0,	VALUE_TYPE_NUMERIC,	1,	10,	0,		1},
	{ "input4_sms_on_active",		0,	VALUE_TYPE_STRING,	1,	0,	15,		1},
	{ "input4_sms_on_inactive",		0,	VALUE_TYPE_STRING,	1,	0,	15,		1},
	{ "adc1",						0,	VALUE_TYPE_BOOL,	1,	0,	0,		1},
	{ "adc1_type",					0,	VALUE_TYPE_NUMERIC,	1,	0,	5,		1},
	{ "adc1_threshold",				0,	VALUE_TYPE_FLOAT,	1,	0,	0,		1},
	{ "adc2",						0,	VALUE_TYPE_BOOL,	1,	0,	0,		1},
	{ "adc2_type",					0,	VALUE_TYPE_NUMERIC,	1,	0,	5,		1},
	{ "adc2_threshold",				0,	VALUE_TYPE_FLOAT,	1,	0,	0,		1},
	{ "sms1_text",					0,	VALUE_TYPE_STRING,	1,	0,	15,		1},
	{ "sms1_script",				0,	VALUE_TYPE_STRING,	1,	0,	31,		1},
	{ "sms2_text",					0,	VALUE_TYPE_STRING,	1,	0,	15,		1},
	{ "sms2_script",				0,	VALUE_TYPE_STRING,	1,	0,	31,		1},
	{ "sms3_text",					0,	VALUE_TYPE_STRING,	1,	0,	15,		1},
	{ "sms3_script",				0,	VALUE_TYPE_STRING,	1,	0,	31,		1},
	{ "sms4_text",					0,	VALUE_TYPE_STRING,	1,	0,	15,		1},
	{ "sms4_script",				0,	VALUE_TYPE_STRING,	1,	0,	31,		1},
	{ "phone",						0,	VALUE_TYPE_STRING,	1,	0,	32,		1},
	{ "audio",						0,	VALUE_TYPE_BOOL,	1,	0,	0,		1},
	{ "volume",						0,	VALUE_TYPE_NUMERIC,	1,	1,	100,	1},
	{ "mic",						0,	VALUE_TYPE_NUMERIC,	1,	1,	15,		1},
	{ "es",							0,	VALUE_TYPE_NUMERIC,	1,	0,	7,		1},
	{ "ses",						0,	VALUE_TYPE_NUMERIC,	1,	0,	5,		1},
	{ "autoanswer",					0,	VALUE_TYPE_BOOL,	1,	0,	0,		1},
	{ "altitude",					0,	VALUE_TYPE_BOOL,	1,	0,	0,		1},
	{ "cog",						0,	VALUE_TYPE_BOOL,	1,	0,	0,		1},
	{ "rpm",						0,	VALUE_TYPE_BOOL,	1,	0,	0,		1}
};

BLOB_RECORD default_config;

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
	{ "\"fw_value***********************\"",	0, VALUE_TYPE_STRING,  offsetof(BLOB_RECORD, fw), sizeof(default_config.fw) },
	{ "\"move_value\"",							0, VALUE_TYPE_NUMERIC, offsetof(BLOB_RECORD, MoveDetectMethod), sizeof(default_config.MoveDetectMethod) },
	{ "\"park_time_value\"",					0, VALUE_TYPE_NUMERIC, offsetof(BLOB_RECORD, OdometerMoveTimeout), sizeof(default_config.OdometerMoveTimeout) },
	{ "\"impulse_count_value\"",				0, VALUE_TYPE_NUMERIC, offsetof(BLOB_RECORD, OdometerTicksCount), sizeof(default_config.OdometerTicksCount) },
	{ "\"active_interval_value\"",				0, VALUE_TYPE_NUMERIC, offsetof(BLOB_RECORD, DriveInterval), sizeof(default_config.DriveInterval) },
	{ "\"active_ppp_value\"",					0, VALUE_TYPE_NUMERIC, offsetof(BLOB_RECORD, DrivePPP), sizeof(default_config.DrivePPP) },
	{ "\"wait_expanded\"",						0, VALUE_TYPE_BOOL,    offsetof(BLOB_RECORD, wait_expanded), sizeof(default_config.wait_expanded) },
	{ "\"wait_delay_value\"",					0, VALUE_TYPE_NUMERIC, offsetof(BLOB_RECORD, SleepDelayInterval), sizeof(default_config.SleepDelayInterval) },
	{ "\"wait_interval_value\"",				0, VALUE_TYPE_NUMERIC, offsetof(BLOB_RECORD, SleepWakeupInterval), sizeof(default_config.SleepWakeupInterval) },
	{ "\"sleep_expanded\"",						0, VALUE_TYPE_BOOL,    offsetof(BLOB_RECORD, sleep_expanded), sizeof(default_config.sleep_expanded) },
	{ "\"sleep_delay_value\"",					0, VALUE_TYPE_NUMERIC, offsetof(BLOB_RECORD, SleepDeepDelayInterval), sizeof(default_config.SleepDeepDelayInterval) },
	{ "\"sleep_interval_value\"",				0, VALUE_TYPE_NUMERIC, offsetof(BLOB_RECORD, SleepDeepWakeupInterval), sizeof(default_config.SleepDeepWakeupInterval) },
	{ "\"sleep_nav_value\"",					0, VALUE_TYPE_NUMERIC, offsetof(BLOB_RECORD, SleepDeepMaxGPSSearchTime), sizeof(default_config.SleepDeepMaxGPSSearchTime) },
	{ "\"sleep_gsm_value\"",					0, VALUE_TYPE_NUMERIC, offsetof(BLOB_RECORD, SleepDeepMaxGSMSearchTime), sizeof(default_config.SleepDeepMaxGSMSearchTime) },
	{ "\"input1_expanded\"",					0, VALUE_TYPE_BOOL,    offsetof(BLOB_RECORD, input1_expanded), sizeof(default_config.input1_expanded) },
	{ "\"input1_type_value\"",					0, VALUE_TYPE_NUMERIC, offsetof(BLOB_RECORD, Input1Mode), sizeof(default_config.Input1Mode) },
	{ "\"input1_min_active_time_value\"",		0, VALUE_TYPE_NUMERIC, offsetof(BLOB_RECORD, Input1ActiveInterval), sizeof(default_config.Input1ActiveInterval) },
	{ "\"input1_min_inactive_time_value\"",		0, VALUE_TYPE_NUMERIC, offsetof(BLOB_RECORD, Input1InactiveInterval), sizeof(default_config.Input1InactiveInterval) },
	{ "\"input1_sms_on_active_value\"",			0, VALUE_TYPE_STRING,  offsetof(BLOB_RECORD, Input1SMSOnActive), sizeof(default_config.Input1SMSOnActive) },
	{ "\"input1_sms_on_inactive_value\"",		0, VALUE_TYPE_STRING,  offsetof(BLOB_RECORD, Input1SMSOnInactive), sizeof(default_config.Input1SMSOnInactive) },
	{ "\"input2_expanded\"",					0, VALUE_TYPE_BOOL,    offsetof(BLOB_RECORD, input2_expanded), sizeof(default_config.input2_expanded) },
	{ "\"input2_type_value\"",					0, VALUE_TYPE_NUMERIC, offsetof(BLOB_RECORD, Input2Mode), sizeof(default_config.Input2Mode) },
	{ "\"input2_min_active_time_value\"",		0, VALUE_TYPE_NUMERIC, offsetof(BLOB_RECORD, Input2ActiveInterval), sizeof(default_config.Input2ActiveInterval) },
	{ "\"input2_min_inactive_time_value\"",		0, VALUE_TYPE_NUMERIC, offsetof(BLOB_RECORD, Input2InactiveInterval), sizeof(default_config.Input2InactiveInterval) },
	{ "\"input2_sms_on_active_value\"",			0, VALUE_TYPE_STRING,  offsetof(BLOB_RECORD, Input2SMSOnActive), sizeof(default_config.Input2SMSOnActive) },
	{ "\"input2_sms_on_inactive_value\"",		0, VALUE_TYPE_STRING,  offsetof(BLOB_RECORD, Input2SMSOnInactive), sizeof(default_config.Input2SMSOnInactive) },
	{ "\"input3_expanded\"",					0, VALUE_TYPE_BOOL,    offsetof(BLOB_RECORD, input3_expanded), sizeof(default_config.input3_expanded) },
	{ "\"input3_type_value\"",					0, VALUE_TYPE_NUMERIC, offsetof(BLOB_RECORD, Input3Mode), sizeof(default_config.Input3Mode) },
	{ "\"input3_min_active_time_value\"",		0, VALUE_TYPE_NUMERIC, offsetof(BLOB_RECORD, Input3ActiveInterval), sizeof(default_config.Input3ActiveInterval) },
	{ "\"input3_min_inactive_time_value\"",		0, VALUE_TYPE_NUMERIC, offsetof(BLOB_RECORD, Input3InactiveInterval), sizeof(default_config.Input3InactiveInterval) },
	{ "\"input3_sms_on_active_value\"",			0, VALUE_TYPE_STRING,  offsetof(BLOB_RECORD, Input3SMSOnActive), sizeof(default_config.Input3SMSOnActive) },
	{ "\"input3_sms_on_inactive_value\"",		0, VALUE_TYPE_STRING,  offsetof(BLOB_RECORD, Input3SMSOnInactive), sizeof(default_config.Input3SMSOnInactive) },
	{ "\"input4_expanded\"",					0, VALUE_TYPE_BOOL,    offsetof(BLOB_RECORD, input4_expanded), sizeof(default_config.input4_expanded) },
	{ "\"input4_type_value\"",					0, VALUE_TYPE_NUMERIC, offsetof(BLOB_RECORD, Input4Mode), sizeof(default_config.Input4Mode) },
	{ "\"input4_min_active_time_value\"",		0, VALUE_TYPE_NUMERIC, offsetof(BLOB_RECORD, Input4ActiveInterval), sizeof(default_config.Input4ActiveInterval) },
	{ "\"input4_min_inactive_time_value\"",		0, VALUE_TYPE_NUMERIC, offsetof(BLOB_RECORD, Input4InactiveInterval), sizeof(default_config.Input4InactiveInterval) },
	{ "\"input4_sms_on_active_value\"",			0, VALUE_TYPE_STRING,  offsetof(BLOB_RECORD, Input4SMSOnActive), sizeof(default_config.Input4SMSOnActive) },
	{ "\"input4_sms_on_inactive_value\"",		0, VALUE_TYPE_STRING,  offsetof(BLOB_RECORD, Input4SMSOnInactive), sizeof(default_config.Input4SMSOnInactive) },
	{ "\"adc1_expanded\"",						0, VALUE_TYPE_BOOL,    offsetof(BLOB_RECORD, adc1_expanded), sizeof(default_config.adc1_expanded) },
	{ "\"adc1_type_value\"",					0, VALUE_TYPE_NUMERIC, offsetof(BLOB_RECORD, ADC1Mode), sizeof(default_config.ADC1Mode) },
	{ "\"adc1_threshold_value\"",				0, VALUE_TYPE_FLOAT,   offsetof(BLOB_RECORD, ADC1_Threshold), sizeof(default_config.ADC1_Threshold) },
	{ "\"adc2_expanded\"",						0, VALUE_TYPE_BOOL,    offsetof(BLOB_RECORD, adc2_expanded), sizeof(default_config.adc2_expanded) },
	{ "\"adc2_type_value\"",					0, VALUE_TYPE_NUMERIC, offsetof(BLOB_RECORD, ADC2Mode), sizeof(default_config.ADC2Mode) },
	{ "\"adc2_threshold_value\"",				0, VALUE_TYPE_FLOAT,   offsetof(BLOB_RECORD, ADC2_Threshold), sizeof(default_config.ADC2_Threshold) },
	{ "\"sms1_text_value\"",					0, VALUE_TYPE_STRING,  offsetof(BLOB_RECORD, sms[0]), sizeof(default_config.sms[0]) },
	{ "\"sms1_script_value**************\"",	0, VALUE_TYPE_STRING,  offsetof(BLOB_RECORD, script[0]), sizeof(default_config.script[0]) },
	{ "\"sms2_text_value\"",					0, VALUE_TYPE_STRING,  offsetof(BLOB_RECORD, sms[1]), sizeof(default_config.sms[1]) },
	{ "\"sms2_script_value**************\"",	0, VALUE_TYPE_STRING,  offsetof(BLOB_RECORD, script[1]), sizeof(default_config.script[1]) },
	{ "\"sms3_text_value\"",					0, VALUE_TYPE_STRING,  offsetof(BLOB_RECORD, sms[2]), sizeof(default_config.sms[2]) },
	{ "\"sms3_script_value**************\"",	0, VALUE_TYPE_STRING,  offsetof(BLOB_RECORD, script[2]), sizeof(default_config.script[2]) },
	{ "\"sms4_text_value\"",					0, VALUE_TYPE_STRING,  offsetof(BLOB_RECORD, sms[3]), sizeof(default_config.sms[3]) },
	{ "\"sms4_script_value**************\"",	0, VALUE_TYPE_STRING,  offsetof(BLOB_RECORD, script[3]), sizeof(default_config.script[3]) },
	{ "\"phone_value********************\"",	0, VALUE_TYPE_STRING,  offsetof(BLOB_RECORD, Phone), sizeof(default_config.Phone) },
	{ "\"audio_expanded\"",						0, VALUE_TYPE_BOOL,    offsetof(BLOB_RECORD, audio_expanded), sizeof(default_config.audio_expanded) },
	{ "\"volume_value\"",						0, VALUE_TYPE_NUMERIC, offsetof(BLOB_RECORD, nCLVL), sizeof(default_config.nCLVL) },
	{ "\"mic_value\"",							0, VALUE_TYPE_NUMERIC, offsetof(BLOB_RECORD, nCMIC), sizeof(default_config.nCMIC) },
	{ "\"es_value\"",							0, VALUE_TYPE_NUMERIC, offsetof(BLOB_RECORD, nES), sizeof(default_config.nES) },
	{ "\"ses_value\"",							0, VALUE_TYPE_NUMERIC, offsetof(BLOB_RECORD, nSES), sizeof(default_config.nSES) },
	{ "\"autoanswer_checked\"",					0, VALUE_TYPE_BOOL,    offsetof(BLOB_RECORD, bAutoAnswer), sizeof(default_config.bAutoAnswer) },
	{ "\"altitude_checked\"",					0, VALUE_TYPE_BOOL,    offsetof(BLOB_RECORD, altitude), sizeof(default_config.altitude) },
	{ "\"cog_checked\"",						0, VALUE_TYPE_BOOL,    offsetof(BLOB_RECORD, cog), sizeof(default_config.cog) },
	{ "\"rpm_checked\"",						0, VALUE_TYPE_BOOL,    offsetof(BLOB_RECORD, rpm), sizeof(default_config.rpm) }
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

	BLOB_RECORD *config = (BLOB_RECORD *)object->module_data;

	if ((config == NULL)||(object->module_data_size != sizeof(BLOB_RECORD))) {
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

	BLOB_RECORD *config = (BLOB_RECORD *)object->module_data;
	if ((config == NULL)||(object->module_data_size != sizeof(BLOB_RECORD)))
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

	BLOB_RECORD *config = (BLOB_RECORD *)object->module_data;
	if ((config == NULL)||(object->module_data_size != sizeof(BLOB_RECORD)))
		config = &default_config;

	*caps = DEVICE_CAPS_HARDWARE_SETTINGS | DEVICE_CAPS_IGNITION | DEVICE_CAPS_ENGINE | DEVICE_CAPS_MOVE | DEVICE_CAPS_DISCRETE_OUTPUT1;

	if (config->altitude)
		*caps |= DEVICE_CAPS_NAV_ALTITUDE;

	if (config->cog)
		*caps |= DEVICE_CAPS_NAV_COG;

	if (config->rpm)
		*caps |= DEVICE_CAPS_ECU_RPM;

	if (config->input1_expanded) {
		switch (config->Input1Mode) {
		case 3:
			*caps |= DEVICE_CAPS_DISCRETE_INPUT1;
			break;
		case 4:
			*caps |= DEVICE_CAPS_COUNTER_INPUT1;
			break;
		}
	}

	if (config->input2_expanded) {
		switch (config->Input2Mode) {
		case 3:
			*caps |= DEVICE_CAPS_DISCRETE_INPUT2;
			break;
		case 4:
			*caps |= DEVICE_CAPS_COUNTER_INPUT2;
			break;
		case 5:
			*caps |= DEVICE_CAPS_FREQUENCY_INPUT2;
			break;
		}
	}

	if (config->input3_expanded) {
		switch (config->Input3Mode) {
		case 3:
			*caps |= DEVICE_CAPS_DISCRETE_INPUT3;
			break;
		case 4:
			*caps |= DEVICE_CAPS_COUNTER_INPUT3;
			break;
		case 5:
			*caps |= DEVICE_CAPS_FREQUENCY_INPUT3;
			break;
		}
	}

	if (config->input4_expanded) {
		switch (config->Input4Mode) {
		case 3:
			*caps |= DEVICE_CAPS_DISCRETE_INPUT4;
			break;
		case 4:
			*caps |= DEVICE_CAPS_COUNTER_INPUT4;
			break;
		case 5:
			*caps |= DEVICE_CAPS_FREQUENCY_INPUT4;
			break;
		case 7:
		case 8:
		case 9:
			*caps |= DEVICE_CAPS_INJECTOR;
			break;
		case 12:
			*caps |= DEVICE_CAPS_FREQUENCY_INPUT4;
			break;		
		}
	}

	if (config->adc1_expanded) {
		switch (config->ADC1Mode) {
		case 3:
			*caps |= DEVICE_CAPS_ANALOG_INPUT1;
			break;
		case 4:
			*caps |= DEVICE_CAPS_DISCRETE_INPUT5;
			break;
		}
	}

	if (config->adc2_expanded) {
		switch (config->ADC2Mode) {
		case 3:
			*caps |= DEVICE_CAPS_ANALOG_INPUT2;
			break;
		case 4:
			*caps |= DEVICE_CAPS_DISCRETE_INPUT6;
			break;
		case 5:
			*caps |= DEVICE_CAPS_VCC;
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
	
	default_config.MoveDetectMethod = AK306_MOVE_BY_IGNITION;
	default_config.OdometerMoveTimeout = 30;
	default_config.OdometerTicksCount = 5;
		
	default_config.DriveInterval = 15;
	default_config.DrivePPP = 4;

	default_config.wait_expanded = true;
	default_config.SleepDelayInterval = 1;
	default_config.SleepWakeupInterval = 30;

	default_config.sleep_expanded = true;
	default_config.SleepDeepDelayInterval = 60;
	default_config.SleepDeepMaxGPSSearchTime = 5;
	default_config.SleepDeepMaxGSMSearchTime = 5;
	default_config.SleepDeepWakeupInterval = 3;

	default_config.input1_expanded = true;
	default_config.Input1Mode = AK306_INPUT_MODE_DISCRETE;
	default_config.Input1ActiveInterval = 100;
	default_config.Input1InactiveInterval = 100;
	default_config.Input1SMSOnActive[0] = '\0';
	default_config.Input1SMSOnInactive[0] = '\0';

	default_config.input2_expanded = false;
	default_config.Input2Mode = AK306_INPUT_MODE_UNCONNECTED;
	default_config.Input2ActiveInterval = 10;
	default_config.Input2InactiveInterval = 10;
	default_config.Input2SMSOnActive[0] = '\0';
	default_config.Input2SMSOnInactive[0] = '\0';

	default_config.input3_expanded = false;
	default_config.Input3Mode = AK306_INPUT_MODE_UNCONNECTED;
	default_config.Input3ActiveInterval = 10;
	default_config.Input3InactiveInterval = 10;
	default_config.Input3SMSOnActive[0] = '\0';
	default_config.Input3SMSOnInactive[0] = '\0';

	default_config.input4_expanded = false;
	default_config.Input4Mode = AK306_INPUT_MODE_UNCONNECTED;
	default_config.Input4ActiveInterval = 10;
	default_config.Input4InactiveInterval = 10;
	default_config.Input4SMSOnActive[0] = '\0';
	default_config.Input4SMSOnInactive[0] = '\0';

	default_config.adc1_expanded = false;
	default_config.ADC1Mode = AK306_ADC_MODE_UNCONNECTED;
	default_config.ADC1_Threshold = 10;

	default_config.adc2_expanded = false;
	default_config.ADC2Mode = AK306_ADC_MODE_UNCONNECTED;
	default_config.ADC2_Threshold = 10;
	
	strcpy(default_config.sms[0], "Start");
	strcpy(default_config.script[0], "A");
	strcpy(default_config.sms[1], "Stop");
	strcpy(default_config.script[1], "N");

	default_config.sms[2][0] = '\0';
	default_config.script[2][0] = '\0';
	default_config.sms[3][0] = '\0';
	default_config.script[3][0] = '\0';

	default_config.audio_expanded = false;
	default_config.nCMIC = 12;
	default_config.nCLVL = 85;
	default_config.nES = 7;
	default_config.nSES = 5;
	default_config.bAutoAnswer = 0;

	default_config.cog = false;
	default_config.altitude = false;
	default_config.rpm = false;
	default_config.requested_fw_ver = 0;
	default_config.actual_fw_ver = 0;
	default_config.need_profile = true;


	default_config.timestamp = time(NULL);
	memset(default_config.info, 0, sizeof(default_config.info));

	json_init();

	return 0;
}

int config_destroy()
{
	json_destroy();
	return 0;
}
