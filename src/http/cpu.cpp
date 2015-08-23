//******************************************************************************
//
// File Name : cpu.cpp
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#include "cpu.h"

#ifdef _MSC_VER
#include <intrin.h>
#endif

static CPUINFO cpuinfo;

void cpuID(unsigned i, unsigned regs[4]) {
#ifdef _WIN32
  __cpuid((int *)regs, (int)i);

#else
  asm volatile
    ("cpuid" : "=a" (regs[0]), "=b" (regs[1]), "=c" (regs[2]), "=d" (regs[3])
     : "a" (i), "c" (0));
  // ECX is set to zero for CPUID function 4
#endif
}

CPUINFO *cpu_get_info()
{
	unsigned regs[4];

	// Get vendor
	char vendor[12];
	cpuID(0, regs);
	((unsigned *)vendor)[0] = regs[1]; // EBX
	((unsigned *)vendor)[1] = regs[3]; // EDX
	((unsigned *)vendor)[2] = regs[2]; // ECX
	cpuinfo.vendor = std::string(vendor, 12);

	// Get CPU features
	cpuID(1, regs);
	unsigned cpuFeatures = regs[3]; // EDX

	// Logical core count per CPU
	cpuID(1, regs);
	cpuinfo.logical_count = (regs[1] >> 16) & 0xff; // EBX[23:16]

	if (cpuinfo.vendor == "GenuineIntel") {
		// Get DCP cache info
		cpuID(4, regs);
		cpuinfo.cores_count = ((regs[0] >> 26) & 0x3f) + 1; // EAX[31:26] + 1
	} 
	else 
	if (cpuinfo.vendor == "AuthenticAMD") {
		// Get NC: Number of CPU cores - 1
		cpuID(0x80000008, regs);
		cpuinfo.cores_count = ((unsigned)(regs[2] & 0xff)) + 1; // ECX[7:0] + 1
	}

	// Detect hyper-threads  
	cpuinfo.hyper_threads = cpuFeatures & (1 << 28) && cpuinfo.cores_count < cpuinfo.logical_count;

	return &cpuinfo;
}
