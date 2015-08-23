//******************************************************************************
//
// File Name : user.h
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#ifndef _USER_H

#define _USER_H

int user_put(HTTP_SESSION *s, unsigned int id, unsigned char **d, size_t *l);
int user_post(HTTP_SESSION *s, DB_OBJECT *object, unsigned char **d, size_t *l);

#endif

// End
