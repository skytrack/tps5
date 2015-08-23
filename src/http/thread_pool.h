//******************************************************************************
//
// File Name : thread_pool.h
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#ifndef _THREAD_POOL_H

#define _THREAD_POOL_H

#include "http.h"
#include "../core/fuel.h"

#define JOB_CODE_REPORT_MILEAGE		1
#define JOB_CODE_REPORT_FUEL		2
#define JOB_CODE_REPORT_IGNITION	3
#define JOB_CODE_REPORT_ENGINE		4
#define JOB_CODE_REPORT_MOVE		5
#define JOB_CODE_REPORT_PARK		6
#define JOB_CODE_REPORT_DI1			7
#define JOB_CODE_REPORT_DI2			8
#define JOB_CODE_REPORT_DI3			9
#define JOB_CODE_REPORT_DI4			10
#define JOB_CODE_REPORT_DI5			11
#define JOB_CODE_REPORT_DI6			12
#define JOB_CODE_REPORT_DI7			13
#define JOB_CODE_REPORT_DI8			14
#define JOB_CODE_REPORT_ACTIVITY	16
#define JOB_CODE_PDF				254
#define JOB_CODE_HISTORY_PNG		255
#define JOB_CODE_HISTORY_BIN		253

typedef struct tagTHREAD_POOL_JOB
{
	unsigned char code;
	int sock;
	unsigned int terminal_id;
	unsigned int tFrom;
	unsigned int tTo;
	unsigned int user_id;
	unsigned int flag;
	HTTP_SESSION *session;

	std::vector<FUEL_TABLE_RECORD> lls_table_left;
	std::vector<FUEL_TABLE_RECORD> lls_table_right;

	int lls_left_source;
	int lls_right_source;

	bool ignition;
	bool engine;
	bool move;

	char ignition_source;
	char engine_source;
	char move_source;

	int filter_level;
	int fill_threshold;
	int drain_threshold;
	int max_consumption;

	unsigned char *buffer;
	size_t buffer_size;

	bool injector;
	int injector_factor;

} THREAD_POOL_JOB;

void thread_pool_init(size_t num_of_threads);
void thread_pool_destroy();
void thread_pool_add_job(THREAD_POOL_JOB * job);

#endif

// End
