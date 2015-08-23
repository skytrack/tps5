
//******************************************************************************
//
// File Name: pla.cpp
// Author	: Skytrack ltd - Copyright (C) 2012
//
// This code is property of Skytrack company
//
//******************************************************************************
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <cmath>  
#include "pla.h"
#include "../core/cross.h"

typedef struct tagPLA_POINT
{
	int		m_nTime;

	float	m_fFuelLevel;
	float	m_fRealFuel;

	float	m_fSumX;
	float	m_fSumY;
	float	m_fSumXY;
	float	m_fSumX2;

	float	a, b;
	float	m_fRMS;

	int		m_nFlags;
} PLA_POINT;

typedef struct tagPLA_DATA
{
	unsigned int	m_nID;

	int		m_nPointsCount;

	int		m_nTime;
	int		m_nBufferStartTime;

	float	m_fSumX;
	float	m_fSumY;
	float	m_fSumXY;
	float	m_fSumX2;

	float	a, b;
	float	m_fRMS;

	int		m_nSegmentLength;

	char m_bFirstSegment;

	float	m_PrevA;
	float	m_PrevB;
	int		m_nPrevTime;

	int		m_nPrevEndTime;
	float	m_fPrevEndFuel;

	char m_bFillInProgress;
	int		m_nFillStartTime;
	float	m_fFillStartFuel;
	int		m_nFillEndTime;
	float	m_fFillEndFuel;

	char	m_bDrainInProgress;
	int		m_nDrainStartTime;
	float	m_fDrainStartFuel;
	int		m_nDrainEndTime;
	float	m_fDrainEndFuel;

} PLA_DATA;

typedef struct tagPLA_HANDLE
{
	int		m_nBufferSize;

	PLA_POINT *m_pBuffer;

	PLA_FUEL_CALLBACK	*m_pFuelCallback;
	PLA_FILL_CALLBACK	*m_pFillCallback;
	PLA_DRAIN_CALLBACK	*m_pDrainCallback;

	void *m_pFuelCallbackData;
	void *m_pFillCallbackData;
	void *m_pDrainCallbackData;

	float m_fFillThreshold;
	float m_fDrainThreshold;
	float m_fMaxFuelRate;

	PLA_DATA data;

} PLA_HANDLE;

#ifdef _WIN32

void PLA_TracePoint(PLA_HANDLE *pPLA, PLA_POINT *pPoint, const char *szLabel)
{
	struct tm tms;
	time_t t;

	t = pPoint->m_nTime;
	localtime_r(&t, &tms);

	printf("%s: %02u.%02u.%04u %02u:%02u:%02u, %d, %f %f\r\n",
		szLabel, 
		tms.tm_mday, tms.tm_mon + 1, tms.tm_year + 1900, tms.tm_hour, tms.tm_min, tms.tm_sec,
		0, pPoint->m_fFuelLevel, pPoint->m_fRealFuel);
}

void PLA_TraceSegment(PLA_HANDLE *pPLA, const char *szLabel)
{
	struct tm tms1;
	struct tm tms2;

	time_t t;

	t = pPLA->data.m_nTime;
	localtime_r(&t, &tms1);

	t = pPLA->m_pBuffer[pPLA->data.m_nSegmentLength - 1].m_nTime;
	localtime_r(&t, &tms2);

	printf("%s: %02u.%02u.%04u %02u:%02u:%02u - %02u.%02u.%04u %02u:%02u:%02u, %d, %f %f %f\r\n", 
		szLabel, 
		tms1.tm_mday, tms1.tm_mon + 1, tms1.tm_year + 1900, tms1.tm_hour, tms1.tm_min, tms1.tm_sec,
		tms2.tm_mday, tms2.tm_mon + 1, tms2.tm_year + 1900, tms2.tm_hour, tms2.tm_min, tms2.tm_sec,
		pPLA->data.m_nSegmentLength, pPLA->data.m_fRMS, pPLA->data.a, pPLA->data.b);
}

#else

void PLA_TracePoint(PLA_HANDLE *pPLA, PLA_POINT *pPoint, const char *szLabel)
{
/*	char szLogMsg[1024];

	struct tm tms;
	time_t t;

	t = pPoint->m_nTime;
	localtime_r(&t, &tms);

	sprintf(szLogMsg, "%s: %02u.%02u.%04u %02u:%02u:%02u, %d, %f %f",
		szLabel, 
		tms.tm_mday, tms.tm_mon + 1, tms.tm_year + 1900, tms.tm_hour, tms.tm_min, tms.tm_sec,
		0, pPoint->m_fFuelLevel, pPoint->m_fRealFuel);

	LOG_AddEntry(szLogMsg);
	*/
}

void PLA_TraceSegment(PLA_HANDLE *pPLA, const char *szLabel)
{
/*	char szLogMsg[1024];

	struct tm tms1;
	struct tm tms2;

	time_t t;

	t = pPLA->data.m_nTime;
	localtime_r(&t, &tms1);

	t = pPLA->m_pBuffer[pPLA->data.m_nSegmentLength - 1].m_nTime;
	localtime_r(&t, &tms2);

	sprintf(szLogMsg, "%s: %02u.%02u.%04u %02u:%02u:%02u - %02u.%02u.%04u %02u:%02u:%02u, %d, %f %f %f", 
		szLabel, 
		tms1.tm_mday, tms1.tm_mon + 1, tms1.tm_year + 1900, tms1.tm_hour, tms1.tm_min, tms1.tm_sec,
		tms2.tm_mday, tms2.tm_mon + 1, tms2.tm_year + 1900, tms2.tm_hour, tms2.tm_min, tms2.tm_sec,
		pPLA->data.m_nSegmentLength, pPLA->data.m_fRMS, pPLA->data.a, pPLA->data.b);

	LOG_AddEntry(szLogMsg);
	*/
}

#endif

PLA PLA_Create()
{
	PLA_HANDLE *pHandle = (PLA_HANDLE *)malloc(sizeof(PLA_HANDLE));

	pHandle->m_nBufferSize = 400;	

	pHandle->m_pFuelCallback = NULL;
	pHandle->m_pFillCallback = NULL;
	pHandle->m_pDrainCallback = NULL;

	pHandle->m_pFuelCallbackData = NULL;
	pHandle->m_pFillCallbackData = NULL;
	pHandle->m_pDrainCallbackData = NULL;

	pHandle->m_pBuffer = NULL;

	pHandle->m_fFillThreshold = 50;
	pHandle->m_fDrainThreshold = 10;
	pHandle->m_fMaxFuelRate = 50;

	pHandle->data.m_nID = 0xAABBCCDD;

	pHandle->data.m_nPointsCount = 0;
	pHandle->data.m_bFirstSegment = 1;
	pHandle->data.m_bFillInProgress = 0;

	return (PLA)pHandle;
}

void PLA_Destroy(PLA pHandle)
{
	PLA_HANDLE *pPLA = (PLA_HANDLE *)(pHandle);

	if (pPLA->m_pBuffer)
		free(pPLA->m_pBuffer);

	free(pPLA);
}

void PLA_Reset(PLA pHandle)
{
	PLA_HANDLE *pPLA = (PLA_HANDLE *)(pHandle);

	pPLA->data.m_nID = 0xAABBCCDD;

	pPLA->data.m_nPointsCount = 0;
	pPLA->data.m_bFirstSegment = 1;
	pPLA->data.m_bFillInProgress = 0;
}

void PLA_GetData(void *pHandle, void **pData, int *pSize)
{
	PLA_HANDLE *pPLA = (PLA_HANDLE *)(pHandle);

	if (pPLA->data.m_nPointsCount > 0)
		pPLA->data.m_nBufferStartTime = pPLA->m_pBuffer->m_nTime;

	*pData = &pPLA->data;
	*pSize = sizeof(PLA_DATA);
}

void PLA_SetData(void *pHandle, void *pData, int nSize)
{

	PLA_HANDLE *pPLA = (PLA_HANDLE *)(pHandle);
	PLA_DATA *pPlaData = (PLA_DATA *)pData;

	if (pPlaData->m_nID == 0xAABBCCDD) {

		if (nSize > (int)sizeof(PLA_DATA))
			nSize = (int)sizeof(PLA_DATA);

		memset(&pPLA->data, 0, sizeof(PLA_DATA));
		memcpy(&pPLA->data, pData, nSize);

		printf("PLA set data: %u %u\r\n", pPLA->m_nBufferSize, pPLA->data.m_nPointsCount);
	}
	else {
		printf("PLA ignore: %x\r\n", pPlaData->m_nID);
	}
}

int PLA_GetBufferItemsCount(void *pHandle)
{
	PLA_HANDLE *pPLA = (PLA_HANDLE *)(pHandle);

	return pPLA->data.m_nPointsCount;
}

void PLA_SetBufferItem(void *pHandle, int iItem, int nTime, float fFuelLevel)
{
	PLA_HANDLE *pPLA = (PLA_HANDLE *)(pHandle);

	pPLA->m_pBuffer[iItem].m_nTime = nTime;
	pPLA->m_pBuffer[iItem].m_fFuelLevel = fFuelLevel;
}

void PLA_SetBufferSize(void *pHandle, int nBufferSize)
{
	PLA_HANDLE *pPLA = (PLA_HANDLE *)(pHandle);

	pPLA->m_nBufferSize = nBufferSize;
	pPLA->m_pBuffer = (PLA_POINT *)malloc(sizeof(PLA_POINT) * nBufferSize);
}

int PLA_GetBufferStartTime(void *pHandle)
{
	PLA_HANDLE *pPLA = (PLA_HANDLE *)(pHandle);

	return pPLA->data.m_nBufferStartTime;
}

void PLA_SetFuelCallback(void *pHandle, PLA_FUEL_CALLBACK *pCallback, void *pData)
{
	PLA_HANDLE *pPLA = (PLA_HANDLE *)(pHandle);

	pPLA->m_pFuelCallback = pCallback;
	pPLA->m_pFuelCallbackData = pData;
}

void PLA_SetFillCallback(void *pHandle, PLA_FILL_CALLBACK *pCallback, void *pData)
{
	PLA_HANDLE *pPLA = (PLA_HANDLE *)(pHandle);

	pPLA->m_pFillCallback = pCallback;
	pPLA->m_pFillCallbackData = pData;
}

void PLA_SetDrainCallback(void *pHandle, PLA_DRAIN_CALLBACK *pCallback, void *pData)
{
	PLA_HANDLE *pPLA = (PLA_HANDLE *)(pHandle);

	pPLA->m_pDrainCallback = pCallback;
	pPLA->m_pDrainCallbackData = pData;
}

void PLA_SetFillThreshold(PLA pHandle, int nFillThreshold)
{
	PLA_HANDLE *pPLA = (PLA_HANDLE *)(pHandle);

	pPLA->m_fFillThreshold = (float)nFillThreshold;
}

void PLA_SetDrainThreshold(PLA pHandle, int nDrainThreshold)
{
	PLA_HANDLE *pPLA = (PLA_HANDLE *)(pHandle);

	pPLA->m_fDrainThreshold = (float)nDrainThreshold;
}

void PLA_SetMaxFuelRate(PLA pHandle, int nMaxFuelRate)
{
	PLA_HANDLE *pPLA = (PLA_HANDLE *)(pHandle);

	pPLA->m_fMaxFuelRate = (float)nMaxFuelRate;
}

int PLA_GetBestBreakPoint(PLA_POINT *pSegmentStart, int nSegmentLength)
{
	int iPoint, iBreakPoint;
	int iBestBreakPoint = 0;
	float fBestRMS = 0;

	if (nSegmentLength < 3)
		return -1;

	if (nSegmentLength == 3)
		return 1;

	// Аппроксимирую весь сегмент
	float fSumX = 0;
	float fSumY = 0;
	float fSumXY = 0;
	float fSumX2 = 0;

	PLA_POINT *pPoint = pSegmentStart;
	for ( iPoint = 0; iPoint < nSegmentLength; iPoint++, pPoint++) {

		float dT = (float)(pPoint->m_nTime - pSegmentStart->m_nTime);

		fSumX += dT;
		fSumY += pPoint->m_fFuelLevel;
		fSumXY += dT * pPoint->m_fFuelLevel;
		fSumX2 += dT * dT;

		pPoint->m_fSumX = fSumX;
		pPoint->m_fSumY = fSumY;
		pPoint->m_fSumXY = fSumXY;
		pPoint->m_fSumX2 = fSumX2;
	}

	// Нахожу наилучшую точку разрыва, на крайних точках разрыв невозможен

	PLA_POINT *pBreakPoint = pSegmentStart + nSegmentLength - 1;

	for (iBreakPoint = nSegmentLength - 1; iBreakPoint > 0; iBreakPoint--, pBreakPoint--) {

		// Аппроксимирую сегмент от начала до точки разрыва
		int nFirstSegmentLength = iBreakPoint + 1;

		float a = (nFirstSegmentLength * pBreakPoint->m_fSumXY - pBreakPoint->m_fSumX * pBreakPoint->m_fSumY) / (nFirstSegmentLength * pBreakPoint->m_fSumX2 - pBreakPoint->m_fSumX * pBreakPoint->m_fSumX);
		float b = (pBreakPoint->m_fSumY - a * pBreakPoint->m_fSumX) / nFirstSegmentLength;

		pBreakPoint->a = a;
		pBreakPoint->b = b;

		// Нахожу сумму квадратичных отклонений первого сегмента
		float fFirstSegmentRMS = 0;

		PLA_POINT *pPoint = pSegmentStart;
		for (iPoint = 0; iPoint < nFirstSegmentLength; iPoint++, pPoint++) {

			float fRealFuel = a * (pPoint->m_nTime - pSegmentStart->m_nTime) + b;

			float fFuelDelta = fRealFuel - pPoint->m_fFuelLevel;

			fFirstSegmentRMS += fFuelDelta * fFuelDelta;
		}

		// Аппроксимирую второй сегмент
		int nSecondSegmentLength = nSegmentLength - iBreakPoint - 1;

		fSumX = 0;
		fSumY = 0;
		fSumXY = 0;
		fSumX2 = 0;

		pPoint = pBreakPoint + 1;

		for (iPoint = iBreakPoint + 1; iPoint < nSegmentLength; iPoint++, pPoint++) {

			float dT = (float)(pPoint->m_nTime - (pBreakPoint + 1)->m_nTime);

			fSumX += dT;
			fSumY += pPoint->m_fFuelLevel;
			fSumXY += dT * pPoint->m_fFuelLevel;
			fSumX2 += dT * dT;
		}

		a = (nSecondSegmentLength * fSumXY - fSumX * fSumY) / (nSecondSegmentLength * fSumX2 - fSumX * fSumX);
		b = (fSumY - a * fSumX) / nSecondSegmentLength;

		// Нахожу сумму квадратичных отклонений второго сегмента
		float fSecondSegmentRMS = 0;

		pPoint = pBreakPoint + 1;
		for (iPoint = iBreakPoint + 1; iPoint < nSegmentLength; iPoint++, pPoint++) {

			float fRealFuel = a * (pPoint->m_nTime - (pBreakPoint + 1)->m_nTime) + b;

			float fFuelDelta = fRealFuel - pPoint->m_fFuelLevel;

			fSecondSegmentRMS += fFuelDelta * fFuelDelta;
		}

		float fRMS = (fFirstSegmentRMS + fSecondSegmentRMS) / (nFirstSegmentLength + nSecondSegmentLength);

		if ((iBreakPoint == nSegmentLength - 1)||(fRMS < fBestRMS)) {
			iBestBreakPoint = iBreakPoint;
			fBestRMS = fRMS;
			if (fBestRMS == 0)
				return iBestBreakPoint;
		}
	}

	return iBestBreakPoint;
}

void PLA_ProcessFuel(void *pHandle, int nTime, float fFuelLevel, int nFlags)
{
	PLA_HANDLE *pPLA = (PLA_HANDLE *)(pHandle);

	int nPointsToProceed, iPoint;

	if ((pPLA->m_nBufferSize == 0)||(pPLA->m_pBuffer == NULL))
		return;

	// Начало нового сегмента
	if (pPLA->data.m_nPointsCount == 0) {

		pPLA->data.m_nTime			= nTime;
		pPLA->data.m_fSumX			= 0;
		pPLA->data.m_fSumX2			= 0;
		pPLA->data.m_fSumXY			= 0;
		pPLA->data.m_fSumY			= fFuelLevel;
		pPLA->data.m_fRMS			= 0;
		pPLA->data.m_nSegmentLength	= 1;
	}

	PLA_POINT *pPoint = NULL;

	if (pPLA->data.m_nPointsCount == pPLA->m_nBufferSize) {

		memcpy(&pPLA->m_pBuffer[0], &pPLA->m_pBuffer[1], sizeof(PLA_POINT) * (pPLA->m_nBufferSize - 1));
		
		pPoint = &pPLA->m_pBuffer[pPLA->m_nBufferSize - 1];
	}
	else {
		pPoint = &pPLA->m_pBuffer[pPLA->data.m_nPointsCount];

		pPLA->data.m_nPointsCount++;
	}

	pPoint->m_nTime = nTime;
	pPoint->m_fFuelLevel = fFuelLevel;
	pPoint->m_nFlags = nFlags;

	if (pPLA->data.m_nPointsCount == 1)
		return;
	
	for (nPointsToProceed = 1; nPointsToProceed > 0; nPointsToProceed--, pPoint++) {

		// Аппроксимация с учетом новой точки
		float dT = (float)(pPoint->m_nTime - pPLA->data.m_nTime);

		pPLA->data.m_fSumX += dT;
		pPLA->data.m_fSumY += pPoint->m_fFuelLevel;
		pPLA->data.m_fSumXY += dT * pPoint->m_fFuelLevel;
		pPLA->data.m_fSumX2 += dT * dT;

		pPLA->data.m_nSegmentLength++;

		if (pPLA->data.m_nSegmentLength > pPLA->m_nBufferSize) {
			PLA_Reset(pPLA);
			return;
		}

		float a = (pPLA->data.m_nSegmentLength * pPLA->data.m_fSumXY - pPLA->data.m_fSumX * pPLA->data.m_fSumY) / (pPLA->data.m_nSegmentLength * pPLA->data.m_fSumX2 - pPLA->data.m_fSumX * pPLA->data.m_fSumX);
		float b = (pPLA->data.m_fSumY - a * pPLA->data.m_fSumX) / pPLA->data.m_nSegmentLength;

		char bSegmentDone = 0;

		// Вычисляем сколько последних точек лежат по одну сторону от тренда
		int nBubbleStartPoint = 0;
		int nBubbleLength = 0;
		float fLastPointDelta = 0;
		char bLastPoint = 1;

		PLA_POINT *pTrendPoint = pPLA->m_pBuffer + pPLA->data.m_nSegmentLength - 1;

		for (iPoint = pPLA->data.m_nSegmentLength - 1; iPoint >= 0; iPoint--, pTrendPoint--) {

			float fRealFuel = a * (pTrendPoint->m_nTime - pPLA->data.m_nTime) + b;
				
			float fFuelDelta = pTrendPoint->m_fFuelLevel - fRealFuel;

			if (bLastPoint && fFuelDelta == 0)
				continue;

			if (bLastPoint) {
				fLastPointDelta = fFuelDelta;
				nBubbleStartPoint = iPoint;
				nBubbleLength = 1;
				bLastPoint = 0;
				continue;
			}

			if ((fFuelDelta == 0)||(abs(fFuelDelta) < 0.01))
				break;

			if ((fLastPointDelta < 0)&&(fFuelDelta > 0))
				break;

			if ((fLastPointDelta > 0)&&(fFuelDelta < 0))
				break;

			nBubbleStartPoint = iPoint;
			nBubbleLength++;
		}


		// Если число точек по одну сторону в конце тренда слишком велико - сегмент делится на два
		if ((nBubbleLength > 5)||(pPLA->data.m_nSegmentLength == pPLA->m_nBufferSize)) {
				
			// Находится оптимальная точка для разбиения сегмента
			int i = PLA_GetBestBreakPoint(pPLA->m_pBuffer, pPLA->data.m_nSegmentLength - 5);

			if (i == -1) {
				// Включаем точку в сегмент

				pPoint->a = a;
				pPoint->b = b;

				pPLA->data.a = a;
				pPLA->data.b = b;
			}
			else {
				PLA_TracePoint(pPLA, pPLA->m_pBuffer + pPLA->data.m_nSegmentLength - 1, "Stop Point");

				bSegmentDone = 1;

				pPLA->data.m_nSegmentLength = i + 1;

				PLA_TracePoint(pPLA, &pPLA->m_pBuffer[i], "BBB Point");

				pPLA->data.a = pPLA->m_pBuffer[i].a;
				pPLA->data.b = pPLA->m_pBuffer[i].b;
			}
		}
		else {
			// Включаем точку в сегмент

			pPoint->a = a;
			pPoint->b = b;

			pPLA->data.a = a;
			pPLA->data.b = b;
		}

		if (bSegmentDone) {

			PLA_TraceSegment(pPLA, "DONE");

			int nTimeToSkip = 0;

			// Находим точку пересечения текущего сегмента с предыдущим сегментом
			if (pPLA->data.m_bFirstSegment == 0) {

				// Находим значение на конце предыдущего сегмента
				float fPrevEndFuel = pPLA->data.m_PrevA * (pPLA->data.m_nPrevEndTime - pPLA->data.m_nPrevTime) + pPLA->data.m_PrevB;

				// Нахожу время на текущем сегменте где он равен концу предыдущего сегмента
				float fPrevDelta = (fPrevEndFuel - pPLA->data.b) / pPLA->data.a;

				nTimeToSkip = (int)fPrevDelta;

				if ((nTimeToSkip > pPLA->m_pBuffer[pPLA->data.m_nSegmentLength - 1].m_nTime - pPLA->m_pBuffer->m_nTime)||(nTimeToSkip <= 0)) {

					nTimeToSkip = 0;

					PLA_TraceSegment(pPLA, "TRUNC 0");
				}
				else {
					// Отсекаю начало текущего сегмента до точки пересечения с предыдущим
					float fTime = pPLA->m_pBuffer[pPLA->data.m_nSegmentLength - 1].m_nTime - pPLA->m_pBuffer->m_nTime;
					pPLA->data.b = fPrevEndFuel;//pPLA->data.m_PrevA * fPrevDelta + pPLA->data.m_PrevB;
					pPLA->data.a *= fTime / (fTime - fPrevDelta);
					pPLA->data.m_nTime += nTimeToSkip;
					
					PLA_TraceSegment(pPLA, "TRUNC 1");
				}

				// Предыдущий сегмент продлевается до начала текущего
				pPLA->data.m_fPrevEndFuel = pPLA->data.b;
				pPLA->data.m_nPrevEndTime = pPLA->data.m_nTime;
			}
			else {
				int nThisPeriodEndTime = pPLA->m_pBuffer[pPLA->data.m_nSegmentLength - 1].m_nTime;

				pPLA->data.m_fPrevEndFuel = pPLA->data.a * (nThisPeriodEndTime - pPLA->data.m_nTime) + pPLA->data.b;
				pPLA->data.m_nPrevEndTime = nThisPeriodEndTime;

				PLA_TraceSegment(pPLA, "TRUNC 2");
			}


			pPoint = pPLA->m_pBuffer;

			PLA_POINT *pLastPoint = NULL;
			PLA_POINT *pFirstPoint = NULL;

			float fFuelLevel = 0;

			// Нужно пересчитать длинну сегмента после транкации
			int nNewSegmentLength = 0;

			// Расставляем real_fuel
			for (iPoint = 0; iPoint < pPLA->data.m_nSegmentLength; iPoint++, pPoint++) {

				if (pFirstPoint == NULL)
					pFirstPoint = pPoint;			

				pLastPoint = pPoint;

				if ((pPoint->m_nTime < pPLA->data.m_nPrevEndTime)&&(pPLA->data.m_bFirstSegment == 0)) {

					float dT = (float)(pPoint->m_nTime - pPLA->data.m_nPrevTime);

					fFuelLevel = pPLA->data.m_fPrevEndFuel;//pPLA->data.m_PrevA * dT + pPLA->data.m_PrevB;

					pPoint->m_fRealFuel = fFuelLevel;
				}
				else {

					float dT = (float)(pPoint->m_nTime - pPLA->data.m_nTime);

					fFuelLevel = pPLA->data.a * dT + pPLA->data.b;

					nNewSegmentLength++;
				}

				pPoint->m_fRealFuel = fFuelLevel;

				if (pPLA->m_pFuelCallback != NULL)
					pPLA->m_pFuelCallback(pPoint->m_nTime, fFuelLevel, pPLA->m_pFuelCallbackData);
			}

			// По факту сегмент стал короче, т.к. его начало отсеклось в пользу предыдущего сегмента
			// Но переменная m_nSegmentLength больше используется как указатель на конец сегмента а он не изменился
			// поэтому изменять ее тут - было ошибкой, возникла проблема что следующий сегмент начинался 
			// не из самого конца текущего, а из какой-то точки близкой к концу текущего.

			//pPLA->data.m_nSegmentLength = nNewSegmentLength;

			float fFuelDelta = pLastPoint->m_fRealFuel - pPLA->data.m_fPrevEndFuel;
			float fFuelRate = 0;
			
			if (pLastPoint->m_nTime != pPLA->data.m_nTime)
				fFuelRate = 3600 / (pLastPoint->m_nTime - pPLA->data.m_nTime) * fFuelDelta;

			char bFillDone = 0;

			// Если это был повышающий сегмент - он либо стартует заправку либо продолжает старую
			if (fFuelDelta > 0) {

				if (pPLA->data.m_bFillInProgress == 0) {
				
					// заправку стартует только достаточно мощный повышающий сегмент
					// Сегмент который поднимается чуть-чуть заправку не стартует, т.к. он ложный
					if (fFuelRate >= 5) {

						pPLA->data.m_bFillInProgress = 1;

						pPLA->data.m_nFillStartTime = pPLA->data.m_nTime;
						pPLA->data.m_fFillStartFuel = pPLA->data.m_fPrevEndFuel;
					}
				}
				else
				// Если сегмент очень длинный, а скорость подъема очень маленькая - заправка обрывается
				if ((pLastPoint->m_nTime - pPLA->data.m_nTime > 1800)&&(fFuelRate < 20))
					bFillDone = 1;
			}
			else
			if (pPLA->data.m_bFillInProgress)
				bFillDone = 1;

			if (bFillDone) {

				// Это был понижающий сегмент в момент заправки
				// Заправка останавливается, если был пройден порог заправки - она регистрируется
				pPLA->data.m_bFillInProgress = 0;

				pPLA->data.m_nFillEndTime = pPLA->data.m_nTime;
				pPLA->data.m_fFillEndFuel = pPLA->data.m_fPrevEndFuel;

				if (pPLA->data.m_fFillEndFuel - pPLA->data.m_fFillStartFuel >= pPLA->m_fFillThreshold) {

					if (pPLA->m_pFillCallback)
						pPLA->m_pFillCallback(pPLA->data.m_nFillStartTime, pPLA->data.m_nFillEndTime, pPLA->data.m_fFillStartFuel, pPLA->data.m_fFillEndFuel, pPLA->m_pFillCallbackData);
				}
			}

			// Если это был понижающий сегмент с темпом расхода выше нормы - 
			// он либо стартует слив либо продолжает старый

			if ((fFuelDelta < 0)&&(-fFuelRate > pPLA->m_fMaxFuelRate)) {

				if (pPLA->data.m_bDrainInProgress == 0) {
					
					pPLA->data.m_bDrainInProgress = 1;

					pPLA->data.m_nDrainStartTime = pPLA->data.m_nTime;
					pPLA->data.m_fDrainStartFuel = pPLA->data.b;
				}
			}
			else
			if (pPLA->data.m_bDrainInProgress) {

				// Это был повышающий сегмент в момент слива или слив прекращен
				// Слив останавливается, если был пройден порог слива - он регистрируется
				pPLA->data.m_bDrainInProgress = 0;

				pPLA->data.m_nDrainEndTime = pPLA->data.m_nPrevEndTime;
				pPLA->data.m_fDrainEndFuel = pPLA->data.m_fPrevEndFuel;

				if (pPLA->data.m_fDrainStartFuel - pPLA->data.m_fDrainEndFuel >= pPLA->m_fDrainThreshold) {

					if (pPLA->m_pDrainCallback)
						pPLA->m_pDrainCallback(pPLA->data.m_nDrainStartTime, pPLA->data.m_nDrainEndTime, pPLA->data.m_fDrainStartFuel, pPLA->data.m_fDrainEndFuel, pPLA->m_pDrainCallbackData);
				}
			}

			// Запоминаем характеристики текущего сегмента
			pPLA->data.m_bFirstSegment = 0;
			pPLA->data.m_PrevA = pPLA->data.a;
			pPLA->data.m_PrevB = pPLA->data.b;
			pPLA->data.m_nPrevTime = pPLA->data.m_nTime;
			pPLA->data.m_nPrevEndTime = pPLA->m_pBuffer[pPLA->data.m_nSegmentLength - 1].m_nTime;

			// Затираем текущий сегмент, т.к. он уже закрыт
			memcpy(&pPLA->m_pBuffer[0], &pPLA->m_pBuffer[pPLA->data.m_nSegmentLength - 1], sizeof(PLA_POINT) * (pPLA->m_nBufferSize - (pPLA->data.m_nSegmentLength - 1)));

			// Если в буфере еще остались точки - они образуют новый сегмент
			pPLA->data.m_nPointsCount -= pPLA->data.m_nSegmentLength - 1;

			if (pPLA->data.m_nPointsCount > 0) {

				PLA_TracePoint(pPLA, pPLA->m_pBuffer, "Segment start");

				pPLA->data.m_nTime			= pPLA->m_pBuffer[0].m_nTime;
				pPLA->data.m_fSumX			= 0;
				pPLA->data.m_fSumX2			= 0;
				pPLA->data.m_fSumXY			= 0;
				pPLA->data.m_fSumY			= pPLA->m_pBuffer[0].m_fFuelLevel;
				pPLA->data.m_fRMS			= 0;
				pPLA->data.m_nSegmentLength	= 1;
				pPLA->data.a = 0;

				if (pPLA->data.m_nPointsCount > 1) {
					pPoint = pPLA->m_pBuffer;
					nPointsToProceed = pPLA->data.m_nPointsCount;
					continue;
				}
			}
		}
	}
}

// End
