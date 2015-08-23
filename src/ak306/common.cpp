//******************************************************************************
//
// File Name : common.cpp
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#include <stdlib.h>
#include <stdio.h>
#include "api.h"
#include "common.h"
#include "config.h"
#include "data.h"
#include "../core/record.h"
#include "../core/almalloc.h"

std::vector<TERMINAL> terminals;

size_t error_len;
unsigned char error_buf[4096];
unsigned char *error_ptr;

_ALIGNED(unsigned char *) id;

const char send_attempt_intervals[MAX_SEND_ATTEMPT] = { 15, 15, 30, 30, 60 };

time_t now;

_ALIGNED(unsigned char) record_buffer[1024];

STORAGE_RECORD_HEADER *record = (STORAGE_RECORD_HEADER *)record_buffer;

unsigned char *record_data = record_buffer + sizeof(STORAGE_RECORD_HEADER);

unsigned char no_error[9] = "No error";

void add_event(TERMINAL *terminal, unsigned short event_id)
{
	if (terminal->object->stream != NULL) {
		record->t = (unsigned int)now;
		record->size = sizeof(STORAGE_RECORD_HEADER) + 3;
		record_data[0] = RECORD_BIT1_EVENT;
		*(unsigned short *)&record_data[1] = event_id;

		api_storage_add_record_to_stream(terminal->object->stream, record, sizeof(STORAGE_RECORD_HEADER) + 3);
	}
}

void close_connection(TERMINAL *terminal)
{
	terminal->online = false;

	add_event(terminal, RECORD_EVENT_TERMINAL_OFFLINE);

	if (terminal->f != NULL) {
		fclose(terminal->f);
		terminal->f = NULL;
	}

	api_log_printf("[AK306] Closed connection with terminal %d\r\n", terminal->id);
}

