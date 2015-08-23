//******************************************************************************
//
// File Name : group.h
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#ifndef _GROUP_H

#define _GROUP_H

int group_put(HTTP_SESSION *s, unsigned int id, unsigned char **d, size_t *l);
int group_post(HTTP_SESSION *s, DB_OBJECT *object, unsigned char **d, size_t *l);

#endif

// End
