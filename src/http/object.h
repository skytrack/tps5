//******************************************************************************
//
// File Name : object.h
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#ifndef _OBJECT_H

#define _OBJECT_H

int object_post(HTTP_SESSION *s, unsigned int id, unsigned char **d, size_t *l);
int object_get(HTTP_SESSION *s, unsigned int id, unsigned char **d, size_t *l);
int object_put_user(HTTP_SESSION *s, unsigned int id, unsigned char **d, size_t *l);
int object_put_device(HTTP_SESSION *s, unsigned int id, unsigned char **d, size_t *l);
int object_put_group(HTTP_SESSION *s, unsigned int id, unsigned char **d, size_t *l);
int object_delete(HTTP_SESSION *s, unsigned int id, unsigned char **d, size_t *l);
int object_report(HTTP_SESSION *s, DB_OBJECT *object, unsigned char **d, size_t *l);
int object_get_tree(HTTP_SESSION *s, unsigned int id, unsigned char **d, size_t *l);
int object_parent(HTTP_SESSION *s, unsigned int id, unsigned char **d, size_t *l);
int object_moveappend(HTTP_SESSION *s, unsigned int id, unsigned char **d, size_t *l);
int object_movebefore(HTTP_SESSION *s, unsigned int id, unsigned char **d, size_t *l);
int object_moveafter(HTTP_SESSION *s, unsigned int id, unsigned char **d, size_t *l);

#endif

// End
