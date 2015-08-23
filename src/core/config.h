//******************************************************************************
//
// File Name	: config.h
// Author	: Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#ifndef _CONFIG_H

#define _CONFIG_H

typedef int (*CONFIGSECTIONHANDLER)(const char *name, const char *value);

int config_validate(const char *config_file);
int config_read_section(const char *config_file, const char *section, CONFIGSECTIONHANDLER handler);

#endif

// End
