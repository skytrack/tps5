//******************************************************************************
//
// File Name : module.h
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#ifndef _MODULE_H

#define _MODULE_H

#ifdef _MSC_VER
#ifdef MODULE_EXPORTS
	#define MODULE_API __declspec(dllexport)
#else
	#define MODULE_API __declspec(dllimport)
#endif
#else
#define MODULE_API
#endif

#ifdef __cplusplus
extern "C"
{
#endif

MODULE_API int get_var(int var_type, void **p);
MODULE_API int set_var(int var_type, void *p);
MODULE_API int start();
MODULE_API int stop();

#ifdef __cplusplus
};
#endif /* __cplusplus */

#endif

// End


