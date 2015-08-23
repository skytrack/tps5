//******************************************************************************
//
// File Name : billing.h
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#ifndef _BILLING_H

#define _BILLING_H

#include "http.h"

typedef struct tagBILLING_JOB
{
	unsigned char code;
	int sock;
	HTTP_SESSION *session;
	unsigned short user_id;
	unsigned short current_user_id;
} BILLING_JOB;

void billing_init();

void billing_destroy();

void billing_add_job(BILLING_JOB *job);

#endif

// End
