//******************************************************************************
//
// File Name : db.cpp
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <map>
#include <algorithm>
#include <string>
#include <string.h>

#include "sqlite3.h"
#include "log.h"
#include "db.h"
#include "storage.h"
#include "sha2.h"
#include "jparse.h"
#include "module.h"

MODULE *get_retranslator_module(int retranslator_type);

static std::map<unsigned int, DB_OBJECT> db_index;

static sqlite3 *db								= NULL;
static sqlite3_stmt *stmt_insert				= NULL;
static sqlite3_stmt *stmt_update				= NULL;
static sqlite3_stmt *stmt_update_module_data	= NULL;
static sqlite3_stmt *stmt_delete				= NULL;
static sqlite3_stmt *stmt_select				= NULL;
static sqlite3_stmt *stmt_id					= NULL;
static sqlite3_stmt *stmt_change				= NULL;
static std::string password;

static void base64_encode(const unsigned char *data, size_t input_length, char *encoded_data, size_t *output_length) 
{
	static char encoding_table[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
									'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
									'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
									'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
									'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
									'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
									'w', 'x', 'y', 'z', '0', '1', '2', '3',
									'4', '5', '6', '7', '8', '9', '+', '/'};
	
	static int mod_table[] = {0, 2, 1};

	*output_length = 4 * ((input_length + 2) / 3);

	for (size_t i = 0, j = 0; i < input_length;) {

		uint32_t octet_a = i < input_length ? (unsigned char)data[i++] : 0;
		uint32_t octet_b = i < input_length ? (unsigned char)data[i++] : 0;
		uint32_t octet_c = i < input_length ? (unsigned char)data[i++] : 0;

		uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

		encoded_data[j++] = encoding_table[(triple >> 3 * 6) & 0x3F];
		encoded_data[j++] = encoding_table[(triple >> 2 * 6) & 0x3F];
		encoded_data[j++] = encoding_table[(triple >> 1 * 6) & 0x3F];
		encoded_data[j++] = encoding_table[(triple >> 0 * 6) & 0x3F];
	}

	for (int i = 0; i < mod_table[input_length % 3]; i++)
		encoded_data[*output_length - 1 - i] = '=';
}

static bool sort_function (DB_OBJECT * i, DB_OBJECT *j) { return (i->position < j->position); }
static unsigned char *buffer;

static void create_retranslator(DB_OBJECT *pObject) 
{
	int *retranslator		= NULL;
	unsigned char *host		= NULL;
	int *port				= NULL;
	unsigned char *login	= NULL;
	unsigned char *password	= NULL;
	unsigned char *id		= NULL;

	JKEY key;

	if (jparse_parse(pObject->core_data, pObject->core_data_size, buffer, 32 * 1024 * 1024, &key) == 0) {

		if (key.value_type == JPARSE_VALUE_TYPE_OBJECT) {

			JKEY *k = key.value.object_val.first_key;

			while (k) {

				if ((k->key_len == 12)&&(memcmp(k->key, "retranslator", 12) == 0)&&(k->value_type == JPARSE_VALUE_TYPE_OBJECT)) {

					JKEY *l = k->value.object_val.first_key;

					while (l) {

						if ((l->key_len == 12)&&(memcmp(l->key, "retranslator", 12) == 0)&&(l->value_type == JPARSE_VALUE_TYPE_NUMBER)) {
							retranslator = &l->value.int_val;
						}
						else
						if ((l->key_len == 4)&&(memcmp(l->key, "host", 4) == 0)&&(l->value_type == JPARSE_VALUE_TYPE_STRING)) {
							host = l->value.str_val;
							host[l->str_len] = '\0';
						}
						else
						if ((l->key_len == 4)&&(memcmp(l->key, "port", 4) == 0)&&(l->value_type == JPARSE_VALUE_TYPE_NUMBER)) {
							port = &l->value.int_val;
						}
						else
						if ((l->key_len == 5)&&(memcmp(l->key, "login", 5) == 0)&&(l->value_type == JPARSE_VALUE_TYPE_STRING)) {
							login = l->value.str_val;
							login[l->str_len] = '\0';
						}
						else
						if ((l->key_len == 8)&&(memcmp(l->key, "password", 8) == 0)&&(l->value_type == JPARSE_VALUE_TYPE_STRING)) {
							password = l->value.str_val;
							password[l->str_len] = '\0';
						}
						else
						if ((l->key_len == 2)&&(memcmp(l->key, "id", 2) == 0)&&(l->value_type == JPARSE_VALUE_TYPE_STRING)) {
							id = l->value.str_val;
							id[l->str_len] = '\0';
						}

						l = l->next_key;
					}

					break;
				}
					
				k = k->next_key;
			}
		}
	}

	pObject->retranslator = NULL;
	pObject->retranslator_module = NULL;
	storage_set_retranslator(pObject->stream, NULL, NULL);

	if ((id != NULL)&&(retranslator != NULL)&&(host != NULL)&&(port != NULL)&&(*host != '\0')&&(*port > 0)&&(*port <= 65535)&&(*retranslator > 0)) {
			
		MODULE *pModule = get_retranslator_module(*retranslator);

		if (pModule != NULL) {

			pObject->retranslator_module = pModule;
			pObject->retranslator = pModule->create_retranslator((char *)id, (char *)host, *port, (char *)login, (char *)password);

			storage_set_retranslator(pObject->stream, pObject->retranslator_module, pObject->retranslator);
		}
	}
}

int db_load()
{
	static const char root_data[] = "{\"id\":0,\"type\":0,\"login\":\"admin\", \"permission_admin\": true, \"permission_login\": true, \"hash\":\"%s\"}";
	char root_with_password[1024];
	unsigned char auth[1024];

	memcpy(auth, "admin:", 6);
	memcpy(auth + 6, password.c_str(), password.length());

	size_t base64_len;
	char base64[256];
	base64_encode(auth, password.length() + 6, base64, &base64_len);

	SHA256_CTX ctx256;
	SHA256_Init(&ctx256);
	SHA256_Update(&ctx256, (unsigned char *)base64, base64_len);
	SHA256_End(&ctx256, (char *)auth);

	sprintf(root_with_password, root_data, (char *)auth);

	DB_OBJECT root;

	root.id = 0;
	root.type = 0;
	root.parent_id = 0;
	root.stream = NULL;

	root.core_data_size = strlen(root_with_password);
	root.core_data = (unsigned char *)malloc(strlen(root_with_password));

	if (root.core_data == NULL)
		return -1;

	memcpy(root.core_data, root_with_password, strlen(root_with_password));

	root.module_data = NULL;
	root.module_data_size = 0;

	db_index.clear();

	db_index[root.id] = root;

	sqlite3_reset(stmt_select);

	buffer = (unsigned char *)malloc(32 * 1024 * 1024);

	while (sqlite3_step(stmt_select) == SQLITE_ROW) {

		DB_OBJECT sub_object;

		sub_object.id			= sqlite3_column_int(stmt_select, 0);
		sub_object.type			= sqlite3_column_int(stmt_select, 1);
		sub_object.parent_id	= sqlite3_column_int(stmt_select, 2);

		sub_object.core_data_size = sqlite3_column_bytes(stmt_select, 3);
		if (sub_object.core_data_size > 0) {
			sub_object.core_data = (unsigned char *)malloc(sub_object.core_data_size);
			if (sub_object.core_data == NULL) {
				sqlite3_reset(stmt_select);
				free(buffer);
				return -1;
			}
			memcpy(sub_object.core_data, sqlite3_column_blob(stmt_select, 3), sub_object.core_data_size);
		}
		else {
			sub_object.core_data = NULL;
		}

		sub_object.module_data_size = sqlite3_column_bytes(stmt_select, 4);
		if (sub_object.module_data_size > 0) {
			sub_object.module_data = (unsigned char *)malloc(sub_object.module_data_size);
			if (sub_object.module_data == NULL) {
				sqlite3_reset(stmt_select);
				free(buffer);
				return -1;
			}
			memcpy(sub_object.module_data, sqlite3_column_blob(stmt_select, 4), sub_object.module_data_size);
		}
		else {
			sub_object.module_data = NULL;
		}

		sub_object.retranslator = NULL;
		sub_object.retranslator_module = NULL;

		if (sub_object.type >= OBJECT_TERMINAL_MIN) {

			sub_object.stream = storage_create_stream(sub_object.id, DEFAULT_STREAM_SIZE);

			create_retranslator(&sub_object);
		}
		else
			sub_object.stream = NULL;

		sub_object.position	= sqlite3_column_int(stmt_select, 5);

		db_index[sub_object.id] = sub_object;
	}

	for (std::map<unsigned int, DB_OBJECT>::iterator it = db_index.begin(); it != db_index.end(); ++it) {
		if (it->second.id != 0) {
			DB_OBJECT *o = db_get_object(it->second.parent_id);
			if (o != NULL) {
				o->sub_objects.push_back(&it->second);
			}
		}
	}

	for (std::map<unsigned int, DB_OBJECT>::iterator it = db_index.begin(); it != db_index.end(); ++it) {
		std::sort(it->second.sub_objects.begin(), it->second.sub_objects.end(), sort_function); 
	}

	sqlite3_reset(stmt_select);
	
	free(buffer);

	return 0;
}

int db_put_object(DB_OBJECT **o, DB_OBJECT *parent_object)
{
	int status;

	DB_OBJECT *object = *o;

	sqlite3_reset(stmt_insert);

	status = sqlite3_bind_int(stmt_insert, 1, parent_object->id);
	if (status != SQLITE_OK) {		
		log_printf("[DB] Unable to bind id field (%s)\r\n", sqlite3_errmsg(db));
		return DB_ERR;
	}

	status = sqlite3_bind_int(stmt_insert, 2, object->type);
	if (status != SQLITE_OK) {		
		log_printf("[DB] Unable to bind type field (%s)\r\n", sqlite3_errmsg(db));
		return DB_ERR;
	}

	status = sqlite3_bind_text(stmt_insert, 3, "", 0, SQLITE_STATIC);
	if (status != SQLITE_OK) {		
		log_printf("[DB] Unable to bind data field (%s)\r\n", sqlite3_errmsg(db));
		return DB_ERR;
	}

	status = sqlite3_bind_null(stmt_insert, 4);
	if (status != SQLITE_OK) {		
		log_printf("[DB] Unable to bind module_data field (%s)\r\n", sqlite3_errmsg(db));
		return DB_ERR;
	}

	sqlite3_exec(db, "BEGIN", 0, 0, 0);

	status = sqlite3_step(stmt_insert);
	sqlite3_reset(stmt_insert);
	if (status != SQLITE_DONE) {
		log_printf("[DB] Error on execution insert statement for object #%u (%s)\r\n", parent_object->id, sqlite3_errmsg(db));
		sqlite3_exec(db, "ROLLBACK", 0, 0, 0);
		return DB_ERR;
	}

	sqlite3_reset(stmt_id);

	if (sqlite3_step(stmt_id) != SQLITE_ROW) {
		log_printf("[DB] Error on execution stmt_id statement for object #%u (%s)\r\n", parent_object->id, sqlite3_errmsg(db));
		sqlite3_exec(db, "ROLLBACK", 0, 0, 0);
		sqlite3_reset(stmt_id);
		return DB_ERR;
	}

	object->id = sqlite3_column_int(stmt_id, 0);
	sqlite3_reset(stmt_id);

	sqlite3_reset(stmt_update);

	object->core_data_size += sprintf((char *)object->core_data + object->core_data_size, ",\"id\":%d}", object->id);

	status = sqlite3_bind_text(stmt_update, 1, (char *)object->core_data, object->core_data_size, SQLITE_STATIC);
	if (status != SQLITE_OK) {	
		log_printf("[DB] Unable to bind data field (%s)\r\n", sqlite3_errmsg(db));
		sqlite3_exec(db, "ROLLBACK", 0, 0, 0);
		sqlite3_reset(stmt_update);
		return DB_ERR;
	}

	if (object->module_data != NULL)
		status = sqlite3_bind_blob(stmt_update, 2, object->module_data, object->module_data_size, SQLITE_STATIC);
	else
		status = sqlite3_bind_null(stmt_update, 2);

	if (status != SQLITE_OK) {		
		log_printf("[DB] Unable to bind module_data field (%s)\r\n", sqlite3_errmsg(db));
		sqlite3_exec(db, "ROLLBACK", 0, 0, 0);
		sqlite3_reset(stmt_update);
		return DB_ERR;
	}

	sqlite3_bind_int(stmt_update, 3, object->type);
	if (status != SQLITE_OK) {		
		log_printf("[DB] Unable to bind type field (%s)\r\n", sqlite3_errmsg(db));
		sqlite3_exec(db, "ROLLBACK", 0, 0, 0);
		sqlite3_reset(stmt_update);
		return DB_ERR;
	}

	sqlite3_bind_int(stmt_update, 4, object->id);
	if (status != SQLITE_OK) {		
		log_printf("[DB] Unable to bind position field (%s)\r\n", sqlite3_errmsg(db));
		sqlite3_exec(db, "ROLLBACK", 0, 0, 0);
		sqlite3_reset(stmt_update);
		return DB_ERR;
	}

	sqlite3_bind_int(stmt_update, 5, object->id);
	if (status != SQLITE_OK) {		
		log_printf("[DB] Unable to bind id field (%s)\r\n", sqlite3_errmsg(db));
		sqlite3_exec(db, "ROLLBACK", 0, 0, 0);
		sqlite3_reset(stmt_update);
		return DB_ERR;
	}

	status = sqlite3_step(stmt_update);
	sqlite3_reset(stmt_update);
	if (status != SQLITE_DONE) {
		log_printf("[DB] Error on execution update statement for object #%u (%s)\r\n", object->id, sqlite3_errmsg(db));
		sqlite3_exec(db, "ROLLBACK", 0, 0, 0);
		return DB_ERR;
	}

	if (object->type >= OBJECT_TERMINAL_MIN)
		object->stream = storage_create_stream(object->id, DEFAULT_STREAM_SIZE);
	else
		object->stream = NULL;

	object->retranslator = NULL;
	object->retranslator_module = NULL;

	object->parent_id = parent_object->id;

	db_index[object->id] = *object;
	*o = db_get_object(object->id);

	parent_object->sub_objects.push_back(*o);

	sqlite3_exec(db, "COMMIT", 0, 0, 0);

	return 0;
}

int db_update_object(DB_OBJECT *object)
{
	int status;

	sqlite3_reset(stmt_update);

	if (object->core_data != NULL) 
		status = sqlite3_bind_text(stmt_update, 1, (char *)object->core_data, object->core_data_size, SQLITE_STATIC);
	else
		status = sqlite3_bind_null(stmt_update, 1);

	if (status != SQLITE_OK) {	
		log_printf("[DB] Unable to bind data field (%s)\r\n", sqlite3_errmsg(db));
		sqlite3_reset(stmt_update);
		return DB_ERR;
	}

	if (object->module_data != NULL)
		status = sqlite3_bind_blob(stmt_update, 2, object->module_data, object->module_data_size, SQLITE_STATIC);
	else
		status = sqlite3_bind_null(stmt_update, 2);

	if (status != SQLITE_OK) {		
		log_printf("[DB] Unable to bind module_data field (%s)\r\n", sqlite3_errmsg(db));
		sqlite3_reset(stmt_update);
		return DB_ERR;
	}

	status = sqlite3_bind_int(stmt_update, 3, object->type);
	if (status != SQLITE_OK) {		
		log_printf("[DB] Unable to bind type field (%s)\r\n", sqlite3_errmsg(db));
		sqlite3_reset(stmt_update);
		return DB_ERR;
	}

	status = sqlite3_bind_int(stmt_update, 4, object->position);
	if (status != SQLITE_OK) {		
		log_printf("[DB] Unable to bind position field (%s)\r\n", sqlite3_errmsg(db));
		sqlite3_reset(stmt_update);
		return DB_ERR;
	}
	status = sqlite3_bind_int(stmt_update, 5, object->id);
	if (status != SQLITE_OK) {		
		log_printf("[DB] Unable to bind id field (%s)\r\n", sqlite3_errmsg(db));
		sqlite3_reset(stmt_update);
		return DB_ERR;
	}

	status = sqlite3_step(stmt_update);
	sqlite3_reset(stmt_update);

	if (status != SQLITE_DONE) {
		log_printf("[DB] Error on execution update statement for object #%u (%s)\r\n", object->id, sqlite3_errmsg(db));
		return DB_ERR;
	}

	if (object->type >= OBJECT_TERMINAL_MIN) {

		buffer = (unsigned char *)malloc(32 * 1024 * 1024);

		if ((object->retranslator != NULL)&&(object->retranslator_module != NULL)) {
			storage_set_retranslator(object->stream, NULL, NULL);
			((MODULE *)object->retranslator_module)->destroy_retranslator(object->retranslator);

			object->retranslator = NULL;
			object->retranslator_module = NULL;
		}

		create_retranslator(object);

		free(buffer);
	}

	return DB_OK;
}

int db_change_object_parent(DB_OBJECT *object, DB_OBJECT *new_parent)
{
	int status;

	DB_OBJECT *parent = new_parent;

	while (parent->id != 0) {
		if (parent->id == object->id)
			return DB_ERR;
		parent = db_get_object(parent->parent_id);
	}

	sqlite3_reset(stmt_change);

	status = sqlite3_bind_int(stmt_change, 1, new_parent->id);
	if (status != SQLITE_OK) {		
		log_printf("[DB] Unable to bind id field (%s)\r\n", sqlite3_errmsg(db));
		return DB_ERR;
	}

	status = sqlite3_bind_int(stmt_change, 2, object->id);
	if (status != SQLITE_OK) {		
		log_printf("[DB] Unable to bind id field (%s)\r\n", sqlite3_errmsg(db));
		return DB_ERR;
	}

	status = sqlite3_step(stmt_change);
	sqlite3_reset(stmt_change);

	if (status != SQLITE_DONE) {
		log_printf("[DB] Error on execution change parent statement for object #%u (%s)\r\n", object->id, sqlite3_errmsg(db));
		return DB_ERR;
	}

	std::map<unsigned int, DB_OBJECT>::iterator i;

	if ((i = db_index.find(object->parent_id)) != db_index.end()) {
		for (size_t k = i->second.sub_objects.size(); k--;) {
			if (i->second.sub_objects[k] == object) {
				i->second.sub_objects.erase(i->second.sub_objects.begin() + k);
				break;
			}
		}
	}

	new_parent->sub_objects.push_back(object);

	object->parent_id = new_parent->id;

	return DB_OK;
}

int db_update_object_module_data(DB_OBJECT *object)
{
	int status;

	sqlite3_reset(stmt_update_module_data);

	if (object->module_data != NULL)
		status = sqlite3_bind_blob(stmt_update_module_data, 1, object->module_data, object->module_data_size, SQLITE_STATIC);
	else
		status = sqlite3_bind_null(stmt_update_module_data, 1);

	if (status != SQLITE_OK) {		
		log_printf("[DB] Unable to bind module_data field (%s)\r\n", sqlite3_errmsg(db));
		return DB_ERR;
	}

	status = sqlite3_bind_int(stmt_update_module_data, 2, object->id);
	if (status != SQLITE_OK) {		
		log_printf("[DB] Unable to bind id field (%s)\r\n", sqlite3_errmsg(db));
		return DB_ERR;
	}

	status = sqlite3_step(stmt_update_module_data);
	sqlite3_reset(stmt_update_module_data);

	if (status != SQLITE_DONE) {
		log_printf("[DB] Error on execution update_module_data statement for object #%u (%s)\r\n", object->id, sqlite3_errmsg(db));
		return DB_ERR;
	}

	return DB_OK;
}

int db_move_object(DB_OBJECT *object, DB_OBJECT *new_parent, DB_OBJECT *next_sibling, DB_OBJECT *prev_sibling)
{
	std::map<unsigned int, DB_OBJECT>::iterator i;

	// append
	if ((object->parent_id == new_parent->id)&&(next_sibling == NULL)&&(prev_sibling == NULL)) {

		if ((i = db_index.find(object->parent_id)) != db_index.end()) {
			for (size_t k = i->second.sub_objects.size(); k--;) {
				if (i->second.sub_objects[k] == object) {
					i->second.sub_objects.erase(i->second.sub_objects.begin() + k);
					i->second.sub_objects.push_back(object);
					break;
				}
			}
		}
	}
	else {
		if (object->parent_id != new_parent->id) {

			DB_OBJECT *parent = new_parent;

			while (parent->id != 0) {
				if (parent->id == object->id)
					return DB_ERR;
				parent = db_get_object(parent->parent_id);
			}

			if ((i = db_index.find(object->parent_id)) != db_index.end()) {
				for (size_t k = i->second.sub_objects.size(); k--;) {
					if (i->second.sub_objects[k] == object) {
						i->second.sub_objects.erase(i->second.sub_objects.begin() + k);
						break;
					}
				}
			}

			new_parent->sub_objects.push_back(object);

			object->parent_id = new_parent->id;
		}
	 
		// before
		if (next_sibling != NULL) {

			std::map<unsigned int, DB_OBJECT>::iterator i;

			if ((i = db_index.find(object->parent_id)) != db_index.end()) {

				for (size_t object_index = i->second.sub_objects.size(); object_index--;) {
				
					if (i->second.sub_objects[object_index] == object) {

						i->second.sub_objects.erase(i->second.sub_objects.begin() + object_index);
						
						for (size_t next_sibling_index = i->second.sub_objects.size(); next_sibling_index--;) {

							if (i->second.sub_objects[next_sibling_index] == next_sibling) {

								i->second.sub_objects.insert(i->second.sub_objects.begin() + next_sibling_index, object);

								break;
							}
						}

						break;
					}
				}
			}
		}

		// after
		if (prev_sibling != NULL) {

			std::map<unsigned int, DB_OBJECT>::iterator i;

			if ((i = db_index.find(object->parent_id)) != db_index.end()) {

				for (size_t object_index = i->second.sub_objects.size(); object_index--;) {
				
					if (i->second.sub_objects[object_index] == object) {

						i->second.sub_objects.erase(i->second.sub_objects.begin() + object_index);
						
						for (size_t prev_sibling_index = i->second.sub_objects.size(); prev_sibling_index--;) {

							if (i->second.sub_objects[prev_sibling_index] == prev_sibling) {

								if (prev_sibling_index == i->second.sub_objects.size() - 1) {
									i->second.sub_objects.push_back(object);
								}
								else {
									i->second.sub_objects.insert(i->second.sub_objects.begin() + prev_sibling_index + 1, object);
								}

								break;
							}
						}

						break;
					}
				}
			}
		}
	}

	if ((i = db_index.find(object->parent_id)) != db_index.end()) {

		for (size_t k = i->second.sub_objects.size(); k--;) {

			i->second.sub_objects[k]->position = k;

			sqlite3_reset(stmt_change);

			sqlite3_bind_int(stmt_change, 1, i->second.sub_objects[k]->parent_id);
			sqlite3_bind_int(stmt_change, 2, i->second.sub_objects[k]->position);
			sqlite3_bind_int(stmt_change, 3, i->second.sub_objects[k]->id);

			sqlite3_step(stmt_change);

			sqlite3_reset(stmt_change);
		}
	}

	return DB_OK;
}

static int db_delete_object_from_db(DB_OBJECT *object)
{
	int status;

	for (std::map<unsigned int, DB_OBJECT>::iterator it = db_index.begin(); it != db_index.end(); ++it) {
		if (it->second.parent_id == object->id) {
			status = db_delete_object_from_db(&it->second);
			if (status != DB_OK)
				return status;
		}
	}

	sqlite3_reset(stmt_delete);

	status = sqlite3_bind_int(stmt_delete, 1, object->id);

	if (status != SQLITE_OK) {
		
		log_printf("[DB] Unable to bind id field (%s)\r\n", sqlite3_errmsg(db));

		return DB_ERR;
	}

	status = sqlite3_step(stmt_delete);

	sqlite3_reset(stmt_delete);

	if (status != SQLITE_DONE) {

		log_printf("[DB] Error on execution delete statement for object #%u (%s)\r\n", object->id, sqlite3_errmsg(db));

		return DB_ERR;
	}

	return DB_OK;
}

static int db_delete_object_from_index(DB_OBJECT *object)
{
	for (std::map<unsigned int, DB_OBJECT>::iterator it = db_index.begin(); it != db_index.end(); ) {
		if (it->second.parent_id == object->id) {
			db_delete_object_from_index(&it->second);
			it = db_index.begin();
		}
		else {
			it++;
		}
	}

	if (object->module_data != NULL) 
		free(object->module_data);
	
	if (object->core_data != NULL) 
		free(object->core_data);

	if (object->stream != NULL) {
		storage_destroy_stream(object->stream);
	}

	if ((object->retranslator != NULL)&&(object->retranslator_module != NULL)) {
		((MODULE *)object->retranslator_module)->destroy_retranslator(object->retranslator);
	}

	std::map<unsigned int, DB_OBJECT>::iterator i;

	if ((i = db_index.find(object->parent_id)) != db_index.end()) {
		for (size_t k = i->second.sub_objects.size(); k--;) {
			if (i->second.sub_objects[k] == object) {
				i->second.sub_objects.erase(i->second.sub_objects.begin() + k);
				break;
			}
		}
	}
	
	db_index.erase(object->id);

	return 0;
}

int db_delete_object(DB_OBJECT *object)
{
	sqlite3_exec(db, "BEGIN", 0, 0, 0);

	int res = db_delete_object_from_db(object);

	if (res != DB_OK) {
		sqlite3_exec(db, "ROLLBACK", 0, 0, 0);
		return res;
	}

	sqlite3_exec(db, "COMMIT", 0, 0, 0);

	db_delete_object_from_index(object);

	return 0;
}

int db_open(const char *path, const char *p)
{
	int status;

	password = p;

	status = sqlite3_open(path, &db);

	if (status != SQLITE_OK) {

        	log_printf("[DB] Unable to open database '%s' %s\r\n", path, (db) ? sqlite3_errmsg(db) : "");

        	return DB_ERR;
	}

	status = sqlite3_prepare_v2(db, "select object_id, type_id, parent_id, data, blob, position from object order by position", -1, &stmt_select, NULL);

	if (status != SQLITE_OK) {

		log_printf("[DB] Unable to prepare select statement (%s)\r\n", sqlite3_errmsg(db));

		stmt_select = NULL;

		return DB_ERR;
	}

	status = sqlite3_prepare_v2(db, "insert into object (parent_id, type_id, data, blob) values (?,?,?,?)", -1, &stmt_insert, NULL);

	if (status != SQLITE_OK) {

		log_printf("[DB] Unable to prepare insert statement (%s)\r\n", sqlite3_errmsg(db));

		stmt_insert = NULL;

		return DB_ERR;
	}

	status = sqlite3_prepare_v2(db, "update object set data = ?, blob = ?, type_id = ?, position = ? where object_id = ?", -1, &stmt_update, NULL);

	if (status != SQLITE_OK) {

		log_printf("[DB] Unable to prepare update statement (%s)\r\n", sqlite3_errmsg(db));

		stmt_update = NULL;

		return DB_ERR;
	}

	status = sqlite3_prepare_v2(db, "update object set blob = ? where object_id = ?", -1, &stmt_update_module_data, NULL);

	if (status != SQLITE_OK) {

		log_printf("[DB] Unable to prepare update_module_data statement (%s)\r\n", sqlite3_errmsg(db));

		stmt_update_module_data = NULL;

		return DB_ERR;
	}

	status = sqlite3_prepare_v2(db, "delete from object where object_id = ?", -1, &stmt_delete, NULL);

	if (status != SQLITE_OK) {

		log_printf("[DB] Unable to prepare delete statement (%s)\r\n", sqlite3_errmsg(db));

		stmt_delete = NULL;

		return DB_ERR;
	}

	status = sqlite3_prepare_v2(db, "SELECT last_insert_rowid()", -1, &stmt_id, NULL);
	if (status != SQLITE_OK) {

		log_printf("[DB] Unable to prepare stmt_id (%s)\r\n", sqlite3_errmsg(db));

		stmt_id = NULL;

		return DB_ERR;
	}

	status = sqlite3_prepare_v2(db, "update object set parent_id = ?, position = ? where object_id = ?", -1, &stmt_change, NULL);

	if (status != SQLITE_OK) {

		log_printf("[DB] Unable to prepare change parent statement (%s)\r\n", sqlite3_errmsg(db));

		stmt_change = NULL;

		return DB_ERR;
	}

	int ac = sqlite3_get_autocommit(db);
	log_printf("[DB] Autocommit is %d\r\n", ac);

	return 0;
}

void db_close()
{
	for (std::map<unsigned int, DB_OBJECT>::iterator it = db_index.begin(); it != db_index.end(); ++it) {

		if (it->second.core_data != NULL)
			free(it->second.core_data);

		if (it->second.module_data != NULL)
			free(it->second.module_data);
	}

	db_index.clear();

	if (stmt_select != NULL) {
		sqlite3_finalize(stmt_select);
		stmt_select = NULL;
	}

	if (stmt_insert != NULL) {
		sqlite3_finalize(stmt_insert);
		stmt_insert = NULL;
	}

	if (stmt_update != NULL) {
		sqlite3_finalize(stmt_update);
		stmt_update = NULL;
	}

	if (stmt_update_module_data != NULL) {
		sqlite3_finalize(stmt_update_module_data);
		stmt_update_module_data = NULL;
	}

	if (stmt_delete != NULL) {
		sqlite3_finalize(stmt_delete);
		stmt_delete = NULL;
	}

	if (stmt_id != NULL) {
		sqlite3_finalize(stmt_id);
		stmt_id = NULL;
	}

	if (stmt_change != NULL) {
		sqlite3_finalize(stmt_change);
		stmt_change = NULL;
	}

	if (db != NULL) {
		sqlite3_close(db);
		db = NULL;
	}
}

const char *db_get_error()
{
	return sqlite3_errmsg(db);
}

int db_enum_objects(ENUM_OBJECTS_CALLBACK callback, void *ctx)
{
	for (std::map<unsigned int, DB_OBJECT>::iterator it = db_index.begin(); it != db_index.end(); ++it)
		if (callback(&it->second, ctx))
			return -1;

	return 0;
}

DB_OBJECT *db_get_object(unsigned int id)
{
	std::map<unsigned int, DB_OBJECT>::iterator i;

	if ((i = db_index.find(id)) != db_index.end())
		return &i->second;

	return NULL;		
}

// End