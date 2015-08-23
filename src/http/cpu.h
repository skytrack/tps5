//******************************************************************************
//
// File Name : cpu.h
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#ifndef _CPU_H

#define _CPU_H

#include <string>

typedef struct _tagCPUINFO
{
	std::string vendor;
	size_t cores_count;
	size_t logical_count;
	bool hyper_threads;
} CPUINFO;

CPUINFO *cpu_get_info();

#endif

// End