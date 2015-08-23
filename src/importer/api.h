//******************************************************************************
//
// File Name : api.h
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#ifndef _API_H

#define _API_H

#include "../core/storage.h"
#include "../core/module.h"
#include "../core/db.h"

extern LOG_PRINTF                       api_log_printf;

extern LISTEN_TCP                       api_listen_tcp;

extern STORAGE_GET_STREAM_BY_ID         api_storage_get_stream_by_id;
extern STORAGE_ADD_RECORD_TO_STREAM     api_storage_add_record_to_stream;

extern DB_ENUM_OBJECTS					api_db_enum_objects;

extern void *api_tag;

#endif

// End
