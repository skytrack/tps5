//******************************************************************************
//
// File Name : events.h
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#ifndef _EVENTS_H

#define _EVENTS_H

int on_object_update(DB_OBJECT *object);
int on_object_remove(DB_OBJECT *object);
int on_object_create(DB_OBJECT *object);
int on_timer();

#endif