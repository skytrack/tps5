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
#include "../core/device.h"

extern LOG_PRINTF                       api_log_printf;

extern LISTEN_TCP                       api_listen_tcp;
extern SEND_TCP                         api_send_tcp;
extern SEND_UDP                         api_send_udp;
extern CLOSE_TCP                        api_close_tcp;
extern LISTEN_UDP                       api_listen_udp;

extern STORAGE_CREATE_STREAM            api_storage_create_stream;
extern STORAGE_DESTROY_STREAM           api_storage_destroy_stream;
extern STORAGE_GET_STREAM_BY_ID         api_storage_get_stream_by_id;
extern STORAGE_LOCK_STREAM              api_storage_lock_stream;
extern STORAGE_UNLOCK_STREAM            api_storage_unlock_stream;
extern STORAGE_ADD_RECORD_TO_STREAM     api_storage_add_record_to_stream;
extern STORAGE_GET_STREAM_FIRST_RECORD  api_storage_get_stream_first_record;
extern STORAGE_GET_STREAM_RECORDS_COUNT api_storage_get_stream_records_count;
extern STORAGE_SORT_STREAM              api_storage_sort_stream;
extern STORAGE_TRIM_STREAM              api_storage_trim_stream;
extern STORAGE_GET_STREAM_INFO          api_storage_get_stream_info;

extern DB_GET_ERROR			            api_db_get_error;
extern DB_GET_OBJECT                    api_db_get_object;
extern DB_PUT_OBJECT                    api_db_put_object;
extern DB_UPDATE_OBJECT                 api_db_update_object;
extern DB_UPDATE_OBJECT_BLOB            api_db_update_object_blob;
extern DB_DELETE_OBJECT                 api_db_delete_object;
extern DB_ENUM_OBJECTS					api_db_enum_objects;

extern void *api_tag;

#endif

// End
