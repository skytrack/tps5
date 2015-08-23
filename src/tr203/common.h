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

#include <map>
#include <time.h>
#include "stdint.h"
#include "data.h"
#include "../core/storage.h"

#define OBJECT_TYPE	1004

typedef struct _TERMINAL{
	unsigned int	id;
	DB_OBJECT		*object;
	SESSION			*session;
} TERMINAL;

extern std::map<uint64_t, TERMINAL> terminals;

extern time_t now;
extern unsigned char record_buffer[1024];
extern STORAGE_RECORD_HEADER *record;
extern unsigned char *record_data;

extern size_t error_len;
extern unsigned char *error_ptr;
extern unsigned char error_buf[4096];
extern unsigned char no_error[9];

void add_event(TERMINAL *terminal, unsigned short event_id);

#endif