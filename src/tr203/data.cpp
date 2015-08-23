//******************************************************************************
//
// File Name : data.cpp
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include "api.h"
#include "config.h"
#include "common.h"
#include "data.h"

#include "../core/record.h"
#include "../core/cross.h"
#include "constants.h"

#ifndef _MSC_VER
#include <netinet/in.h>
#endif

#define TR20x_COMMAND_STATE_G		0x00
#define TR20x_COMMAND_STATE_S		0x01
#define TR20x_COMMAND_STATE_DATA	0x02

static unsigned char *bit1 = record_data + 0;
static unsigned char *bit2 = record_data + 1;
static unsigned char *bit3 = record_data + 2;
static unsigned char *bit4 = record_data + 3;
static unsigned char *bit5 = record_data + 4;

static TERMINAL dummy_terminal = { 0, NULL, NULL };

SESSION *data_session_open()
{
	SESSION *session = (SESSION *)malloc(sizeof(SESSION));

	memset(session, 0, sizeof(SESSION));

	session->nCommandState	= TR20x_COMMAND_STATE_G;
	session->terminal		= &dummy_terminal;
	
	return session;
}

void data_session_close(SESSION *session)
{
	TERMINAL *terminal = (TERMINAL *)session->terminal;

	if (terminal->session == session) {
		terminal->session = NULL;
		add_event(terminal, RECORD_EVENT_TERMINAL_OFFLINE);
		api_log_printf("[TR20x] Connection closed, terminal_id=%u\r\n", terminal->id);
	}

	api_log_printf("[TR20x] Closing session 0x%08X\r\n", session);

	free(session);
}

int data_session_timer(SESSION *session, char **p, size_t *l)
{
	data_session_close(session);

	*l = 0;

	return SESSION_COMPLETE;
}

int data_session_data(SESSION *session, unsigned char **p, size_t *l)
{
	unsigned char	*dst;

	TERMINAL *terminal = (TERMINAL *)session->terminal;

	unsigned char *data = *p;

	while (*l > 0) {

		switch (session->nCommandState) {
		
		case TR20x_COMMAND_STATE_G:
			if (*data == 'G') {
				session->nCommandState = TR20x_COMMAND_STATE_S;
			}

			break;

		case TR20x_COMMAND_STATE_S:
			if (*data == 'S') {
				session->nCommandState = TR20x_COMMAND_STATE_DATA;
				session->nBytesReceived = 0;
			}
			else
				session->nCommandState = TR20x_COMMAND_STATE_G;
			break;

		case TR20x_COMMAND_STATE_DATA:

			session->nData[session->nBytesReceived] = *data;
			session->nBytesReceived++;

			if (session->nBytesReceived == sizeof(session->nData)) {
				session->nCommandState = TR20x_COMMAND_STATE_G;
				continue;
			}

			if (*data == '!') {

				session->nData[session->nBytesReceived] = '\0';

				api_log_printf("[TR20x] Received command '%s'\r\n", session->nData);
				
				session->nCommandState = TR20x_COMMAND_STATE_G;

				if (session->nData[0] == 'r') {

					char *pIMEI;
					char *pMode;
					char *pType;
					char *pAlarm;
					char *pGeofence;
					char *pGPSFix;
					char *pUTCDate;
					char *pUTCTime;
					char *pLongitude;
					char *pLatitude;
					char *pAltitude;
					char *pSpeed;
					char *pHeading;
					char *pSatCount;
					char *pHDOP;
					char *pBattery;
					char *pCRC;
					char *ptr;

					ptr = (char *)session->nData;

					while ((*ptr)&&(*ptr != ',')) ptr++;
					if (*ptr != ',') break;
					ptr++;
					pIMEI = ptr;

					while ((*ptr)&&(*ptr != ',')) ptr++;
					if (*ptr != ',') break;
					ptr++;
					pMode = ptr;

					while ((*ptr)&&(*ptr != ',')) ptr++;
					if (*ptr != ',') break;
					ptr++;
					pType = ptr;

					while ((*ptr)&&(*ptr != ',')) ptr++;
					if (*ptr != ',') break;
					ptr++;
					pAlarm = ptr;

					while ((*ptr)&&(*ptr != ',')) ptr++;
					if (*ptr != ',') break;
					ptr++;
					pGeofence = ptr;

					while ((*ptr)&&(*ptr != ',')) ptr++;
					if (*ptr != ',') break;
					ptr++;
					pGPSFix = ptr;

					while ((*ptr)&&(*ptr != ',')) ptr++;
					if (*ptr != ',') break;
					ptr++;
					pUTCDate = ptr;

					while ((*ptr)&&(*ptr != ',')) ptr++;
					if (*ptr != ',') break;
					ptr++;
					pUTCTime = ptr;

					while ((*ptr)&&(*ptr != ',')) ptr++;
					if (*ptr != ',') break;
					ptr++;
					pLongitude = ptr;

					while ((*ptr)&&(*ptr != ',')) ptr++;
					if (*ptr != ',') break;
					ptr++;
					pLatitude = ptr;

					while ((*ptr)&&(*ptr != ',')) ptr++;
					if (*ptr != ',') break;
					ptr++;
					pAltitude = ptr;

					while ((*ptr)&&(*ptr != ',')) ptr++;
					if (*ptr != ',') break;
					ptr++;
					pSpeed = ptr;

					while ((*ptr)&&(*ptr != ',')) ptr++;
					if (*ptr != ',') break;
					ptr++;
					pHeading = ptr;

					while ((*ptr)&&(*ptr != ',')) ptr++;
					if (*ptr != ',') break;
					ptr++;
					pSatCount = ptr;

					while ((*ptr)&&(*ptr != ',')) ptr++;
					if (*ptr != ',') break;
					ptr++;
					pHDOP = ptr;

					while ((*ptr)&&(*ptr != ',')) ptr++;
					if (*ptr != ',') break;
					ptr++;
					pBattery = ptr;

					while ((*ptr)&&(*ptr != '*')) ptr++;
					if (*ptr != '*') break;
					ptr++;
					pCRC = ptr;

					if (terminal == &dummy_terminal) {

						unsigned char dev_id[8];

						dev_id[0] = ((pIMEI[0]  - '0') << 4) | (pIMEI[1]  - '0');
						dev_id[1] = ((pIMEI[2]  - '0') << 4) | (pIMEI[3]  - '0');
						dev_id[2] = ((pIMEI[4]  - '0') << 4) | (pIMEI[5]  - '0');
						dev_id[3] = ((pIMEI[6]  - '0') << 4) | (pIMEI[7]  - '0');
						dev_id[4] = ((pIMEI[8]  - '0') << 4) | (pIMEI[9]  - '0');
						dev_id[5] = ((pIMEI[10] - '0') << 4) | (pIMEI[11] - '0');
						dev_id[6] = ((pIMEI[12] - '0') << 4) | (pIMEI[13] - '0');
						dev_id[7] = ((pIMEI[14] - '0') << 4);

						std::map<uint64_t, TERMINAL>::iterator it = terminals.find(*(uint64_t *)dev_id);

						if (it == terminals.end()) {

							api_log_printf("[TR20x] Unknown terminal [%.15s]\r\n", pIMEI);

							*l = 0;

							data_session_close(session);

							return SESSION_COMPLETE;
						}

						terminal = &it->second;
						session->terminal = terminal;
						terminal->session = session;

						api_log_printf("[TR20x] Terminal Authorized [%.15s], terminal_id=%u\r\n", pIMEI, terminal->id);

						add_event(terminal, RECORD_EVENT_TERMINAL_ONLINE);
					}

					api_log_printf("[TR20x] Received data from terminal_id=%u\r\n", terminal->id);

					*bit1 = RECORD_BIT1_FLAGS | RECORD_BIT_MORE;

//					if (*pType == 'I')
//			        		DataRecord.flags |= TPS_POINT_FLAG_ALARM;

					if ((*pGPSFix == '2')||(*pGPSFix == '3')) {
						*bit1 |= RECORD_BIT1_NAV;
					}

					if (*pGPSFix == '3')
						*bit1 |= RECORD_BIT1_ALT;

					*bit2 = RECORD_BIT_MORE;
					*bit3 = RECORD_BIT3_VCC;

					dst = bit3 + 1;

					*dst++ = 0; // Flags

					if (*bit1 & RECORD_BIT1_NAV) {

						unsigned int deg;
						double mins;

						char cNS = *pLatitude++;

						deg =  (*pLatitude++ - '0') * 10;
						deg += (*pLatitude++ - '0') * 1;
										
						mins =  (*pLatitude++ - '0') * 10;
						mins += (*pLatitude++ - '0') * 1;
						pLatitude++;
						mins += (*pLatitude++ - '0') * 0.1;
						mins += (*pLatitude++ - '0') * 0.01;
						mins += (*pLatitude++ - '0') * 0.001;
					
						if (cNS == 'N')
							*(int *)dst = (int)((deg + mins / 60) * 10000000);
						else
							*(int *)dst = (int)((deg + mins / 60) * -10000000);

						dst += 4;

						char cEW = *pLongitude++;

						deg =  (*pLongitude++ - '0') * 100;
						deg += (*pLongitude++ - '0') * 10;
						deg += (*pLongitude++ - '0') * 1;
										
						mins =  (*pLongitude++ - '0') * 10;
						mins += (*pLongitude++ - '0') * 1;
						pLongitude++;
						mins += (*pLongitude++ - '0') * 0.1;
						mins += (*pLongitude++ - '0') * 0.01;
						mins += (*pLongitude++ - '0') * 0.001;

						if (cEW == 'E')
							*(int *)dst = (unsigned int)((deg + mins / 60) * 10000000);
						else
							*(int *)dst = (unsigned int)((deg + mins / 60) * -10000000);

						dst += 4;

						unsigned short speed = (unsigned short)(strtod(pSpeed, NULL) * 1.85200 * 10);

						if (speed & 0x0100)
							*(bit3 + 1) |= RECORD_FLAG1_SPEED_9;

						if (speed & 0x0200)
							*(bit3 + 1) |= RECORD_FLAG1_SPEED_10;

						if (speed & 0x0400)
							*(bit3 + 1) |= RECORD_FLAG1_SPEED_11;

						*dst++ = speed & 0xFF;
					}

					if (*bit1 & RECORD_BIT1_ALT) {
						*(short *)dst = (short)strtol(pAltitude, NULL, 10);
						dst += 2;
					}

					*(unsigned short *)dst = (unsigned short)strtoul(pBattery, NULL, 10) * 10;
					dst += 2;

					unsigned char dd;
					unsigned char mm;
					unsigned char yy;

					dd =  (*pUTCDate++ - '0') * 10;
					dd += (*pUTCDate++ - '0');
					mm =  (*pUTCDate++ - '0') * 10;
					mm += (*pUTCDate++ - '0');
					yy =  (*pUTCDate++ - '0') * 10;
					yy += (*pUTCDate++ - '0');

					unsigned char h;
					unsigned char m;
					unsigned char s;

					h =  (*pUTCTime++ - '0') * 10;
					h += (*pUTCTime++ - '0');
					m =  (*pUTCTime++ - '0') * 10;
					m += (*pUTCTime++ - '0');
					s =  (*pUTCTime++ - '0') * 10;
					s += (*pUTCTime++ - '0');

					struct tm tms;

				  	tms.tm_year	= 100 + yy;
					tms.tm_mon	= mm - 1;
					tms.tm_mday	= dd;
					tms.tm_hour	= h;
					tms.tm_min	= m;		
					tms.tm_sec	= s;

					record->t = (unsigned int)timegm(&tms);
					record->size = dst - record_buffer;

					api_storage_add_record_to_stream(terminal->object->stream, record, record->size);
	
					static const char * const ack = "ACK\r";

					*p = (unsigned char *)ack;
					*l = 4;

					return 300;
				}
			}

			break;							
		}

		(*l)--;
		data++;			
	}

	*l = 0;

	return 300;
}

