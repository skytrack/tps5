//******************************************************************************
//
// File Name : update.h
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#ifndef _UPDATE_H

#define _UPDATE_H

SESSION *update_session_open();
int update_session_data(SESSION *s, unsigned char **p, size_t *l);
void update_session_close(SESSION *s);
int update_session_timer(SESSION *s, char **p, size_t *l);

#endif
