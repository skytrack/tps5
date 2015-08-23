//******************************************************************************
//
// File Name : db.h
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#ifndef _DB_H

#define _DB_H

#include <vector>

#define DB_OK           0
#define DB_ERR          1
#define DB_NO_PARENT_ID 2

#define OBJECT_USER		0
#define OBJECT_GROUP	1
#define OBJECT_TERMINAL_MIN 1000

typedef struct db_object
{
	unsigned short id;
	unsigned short parent_id;
	unsigned short type;
	unsigned char *core_data;
	unsigned short core_data_size;
	unsigned char *module_data;
	unsigned short module_data_size;
	unsigned short position;
	void *stream;
	void *retranslator;
	void *retranslator_module;
	std::vector<struct db_object *> sub_objects;
} DB_OBJECT;

typedef int (*ENUM_OBJECTS_CALLBACK)(DB_OBJECT *object, void *ctx);

int db_open(const char *path, const char *password);
void db_close();
int db_load();

DB_OBJECT *db_get_object(unsigned int object_id);
int db_put_object(DB_OBJECT **object, DB_OBJECT *parent_object);
int db_delete_object(DB_OBJECT *object);
int db_update_object(DB_OBJECT *object);
int db_update_object_module_data(DB_OBJECT *object);
int db_change_object_parent(DB_OBJECT *object, DB_OBJECT *new_parent);
int db_move_object(DB_OBJECT *object, DB_OBJECT *new_parent, DB_OBJECT *next_sibling, DB_OBJECT *prev_sibling);
int db_enum_objects(ENUM_OBJECTS_CALLBACK callback, void *ctx);
const char *db_get_error();

#endif

// End
