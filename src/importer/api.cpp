//******************************************************************************
//
// File Name : api.cpp
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#include "api.h"

LOG_PRINTF                       api_log_printf;
LISTEN_TCP                       api_listen_tcp;
STORAGE_GET_STREAM_BY_ID         api_storage_get_stream_by_id;
STORAGE_ADD_RECORD_TO_STREAM     api_storage_add_record_to_stream;
DB_ENUM_OBJECTS			         api_db_enum_objects;

void *api_tag;

// End
