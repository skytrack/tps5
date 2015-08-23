//******************************************************************************
//
// File Name : almalloc.h
// Copyright : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#ifndef _ALIGNED_MALLOC

#define _ALIGNED_MALLOC

#if defined(_MSC_VER)
#define _ALIGNED(type) __declspec(align(32)) type
#else
#if defined(__GNUC__)
#define _ALIGNED(type) type __attribute__ ((aligned(32)))
#endif
#endif


static void * aligned_malloc(size_t alignment, size_t size)
{
	void *p;

#ifdef _MSC_VER
	p = _aligned_malloc(size, alignment);
#else
	if (posix_memalign(&p, alignment, size) != 0)
		return NULL;
#endif

	return p;
}

static void aligned_free(void *ptr)
{
#ifdef _MSC_VER
	_aligned_free(ptr);
#else
	free(ptr);
#endif
}

#endif

// End