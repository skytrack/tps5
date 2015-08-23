//******************************************************************************
//
// File Name	: config.c
// Author	: Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#include <string.h>
#include "likely.h"
#include "config.h"
#include "ini.h"

typedef struct ctx
{
	const char *section;
	CONFIGSECTIONHANDLER handler;
} CTX;

int config_validate_handler(void* user, const char* section, const char* name, const char* value)
{
	return 1;
}

int config_validate(const char *config_file)
{
	int status;

	printf("Validating config '%s'\r\n", config_file);

	status = ini_parse(config_file, config_validate_handler, NULL);

	if (unlikely(status != 0)) {

		switch (status) {
		case -1:
			printf("[CONFIG] Unable to open file `%s`\r\n", config_file);
			break;
		case -2:
			printf("[CONFIG] Memory allocation error\r\n");
			break;
		default:
			printf("[CONFIG] Parse error on line %d\r\n", status);
			break;
		}

		return -1;
	}

	return 0;
}

int config_section_handler(void* user, const char* section, const char* name, const char* value)
{
	CTX *ctx = (CTX *)user;

	if (unlikely(strcmp(section, ctx->section) == 0))
		return ctx->handler(name, value);

	return 1;
}

int config_read_section(const char *config_file, const char *section, CONFIGSECTIONHANDLER handler)
{
	int status;
	CTX ctx;

	ctx.section = section;
	ctx.handler = handler;

	status = ini_parse(config_file, config_section_handler, &ctx);

	if (unlikely(status != 0)) {

		switch (status) {
		case -1:
			printf("[CONFIG] Unable to open file `%s`\r\n", config_file);
			break;
		case -2:
			printf("[CONFIG] Memory allocation error\r\n");
			break;
		default:
			printf("[CONFIG] Parse error on line %d\r\n", status);
			break;
		}

		return -1;
	}

	return 0;
}

// End
