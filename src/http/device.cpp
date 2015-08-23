//******************************************************************************
//
// File Name : terminal.cpp
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#include <stdio.h>
#include <limits.h>
#include <string.h>
#include "api.h"
#include "rest.h"
#include "device.h"
#include "response.h"
#include "../core/jparse.h"
#include "json.h"
#include "object.h"

#define DEVICE_CAPS_COUNT				57
#define DEVICE_PUT_VALIDATORS_COUNT		5
#define DEVICE_POST_VALIDATORS_COUNT	5

static JKEY_VALIDATOR device_put_validators[DEVICE_PUT_VALIDATORS_COUNT] = {
	{ "type",					4,	JPARSE_VALUE_TYPE_NUMBER,	true,	OBJECT_TERMINAL_MIN,	USHRT_MAX},
	{ "dev_id",					6,	JPARSE_VALUE_TYPE_STRING,	true,	1,	15},
	{ "name",					4,	JPARSE_VALUE_TYPE_STRING,	true,	1,	256},
	{ "phone",					5,	JPARSE_VALUE_TYPE_STRING,	true,	1,	33},
	{ "custom",					6,	JPARSE_VALUE_TYPE_STRING,	false,	0,	4096}
};

static JKEY_VALIDATOR device_post_validators[DEVICE_POST_VALIDATORS_COUNT] = {
	{ "type",					4,	JPARSE_VALUE_TYPE_NUMBER,	true,	OBJECT_TERMINAL_MIN,	USHRT_MAX},
	{ "dev_id",					6,	JPARSE_VALUE_TYPE_STRING,	true,	1,	15},
	{ "name",					4,	JPARSE_VALUE_TYPE_STRING,	true,	1,	256},
	{ "phone",					5,	JPARSE_VALUE_TYPE_STRING,	true,	1,	33},
	{ "custom",					6,	JPARSE_VALUE_TYPE_STRING,	false,	0,	4096}
};

static JKEY key_caps[DEVICE_CAPS_COUNT] = {
	{"analog1",		0, JPARSE_VALUE_TYPE_TRUE, {}, 0, NULL, NULL },
	{"analog2",		0, JPARSE_VALUE_TYPE_TRUE, {}, 0, NULL, NULL },
	{"analog3",		0, JPARSE_VALUE_TYPE_TRUE, {}, 0, NULL, NULL },
	{"analog4",		0, JPARSE_VALUE_TYPE_TRUE, {}, 0, NULL, NULL },
	{"analog5",		0, JPARSE_VALUE_TYPE_TRUE, {}, 0, NULL, NULL },
	{"analog6",		0, JPARSE_VALUE_TYPE_TRUE, {}, 0, NULL, NULL },
	{"analog7",		0, JPARSE_VALUE_TYPE_TRUE, {}, 0, NULL, NULL },
	{"analog8",		0, JPARSE_VALUE_TYPE_TRUE, {}, 0, NULL, NULL },
	{"discrete1",	0, JPARSE_VALUE_TYPE_TRUE, {}, 0, NULL, NULL },
	{"discrete2",	0, JPARSE_VALUE_TYPE_TRUE, {}, 0, NULL, NULL },
	{"discrete3",	0, JPARSE_VALUE_TYPE_TRUE, {}, 0, NULL, NULL },
	{"discrete4",	0, JPARSE_VALUE_TYPE_TRUE, {}, 0, NULL, NULL },
	{"discrete5",	0, JPARSE_VALUE_TYPE_TRUE, {}, 0, NULL, NULL },
	{"discrete6",	0, JPARSE_VALUE_TYPE_TRUE, {}, 0, NULL, NULL },
	{"discrete7",	0, JPARSE_VALUE_TYPE_TRUE, {}, 0, NULL, NULL },
	{"discrete8",	0, JPARSE_VALUE_TYPE_TRUE, {}, 0, NULL, NULL },
	{"counter1",	0, JPARSE_VALUE_TYPE_TRUE, {}, 0, NULL, NULL },
	{"counter2",	0, JPARSE_VALUE_TYPE_TRUE, {}, 0, NULL, NULL },
	{"counter3",	0, JPARSE_VALUE_TYPE_TRUE, {}, 0, NULL, NULL },
	{"counter4",	0, JPARSE_VALUE_TYPE_TRUE, {}, 0, NULL, NULL },
	{"counter5",	0, JPARSE_VALUE_TYPE_TRUE, {}, 0, NULL, NULL },
	{"counter6",	0, JPARSE_VALUE_TYPE_TRUE, {}, 0, NULL, NULL },
	{"counter7",	0, JPARSE_VALUE_TYPE_TRUE, {}, 0, NULL, NULL },
	{"counter8",	0, JPARSE_VALUE_TYPE_TRUE, {}, 0, NULL, NULL },
	{"frequency1",	0, JPARSE_VALUE_TYPE_TRUE, {}, 0, NULL, NULL },
	{"frequency2",	0, JPARSE_VALUE_TYPE_TRUE, {}, 0, NULL, NULL },
	{"frequency3",	0, JPARSE_VALUE_TYPE_TRUE, {}, 0, NULL, NULL },
	{"frequency4",	0, JPARSE_VALUE_TYPE_TRUE, {}, 0, NULL, NULL },
	{"frequency5",	0, JPARSE_VALUE_TYPE_TRUE, {}, 0, NULL, NULL },
	{"frequency6",	0, JPARSE_VALUE_TYPE_TRUE, {}, 0, NULL, NULL },
	{"frequency7",	0, JPARSE_VALUE_TYPE_TRUE, {}, 0, NULL, NULL },
	{"frequency8",	0, JPARSE_VALUE_TYPE_TRUE, {}, 0, NULL, NULL },
	{"output1",		0, JPARSE_VALUE_TYPE_TRUE, {}, 0, NULL, NULL },
	{"output2",		0, JPARSE_VALUE_TYPE_TRUE, {}, 0, NULL, NULL },
	{"output3",		0, JPARSE_VALUE_TYPE_TRUE, {}, 0, NULL, NULL },
	{"output4",		0, JPARSE_VALUE_TYPE_TRUE, {}, 0, NULL, NULL },
	{"output5",		0, JPARSE_VALUE_TYPE_TRUE, {}, 0, NULL, NULL },
	{"output6",		0, JPARSE_VALUE_TYPE_TRUE, {}, 0, NULL, NULL },
	{"output7",		0, JPARSE_VALUE_TYPE_TRUE, {}, 0, NULL, NULL },
	{"output8",		0, JPARSE_VALUE_TYPE_TRUE, {}, 0, NULL, NULL },
	{"cog",			0, JPARSE_VALUE_TYPE_TRUE, {}, 0, NULL, NULL },
	{"fix3d",		0, JPARSE_VALUE_TYPE_TRUE, {}, 0, NULL, NULL },
	{"sat_count",	0, JPARSE_VALUE_TYPE_TRUE, {}, 0, NULL, NULL },
	{"altitude",	0, JPARSE_VALUE_TYPE_TRUE, {}, 0, NULL, NULL },
	{"rpm",			0, JPARSE_VALUE_TYPE_TRUE, {}, 0, NULL, NULL },
	{"ignition",	0, JPARSE_VALUE_TYPE_TRUE, {}, 0, NULL, NULL },
	{"engine",		0, JPARSE_VALUE_TYPE_TRUE, {}, 0, NULL, NULL },
	{"move",		0, JPARSE_VALUE_TYPE_TRUE, {}, 0, NULL, NULL },
	{"alarm",		0, JPARSE_VALUE_TYPE_TRUE, {}, 0, NULL, NULL },
	{"rs485",		0, JPARSE_VALUE_TYPE_TRUE, {}, 0, NULL, NULL },
	{"rs232_1",		0, JPARSE_VALUE_TYPE_TRUE, {}, 0, NULL, NULL },
	{"rs232_2",		0, JPARSE_VALUE_TYPE_TRUE, {}, 0, NULL, NULL },
	{"voice",		0, JPARSE_VALUE_TYPE_TRUE, {}, 0, NULL, NULL },
	{"vcc",			0, JPARSE_VALUE_TYPE_TRUE, {}, 0, NULL, NULL },
	{"injector",	0, JPARSE_VALUE_TYPE_TRUE, {}, 0, NULL, NULL },
	{"odometer",	0, JPARSE_VALUE_TYPE_TRUE, {}, 0, NULL, NULL },
	{"hwsettings",	0, JPARSE_VALUE_TYPE_TRUE, {}, 0, NULL, NULL }
};
	
#define DEVICE_SENSOR_VALIDATORS_COUNT	85

static JKEY_VALIDATOR device_sensor_validators[DEVICE_SENSOR_VALIDATORS_COUNT] = {
	{ "discrete1_name",			14,	JPARSE_VALUE_TYPE_STRING,	false,	0,	256},
	{ "discrete1_type",			14,	JPARSE_VALUE_TYPE_NUMBER,	false,	0,	0},
	{ "discrete2_name",			14,	JPARSE_VALUE_TYPE_STRING,	false,	0,	256},
	{ "discrete2_type",			14,	JPARSE_VALUE_TYPE_NUMBER,	false,	0,	0},
	{ "discrete3_name",			14,	JPARSE_VALUE_TYPE_STRING,	false,	0,	256},
	{ "discrete3_type",			14,	JPARSE_VALUE_TYPE_NUMBER,	false,	0,	0},
	{ "discrete4_name",			14,	JPARSE_VALUE_TYPE_STRING,	false,	0,	256},
	{ "discrete4_type",			14,	JPARSE_VALUE_TYPE_NUMBER,	false,	0,	0},
	{ "discrete5_name",			14,	JPARSE_VALUE_TYPE_STRING,	false,	0,	256},
	{ "discrete5_type",			14,	JPARSE_VALUE_TYPE_NUMBER,	false,	0,	0},
	{ "discrete6_name",			14,	JPARSE_VALUE_TYPE_STRING,	false,	0,	256},
	{ "discrete6_type",			14,	JPARSE_VALUE_TYPE_NUMBER,	false,	0,	0},
	{ "discrete7_name",			14,	JPARSE_VALUE_TYPE_STRING,	false,	0,	256},
	{ "discrete7_type",			14,	JPARSE_VALUE_TYPE_NUMBER,	false,	0,	0},
	{ "discrete8_name",			14,	JPARSE_VALUE_TYPE_STRING,	false,	0,	256},
	{ "discrete8_type",			14,	JPARSE_VALUE_TYPE_NUMBER,	false,	0,	0},
	{ "frequency1_name",		15,	JPARSE_VALUE_TYPE_STRING,	false,	0,	256},
	{ "frequency1_type",		15,	JPARSE_VALUE_TYPE_NUMBER,	false,	0,	0},
	{ "frequency1_factor",		17,	JPARSE_VALUE_TYPE_NUMBER,	false,	1,	0},
	{ "frequency2_name",		15,	JPARSE_VALUE_TYPE_STRING,	false,	0,	256},
	{ "frequency2_type",		15,	JPARSE_VALUE_TYPE_NUMBER,	false,	0,	0},
	{ "frequency2_factor",		17,	JPARSE_VALUE_TYPE_NUMBER,	false,	1,	0},
	{ "frequency3_name",		15,	JPARSE_VALUE_TYPE_STRING,	false,	0,	256},
	{ "frequency3_type",		15,	JPARSE_VALUE_TYPE_NUMBER,	false,	0,	0},
	{ "frequency3_factor",		17,	JPARSE_VALUE_TYPE_NUMBER,	false,	1,	0},
	{ "frequency4_name",		15,	JPARSE_VALUE_TYPE_STRING,	false,	0,	256},
	{ "frequency4_type",		15,	JPARSE_VALUE_TYPE_NUMBER,	false,	0,	0},
	{ "frequency4_factor",		17,	JPARSE_VALUE_TYPE_NUMBER,	false,	1,	0},
	{ "frequency5_name",		15,	JPARSE_VALUE_TYPE_STRING,	false,	0,	256},
	{ "frequency5_type",		15,	JPARSE_VALUE_TYPE_NUMBER,	false,	0,	0},
	{ "frequency5_factor",		17,	JPARSE_VALUE_TYPE_NUMBER,	false,	1,	0},
	{ "frequency6_name",		15,	JPARSE_VALUE_TYPE_STRING,	false,	0,	256},
	{ "frequency6_type",		15,	JPARSE_VALUE_TYPE_NUMBER,	false,	0,	0},
	{ "frequency6_factor",		17,	JPARSE_VALUE_TYPE_NUMBER,	false,	1,	0},
	{ "frequency7_name",		15,	JPARSE_VALUE_TYPE_STRING,	false,	0,	256},
	{ "frequency7_type",		15,	JPARSE_VALUE_TYPE_NUMBER,	false,	0,	0},
	{ "frequency7_factor",		17,	JPARSE_VALUE_TYPE_NUMBER,	false,	1,	0},
	{ "frequency8_name",		15,	JPARSE_VALUE_TYPE_STRING,	false,	0,	256},
	{ "frequency8_type",		15,	JPARSE_VALUE_TYPE_NUMBER,	false,	0,	0},
	{ "frequency8_factor",		17,	JPARSE_VALUE_TYPE_NUMBER,	false,	1,	0},
	{ "counter1_name",			13,	JPARSE_VALUE_TYPE_STRING,	false,	0,	256},
	{ "counter1_type",			13,	JPARSE_VALUE_TYPE_NUMBER,	false,	0,	0},
	{ "counter1_factor",		15,	JPARSE_VALUE_TYPE_NUMBER,	false,	1,	0},
	{ "counter2_name",			13,	JPARSE_VALUE_TYPE_STRING,	false,	0,	256},
	{ "counter2_type",			13,	JPARSE_VALUE_TYPE_NUMBER,	false,	0,	0},
	{ "counter2_factor",		15,	JPARSE_VALUE_TYPE_NUMBER,	false,	1,	0},
	{ "counter3_name",			13,	JPARSE_VALUE_TYPE_STRING,	false,	0,	256},
	{ "counter3_type",			13,	JPARSE_VALUE_TYPE_NUMBER,	false,	0,	0},
	{ "counter3_factor",		15,	JPARSE_VALUE_TYPE_NUMBER,	false,	1,	0},
	{ "counter4_name",			13,	JPARSE_VALUE_TYPE_STRING,	false,	0,	256},
	{ "counter4_type",			13,	JPARSE_VALUE_TYPE_NUMBER,	false,	0,	0},
	{ "counter4_factor",		15,	JPARSE_VALUE_TYPE_NUMBER,	false,	1,	0},
	{ "counter5_name",			13,	JPARSE_VALUE_TYPE_STRING,	false,	0,	256},
	{ "counter5_type",			13,	JPARSE_VALUE_TYPE_NUMBER,	false,	0,	0},
	{ "counter5_factor",		15,	JPARSE_VALUE_TYPE_NUMBER,	false,	1,	0},
	{ "counter6_name",			13,	JPARSE_VALUE_TYPE_STRING,	false,	0,	256},
	{ "counter6_type",			13,	JPARSE_VALUE_TYPE_NUMBER,	false,	0,	0},
	{ "counter6_factor",		15,	JPARSE_VALUE_TYPE_NUMBER,	false,	1,	0},
	{ "counter7_name",			13,	JPARSE_VALUE_TYPE_STRING,	false,	0,	256},
	{ "counter7_type",			13,	JPARSE_VALUE_TYPE_NUMBER,	false,	0,	0},
	{ "counter7_factor",		15,	JPARSE_VALUE_TYPE_NUMBER,	false,	1,	0},
	{ "counter8_name",			13,	JPARSE_VALUE_TYPE_STRING,	false,	0,	256},
	{ "counter8_type",			13,	JPARSE_VALUE_TYPE_NUMBER,	false,	0,	0},
	{ "counter8_factor",		15,	JPARSE_VALUE_TYPE_NUMBER,	false,	1,	0},
	{ "analog1_name",			12,	JPARSE_VALUE_TYPE_STRING,	false,	0,	256},
	{ "analog1_type",			12,	JPARSE_VALUE_TYPE_NUMBER,	false,	0,	0},
	{ "analog2_name",			12,	JPARSE_VALUE_TYPE_STRING,	false,	0,	256},
	{ "analog2_type",			12,	JPARSE_VALUE_TYPE_NUMBER,	false,	0,	0},
	{ "analog3_name",			12,	JPARSE_VALUE_TYPE_STRING,	false,	0,	256},
	{ "analog3_type",			12,	JPARSE_VALUE_TYPE_NUMBER,	false,	0,	0},
	{ "analog4_name",			12,	JPARSE_VALUE_TYPE_STRING,	false,	0,	256},
	{ "analog4_type",			12,	JPARSE_VALUE_TYPE_NUMBER,	false,	0,	0},
	{ "analog5_name",			12,	JPARSE_VALUE_TYPE_STRING,	false,	0,	256},
	{ "analog5_type",			12,	JPARSE_VALUE_TYPE_NUMBER,	false,	0,	0},
	{ "analog6_name",			12,	JPARSE_VALUE_TYPE_STRING,	false,	0,	256},
	{ "analog6_type",			12,	JPARSE_VALUE_TYPE_NUMBER,	false,	0,	0},
	{ "analog7_name",			12,	JPARSE_VALUE_TYPE_STRING,	false,	0,	256},
	{ "analog7_type",			12,	JPARSE_VALUE_TYPE_NUMBER,	false,	0,	0},
	{ "analog8_name",			12,	JPARSE_VALUE_TYPE_STRING,	false,	0,	256},
	{ "analog8_type",			12,	JPARSE_VALUE_TYPE_NUMBER,	false,	0,	0},
	{ "injector_name",			13,	JPARSE_VALUE_TYPE_STRING,	false,	0,	256},
	{ "injector_factor",		15,	JPARSE_VALUE_TYPE_NUMBER,	false,	1,	0},
	{ "rs485_lls_left",			14,	JPARSE_VALUE_TYPE_BOOLEAN,	false,	1,	0},
	{ "rs485_lls_right",		15,	JPARSE_VALUE_TYPE_BOOLEAN,	false,	1,	0},
	{ "rs485_lls_swap",			14,	JPARSE_VALUE_TYPE_BOOLEAN,	false,	1,	0}
};

#define DEVICE_LLS_VALIDATORS_COUNT	2

static JKEY_VALIDATOR device_lls_validators[DEVICE_LLS_VALIDATORS_COUNT] = {
	{ "ignition_power",			14,	JPARSE_VALUE_TYPE_BOOLEAN,	true,	0,	0},
	{ "table",					5,	JPARSE_VALUE_TYPE_ARRAY,	true,	0,	0}
};

#define DEVICE_FUEL_VALIDATORS_COUNT	4

static JKEY_VALIDATOR device_fuel_validators[DEVICE_FUEL_VALIDATORS_COUNT] = {
	{ "filter_level",			12,	JPARSE_VALUE_TYPE_NUMBER,	true,	0,	3},
	{ "fill_threshold",			14,	JPARSE_VALUE_TYPE_NUMBER,	true,	0,	0},
	{ "drain_threshold",		15,	JPARSE_VALUE_TYPE_NUMBER,	true,	0,	0},
	{ "max_consumption",		15,	JPARSE_VALUE_TYPE_NUMBER,	true,	0,	0}
};

#define DEVICE_RETRANSLATOR_VALIDATORS_COUNT	6

static JKEY_VALIDATOR device_retranslator_validators[DEVICE_RETRANSLATOR_VALIDATORS_COUNT] = {
	{ "retranslator",	12,	JPARSE_VALUE_TYPE_NUMBER,	true,	0,	65535},
	{ "host",			4,	JPARSE_VALUE_TYPE_STRING,	true,	1,	256},
	{ "port",			4,	JPARSE_VALUE_TYPE_NUMBER,	true,	1,	65535},
	{ "login",			5,	JPARSE_VALUE_TYPE_STRING,	true,	0,	256},
	{ "password",		8,	JPARSE_VALUE_TYPE_STRING,	true,	0,	256},
	{ "id",				2,	JPARSE_VALUE_TYPE_STRING,	true,	1,	256}
};

#define DEVICE_POST_MODIFY_VALIDATORS_COUNT	3

static JKEY_VALIDATOR device_post_modify_validators[DEVICE_POST_MODIFY_VALIDATORS_COUNT] = {
	{ "time",					4,	JPARSE_VALUE_TYPE_NUMBER,	true,	0,	0},
	{ "field",					5,	JPARSE_VALUE_TYPE_STRING,	true,	1,	32},
	{ "value",					5,	JPARSE_VALUE_TYPE_NUMBER,	true,	0,	0}
};

static JKEY caps_key =	{ "caps", 4, JPARSE_VALUE_TYPE_OBJECT, {}, 0, NULL, NULL };
static JKEY id_key =	{ "id", 2, JPARSE_VALUE_TYPE_NUMBER, {}, 0, NULL, NULL};
static JKEY fuel_key =	{ "fuel", 4, JPARSE_VALUE_TYPE_OBJECT, {}, 0, NULL, NULL };

///////////////////////////////////////////////////////////////////////////////

static unsigned char *ptr;
static bool first;
static unsigned int id;
static unsigned char buffer[4096];

///////////////////////////////////////////////////////////////////////////////

JKEY *device_construct_caps(unsigned long long caps)
{
	key_caps[0].value_type = (caps & DEVICE_CAPS_ANALOG_INPUT1) ? JPARSE_VALUE_TYPE_TRUE : JPARSE_VALUE_TYPE_FALSE;
	key_caps[1].value_type = (caps & DEVICE_CAPS_ANALOG_INPUT2) ? JPARSE_VALUE_TYPE_TRUE : JPARSE_VALUE_TYPE_FALSE;
	key_caps[2].value_type = (caps & DEVICE_CAPS_ANALOG_INPUT3) ? JPARSE_VALUE_TYPE_TRUE : JPARSE_VALUE_TYPE_FALSE;
	key_caps[3].value_type = (caps & DEVICE_CAPS_ANALOG_INPUT4) ? JPARSE_VALUE_TYPE_TRUE : JPARSE_VALUE_TYPE_FALSE;
	key_caps[4].value_type = (caps & DEVICE_CAPS_ANALOG_INPUT5) ? JPARSE_VALUE_TYPE_TRUE : JPARSE_VALUE_TYPE_FALSE;
	key_caps[5].value_type = (caps & DEVICE_CAPS_ANALOG_INPUT6) ? JPARSE_VALUE_TYPE_TRUE : JPARSE_VALUE_TYPE_FALSE;
	key_caps[6].value_type = (caps & DEVICE_CAPS_ANALOG_INPUT7) ? JPARSE_VALUE_TYPE_TRUE : JPARSE_VALUE_TYPE_FALSE;
	key_caps[7].value_type = (caps & DEVICE_CAPS_ANALOG_INPUT8) ? JPARSE_VALUE_TYPE_TRUE : JPARSE_VALUE_TYPE_FALSE;
	key_caps[8].value_type = (caps & DEVICE_CAPS_DISCRETE_INPUT1) ? JPARSE_VALUE_TYPE_TRUE : JPARSE_VALUE_TYPE_FALSE;
	key_caps[9].value_type = (caps & DEVICE_CAPS_DISCRETE_INPUT2) ? JPARSE_VALUE_TYPE_TRUE : JPARSE_VALUE_TYPE_FALSE;
	key_caps[10].value_type = (caps & DEVICE_CAPS_DISCRETE_INPUT3) ? JPARSE_VALUE_TYPE_TRUE : JPARSE_VALUE_TYPE_FALSE;
	key_caps[11].value_type = (caps & DEVICE_CAPS_DISCRETE_INPUT4) ? JPARSE_VALUE_TYPE_TRUE : JPARSE_VALUE_TYPE_FALSE;
	key_caps[12].value_type = (caps & DEVICE_CAPS_DISCRETE_INPUT5) ? JPARSE_VALUE_TYPE_TRUE : JPARSE_VALUE_TYPE_FALSE;
	key_caps[13].value_type = (caps & DEVICE_CAPS_DISCRETE_INPUT6) ? JPARSE_VALUE_TYPE_TRUE : JPARSE_VALUE_TYPE_FALSE;
	key_caps[14].value_type = (caps & DEVICE_CAPS_DISCRETE_INPUT7) ? JPARSE_VALUE_TYPE_TRUE : JPARSE_VALUE_TYPE_FALSE;
	key_caps[15].value_type = (caps & DEVICE_CAPS_DISCRETE_INPUT8) ? JPARSE_VALUE_TYPE_TRUE : JPARSE_VALUE_TYPE_FALSE;
	key_caps[16].value_type = (caps & DEVICE_CAPS_COUNTER_INPUT1) ? JPARSE_VALUE_TYPE_TRUE : JPARSE_VALUE_TYPE_FALSE;
	key_caps[17].value_type = (caps & DEVICE_CAPS_COUNTER_INPUT2) ? JPARSE_VALUE_TYPE_TRUE : JPARSE_VALUE_TYPE_FALSE;
	key_caps[18].value_type = (caps & DEVICE_CAPS_COUNTER_INPUT3) ? JPARSE_VALUE_TYPE_TRUE : JPARSE_VALUE_TYPE_FALSE;
	key_caps[19].value_type = (caps & DEVICE_CAPS_COUNTER_INPUT4) ? JPARSE_VALUE_TYPE_TRUE : JPARSE_VALUE_TYPE_FALSE;
	key_caps[20].value_type = (caps & DEVICE_CAPS_COUNTER_INPUT5) ? JPARSE_VALUE_TYPE_TRUE : JPARSE_VALUE_TYPE_FALSE;
	key_caps[21].value_type = (caps & DEVICE_CAPS_COUNTER_INPUT6) ? JPARSE_VALUE_TYPE_TRUE : JPARSE_VALUE_TYPE_FALSE;
	key_caps[22].value_type = (caps & DEVICE_CAPS_COUNTER_INPUT7) ? JPARSE_VALUE_TYPE_TRUE : JPARSE_VALUE_TYPE_FALSE;
	key_caps[23].value_type = (caps & DEVICE_CAPS_COUNTER_INPUT8) ? JPARSE_VALUE_TYPE_TRUE : JPARSE_VALUE_TYPE_FALSE;
	key_caps[24].value_type = (caps & DEVICE_CAPS_FREQUENCY_INPUT1) ? JPARSE_VALUE_TYPE_TRUE : JPARSE_VALUE_TYPE_FALSE;
	key_caps[25].value_type = (caps & DEVICE_CAPS_FREQUENCY_INPUT2) ? JPARSE_VALUE_TYPE_TRUE : JPARSE_VALUE_TYPE_FALSE;
	key_caps[26].value_type = (caps & DEVICE_CAPS_FREQUENCY_INPUT3) ? JPARSE_VALUE_TYPE_TRUE : JPARSE_VALUE_TYPE_FALSE;
	key_caps[27].value_type = (caps & DEVICE_CAPS_FREQUENCY_INPUT4) ? JPARSE_VALUE_TYPE_TRUE : JPARSE_VALUE_TYPE_FALSE;
	key_caps[28].value_type = (caps & DEVICE_CAPS_FREQUENCY_INPUT5) ? JPARSE_VALUE_TYPE_TRUE : JPARSE_VALUE_TYPE_FALSE;
	key_caps[29].value_type = (caps & DEVICE_CAPS_FREQUENCY_INPUT6) ? JPARSE_VALUE_TYPE_TRUE : JPARSE_VALUE_TYPE_FALSE;
	key_caps[30].value_type = (caps & DEVICE_CAPS_FREQUENCY_INPUT7) ? JPARSE_VALUE_TYPE_TRUE : JPARSE_VALUE_TYPE_FALSE;
	key_caps[31].value_type = (caps & DEVICE_CAPS_FREQUENCY_INPUT8) ? JPARSE_VALUE_TYPE_TRUE : JPARSE_VALUE_TYPE_FALSE;
	key_caps[32].value_type = (caps & DEVICE_CAPS_DISCRETE_OUTPUT1) ? JPARSE_VALUE_TYPE_TRUE : JPARSE_VALUE_TYPE_FALSE;
	key_caps[33].value_type = (caps & DEVICE_CAPS_DISCRETE_OUTPUT2) ? JPARSE_VALUE_TYPE_TRUE : JPARSE_VALUE_TYPE_FALSE;
	key_caps[34].value_type = (caps & DEVICE_CAPS_DISCRETE_OUTPUT3) ? JPARSE_VALUE_TYPE_TRUE : JPARSE_VALUE_TYPE_FALSE;
	key_caps[35].value_type = (caps & DEVICE_CAPS_DISCRETE_OUTPUT4) ? JPARSE_VALUE_TYPE_TRUE : JPARSE_VALUE_TYPE_FALSE;
	key_caps[36].value_type = (caps & DEVICE_CAPS_DISCRETE_OUTPUT5) ? JPARSE_VALUE_TYPE_TRUE : JPARSE_VALUE_TYPE_FALSE;
	key_caps[37].value_type = (caps & DEVICE_CAPS_DISCRETE_OUTPUT6) ? JPARSE_VALUE_TYPE_TRUE : JPARSE_VALUE_TYPE_FALSE;
	key_caps[38].value_type = (caps & DEVICE_CAPS_DISCRETE_OUTPUT7) ? JPARSE_VALUE_TYPE_TRUE : JPARSE_VALUE_TYPE_FALSE;
	key_caps[39].value_type = (caps & DEVICE_CAPS_DISCRETE_OUTPUT8) ? JPARSE_VALUE_TYPE_TRUE : JPARSE_VALUE_TYPE_FALSE;
	key_caps[40].value_type = (caps & DEVICE_CAPS_NAV_COG) ? JPARSE_VALUE_TYPE_TRUE : JPARSE_VALUE_TYPE_FALSE;
	key_caps[41].value_type = (caps & DEVICE_CAPS_NAV_3D) ? JPARSE_VALUE_TYPE_TRUE : JPARSE_VALUE_TYPE_FALSE;
	key_caps[42].value_type = (caps & DEVICE_CAPS_NAV_SAT_COUNT) ? JPARSE_VALUE_TYPE_TRUE : JPARSE_VALUE_TYPE_FALSE;
	key_caps[43].value_type = (caps & DEVICE_CAPS_NAV_ALTITUDE) ? JPARSE_VALUE_TYPE_TRUE : JPARSE_VALUE_TYPE_FALSE;
	key_caps[44].value_type = (caps & DEVICE_CAPS_ECU_RPM) ? JPARSE_VALUE_TYPE_TRUE : JPARSE_VALUE_TYPE_FALSE;
	key_caps[45].value_type = (caps & DEVICE_CAPS_IGNITION) ? JPARSE_VALUE_TYPE_TRUE : JPARSE_VALUE_TYPE_FALSE;
	key_caps[46].value_type = (caps & DEVICE_CAPS_ENGINE) ? JPARSE_VALUE_TYPE_TRUE : JPARSE_VALUE_TYPE_FALSE;
	key_caps[47].value_type = (caps & DEVICE_CAPS_MOVE) ? JPARSE_VALUE_TYPE_TRUE : JPARSE_VALUE_TYPE_FALSE;
	key_caps[48].value_type = (caps & DEVICE_CAPS_ALARM) ? JPARSE_VALUE_TYPE_TRUE : JPARSE_VALUE_TYPE_FALSE;
	key_caps[49].value_type = (caps & DEVICE_CAPS_RS485) ? JPARSE_VALUE_TYPE_TRUE : JPARSE_VALUE_TYPE_FALSE;
	key_caps[50].value_type = (caps & DEVICE_CAPS_RS232_1) ? JPARSE_VALUE_TYPE_TRUE : JPARSE_VALUE_TYPE_FALSE;
	key_caps[51].value_type = (caps & DEVICE_CAPS_RS232_2) ? JPARSE_VALUE_TYPE_TRUE : JPARSE_VALUE_TYPE_FALSE;
	key_caps[52].value_type = (caps & DEVICE_CAPS_VOICE) ? JPARSE_VALUE_TYPE_TRUE : JPARSE_VALUE_TYPE_FALSE;
	key_caps[53].value_type = (caps & DEVICE_CAPS_VCC) ? JPARSE_VALUE_TYPE_TRUE : JPARSE_VALUE_TYPE_FALSE;
	key_caps[54].value_type = (caps & DEVICE_CAPS_INJECTOR) ? JPARSE_VALUE_TYPE_TRUE : JPARSE_VALUE_TYPE_FALSE;
	key_caps[55].value_type = (caps & DEVICE_CAPS_ODOMETER) ? JPARSE_VALUE_TYPE_TRUE : JPARSE_VALUE_TYPE_FALSE;
	key_caps[56].value_type = (caps & DEVICE_CAPS_HARDWARE_SETTINGS) ? JPARSE_VALUE_TYPE_TRUE : JPARSE_VALUE_TYPE_FALSE;

	return &caps_key;
}

/////////////////////////////////////////////////////////////////////////////////////////

typedef struct enum_ctx
{
	size_t bytes_left;
	unsigned char *ptr;
	size_t module_no;
	unsigned char *dev_id;
	size_t id_len;
	DB_OBJECT *object;
	unsigned short type;
} ENUM_CTX;

static int unique_callback(DB_OBJECT *object, void *c)
{
	ENUM_CTX *ctx = (ENUM_CTX *)c;

	if ((object == ctx->object)||(object->type != ctx->type))
		return 0;

	JKEY key;
	if (jparse_extract_key((unsigned char *)"dev_id", 6, (unsigned char *)object->core_data, object->core_data_size, &key))
		return 0;

	if ((key.value_type == JPARSE_VALUE_TYPE_STRING)&&(key.str_len == ctx->id_len)&&(memcmp(key.value.str_val, ctx->dev_id, ctx->id_len) == 0))
		return 1;

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static int devices_callback(MODULE *module, void *c)
{
	const char *pName;
	const int  *pType;
	size_t len;
	size_t this_item_len;
	unsigned char *this_item_start;

	ENUM_CTX *ctx = (ENUM_CTX *)c;

	module->get_var(MODULE_VAR_TERMINAL_NAME, (void**)&pName);
	module->get_var(MODULE_VAR_TERMINAL_TYPE, (void**)&pType);

	len = strlen(pName);

 	             // " + { + "name" + : + " + len + " + , + "id" + : + 13 + }
	this_item_len = 1 + 1 + 6      + 1 + 1 + len + 1 + 1 + 4    + 1 + 13 + 1;

	if (this_item_len <= ctx->bytes_left) {

		this_item_start = ctx->ptr;

		ctx->module_no++;
		if (ctx->module_no > 1)
			*ctx->ptr++ = ',';

		*ctx->ptr++ = '{';

		ctx->ptr = json_add_string(ctx->ptr, "name", pName, len);

		*ctx->ptr++ = ',';

		ctx->ptr = json_add_uint(ctx->ptr, "id", *pType);

		*ctx->ptr++ = '}';

		ctx->bytes_left -= (ctx->ptr - this_item_start);

		return 0;
	}

	return -1;
}

int devices(HTTP_SESSION *s, unsigned int id, unsigned char **d, size_t *l)
{
	unsigned char *content_length_ptr;
	size_t content_length;
	ENUM_CTX ctx;

	unsigned char *ptr = response_success_object(*d, l, s->zero_init.keep_alive > 0, &content_length_ptr, &content_length);
	if (ptr == NULL)
		return 500;

	ctx.bytes_left = *l;

	if (ctx.bytes_left < 3)
		return 500;

	ctx.ptr = ptr;
	ctx.module_no = 0;

	if (ctx.bytes_left < 1)
		return 500;

	*ctx.ptr++ = '[';
	ctx.bytes_left--;

	if (api_enum_modules(MODULE_FAMILY_DEVICE, devices_callback, &ctx))
		return 500;

	if (ctx.bytes_left < 2)
		return 500;

	*ctx.ptr++ = ']';
	*ctx.ptr++ = '}';

	ctx.bytes_left -= 2;

	content_length += ctx.ptr - ptr;
    do {
        *--content_length_ptr = '0' + content_length % 10;
        content_length /= 10;
    } while (content_length != 0);

	*l = ctx.ptr - *d;

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int device_post_settings(HTTP_SESSION *s, unsigned int id, unsigned char **d, size_t *l)
{
	unsigned char *existing_key_buffer = *d;
	const size_t existing_key_buffer_size = *l / 2;
	unsigned char *new_key_buffer = existing_key_buffer + existing_key_buffer_size;
	const size_t new_key_buffer_size = *l / 2;

	DB_OBJECT *object;
	object = api_db_get_object(id);

	if ((object == NULL)||(object->type < OBJECT_TERMINAL_MIN))
		return 404;

	REST_USER *user = rest_find_user(s);
	if (user == NULL)
		return 401;

	if (!rest_check_access_to_object(user, object))
		return 401;

	MODULE *module = api_get_device_module(object->type);
	if (module == NULL)
		return 500;

	if (module->config_put_json(object, s->http_body, s->zero_init.body_len)) {
		size_t error_len;
		unsigned char *error = module->config_get_error(&error_len);

		response_fail_with_message(*d, l, error, error_len, s->zero_init.keep_alive > 0);

		return 0;
	}

	unsigned long long caps;
	module->config_get_device_caps(object, &caps);

	JKEY key;

	if (jparse_parse((unsigned char *)object->core_data, object->core_data_size, existing_key_buffer, existing_key_buffer_size, &key))
		return 500;

	if (key.value_type != JPARSE_VALUE_TYPE_OBJECT)
		return 500;

	if (jparse_set_key(&key, device_construct_caps(caps)))
		return 500;

	size_t json_buffer_size = new_key_buffer_size;
	if (jparse_build_json(&key, new_key_buffer, &json_buffer_size))
		return 500;

	unsigned char *new_core_data = (unsigned char *)realloc(object->core_data, json_buffer_size);

	if (new_core_data == NULL)
		return 500;

	object->core_data = new_core_data;
	object->core_data_size = json_buffer_size;

	memcpy(object->core_data, new_key_buffer, json_buffer_size);

	int res = api_db_update_object(object);

	if (res != DB_OK) {
		
		unsigned char *error = (unsigned char *)api_db_get_error();

		response_fail_with_message(*d, l, error, strlen((char *)error), s->zero_init.keep_alive > 0);

		return 0;
	}

	object_report(s, object, d, l);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int device_post_sensors(HTTP_SESSION *s, unsigned int id, unsigned char **d, size_t *l)
{
	#define KEY_SIZE (1024 * 1024)

	DB_OBJECT *object;
	object = api_db_get_object(id);

	if ((object == NULL)||(object->type < OBJECT_TERMINAL_MIN))
		return 404;

	REST_USER *user = rest_find_user(s);
	if (user == NULL)
		return 401;

	if (!rest_check_access_to_object(user, object))
		return 401;

	JKEY sensors_key;

	if (jparse_parse(s->http_body, s->zero_init.body_len, *d, *l / 2, &sensors_key)) {

		unsigned char *error = jparse_get_error(s->http_body, s->zero_init.body_len);

		response_fail_with_message(*d, l, error, strlen((char *)error), s->zero_init.keep_alive > 0);

		jparse_free_error(error);

		return 0;
	}

	if (sensors_key.value_type != JPARSE_VALUE_TYPE_OBJECT)
		return 500;

	if (jparse_validate_object(&sensors_key.value.object_val, device_sensor_validators, DEVICE_SENSOR_VALIDATORS_COUNT)) {

		unsigned char *error = jparse_get_error(s->http_body, s->zero_init.body_len);

		response_fail_with_message(*d, l, error, strlen((char *)error), s->zero_init.keep_alive > 0);

		jparse_free_error(error);

		return 0;
	}

	jparse_minify_object(&sensors_key.value.object_val, device_sensor_validators, DEVICE_SENSOR_VALIDATORS_COUNT);

	memcpy(sensors_key.key, "sensors", 7);
	sensors_key.key_len = 7;

	JKEY key;

	if (jparse_parse((unsigned char *)object->core_data, object->core_data_size, *d + KEY_SIZE, (*l - KEY_SIZE) / 2, &key))
		return 500;

	if (key.value_type != JPARSE_VALUE_TYPE_OBJECT)
		return 500;

	if (jparse_set_key(&key, &sensors_key))
		return 500;

	size_t json_buffer_size = *l / 2;
	if (jparse_build_json(&key, *d + KEY_SIZE + (*l - KEY_SIZE) / 2, &json_buffer_size))
		return 500;

	unsigned char *new_core_data = (unsigned char *)realloc(object->core_data, json_buffer_size);

	if (new_core_data == NULL)
		return 500;

	object->core_data = new_core_data;
	object->core_data_size = json_buffer_size;

	memcpy(object->core_data, (*d + KEY_SIZE + (*l - KEY_SIZE) / 2), json_buffer_size);

	int res = api_db_update_object(object);

	if (res != DB_OK) {
		
		unsigned char *error = (unsigned char *)api_db_get_error();

		response_fail_with_message(*d, l, error, strlen((char *)error), s->zero_init.keep_alive > 0);

		return 0;
	}

	object_report(s, object, d, l);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int device_get_settings(HTTP_SESSION *s, unsigned int _id, unsigned char **d, size_t *l)
{
	DB_OBJECT *object;
	object = api_db_get_object(_id);

	if ((object == NULL)||(object->type < OBJECT_TERMINAL_MIN))
		return 404;

	REST_USER *user = rest_find_user(s);
	if (user == NULL)
		return 401;

	if (!rest_check_access_to_object(user, object))
		return 401;

	MODULE *module = api_get_device_module(object->type);
	if (module == NULL)
		return 500;

	unsigned char *content_length_ptr;
	size_t content_length;

	ptr = response_success_object(*d, l, s->zero_init.keep_alive > 0, &content_length_ptr, &content_length);
	if (ptr == NULL)
		return 500;

	size_t json_len = *l - (ptr - *d);

	if (module->config_get_json(object, ptr, &json_len)) {

		size_t error_len;
		unsigned char *error = module->config_get_error(&error_len);

		response_fail_with_message(*d, l, error, error_len, s->zero_init.keep_alive > 0);

		return 0;
	}

	content_length += json_len + 1;

	ptr += json_len;

	*ptr++ = '}';

	// inlinded itoa
    do {
        *--content_length_ptr = '0' + content_length % 10;
        content_length /= 10;
    } while (content_length != 0);

	*l = ptr - *d;

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static int device_post_lls(HTTP_SESSION *s, unsigned int id, unsigned char **d, size_t *l, const char *key_name, size_t key_len)
{
	#define KEY_SIZE (1024 * 1024)

	DB_OBJECT *object;
	object = api_db_get_object(id);

	if ((object == NULL)||(object->type < OBJECT_TERMINAL_MIN))
		return 404;

	REST_USER *user = rest_find_user(s);
	if (user == NULL)
		return 401;

	if (!rest_check_access_to_object(user, object))
		return 401;

	JKEY sensors_key;

	if (jparse_parse(s->http_body, s->zero_init.body_len, *d, *l / 2, &sensors_key)) {

		unsigned char *error = jparse_get_error(s->http_body, s->zero_init.body_len);

		response_fail_with_message(*d, l, error, strlen((char *)error), s->zero_init.keep_alive > 0);

		jparse_free_error(error);

		return 0;
	}

	if (sensors_key.value_type != JPARSE_VALUE_TYPE_OBJECT)
		return 500;

	if (jparse_validate_object(&sensors_key.value.object_val, device_lls_validators, DEVICE_LLS_VALIDATORS_COUNT)) {

		unsigned char *error = jparse_get_error(s->http_body, s->zero_init.body_len);

		response_fail_with_message(*d, l, error, strlen((char *)error), s->zero_init.keep_alive > 0);

		jparse_free_error(error);

		return 0;
	}

	jparse_minify_object(&sensors_key.value.object_val, device_lls_validators, DEVICE_LLS_VALIDATORS_COUNT);

	memcpy(sensors_key.key, key_name, key_len);
	sensors_key.key_len = key_len;

	JKEY key;

	if (jparse_parse((unsigned char *)object->core_data, object->core_data_size, *d + KEY_SIZE, (*l - KEY_SIZE) / 2, &key))
		return 500;

	if (key.value_type != JPARSE_VALUE_TYPE_OBJECT)
		return 500;

	if (jparse_set_key(&key, &sensors_key))
		return 500;

	size_t json_buffer_size = *l / 2;
	if (jparse_build_json(&key, *d + KEY_SIZE + (*l - KEY_SIZE) / 2, &json_buffer_size))
		return 500;

	unsigned char *new_core_data = (unsigned char *)realloc(object->core_data, json_buffer_size);

	if (new_core_data == NULL)
		return 500;

	object->core_data = new_core_data;
	object->core_data_size = json_buffer_size;

	memcpy(object->core_data, *d + KEY_SIZE + (*l - KEY_SIZE) / 2, json_buffer_size);

	int res = api_db_update_object(object);

	if (res != DB_OK) {
		
		unsigned char *error = (unsigned char *)api_db_get_error();

		response_fail_with_message(*d, l, error, strlen((char *)error), s->zero_init.keep_alive > 0);

		return 0;
	}

	object_report(s, object, d, l);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int device_post_llsleft(HTTP_SESSION *s, unsigned int id, unsigned char **d, size_t *l)
{
	return device_post_lls(s, id, d, l, "llsleft", 7);
}

/////////////////////////////////////////////////////////////////////////////////////////

int device_post_llsright(HTTP_SESSION *s, unsigned int id, unsigned char **d, size_t *l)
{
	return device_post_lls(s, id, d, l, "llsright", 8);
}

/////////////////////////////////////////////////////////////////////////////////////////

int device_post_fuel(HTTP_SESSION *s, unsigned int id, unsigned char **d, size_t *l)
{
	#define KEY_SIZE (1024 * 1024)

	DB_OBJECT *object;
	object = api_db_get_object(id);

	if ((object == NULL)||(object->type < OBJECT_TERMINAL_MIN))
		return 404;

	REST_USER *user = rest_find_user(s);
	if (user == NULL)
		return 401;

	if (!rest_check_access_to_object(user, object))
		return 401;

	JKEY incoming_key;

	if (jparse_parse(s->http_body, s->zero_init.body_len, *d, *l / 2, &incoming_key)) {

		unsigned char *error = jparse_get_error(s->http_body, s->zero_init.body_len);

		response_fail_with_message(*d, l, error, strlen((char *)error), s->zero_init.keep_alive > 0);

		jparse_free_error(error);

		return 0;
	}

	if (incoming_key.value_type != JPARSE_VALUE_TYPE_OBJECT)
		return 500;

	if (jparse_validate_object(&incoming_key.value.object_val, device_fuel_validators, DEVICE_FUEL_VALIDATORS_COUNT)) {

		unsigned char *error = jparse_get_error(s->http_body, s->zero_init.body_len);

		response_fail_with_message(*d, l, error, strlen((char *)error), s->zero_init.keep_alive > 0);

		jparse_free_error(error);

		return 0;
	}

	jparse_minify_object(&incoming_key.value.object_val, device_fuel_validators, DEVICE_FUEL_VALIDATORS_COUNT);

	memcpy(incoming_key.key, "fuel", 4);
	incoming_key.key_len = 4;

	JKEY key;

	if (jparse_parse((unsigned char *)object->core_data, object->core_data_size, *d + KEY_SIZE, (*l - KEY_SIZE) / 2, &key))
		return 500;

	if (key.value_type != JPARSE_VALUE_TYPE_OBJECT)
		return 500;

	if (jparse_set_key(&key, &incoming_key))
		return 500;

	size_t json_buffer_size = *l / 2;
	if (jparse_build_json(&key, *d + KEY_SIZE + (*l - KEY_SIZE) / 2, &json_buffer_size))
		return 500;

	unsigned char *new_core_data = (unsigned char *)realloc(object->core_data, json_buffer_size);

	if (new_core_data == NULL)
		return 500;

	object->core_data = new_core_data;
	object->core_data_size = json_buffer_size;

	memcpy(object->core_data, *d + KEY_SIZE + (*l - KEY_SIZE) / 2, json_buffer_size);

	int res = api_db_update_object(object);

	if (res != DB_OK) {
		
		unsigned char *error = (unsigned char *)api_db_get_error();

		response_fail_with_message(*d, l, error, strlen((char *)error), s->zero_init.keep_alive > 0);

		return 0;
	}

	object_report(s, object, d, l);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int device_post_retranslator(HTTP_SESSION *s, unsigned int id, unsigned char **d, size_t *l)
{
	#define KEY_SIZE (1024 * 1024)

	DB_OBJECT *object;
	object = api_db_get_object(id);

	if ((object == NULL)||(object->type < OBJECT_TERMINAL_MIN))
		return 404;

	REST_USER *user = rest_find_user(s);
	if (user == NULL)
		return 401;

	if (!rest_check_access_to_object(user, object))
		return 401;

	JKEY incoming_key;

	if (jparse_parse(s->http_body, s->zero_init.body_len, *d, *l / 2, &incoming_key)) {

		unsigned char *error = jparse_get_error(s->http_body, s->zero_init.body_len);

		response_fail_with_message(*d, l, error, strlen((char *)error), s->zero_init.keep_alive > 0);

		jparse_free_error(error);

		return 0;
	}

	if (incoming_key.value_type != JPARSE_VALUE_TYPE_OBJECT)
		return 500;

	if (jparse_validate_object(&incoming_key.value.object_val, device_retranslator_validators, DEVICE_RETRANSLATOR_VALIDATORS_COUNT)) {

		unsigned char *error = jparse_get_error(s->http_body, s->zero_init.body_len);

		response_fail_with_message(*d, l, error, strlen((char *)error), s->zero_init.keep_alive > 0);

		jparse_free_error(error);

		return 0;
	}

	jparse_minify_object(&incoming_key.value.object_val, device_retranslator_validators, DEVICE_RETRANSLATOR_VALIDATORS_COUNT);

	memcpy(incoming_key.key, "retranslator", 12);
	incoming_key.key_len = 12;

	JKEY key;

	if (jparse_parse((unsigned char *)object->core_data, object->core_data_size, *d + KEY_SIZE, (*l - KEY_SIZE) / 2, &key))
		return 500;

	if (key.value_type != JPARSE_VALUE_TYPE_OBJECT)
		return 500;

	if (jparse_set_key(&key, &incoming_key))
		return 500;

	size_t json_buffer_size = *l / 2;
	if (jparse_build_json(&key, *d + KEY_SIZE + (*l - KEY_SIZE) / 2, &json_buffer_size))
		return 500;

	unsigned char *new_core_data = (unsigned char *)realloc(object->core_data, json_buffer_size);

	if (new_core_data == NULL)
		return 500;

	object->core_data = new_core_data;
	object->core_data_size = json_buffer_size;

	memcpy(object->core_data, *d + KEY_SIZE + (*l - KEY_SIZE) / 2, json_buffer_size);

	int res = api_db_update_object(object);

	if (res != DB_OK) {
		
		unsigned char *error = (unsigned char *)api_db_get_error();

		response_fail_with_message(*d, l, error, strlen((char *)error), s->zero_init.keep_alive > 0);

		return 0;
	}

	object_report(s, object, d, l);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int device_put(HTTP_SESSION *s, unsigned int id, unsigned char **d, size_t *l)
{
	ENUM_CTX ctx;
	DB_OBJECT db_object, *parent_object;
	parent_object = api_db_get_object(id);

	if ((parent_object == NULL)||((parent_object->type != OBJECT_USER)&&(parent_object->type != OBJECT_GROUP)))
		return 404;

	REST_USER *user = rest_find_user(s);
	if (user == NULL)
		return 401;

	if (!rest_check_access_to_object(user, parent_object))
		return 401;

	JKEY key;

	if (jparse_parse(s->http_body, s->zero_init.body_len, *d, *l / 2, &key)) {

		unsigned char *error = jparse_get_error(s->http_body, s->zero_init.body_len);

		response_fail_with_message(*d, l, error, strlen((char *)error), s->zero_init.keep_alive > 0);

		jparse_free_error(error);

		return 0;
	}

	if (key.value_type != JPARSE_VALUE_TYPE_OBJECT)
		return 500;

	if (jparse_validate_object(&key.value.object_val, device_put_validators, DEVICE_PUT_VALIDATORS_COUNT)) {

		unsigned char *error = jparse_get_error(s->http_body, s->zero_init.body_len);

		response_fail_with_message(*d, l, error, strlen((char *)error), s->zero_init.keep_alive > 0);

		jparse_free_error(error);

		return 0;
	}

	jparse_minify_object(&key.value.object_val, device_put_validators, DEVICE_PUT_VALIDATORS_COUNT);

	ctx.object = NULL;
	ctx.dev_id = key.value.object_val.first_key->next_key->value.str_val;
	ctx.id_len = key.value.object_val.first_key->next_key->str_len;
	ctx.type   = device_put_validators[0].jkey->value.int_val;

	if (api_db_enum_objects(unique_callback, &ctx)) {
		response_fail_with_message(*d, l, (unsigned char *)"Device of this type with such id already exists", 50, s->zero_init.keep_alive > 0);
		return 0;
	}

	db_object.type				= device_put_validators[0].jkey->value.int_val;
	db_object.module_data		= NULL;
	db_object.module_data_size	= 0;

	MODULE *module = api_get_device_module(db_object.type);
	if (module == NULL)
		return 500;

	unsigned long long caps;
	module->config_get_device_caps(&db_object, &caps);

	if (jparse_set_key(&key, device_construct_caps(caps)))
		return 500;

	size_t json_buffer_size = *l / 2;
	if (jparse_build_json(&key, *d + *l / 2, &json_buffer_size))
		return 500;

	db_object.core_data = (unsigned char *)malloc(json_buffer_size + 33);

	if (db_object.core_data == NULL) 
		return 500;

	memcpy(db_object.core_data, *d + *l / 2, json_buffer_size - 1);
	db_object.core_data_size = json_buffer_size - 1;

	DB_OBJECT *pObject = &db_object;
	int res = api_db_put_object(&pObject, parent_object);

	if (res == DB_OK) {

		on_object_create(pObject);

		module->on_object_create(pObject);

		object_report(s, pObject, d, l);

		return 0;
	}

	unsigned char *error = (unsigned char *)api_db_get_error();

	response_fail_with_message(*d, l, error, strlen((char *)error), s->zero_init.keep_alive > 0);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int device_post(HTTP_SESSION *s, DB_OBJECT *object, unsigned char **d, size_t *l)
{
	unsigned char *existing_key_buffer = *d;
	const size_t existing_key_buffer_size = *l / 3;
	unsigned char *incoming_key_buffer = existing_key_buffer + existing_key_buffer_size;
	const size_t incoming_key_buffer_size = *l / 3;
	unsigned char *new_key_buffer = incoming_key_buffer + incoming_key_buffer_size;
	const size_t new_key_buffer_size = *l / 3;

	ENUM_CTX ctx;

	JKEY incoming_key;

	if (jparse_parse(s->http_body, s->zero_init.body_len, incoming_key_buffer, incoming_key_buffer_size, &incoming_key)) {

		unsigned char *error = jparse_get_error(s->http_body, s->zero_init.body_len);

		response_fail_with_message(*d, l, error, strlen((char *)error), s->zero_init.keep_alive > 0);

		jparse_free_error(error);

		return 0;
	}

	if (incoming_key.value_type != JPARSE_VALUE_TYPE_OBJECT)
		return 500;

	if (jparse_validate_object(&incoming_key.value.object_val, device_post_validators, DEVICE_POST_VALIDATORS_COUNT)) {

		unsigned char *error = jparse_get_error(s->http_body, s->zero_init.body_len);

		response_fail_with_message(*d, l, error, strlen((char *)error), s->zero_init.keep_alive > 0);

		jparse_free_error(error);

		return 0;
	}

	jparse_minify_object(&incoming_key.value.object_val, device_post_validators, DEVICE_POST_VALIDATORS_COUNT);

	ctx.object = object;
	ctx.dev_id = incoming_key.value.object_val.first_key->next_key->value.str_val;
	ctx.id_len = incoming_key.value.object_val.first_key->next_key->str_len;
	ctx.type   = device_post_validators[0].jkey->value.int_val;

	if (api_db_enum_objects(unique_callback, &ctx)) {
		response_fail_with_message(*d, l, (unsigned char *)"Device of this type with such id already exists", 50, s->zero_init.keep_alive > 0);
		return 0;
	}

	JKEY existing_key;

	if (jparse_parse((unsigned char *)object->core_data, object->core_data_size, existing_key_buffer, existing_key_buffer_size, &existing_key))
		return 500;

	if (existing_key.value_type != JPARSE_VALUE_TYPE_OBJECT)
		return 500;

	JKEY *jobjkey = incoming_key.value.object_val.first_key;
	while (jobjkey) {
		JKEY *jobjkey_next = jobjkey->next_key;
		if (jparse_set_key(&existing_key, jobjkey))
			return 500;
		jobjkey = jobjkey_next;
	}

	MODULE *module = api_get_device_module(device_post_validators[0].jkey->value.int_val);
	if (module == NULL)
		return 500;

	bool type_changed = false;

	if (object->type != device_post_validators[0].jkey->value.int_val) {

		type_changed = true;

		MODULE *old_module = api_get_device_module(object->type);

		if (old_module != NULL) {
			old_module->on_object_remove(object);
			if (object->module_data != NULL) {
				free(object->module_data);
				object->module_data = NULL;
				object->module_data_size = 0;
			}
		}

		object->type = device_post_validators[0].jkey->value.int_val;
	}

	unsigned long long caps;
	module->config_get_device_caps(object, &caps);

	if (jparse_set_key(&existing_key, device_construct_caps(caps)))
		return 500;

	size_t json_buffer_size = new_key_buffer_size;
	if (jparse_build_json(&existing_key, new_key_buffer, &json_buffer_size))
		return 500;

	unsigned char *new_core_data = (unsigned char *)realloc(object->core_data, json_buffer_size);

	if (new_core_data == NULL)
		return 500;

	object->core_data = new_core_data;
	object->core_data_size = json_buffer_size;

	memcpy(object->core_data, new_key_buffer, json_buffer_size);

	int res = api_db_update_object(object);

	if (res == DB_OK) {

		if (type_changed)
			module->on_object_create(object);
		else
			module->on_object_update(object);

		object_report(s, object, d, l);

		return 0;
	}

	unsigned char *error = (unsigned char *)api_db_get_error();

	response_fail_with_message(*d, l, error, strlen((char *)error), s->zero_init.keep_alive > 0);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int device_post_modify(HTTP_SESSION *s, unsigned int id, unsigned char **d, size_t *l)
{
	DB_OBJECT *object;
	object = api_db_get_object(id);

	if ((object == NULL)||(object->type < OBJECT_TERMINAL_MIN))
		return 404;

	REST_USER *user = rest_find_user(s);
	if (user == NULL)
		return 401;

	if (!rest_check_access_to_object(user, object))
		return 401;

	unsigned char *incoming_key_buffer = *d;
	const size_t incoming_key_buffer_size = *l;

	JKEY incoming_key;

	if (jparse_parse(s->http_body, s->zero_init.body_len, incoming_key_buffer, incoming_key_buffer_size, &incoming_key)) {

		unsigned char *error = jparse_get_error(s->http_body, s->zero_init.body_len);

		response_fail_with_message(*d, l, error, strlen((char *)error), s->zero_init.keep_alive > 0);

		jparse_free_error(error);

		return 0;
	}

	if (incoming_key.value_type != JPARSE_VALUE_TYPE_OBJECT)
		return 500;

	if (jparse_validate_object(&incoming_key.value.object_val, device_post_modify_validators, DEVICE_POST_MODIFY_VALIDATORS_COUNT)) {

		unsigned char *error = jparse_get_error(s->http_body, s->zero_init.body_len);

		response_fail_with_message(*d, l, error, strlen((char *)error), s->zero_init.keep_alive > 0);

		jparse_free_error(error);

		return 0;
	}

	void *stream = api_storage_get_stream_by_id(id);

	if (stream != NULL) {

		api_storage_lock_stream(stream);

		STORAGE_RECORD_HEADER *rh = api_storage_get_stream_first_record(stream);

		if (rh != NULL) {

			size_t records_count = api_storage_get_stream_records_count(stream);
	
			device_post_modify_validators[1].jkey->value.str_val[device_post_modify_validators[1].jkey->str_len] = '\0';

			for (size_t i = 0; i < records_count; i++) {

				if (rh->t == device_post_modify_validators[0].jkey->value.int_val) {
					unsigned int value = (device_post_modify_validators[2].jkey->value_type == JPARSE_VALUE_TYPE_FLOAT) ? (unsigned int)device_post_modify_validators[2].jkey->value.float_val : device_post_modify_validators[2].jkey->value.int_val;
					api_storage_update_record(id, rh, (char *)device_post_modify_validators[1].jkey->value.str_val, &value);
				}

				rh = (STORAGE_RECORD_HEADER *)(((unsigned char *)rh) + rh->size);
			}
		}

		api_storage_unlock_stream(stream);
	}

	return REST_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////////

int device_init()
{
	for (int i = 0; i < DEVICE_CAPS_COUNT; i++) {
		key_caps[i].key_len = strlen(key_caps[i].key);
		key_caps[i].next_key = &key_caps[i + 1];
	}

	key_caps[DEVICE_CAPS_COUNT - 1].next_key = NULL;

	caps_key.value.object_val.first_key = key_caps;
	caps_key.value.object_val.last_key = &key_caps[DEVICE_CAPS_COUNT - 1];
	caps_key.value.object_val.next_object = NULL;

	return 0;
}