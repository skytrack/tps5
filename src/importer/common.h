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
#include "../core/db.h"
#include "../core/storage.h"

extern unsigned char *dev_id;
extern std::vector<unsigned short> object_id;

extern unsigned char record_buffer[1024];
extern STORAGE_RECORD_HEADER *record;
extern unsigned char *record_data;

extern size_t error_len;
extern unsigned char *error_ptr;
extern unsigned char error_buf[4096];
extern unsigned char no_error[9];

#endif