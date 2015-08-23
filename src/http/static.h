//******************************************************************************
//
// File Name : static.h
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#ifndef _STATIC_H

#define _STATIC_H

int static_handle_request(HTTP_SESSION *s, unsigned char **d, size_t *l);
void static_set_rootdir(const char *s);

#endif

// End
