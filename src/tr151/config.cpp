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
#include "api.h"
#include "config.h"

int config_get_device_caps(DB_OBJECT *object, unsigned long long *caps)
{
	*caps = 0;

	*caps = DEVICE_CAPS_ALARM | DEVICE_CAPS_NAV_ALTITUDE;

	return 0;
}

int config_put_json(DB_OBJECT *object, unsigned char *json, size_t len)
{
	return 0;
}

int config_get_json(DB_OBJECT *object, unsigned char *s, size_t *len)
{
	*s++ = '[';
	*s++ = ']';

	return 0;
}