//******************************************************************************
//
// File Name : spinlock.h
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#ifndef _SPINLOCK_H

#define _SPINLOCK_H

#ifndef _MSC_VER

#include <atomic>

#define spinlock_t std::atomic_flag

static void spinlock_lock(spinlock_t *sl) 
{
	while (sl->test_and_set(std::memory_order_acquire));
}

static void spinlock_unlock(spinlock_t *sl) 
{
	sl->clear(std::memory_order_release);
}

static void spinlock_init(spinlock_t *sl)
{
	sl->clear(std::memory_order_release);
}

#else

#include <windows.h>

#define spinlock_t volatile unsigned int 

static void spinlock_lock(spinlock_t *sl) 
{
	while (InterlockedCompareExchange(sl, 1, 0) != 0);
}

static void spinlock_unlock(spinlock_t *sl) 
{
	InterlockedExchange(sl, 0); 
}

static void spinlock_init(spinlock_t *sl)
{
	InterlockedExchange(sl, 0);
}

#endif

#endif

// End
