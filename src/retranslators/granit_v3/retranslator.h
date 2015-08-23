//******************************************************************************
//
// File Name : retranslators.h
// Author    : Skytrack ltd - Copyright (C) 2015
//
// This code is property of Skytrack company
//
//******************************************************************************

#ifndef _RETRANSLATOR_H

#define _RETRANSLATOR_H

#include <string>
#include <queue>
#include <time.h>

#include "../../core/spinlock.h"
#include "../../core/module.h"

#define RETRANSLATOR_STATUS_INIT		0x00
#define RETRANSLATOR_STATUS_CONNECTING	0x01
#define RETRANSLATOR_STATUS_CONNECTED	0x02
#define RETRANSLATOR_STATUS_WAITACK		0x03

typedef struct tagRETRANSLATOR
{
	std::string		device_id;
	std::string		host;
	unsigned short	port;
	std::string		login;
	std::string		password;

	std::queue<RETRANSLATOR_RECORD> records_queue;

	spinlock_t		spinlock;

	int				sock;
	int				status;

	unsigned short	nupe;

	time_t			timeout;
} RETRANSLATOR;

typedef void (*RETRANSLATORS_ENUM_CALLBACK)(RETRANSLATOR *pRetranslator, void *ctx);

void retranslators_init();
void retranslators_destroy();
void retranslators_add_item(RETRANSLATOR *pRetranslator);
void retranslators_remove_item(RETRANSLATOR *pRetranslator);
void retranslators_enum(RETRANSLATORS_ENUM_CALLBACK callback, void *ctx);

#endif

// End
