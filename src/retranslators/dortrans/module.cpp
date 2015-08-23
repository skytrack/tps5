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
static const char *name					= "DorTransNavi";
static const unsigned int protocol_id	= 3;

REFERENCE *create_retranslator(const char *device_id, const char *host, unsigned short port, const char *login, const char *password)
{
	api_log_printf("[DORTRANS] Create retranslator for '%s'\r\n", device_id);

	return retranslators_create_reference(device_id, host, port, login, password);
}

void destroy_retranslator(REFERENCE *pReference) 
{
	api_log_printf("[DORTRANS] Destroy retranslator for '%u' with socket #%d\r\n", pReference->nupe, pReference->pRetranslator->sock);
	retranslators_destroy_reference(pReference);
}

void add_record_to_retranslator(REFERENCE *pReference, RETRANSLATOR_RECORD *pRecord)
{
	pRecord->nupe = pReference->nupe;

	spinlock_lock(&pReference->pRetranslator->spinlock);
	pReference->pRetranslator->records_list.insert(pReference->pRetranslator->records_list.end(), *pRecord);
	spinlock_unlock(&pReference->pRetranslator->spinlock);
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

	api_log_printf("[DORTRANS] Started\r\n");

	return 0;
}

int stop()
{
	thread_stop();
	retranslators_destroy();

	api_log_printf("[DORTRANS] Stopped\r\n");

	return 0;
}

// End
