//******************************************************************************
//
// File Name : module.h
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#ifndef _MODULE2_H

#define _MODULE2_H

#ifdef _MSC_VER
#ifdef HTTP_EXPORTS
	#define HTTP_API __declspec(dllexport)
#else
	#define HTTP_API __declspec(dllimport)
#endif
#else
#define HTTP_API
#endif

#ifdef __cplusplus
extern "C"
{
#endif

HTTP_API int get_var(int var_type, void **p);
HTTP_API int set_var(int var_type, void *p);
HTTP_API int start();
HTTP_API int stop();

#ifdef __cplusplus
};
#endif /* __cplusplus */

#endif

// End


