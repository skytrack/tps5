//******************************************************************************
//
// File Name	: module.h
// Author	: Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#ifndef _MODULET_H

#define _MODULET_H

#include <stdlib.h>
#include <string>
#include "storage.h"
#include "db.h"

#ifndef _MSC_VER
	#define MODULE_HANDLE void *
#else
	#include <Windows.h>
	#define MODULE_HANDLE HMODULE
#endif

typedef enum { MODULE_FAMILY_ANY, MODULE_FAMILY_API = 0, MODULE_FAMILY_DEVICE, MODULE_FAMILY_RETRANSLATOR } MODULE_FAMILY;

// ANY MODULE EXPORT FUNCTIONS
typedef int (*GETVAR)(int var, void **p);
typedef int (*SETVAR)(int var, void *p);
typedef int (*START)();
typedef int (*STOP)();

// DEVICE MODULE EXPORT FUNCTIONS

#define MODULE_VAR_FUNC_CONFIG_GET_JSON			2000
#define MODULE_VAR_FUNC_CONFIG_PUT_JSON			2001
#define MODULE_VAR_FUNC_GET_DEVICE_CAPS			2002
#define MODULE_VAR_FUNC_ON_OBJECT_CREATE		2004
#define MODULE_VAR_FUNC_ON_OBJECT_REMOVE		2005
#define MODULE_VAR_FUNC_ON_OBJECT_UPDATE		2006
#define MODULE_VAR_FUNC_ON_TIMER				2003
#define MODULE_VAR_FUNC_GET_ERROR				2007
#define MODULE_VAR_FUNC_GET_INFO				2008

typedef int (*CONFIG_GET_JSON)(DB_OBJECT *object, unsigned char *json, size_t *len);
typedef int (*CONFIG_PUT_JSON)(DB_OBJECT *object, unsigned char *json, size_t len);
typedef int (*GET_DEVICE_CAPS)(DB_OBJECT *object, unsigned long long *result);
typedef int (*ON_OBJECT_CREATE)(DB_OBJECT *object);
typedef int (*ON_OBJECT_REMOVE)(DB_OBJECT *object);
typedef int (*ON_OBJECT_UPDATE)(DB_OBJECT *object);
typedef int (*ON_TIMER)();
typedef unsigned char *(*GET_ERROR)(size_t *len);
typedef int (*GET_INFO)(DB_OBJECT *object, unsigned char *json, size_t len);

// RETRANSLATOR MODULE EXPORT FUNCTIONS

typedef struct retranslator_record 
{
	unsigned int	t;
	int				latitude;
	int				longitude;
	short			altitude;
	unsigned short	speed;
	unsigned short	cog;
	unsigned char	flags1;
	unsigned char	flags2;
	unsigned int	id;
	unsigned int	nupe;
} RETRANSLATOR_RECORD;

#define MODULE_VAR_FUNC_CREATE_RETRANSLATOR				3000
#define MODULE_VAR_FUNC_DESTROY_RETRANSLATOR			3001
#define MODULE_VAR_FUNC_ADD_RECORD_TO_RETRANSLATOR		3002

typedef void *(*CREATE_RETRANSLATOR)(const char *device_id, const char *host, unsigned short port, const char *login, const char *password);
typedef void *(*DESTROY_RETRANSLATOR)(void *pRetranslator);
typedef void *(*ADD_RECORD_TO_RETRANSLATOR)(void *pRetranslator, RETRANSLATOR_RECORD *pRecord);

typedef struct module
{
	MODULE_FAMILY family;
	std::string name;
	
	std::string path;
	
	MODULE_HANDLE handle;
	int *terminal_type;

	GETVAR get_var;
	SETVAR set_var;
	START start;
	STOP stop;

	CONFIG_GET_JSON			config_get_json;
	CONFIG_PUT_JSON			config_put_json;
	GET_DEVICE_CAPS			config_get_device_caps;
	GET_ERROR				config_get_error;
	GET_INFO				get_info;

	ON_TIMER timer;
	ON_OBJECT_CREATE		on_object_create;
	ON_OBJECT_REMOVE		on_object_remove;
	ON_OBJECT_UPDATE		on_object_update;

	CREATE_RETRANSLATOR			create_retranslator;
	DESTROY_RETRANSLATOR		destroy_retranslator;
	ADD_RECORD_TO_RETRANSLATOR	add_record_to_retranslator;

	int *protocol_id;
} MODULE;

#define TERMINAL_CAPS_IGNITION         0x00000001
#define TERMINAL_CAPS_MOVE             0x00000002
#define TERMINAL_CAPS_ENGINE           0x00000004
#define TERMINAL_CAPS_ALARM            0x00000008
#define TERMINAL_CAPS_DI1              0x00000010
#define TERMINAL_CAPS_DI2              0x00000020
#define TERMINAL_CAPS_DI3              0x00000040
#define TERMINAL_CAPS_DI4              0x00000080
#define TERMINAL_CAPS_ADC1             0x00000100
#define TERMINAL_CAPS_ADC2             0x00000200
#define TERMINAL_CAPS_ADC3             0x00000400
#define TERMINAL_CAPS_ADC4             0x00000800
#define TERMINAL_CAPS_FREQ1            0x00001000
#define TERMINAL_CAPS_FREQ2            0x00002000
#define TERMINAL_CAPS_FREQ3            0x00004000
#define TERMINAL_CAPS_FREQ4            0x00008000
#define TERMINAL_CAPS_COUNTER1         0x00010000
#define TERMINAL_CAPS_COUNTER2         0x00020000
#define TERMINAL_CAPS_COUNTER3         0x00040000
#define TERMINAL_CAPS_COUNTER4         0x00080000
#define TERMINAL_CAPS_LLS1             0x00100000
#define TERMINAL_CAPS_LLS2             0x00200000
#define TERMINAL_CAPS_LLS3             0x00400000
#define TERMINAL_CAPS_LLS4             0x00800000
#define TERMINAL_CAPS_VCC              0x01000000
#define TERMINAL_CAPS_ACC              0x02000000
#define TERMINAL_CAPS_AUDIO            0x04000000

#define MODULE_RESULT_FLAG_OK		0x01
#define MODULE_RESULT_FLAG_ERROR	0x02
#define MODULE_RESULT_FLAG_DATA		0x04
#define MODULE_RESULT_FLAG_LOG		0x08
#define MODULE_RESULT_FLAG_TX		0x10
#define MODULE_RESULT_FLAG_CLOSE	0x20
#define MODULE_RESULT_FLAG_ID		0x40
#define MODULE_RESULT_FLAG_NEWSESSION   0x80

#define MODULE_VAR_FUNC_LOG		1
#define MODULE_VAR_FUNC_LISTEN_TCP      2
#define MODULE_VAR_FUNC_TIMER           3
#define MODULE_VAR_FUNC_SEND_TCP        4
#define MODULE_VAR_FUNC_CLOSE_TCP       5
#define MODULE_VAR_FUNC_LISTEN_UDP      6
#define MODULE_VAR_FUNC_SEND_UDP        7

#define MODULE_VAR_FAMILY				100
#define MODULE_VAR_NAME					101
#define MODULE_VAR_TERMINAL_TYPE        102
#define MODULE_VAR_TERMINAL_NAME        103
#define MODULE_VAR_OPTION               104
#define MODULE_VAR_TAG                  105
#define MODULE_VAR_SETTINGSFORM         106
#define MODULE_VAR_PROTOCOL_ID          107


#define MODULE_VAR_FUNC_STORAGE_CREATE_STREAM				3000
#define MODULE_VAR_FUNC_STORAGE_DESTROY_STREAM				3001
#define MODULE_VAR_FUNC_STORAGE_GET_STREAM_BY_ID			3002
#define MODULE_VAR_FUNC_STORAGE_LOCK_STREAM					3003
#define MODULE_VAR_FUNC_STORAGE_UNLOCK_STREAM				3004
#define MODULE_VAR_FUNC_STORAGE_ADD_RECORD_TO_STREAM		3005
#define MODULE_VAR_FUNC_STORAGE_GET_STREAM_FIRST_RECORD		3006
#define MODULE_VAR_FUNC_STORAGE_GET_STREAM_RECORDS_COUNT	3007
#define MODULE_VAR_FUNC_STORAGE_SORT_STREAM					3008
#define MODULE_VAR_FUNC_STORAGE_TRIM_STREAM					3009
#define MODULE_VAR_FUNC_STORAGE_GET_STREAM_INFO				3010
#define MODULE_VAR_FUNC_STORAGE_UPDATE_RECORD				3011

#define MODULE_VAR_FUNC_DB_GET_OBJECT				1000
#define MODULE_VAR_FUNC_DB_PUT_OBJECT				1001
#define MODULE_VAR_FUNC_DB_DELETE_OBJECT			1002
#define MODULE_VAR_FUNC_DB_UPDATE_OBJECT			1003
#define MODULE_VAR_FUNC_DB_UPDATE_OBJECT_BLOB		1004
#define MODULE_VAR_FUNC_DB_CHANGE_OBJECT_PARENT		1005
#define MODULE_VAR_FUNC_DB_GET_ERROR				1006
#define MODULE_VAR_FUNC_DB_ENUM_OBJECTS				1007
#define MODULE_VAR_FUNC_DB_MOVE_OBJECT				1008

#define MODULE_VAR_FUNC_FUEL_PROCESS				4000

#define MODULE_VAR_FUNC_ENUM_MODULES				129
#define MODULE_VAR_FUNC_GET_DEVICE_MODULE			131

#define MODULE_OPTION_DEFAULT_PROTOCOL	1
#define MODULE_OPTION_DEFAULT_PORT	2
#define MODULE_OPTION_SESSION_LEN       3
#define MODULE_OPTION_DATA_ADDRESS	4
#define MODULE_OPTION_DATA_LEN		5
#define MODULE_OPTION_LOG_ADDRESS	6
#define MODULE_OPTION_LOG_LEN		7
#define MODULE_OPTION_TX_ADDRESS	8
#define MODULE_OPTION_TX_LEN		9
#define MODULE_OPTION_RX_ADDRESS	10
#define MODULE_OPTION_RX_LEN		11
#define MODULE_OPTION_SESSION_DATA	12
#define MODULE_OPTION_TERMINAL_ID	13
#define MODULE_OPTION_TIMER_ADDRESS     14

typedef enum { MODULE_RESULT_OK, MODULE_RESULT_FAILED } MODULE_RESULT;

// SESSION 
#define SESSION_COMPLETE 0
typedef void *(*SESSION_OPEN)();
typedef void  (*SESSION_CLOSE)(void *session);
typedef int   (*SESSION_DATA)(void *session, char **data, size_t *len);
typedef int   (*SESSION_TIMER)(void *session, char **data, size_t *len);

typedef struct session_handlers
{
	SESSION_OPEN  open;
	SESSION_CLOSE close;
	SESSION_DATA  data;
	SESSION_TIMER timer;
	int sock;
	unsigned char *buffer;
	size_t buffer_size;
} SESSION_HANDLERS;

typedef int (*UDP_DATA)(unsigned char **data, size_t *data_len, void *ctx, size_t ctx_len);

typedef int (*LOG_PRINTF)( const char * format, ... );
typedef int (*DATA_APPEND)(void *tag, const unsigned char *data, int len);
typedef int (*DATA_HANDLER)(int sock, char closed);
typedef int (*LISTEN_TCP)(void *tag, const char *host, const char *port, const SESSION_HANDLERS *handlers);
typedef int (*LISTEN_UDP)(void *tag, const char *host, const char *port, UDP_DATA data_handler);
typedef int (*SEND_TCP)(void *session, unsigned char *data, size_t len);
typedef int (*SEND_UDP)(unsigned char *data, size_t len, void *context);
typedef int (*CLOSE_TCP)(void *session);


typedef void (*STORAGE_CREATE_STREAM)(int id, size_t capacity);
typedef void (*STORAGE_DESTROY_STREAM)(void *s);
typedef void* (*STORAGE_GET_STREAM_BY_ID)(int id);

typedef void (*STORAGE_LOCK_STREAM)(void *s);
typedef void (*STORAGE_UNLOCK_STREAM)(void *s);

typedef int (*STORAGE_ADD_RECORD_TO_STREAM)(void *s, STORAGE_RECORD_HEADER *rh, size_t len);
typedef STORAGE_RECORD_HEADER* (*STORAGE_GET_STREAM_FIRST_RECORD)(void *s);
typedef size_t (*STORAGE_GET_STREAM_RECORDS_COUNT)(void *s);
typedef void (*STORAGE_UPDATE_RECORD)(unsigned short id, STORAGE_RECORD_HEADER *rh, const char *field, void *value);

typedef void (*STORAGE_SORT_STREAM)(void *s, void *sort_buffer, std::vector<STORAGE_SORT_ITEM> *si);
typedef void (*STORAGE_TRIM_STREAM)(void *s, unsigned int base_time);
typedef STREAM_INFO *(*STORAGE_GET_STREAM_INFO)(void *s);

typedef const char* (*DB_GET_ERROR)();
typedef DB_OBJECT* (*DB_GET_ROOT)();
typedef DB_OBJECT* (*DB_GET_OBJECT)(unsigned int object_id);
typedef int (*DB_PUT_OBJECT)(DB_OBJECT **object, DB_OBJECT *parent_object);
typedef int (*DB_UPDATE_OBJECT)(DB_OBJECT *object);
typedef int (*DB_UPDATE_OBJECT_BLOB)(DB_OBJECT *object);
typedef int (*DB_DELETE_OBJECT)(DB_OBJECT *object);
typedef int (*DB_CHANGE_OBJECT_PARENT)(DB_OBJECT *object, DB_OBJECT *new_parent);
typedef int (*DB_MOVE_OBJECT)(DB_OBJECT *object, DB_OBJECT *new_parent, DB_OBJECT *next_sibling, DB_OBJECT *prev_sibling);
typedef int (*DB_ENUM_OBJECTS_CALLBACK)(DB_OBJECT *object, void *ctx);
typedef int (*DB_ENUM_OBJECTS)(DB_ENUM_OBJECTS_CALLBACK callback, void *ctx);

typedef int (*ENUM_MODULES_CALLBACK)(MODULE *module, void *ctx);
typedef int (*ENUM_MODULES)(MODULE_FAMILY family, ENUM_MODULES_CALLBACK callback, void *ctx);
typedef MODULE *(*GET_DEVICE_MODULE)(int device_type);

typedef size_t (*FUEL_PROCESS)(	int *time, float *lat, float *lng, unsigned short *speed, float *fuel, size_t data_length, 
								int t_from, int t_to,
								float fill_threshold, float drain_threshold, float max_rate, size_t filter_length,
								unsigned char *buffer, size_t bytes_left, unsigned int flags);

typedef struct config_option
{
	const char *name;
	const char *value;
} CONFIG_OPTION;

#ifdef EXPORT

#ifdef  __cplusplus
extern "C" {
#endif

int get_var(int var, void **p);
int set_var(int var, void *p);
int start();
int stop();

#ifdef  __cplusplus
}
#endif

#endif

#endif // _MODULE_H

// End
