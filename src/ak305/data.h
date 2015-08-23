//******************************************************************************
//
// File Name : data.h
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#ifndef _DATA_H

#define _DATA_H

#include "ak305.h"

typedef struct _SESSION{

	char			module_data[64];

	unsigned char	nCommandState;
	unsigned char	nNegState;
	unsigned char	SessionKey[16];
	unsigned char	buffer[16];
	unsigned char	dev_id[16];

	unsigned char	*ptr;
	unsigned short	nDataBytesReceived;
	unsigned char	nTransactionStatus;

	AK300COMMAND 	cmd;
	
	FILE			*f;
	void			*terminal;
} SESSION;

SESSION *data_session_open();
int data_session_data(SESSION *s, unsigned char **p, size_t *l);
void data_session_close(SESSION *s);
int data_session_timer(SESSION *s, char **p, size_t *l);

#endif
