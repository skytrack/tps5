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
SEND_TCP                         api_send_tcp;
SEND_UDP                         api_send_udp;
CLOSE_TCP                        api_close_tcp;
LISTEN_UDP                       api_listen_udp;
STORAGE_CREATE_STREAM            api_storage_create_stream;
STORAGE_DESTROY_STREAM           api_storage_destroy_stream;
STORAGE_GET_STREAM_BY_ID         api_storage_get_stream_by_id;
STORAGE_LOCK_STREAM              api_storage_lock_stream;
STORAGE_UNLOCK_STREAM            api_storage_unlock_stream;
STORAGE_ADD_RECORD_TO_STREAM     api_storage_add_record_to_stream;
STORAGE_GET_STREAM_FIRST_RECORD  api_storage_get_stream_first_record;
STORAGE_GET_STREAM_RECORDS_COUNT api_storage_get_stream_records_count;
STORAGE_SORT_STREAM              api_storage_sort_stream;
STORAGE_TRIM_STREAM              api_storage_trim_stream;
STORAGE_GET_STREAM_INFO          api_storage_get_stream_info;

DB_GET_ERROR			         api_db_get_error;
DB_GET_OBJECT                    api_db_get_object;
DB_PUT_OBJECT                    api_db_put_object;
DB_UPDATE_OBJECT_BLOB            api_db_update_object_blob;
DB_DELETE_OBJECT                 api_db_delete_object;
DB_ENUM_OBJECTS			         api_db_enum_objects;

void *api_tag;

// End
