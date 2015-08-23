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

void retranslators_add_item(RETRANSLATOR *pRetranslator)
{
	pRetranslator->sock	= -1;
	pRetranslator->status = RETRANSLATOR_STATUS_INIT;
	
	spinlock_lock(&spinlock);

	retranslators.push_back(pRetranslator);

	spinlock_unlock(&spinlock);
}

void retranslators_remove_item(RETRANSLATOR *pRetranslator)
{
	spinlock_lock(&spinlock);

	if (pRetranslator->sock != -1) {
		closesocket(pRetranslator->sock);
		pRetranslator->sock = -1;
	}
	
	retranslators.remove(pRetranslator);

	spinlock_unlock(&spinlock);
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
