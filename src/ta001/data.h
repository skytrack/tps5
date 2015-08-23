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

typedef struct _SESSION{

	char			module_data[64];

	unsigned char	nCommandState;
	int				nMsgNum;
	unsigned int	nCmdLen;
	int				nCmdType;
	unsigned int	nBytesReceived;
	char			nData[255];

	unsigned char	flags;
	unsigned int	latitude;
	unsigned int	longitude;
	unsigned short	speed;
	unsigned short	altitude;
	unsigned short	cog;

	unsigned char	nav_valid;
	unsigned char	alt_valid;

	unsigned int	t;

	void			*terminal;
} SESSION;

SESSION *data_session_open();
int data_session_data(SESSION *s, unsigned char **p, size_t *l);
void data_session_close(SESSION *s);
int data_session_timer(SESSION *s, char **p, size_t *l);

#endif
