//******************************************************************************
//
// File Name : device.h
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#ifndef _DEVICE_H

#define _DEVICE_H

int device_process_put_keys(HTTP_SESSION *s);
int device_process_post_keys(HTTP_SESSION *s);
int device_get_settings(HTTP_SESSION *s, unsigned int id, unsigned char **d, size_t *l);
int device_post_settings(HTTP_SESSION *s, unsigned int id, unsigned char **d, size_t *l);
int device_post_sensors(HTTP_SESSION *s, unsigned int id, unsigned char **d, size_t *l);
int device_post_llsleft(HTTP_SESSION *s, unsigned int id, unsigned char **d, size_t *l);
int device_post_llsright(HTTP_SESSION *s, unsigned int id, unsigned char **d, size_t *l);
int device_post_fuel(HTTP_SESSION *s, unsigned int id, unsigned char **d, size_t *l);
int device_post_retranslator(HTTP_SESSION *s, unsigned int id, unsigned char **d, size_t *l);
int devices(HTTP_SESSION *s, unsigned int id, unsigned char **d, size_t *l);
int device_put(HTTP_SESSION *s, unsigned int id, unsigned char **d, size_t *l);
int device_post(HTTP_SESSION *s, DB_OBJECT *object, unsigned char **d, size_t *l);
int device_post_modify(HTTP_SESSION *s, unsigned int id, unsigned char **d, size_t *l);
int device_init();

#endif

// End
