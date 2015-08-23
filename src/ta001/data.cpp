//******************************************************************************
//
// File Name : data.cpp
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include "api.h"
#include "config.h"
#include "common.h"
#include "data.h"

#include "../core/record.h"
#include "constants.h"

#ifndef _MSC_VER
#include <netinet/in.h>
#endif

#define COMMAND_STATE_10		0
#define COMMAND_STATE_02        1
#define COMMAND_STATE_MSGNUM_HI	2
#define COMMAND_STATE_MSGNUM_LO 3
#define COMMAND_STATE_MSGTYPE   4
#define COMMAND_STATE_MSGLEN_HI	5
#define COMMAND_STATE_MSGLEN_LO 6
#define COMMAND_STATE_DATA      7
#define COMMAND_STATE_20        8
#define COMMAND_STATE_01        9

#define COMMAND_TYPE_NAVDATA	0x01
#define COMMAND_TYPE_ADCDATA	0x02
#define COMMAND_TYPE_FUELDATA	0x03

#pragma pack(1)

typedef struct _NAVDATA {
	unsigned long long nNumber;
	unsigned char	nFlags;
	unsigned int	nTime;
	unsigned int	nLatitude;
	unsigned int	nLongitude;
	unsigned short	nAltitude;
	unsigned short	nSpeed;
	unsigned short	nSpeedTaho;
	unsigned int	nOdometer;
	unsigned char	reserved;
	unsigned char	bStatus;
	unsigned short	cog;
} NAVDATA;

typedef struct _FUELDATA {
	unsigned long long nNumber;
	unsigned int	nTime;
	unsigned short	nFuel1;
	char		nTemp1;
	unsigned short	nFuel2;
	char		nTemp2;
	char		dummy[2];
} FUELDATA;

typedef struct _ADCDATA {
	unsigned long long nNumber;
	unsigned int	nTime;
	unsigned short	adc1;
	unsigned short	adc2;
	unsigned short	adc3;
	unsigned short	adc4;
	unsigned short	adc5;
	unsigned short	adc6;
} ADCDATA;

typedef struct _CMDHEADER {
	unsigned short	DataStart;
	unsigned short	nMsgNo;
	unsigned char	nMessageType;
	unsigned short	nDataLength;
	unsigned long long nNumber;
} CMDHEADER;

#pragma pack()

#define htonll(x) \
((((x) & 0xff00000000000000LL) >> 56) | \
(((x) & 0x00ff000000000000LL) >> 40) | \
(((x) & 0x0000ff0000000000LL) >> 24) | \
(((x) & 0x000000ff00000000LL) >> 8) | \
(((x) & 0x00000000ff000000LL) << 8) | \
(((x) & 0x0000000000ff0000LL) << 24) | \
(((x) & 0x000000000000ff00LL) << 40) | \
(((x) & 0x00000000000000ffLL) << 56))

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

	session->nCommandState	= COMMAND_STATE_10;
	session->terminal		= &dummy_terminal;
	
	return session;
}

void data_session_close(SESSION *session)
{
	TERMINAL *terminal = (TERMINAL *)session->terminal;

	if (terminal->session == session) {
		terminal->session = NULL;
		add_event(terminal, RECORD_EVENT_TERMINAL_OFFLINE);
		api_log_printf("[TA001] Connection closed, terminal_id=%u\r\n", terminal->id);
	}

	api_log_printf("[TA001] Closing session 0x%08X\r\n", session);

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
	NAVDATA *pNavData;
	ADCDATA *pAdcData;
	FUELDATA *pFuelData;
	char imei[16];

	unsigned char	*dst;

	TERMINAL *terminal = (TERMINAL *)session->terminal;

	unsigned char *data = *p;

	while (*l > 0) {

		switch (session->nCommandState) {
		
		case COMMAND_STATE_10:
			if (*data == 0x10) {
				session->nCommandState = COMMAND_STATE_02;
			}
			(*l)--;

			break;

		case COMMAND_STATE_02:
			if (*data == 0x02) {
				session->nCommandState = COMMAND_STATE_MSGNUM_HI;
				(*l)--;
			}
			else
				session->nCommandState = COMMAND_STATE_10;
			break;

		case COMMAND_STATE_MSGNUM_HI:
			session->nMsgNum = *data << 8;
			session->nCommandState = COMMAND_STATE_MSGNUM_LO;
			(*l)--;
			break;

		case COMMAND_STATE_MSGNUM_LO:
			session->nMsgNum |= *data;
			session->nCommandState = COMMAND_STATE_MSGTYPE;
			(*l)--;
			break;

		case COMMAND_STATE_MSGTYPE:

			session->nCmdType = *data;
			session->nCmdType &= 0x7F;

			(*l)--;

			if ((session->nCmdType == COMMAND_TYPE_NAVDATA)||(session->nCmdType == COMMAND_TYPE_ADCDATA)||(session->nCmdType == COMMAND_TYPE_FUELDATA))
				session->nCommandState = COMMAND_STATE_MSGLEN_HI;
			else
				session->nCommandState = COMMAND_STATE_10;
			break;

		case COMMAND_STATE_MSGLEN_HI:
			session->nCmdLen = *data << 8;
			session->nCommandState = COMMAND_STATE_MSGLEN_LO;
			(*l)--;
			break;

		case COMMAND_STATE_MSGLEN_LO:
			session->nCmdLen |= *data;
			(*l)--;

			if ((session->nCmdLen > sizeof(session->nData))||(session->nCmdLen == 0))
				session->nCommandState = COMMAND_STATE_10;
			else {
				session->nCommandState = COMMAND_STATE_DATA;
				session->nBytesReceived = 0;
			}
			break;

		case COMMAND_STATE_DATA:
				
			if (session->nCmdLen > sizeof(session->nData)) {
				api_log_printf("[TA001] wrong len %u\r\n", session->nCmdLen);
			}

			if (session->nBytesReceived >= 256) {
				api_log_printf("[TA001] wrong recv %u %u %u\r\n", session->nBytesReceived, session->nCmdLen, sizeof(session->nData));
			}

			session->nData[session->nBytesReceived] = *data;
			session->nBytesReceived++;
			(*l)--;

			if (session->nBytesReceived == session->nCmdLen)
				session->nCommandState = COMMAND_STATE_20;
			break;

		case COMMAND_STATE_20:
			if (*data == 0x20) {
				session->nCommandState = COMMAND_STATE_01;
				(*l)--;
			}
			else
				session->nCommandState = COMMAND_STATE_10;
			break;

		case COMMAND_STATE_01:
				
			session->nCommandState = COMMAND_STATE_10;

			if (*data == 0x01) {

				api_log_printf("[TA001] command\r\n");

				switch (session->nCmdType) {
				case COMMAND_TYPE_NAVDATA:
					
					pNavData = (NAVDATA *)&session->nData;

					if (terminal == &dummy_terminal) {

						sprintf(imei, "%llu", htonll(pNavData->nNumber));

						unsigned char dev_id[8];

						dev_id[0] = ((imei[0]  - '0') << 4) | (imei[1]  - '0');
						dev_id[1] = ((imei[2]  - '0') << 4) | (imei[3]  - '0');
						dev_id[2] = ((imei[4]  - '0') << 4) | (imei[5]  - '0');
						dev_id[3] = ((imei[6]  - '0') << 4) | (imei[7]  - '0');
						dev_id[4] = ((imei[8]  - '0') << 4) | (imei[9]  - '0');
						dev_id[5] = ((imei[10] - '0') << 4) | (imei[11] - '0');
						dev_id[6] = ((imei[12] - '0') << 4) | (imei[13] - '0');
						dev_id[7] = ((imei[14] - '0') << 4);

						std::map<uint64_t, TERMINAL>::iterator it = terminals.find(*(uint64_t *)dev_id);

						if (it == terminals.end()) {

							api_log_printf("[TA001] Unknown terminal [%s]\r\n", imei);

							*l = 0;

							data_session_close(session);

							return SESSION_COMPLETE;
						}

						terminal = &it->second;
						session->terminal = terminal;
						terminal->session = session;

						api_log_printf("[TA001] Terminal Authorized [%s], terminal_id=%u\r\n", imei, terminal->id);

						add_event(terminal, RECORD_EVENT_TERMINAL_ONLINE);
					}

					session->nav_valid = (pNavData->nFlags & 0x80);
					session->alt_valid = (pNavData->nFlags & 0x40);

					session->flags = 0;

					if ((pNavData->bStatus & 0x01) > 0) session->flags |= RECORD_FLAG1_MOVE;
					if ((pNavData->bStatus & 0x02) > 0) session->flags |= RECORD_FLAG1_IGNITION;

					session->latitude	= htonl(pNavData->nLatitude) * 100;
					session->longitude	= htonl(pNavData->nLongitude) * 100;
					session->speed		= htons(pNavData->nSpeed);
					session->altitude	= htons(pNavData->nAltitude);
					session->cog		= htons(pNavData->cog) / 100;

					if (session->nav_valid) {

						if (session->speed & 0x0100)
							session->flags |= RECORD_FLAG1_SPEED_9;

						if (session->speed & 0x0200)
							session->flags |= RECORD_FLAG1_SPEED_10;

						if (session->speed & 0x0400)
							session->flags |= RECORD_FLAG1_SPEED_11;

						if (session->cog & 0x0100)
							session->flags |= RECORD_FLAG1_COG_9;
					}

					session->t = htonl(pNavData->nTime) + 946684800;

					break;

				case COMMAND_TYPE_ADCDATA:
					pAdcData = (ADCDATA *)&session->nData;
					break;

				case COMMAND_TYPE_FUELDATA:

					pFuelData = (FUELDATA *)&session->nData;

					if (terminal != &dummy_terminal) {

						api_log_printf("[TA001] Received data 0x%08X from terminal_id=%u\r\n", session->t, terminal->id);

						*bit1 = RECORD_BIT1_FLAGS;

						if (session->nav_valid) {
							*bit1 |= RECORD_BIT1_NAV | RECORD_BIT1_COG;
						}

						if (session->alt_valid) {
							*bit1 |= RECORD_BIT1_ALT;
						}

						dst = bit1 + 1;

						*dst++ = session->flags;

						if (*bit1 & RECORD_BIT1_NAV) {
							*(int *)dst = session->latitude;
							dst += 4;
							*(int *)dst = session->longitude;
							dst += 4;
							*dst++ = session->speed & 0xFF;
						}

						if (*bit1 & RECORD_BIT1_ALT) {
							*(short *)dst = session->altitude;
							dst += 2;
						}

						if (*bit1 & RECORD_BIT1_COG) {
							*dst++ = session->cog & 0xFF;
						}

						if (pFuelData->nFuel1 != 0xFFFF) {
							*bit1 |= RECORD_BIT1_RS485_1;
							*(unsigned short *)dst = pFuelData->nFuel1;
							dst += 2;
						}

						if (pFuelData->nFuel2 != 0xFFFF) {
							*bit1 |= RECORD_BIT1_RS485_2;
							*(unsigned short *)dst = pFuelData->nFuel2;
							dst += 2;
						}

						record->t = session->t;
						record->size = dst - record_buffer;

						if (record->t != 0) {
							api_storage_add_record_to_stream(terminal->object->stream, record, record->size);
						}
					}

					break;
				}

			}

			break;							
		}

		data++;			
	}

	*l = 0;

	return 300;
}

