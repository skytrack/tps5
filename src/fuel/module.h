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

MODULE_API size_t fuel_process(	int *time, float *lat, float *lng, unsigned short *speed, float *fuel, size_t data_length, 
								int t_from, int t_to,
								float fill_threshold, float drain_threshold, float max_rate, size_t filter_length,
								unsigned char *buffer, size_t bytes_left, unsigned int flags);

#ifdef __cplusplus
};
#endif /* __cplusplus */

#endif

// End


