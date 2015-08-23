//******************************************************************************
//
// File Name: pla.h
// Author	: Skytrack ltd - Copyright (C) 2012
//
// This code is property of Skytrack company
//
//******************************************************************************

#ifndef _PLA_H

#define _PLA_H

#define PLA_FLAG_ENGINE 0x01

typedef void PLA_FUEL_CALLBACK(int nTime, float fFuelLevel, void *pData);
typedef void PLA_FILL_CALLBACK(int nTimeFrom, int nTimeTo, float fFuelFrom, float fFuelTo, void *pData);
typedef void PLA_DRAIN_CALLBACK(int nTimeFrom, int nTimeTo, float fFuelFrom, float fFuelTo, void *pData);

typedef void *PLA;

PLA PLA_Create();
void PLA_Destroy(PLA pHandle);

void PLA_SetBufferSize(PLA pHandle, int nBufferSize);

void PLA_SetFuelCallback(PLA pHandle, PLA_FUEL_CALLBACK *pCallback, void *pData);
void PLA_SetFillCallback(PLA pHandle, PLA_FILL_CALLBACK *pCallback, void *pData);
void PLA_SetDrainCallback(PLA pHandle, PLA_DRAIN_CALLBACK *pCallback, void *pData);
void PLA_SetFillThreshold(PLA pHandle, int nFillThreshold);
void PLA_SetDrainThreshold(PLA pHandle, int nDrainThreshold);
void PLA_SetMaxFuelRate(PLA pHandle, int nMaxFuelRate);

void PLA_ProcessFuel(PLA pHandle, int nTime, float fFuelLevel, int nFlags);

void PLA_GetData(void *pHandle, void **pData, int *pSize);
void PLA_SetData(void *pHandle, void *pData, int nSize);

int PLA_GetBufferItemsCount(void *pHandle);
int PLA_GetBufferStartTime(void *pHandle);
void PLA_SetBufferItem(void *pHandle, int iItem, int nTime, float fFuelLevel);


#endif

// End
