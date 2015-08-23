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

int data(unsigned char **p, size_t *l, void *ctx, size_t ctx_len);
void construct_command(TERMINAL *terminal, size_t payload_len);
void construct_profile(TERMINAL *terminal);

#endif
