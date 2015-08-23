//******************************************************************************
//
// File Name : config.h
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#ifndef _CONFIG_TA001_H

#define _CONFIG_TA001_H

int config_get_json(DB_OBJECT *object, unsigned char *s, size_t *len);
int config_put_json(DB_OBJECT *object, unsigned char *json, size_t len);
int config_get_device_caps(DB_OBJECT *object, unsigned long long *result);

#endif

// End
