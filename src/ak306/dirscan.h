//******************************************************************************
//
// File Name : dirscan.h
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#ifndef _DIRSCAN_H

#define _DIRSCAN_H

typedef int (*DIRSCAN_CALLBACK)(const char *file_name, void *ctx);

int dirscan(const char *dir_path, DIRSCAN_CALLBACK callback, void *ctx);

#endif
