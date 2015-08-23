//******************************************************************************
//
// File Name : common.h
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#ifndef _COMMON_H

#define _COMMON_H

#include <vector>
#include <time.h>
#include "ak306.h"
#include "../core/db.h"
#include "../core/storage.h"

#define OBJECT_TYPE			1000
#define MAX_SEND_ATTEMPT	5

typedef struct _TERMINAL{
	unsigned int	id;
	bool			online;

	unsigned char	send_attempt;
	time_t			ack_timeout;
	time_t			session_timeout;

	unsigned short	next_tx_seq_no;
	unsigned short	last_rx_seq_no;

	unsigned int	last_command_time;

	AK306COMMAND	tx_cmd;

	DB_OBJECT		*object;

	char			context[256];

	FILE			*f;
} TERMINAL;

extern std::vector<TERMINAL> terminals;
extern unsigned char *id;
extern time_t now;
extern const char send_attempt_intervals[MAX_SEND_ATTEMPT];
extern unsigned char record_buffer[1024];
extern STORAGE_RECORD_HEADER *record;
extern unsigned char *record_data;

extern size_t error_len;
extern unsigned char *error_ptr;
extern unsigned char error_buf[4096];
extern unsigned char no_error[9];

void add_event(TERMINAL *terminal, unsigned short event_id);
void close_connection(TERMINAL *terminal);

#endif