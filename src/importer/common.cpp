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
#include "data.h"
#include "../core/record.h"
#include "../core/aligned.h"

_ALIGNED(unsigned char *)	dev_id;
_ALIGNED(unsigned char)		record_buffer[1024];

std::vector<unsigned short> object_id;

size_t error_len;
unsigned char error_buf[4096];
unsigned char *error_ptr;

STORAGE_RECORD_HEADER *record = (STORAGE_RECORD_HEADER *)record_buffer;

unsigned char *record_data = record_buffer + sizeof(STORAGE_RECORD_HEADER);

unsigned char no_error[9] = "No error";

