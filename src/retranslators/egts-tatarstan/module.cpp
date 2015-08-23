//******************************************************************************
//
// File Name : module.cpp
// Author    : Skytrack ltd - Copyright (C) 2015
//
// This code is property of Skytrack company
//
//******************************************************************************

#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <queue>

#include "module.h"
#include "thread.h"
#include "retranslator.h"

#include "../../core/spinlock.h"

#include "api.h"

static const MODULE_FAMILY family		= MODULE_FAMILY_RETRANSLATOR;
static const char *name					= "egts tatarstan";
static const unsigned int protocol_id	= 2;

RETRANSLATOR *create_retranslator(const char *device_id, const char *host, unsigned short port, const char *login, const char *password)
{
	RETRANSLATOR *pRetranslator = new RETRANSLATOR;

	pRetranslator->device_id	= device_id;
	pRetranslator->host			= host;
	pRetranslator->port			= port;
	pRetranslator->login		= login;
	pRetranslator->password		= password;

	pRetranslator->oid			= strtoul(device_id, NULL, 10);

	spinlock_init(&pRetranslator->spinlock);

	api_log_printf("[EGTS-TATARSTAN] Create retranslator for '%s'\r\n", device_id);

	retranslators_add_item(pRetranslator);

	return pRetranslator;
}

void destroy_retranslator(RETRANSLATOR *pRetranslator) 
{
	api_log_printf("[EGTS-TATARSTAN] Destroy retranslator for '%s' with socket #%d\r\n", pRetranslator->device_id.c_str(), pRetranslator->sock);
	retranslators_remove_item(pRetranslator);

	delete pRetranslator;
}

void add_record_to_retranslator(RETRANSLATOR *pRetranslator, RETRANSLATOR_RECORD *pRecord)
{
	spinlock_lock(&pRetranslator->spinlock);
	pRetranslator->records_queue.push(*pRecord);
	spinlock_unlock(&pRetranslator->spinlock);
}

int get_var(int var_type, void **p)
{
	switch (var_type) {

	case MODULE_VAR_FAMILY:
		*p = (void *)family;
		break;

	case MODULE_VAR_NAME:
		*p = (void *)name;
		break;

	case MODULE_VAR_PROTOCOL_ID:
		*p = (void *)&protocol_id;
		break;

	case MODULE_VAR_FUNC_CREATE_RETRANSLATOR:
		*p = (void *)create_retranslator;
		break;

	case MODULE_VAR_FUNC_DESTROY_RETRANSLATOR:
		*p = (void *)destroy_retranslator;
		break;

	case MODULE_VAR_FUNC_ADD_RECORD_TO_RETRANSLATOR:
		*p = (void *)add_record_to_retranslator;
		break;

	default:
		*p = NULL;
		return -1;
	}

	return 0;
}

int set_var(int var_type, void *p)
{
	CONFIG_OPTION *op = (CONFIG_OPTION *)p;

	switch(var_type) {

	case MODULE_VAR_FUNC_LOG:
		api_log_printf = (LOG_PRINTF)p;
		break;
	}

	return 0;
}


int start()
{
	retranslators_init();
	thread_start();

	api_log_printf("[EGTS-TATARSTAN] Started\r\n");

	return 0;
}

int stop()
{
	thread_stop();
	retranslators_destroy();

	api_log_printf("[EGTS-TATARSTAN] Stopped\r\n");

	return 0;
}

// End
