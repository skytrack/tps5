//******************************************************************************
//
// File Name : retranslator.cpp
// Author    : Skytrack ltd - Copyright (C) 2015
//
// This code is property of Skytrack company
//
//******************************************************************************

#include <list>
#include <stdlib.h>
#include <string.h>
#include "retranslator.h"
#include "../../core/cross.h"

static spinlock_t					spinlock;
static std::list<RETRANSLATOR *>	retranslators;

void retranslators_init()
{
	spinlock_init(&spinlock);
}

void retranslators_destroy()
{
	spinlock_lock(&spinlock);

	for (std::list<RETRANSLATOR *>::const_iterator iterator = retranslators.begin(), end = retranslators.end(); iterator != end; ++iterator) {
		if ((*iterator)->sock != -1) {
			closesocket((*iterator)->sock);
			delete *iterator;
		}
	}

	retranslators.clear();

	spinlock_unlock(&spinlock);
}

REFERENCE *retranslators_create_reference(const char *device_id, const char *host, unsigned short port, const char *login, const char *password)
{
	RETRANSLATOR *pRetranslator = NULL;

	spinlock_lock(&spinlock);

	for (std::list<RETRANSLATOR *>::const_iterator iterator = retranslators.begin(), end = retranslators.end(); iterator != end; ++iterator) {
	
		if ((host == (*iterator)->host) && (port == (*iterator)->port) && 
			(login == (*iterator)->login) && (password == (*iterator)->password)) {

			pRetranslator = *iterator;
			break;
		}
	}

	if (pRetranslator == NULL) {
		
		pRetranslator					= new RETRANSLATOR;

		pRetranslator->host				= host;
		pRetranslator->port				= port;
		pRetranslator->login			= login;
		pRetranslator->password			= password;
		pRetranslator->reference_count	= 0;
		pRetranslator->sock				= -1;
		pRetranslator->connection_status= RETRANSLATOR_STATUS_INIT;

		spinlock_init(&pRetranslator->spinlock);

		retranslators.push_back(pRetranslator);
	}

	pRetranslator->reference_count++;

	spinlock_unlock(&spinlock);

	REFERENCE *pReference = new REFERENCE;

	pReference->pRetranslator = pRetranslator;
	pReference->nupe = strtoul(device_id, NULL, 10);

	return pReference;
}

void retranslators_destroy_reference(REFERENCE *pReference)
{	
	RETRANSLATOR *pRetranslator = pReference->pRetranslator;

	spinlock_lock(&spinlock);

	pRetranslator->reference_count--;

	if (pRetranslator->reference_count == 0) {

		if (pRetranslator->sock != -1) {
			closesocket(pRetranslator->sock);
			pRetranslator->sock = -1;
		}
	
		retranslators.remove(pRetranslator);

		delete pRetranslator;
	}

	spinlock_unlock(&spinlock);

	delete pReference;
}

void retranslators_enum(RETRANSLATORS_ENUM_CALLBACK callback, void *ctx)
{
	spinlock_lock(&spinlock);

	for (std::list<RETRANSLATOR *>::const_iterator iterator = retranslators.begin(), end = retranslators.end(); iterator != end; ++iterator) {
		callback(*iterator, ctx);
	}

	spinlock_unlock(&spinlock);
}

// End
