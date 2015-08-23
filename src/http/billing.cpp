//******************************************************************************
//
// File Name : billing.cpp
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#include <queue>
#include <vector>
#include <stdlib.h>
#include <string.h>

#include "api.h"
#include "billing.h"
#include "response.h"
#include "../core/record.h"
#include "../core/sqlite3.h"

extern std::string billing;

static std::queue<BILLING_JOB> job_queue;
static BILLING_JOB job;

typedef struct tagBILLING_DB {

	sqlite3 *db;
	sqlite3_stmt *stmt_insert;

} BILLING_DB;

static int do_job(BILLING_JOB *job, BILLING_DB *db)
{
	int result;

	switch (job->code) {
	default:
		result = 500;
		break;
	}

	if (job->session->zero_init.keep_alive) {

		memset(&job->session->zero_init, 0, sizeof(job->session->zero_init));

		http_parser_init(&job->session->parser, HTTP_REQUEST);

		job->session->parser.data = job->session;		
	}
	else {
		api_close_tcp(job->session);
	}

	return 0;
}

static BILLING_DB *billing_init_db() {
	
	BILLING_DB *db = (BILLING_DB *)malloc(sizeof(BILLING_DB));

	if (db == NULL) {
        api_log_printf("[BILLING] Unable to allocate db struct\r\n");
		return NULL;
	}

	memset(db, 0, sizeof(BILLING_DB));

	int status;

	for (;;) {

		status = sqlite3_open(billing.c_str(), &db->db);

		if (status != SQLITE_OK) {

        	api_log_printf("[BILLING] Unable to open database '%s' %s\r\n", billing.c_str(), (db) ? sqlite3_errmsg(db->db) : "");

			break;
		}

		return db;
	}

	free(db);

	return NULL;
}

static void billing_free_db(BILLING_DB *db) {

	if (db->db != NULL)
		sqlite3_close(db->db);

	free(db);
}

#ifndef _MSC_VER

#include <pthread.h>
#include <atomic>
#include <unistd.h>
#include <signal.h>

static pthread_mutex_t job_queue_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t job_queue_cond = PTHREAD_COND_INITIALIZER;
 
static pthread_t t;

static void *billing_proc(void *param)
{
	api_log_printf("[BILLING] Thread is running\r\n");

	BILLING_DB *db = billing_init_db();

	for (; db != NULL;) {

		pthread_mutex_lock(&job_queue_mutex);
	
		while (job_queue.empty())
			pthread_cond_wait(&job_queue_cond, &job_queue_mutex);

		BILLING_JOB job = job_queue.front();
		job_queue.pop();

		api_log_printf("[BILLING] Thread got job %d\r\n", job.code);

		pthread_mutex_unlock(&job_queue_mutex);
		
		if (job.code == 0)
			break;

		do_job(&job, db);
	}

	if (db != NULL)
		billing_free_db(db);

	api_log_printf("[BILLING] Thread stopped\r\n");

	return NULL;
}

void billing_add_job(BILLING_JOB *job)
{
	pthread_mutex_lock(&job_queue_mutex);
	
	job_queue.push(*job);
	
	pthread_mutex_unlock(&job_queue_mutex);
	
	pthread_cond_signal(&job_queue_cond);
}
 
void billing_init()
{
	api_log_printf("[BILLING] Running thread\r\n");

	sigset_t sigset, oldset;
	sigemptyset(&sigset);
	sigaddset(&sigset, SIGINT);
	pthread_sigmask(SIG_BLOCK, &sigset, &oldset);

	pthread_create(&t, NULL, billing_proc, NULL);

	// Restore the old signal mask only for this thread.
	pthread_sigmask(SIG_SETMASK, &oldset, NULL);

}

void billing_destroy()
{
	api_log_printf("[BILLING] Terminating thread\r\n");

	job.code = 0;
	billing_add_job(&job);

	pthread_join(t, NULL);
}

#else

static CRITICAL_SECTION CriticalSection;
static CONDITION_VARIABLE ConditionVariable;

HANDLE thread;

static DWORD WINAPI billing_proc(LPVOID lpThreadParameter)
{
	api_log_printf("[BILLING] Thread is running\r\n");

	BILLING_DB *db = billing_init_db();

	for (; db != NULL;) {
	
		EnterCriticalSection(&CriticalSection);

		while (job_queue.size() == 0)
            SleepConditionVariableCS(&ConditionVariable, &CriticalSection, INFINITE);

        BILLING_JOB job = job_queue.front();
        job_queue.pop();

        LeaveCriticalSection(&CriticalSection);

		api_log_printf("[BILLING] Thread got job %d\r\n", job.code);

		if (job.code == 0)
			break;

		do_job(&job, db);
	}

	if (db != NULL)
		billing_free_db(db);

	api_log_printf("[BILLING] Thread stopped\r\n");

	return 0;
}

void billing_init()
{
	api_log_printf("[BILLING] Running thread\r\n");

	InitializeCriticalSection(&CriticalSection);
	InitializeConditionVariable(&ConditionVariable);

	thread = CreateThread(NULL, 0, billing_proc, NULL, 0, NULL);
}

void billing_destroy()
{
	job.code = 0;

	billing_add_job(&job);

	WaitForSingleObject(thread, INFINITE);
	CloseHandle(thread);

	DeleteCriticalSection(&CriticalSection);
}

void billing_add_job(BILLING_JOB *job)
{
	EnterCriticalSection(&CriticalSection);

	job_queue.push(*job);

	LeaveCriticalSection(&CriticalSection);

	WakeConditionVariable(&ConditionVariable); 
}

#endif

// End
