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

#define TR151_COMMAND_STATE_$		0
#define TR151_COMMAND_STATE_DATA	1

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

	session->nCommandState	= TR151_COMMAND_STATE_$;
	session->terminal		= &dummy_terminal;
	
	return session;
}

void data_session_close(SESSION *session)
{
	TERMINAL *terminal = (TERMINAL *)session->terminal;

	if (terminal->session == session) {
		terminal->session = NULL;
		add_event(terminal, RECORD_EVENT_TERMINAL_OFFLINE);
		api_log_printf("[TR151] Connection closed, terminal_id=%u\r\n", terminal->id);
	}

	api_log_printf("[TR151] Closing session 0x%08X\r\n", session);

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
		
		case TR151_COMMAND_STATE_$:
			if (*data == '$') {
				session->nCommandState = TR151_COMMAND_STATE_DATA;
				session->nBytesReceived = 0;
			}

			break;

		case TR151_COMMAND_STATE_DATA:

			session->nData[session->nBytesReceived] = *data;
			session->nBytesReceived++;

			if (session->nBytesReceived == sizeof(session->nData)) {
				session->nCommandState = TR151_COMMAND_STATE_$;
				continue;
			}

			if (*data == '!') {

				session->nData[session->nBytesReceived] = '\0';

				api_log_printf("[TR151] Received command '%s'\r\n", session->nData);
				
				session->nCommandState = TR151_COMMAND_STATE_$;

				if (terminal == &dummy_terminal) {

					unsigned char dev_id[8];

					dev_id[0] = ((session->nData[0]  - '0') << 4) | (session->nData[1]  - '0');
					dev_id[1] = ((session->nData[2]  - '0') << 4) | (session->nData[3]  - '0');
					dev_id[2] = ((session->nData[4]  - '0') << 4) | (session->nData[5]  - '0');
					dev_id[3] = ((session->nData[6]  - '0') << 4) | (session->nData[7]  - '0');
					dev_id[4] = ((session->nData[8]  - '0') << 4) | (session->nData[9]  - '0');
					dev_id[5] = ((session->nData[10] - '0') << 4) | (session->nData[11] - '0');
					dev_id[6] = ((session->nData[12] - '0') << 4) | (session->nData[13] - '0');
					dev_id[7] = ((session->nData[14] - '0') << 4);

					std::map<uint64_t, TERMINAL>::iterator it = terminals.find(*(uint64_t *)dev_id);

					if (it == terminals.end()) {

						api_log_printf("[TR151] Unknown terminal [%.15s]\r\n", session->nData);

						*l = 0;

						data_session_close(session);

						return SESSION_COMPLETE;
					}

					terminal = &it->second;
					session->terminal = terminal;
					terminal->session = session;

					api_log_printf("[TR151] Terminal Authorized [%.15s], terminal_id=%u\r\n", session->nData, terminal->id);

					add_event(terminal, RECORD_EVENT_TERMINAL_ONLINE);
				}

				api_log_printf("[TR151] Received data from terminal_id=%u\r\n", terminal->id);

				char *pReportMode, *pGPSFix, *pDate, *pTime, *pLongitude, *pLatitude, *pAltitude, *pSpeed;
				struct tm timeinfo;
				int degs;
				float mins;
				char letter;

				float fLatitude, fLongitude;

				pReportMode = strchr(session->nData, ',');
				if (!pReportMode) return 0;
				pReportMode++;

				pGPSFix = strchr(pReportMode, ',');
				if (!pGPSFix) return 0;
				pGPSFix++;

				pDate = strchr(pGPSFix, ',');
				if (!pDate) return 0;
				pDate++;
				
				pTime = strchr(pDate, ',');
				if (!pTime) return 0;
				pTime++;

				pLongitude = strchr(pTime, ',');
				if (!pLongitude) return 0;
				pLongitude++;

				pLatitude = strchr(pLongitude, ',');
				if (!pLatitude) return 0;
				pLatitude++;

				pAltitude = strchr(pLatitude, ',');
				if (!pAltitude) return 0;
				pAltitude++;

				pSpeed = strchr(pAltitude, ',');
				if (!pSpeed) return 0;
				pSpeed++;

				sscanf(pDate, "%2d%2d%2d", &timeinfo.tm_mday, &timeinfo.tm_mon, &timeinfo.tm_year);
				sscanf(pTime , "%2d%2d%2d", &timeinfo.tm_hour, &timeinfo.tm_min, &timeinfo.tm_sec);

				if (timeinfo.tm_year > 0) {

					*bit1 = RECORD_BIT1_FLAGS;
					dst = bit1 + 1;
	
					if ((*pGPSFix == '2')||(*pGPSFix == '3')) {
						*bit1 |= RECORD_BIT1_NAV;
					}

					if (*pGPSFix == '3')
						*bit1 |= RECORD_BIT1_ALT;

					*dst++ = 0; // Flags

					if (*bit1 & RECORD_BIT1_NAV) {
		
						if (sscanf(pLatitude, "%1c%2d%f", &letter, &degs, &mins) == 3) {
							fLatitude = degs + mins / 60;
							if (letter == 'S') fLatitude = -fLatitude;
						}
						else fLatitude = 0;

						*(int *)dst = (int)(fLatitude * 10000000);
						dst += 4;

						if (sscanf(pLongitude, "%1c%3d%f", &letter, &degs, &mins) == 3) {
							fLongitude = degs + mins / 60;
							if (letter == 'W') fLongitude = -fLongitude;
						}
						else fLongitude = 0;

						*(int *)dst = (int)(fLongitude * 10000000);
						dst += 4;

						unsigned short speed = (unsigned short)(strtod(pSpeed, NULL) * 1.85200 * 10);
						*dst++ = speed & 0xFF;

						if (speed & 0x0100)
							*(bit1 + 1) |= RECORD_FLAG1_SPEED_9;

						if (speed & 0x0200)
							*(bit1 + 1) |= RECORD_FLAG1_SPEED_10;

						if (speed & 0x0400)
							*(bit1 + 1) |= RECORD_FLAG1_SPEED_11;
					}

					if (*bit1 & RECORD_BIT1_ALT) {
						*(short *)dst = (short)strtod(pAltitude, NULL);
						dst += 2;
					}

					timeinfo.tm_mon--;
					timeinfo.tm_year += 100;

					record->t = (unsigned int)timegm(&timeinfo);
					record->size = dst - record_buffer;

					api_storage_add_record_to_stream(terminal->object->stream, record, record->size);
	
					static const char * const ack = "OK!";

					*p = (unsigned char *)ack;
					*l = 3;

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

