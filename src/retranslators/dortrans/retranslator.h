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
#include <list>
#include <time.h>

#include "../../core/spinlock.h"
#include "../../core/module.h"

#define RETRANSLATOR_STATUS_INIT		0x00
#define RETRANSLATOR_STATUS_CONNECTING	0x01
#define RETRANSLATOR_STATUS_CONNECTED	0x02

typedef struct tagRETRANSLATOR
{
	std::string		host;
	unsigned short	port;
	std::string		login;
	std::string		password;

	std::list<RETRANSLATOR_RECORD> records_list;

	spinlock_t		spinlock;

	int				sock;
	int				connection_status;
	int				status;
	int				frame_state;
	size_t			frame_bytes_received;
	size_t			frame_len;
	unsigned char	frame_crc;

	unsigned char	frame_body[16384];
	unsigned int	pack_num;
	size_t			nRecordsToAck;

	time_t			timeout;
	time_t			last_send;

	size_t			reference_count;
} RETRANSLATOR;

typedef struct REFERENCE
{
	RETRANSLATOR *pRetranslator;
	unsigned int nupe;
} REFERENCE;

typedef void (*RETRANSLATORS_ENUM_CALLBACK)(RETRANSLATOR *pRetranslator, void *ctx);

void retranslators_init();
void retranslators_destroy();
REFERENCE *retranslators_create_reference(const char *device_id, const char *host, unsigned short port, const char *login, const char *password);
void retranslators_destroy_reference(REFERENCE *pReference);
void retranslators_enum(RETRANSLATORS_ENUM_CALLBACK callback, void *ctx);

#endif

// End
