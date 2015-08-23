//******************************************************************************
//
// File Name : aligned.h
// Copyright : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#ifndef _ALIGNED_H

#define _ALIGNED_H

#if defined(_MSC_VER)
#define _ALIGNED(type) __declspec(align(32)) type
#else
#if defined(__GNUC__)
#define _ALIGNED(type) type __attribute__ ((aligned(32)))
#endif
#endif

#endif

// End