//******************************************************************************
//
// File Name : retranslator.cpp
// Author    : Skytrack ltd - Copyright (C) 2015
//
// This code is property of Skytrack company
//
//******************************************************************************

#include <stdio.h>
#include <limits.h>
#include <string.h>
#include "api.h"
#include "rest.h"
#include "retranslator.h"
#include "response.h"
#include "../core/jparse.h"
#include "json.h"
#include "object.h"

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

static int retranslators_callback(MODULE *module, void *c)
{
	const char *pName;
	const int  *pProtocol;
	size_t len;
	size_t this_item_len;
	unsigned char *this_item_start;

	ENUM_CTX *ctx = (ENUM_CTX *)c;

	module->get_var(MODULE_VAR_NAME, (void**)&pName);
	module->get_var(MODULE_VAR_PROTOCOL_ID, (void**)&pProtocol);

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

		ctx->ptr = json_add_uint(ctx->ptr, "id", *pProtocol);

		*ctx->ptr++ = '}';

		ctx->bytes_left -= (ctx->ptr - this_item_start);

		return 0;
	}

	return -1;
}

int retranslators(HTTP_SESSION *s, unsigned int id, unsigned char **d, size_t *l)
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

	if (api_enum_modules(MODULE_FAMILY_RETRANSLATOR, retranslators_callback, &ctx))
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
