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

#include "importer.h"

typedef struct tagDATA_SESSION
{
	char			module_data[40];

	unsigned char	pack_state;
	unsigned int	pack_bytes_received;

	unsigned char	record_state;
	unsigned char	record_bytes_received;

	unsigned int	counter;
	unsigned char	*ptr;

	IMPORT_PACK		pack;

} DATA_SESSION;

DATA_SESSION *data_session_open();
int data_session_data(DATA_SESSION *s, unsigned char **p, size_t *l);
void data_session_close(DATA_SESSION *s);
int data_session_timer(DATA_SESSION *s, char **p, size_t *l);

#endif
