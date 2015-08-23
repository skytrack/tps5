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
#include "crc16.h"
#include "config.h"
#include "common.h"
#include "ak305.h"
#include "data.h"
#include "tea.h"

#include "../core/record.h"
#include "global.h"
#include "md5.h"
#include "constants.h"

#ifndef _MSC_VER
#include <netinet/in.h>
#endif

#define AK300_COMMAND_STATE_LEN_LO		0
#define AK300_COMMAND_STATE_LEN_HI      1
#define AK300_COMMAND_STATE_DATA        2
#define AK300_COMMAND_STATE_UPDATEAUTH	3

#define AK300_TRANSACTION_COMBO			0
#define AK300_TRANSACTION_INPUTS		1
#define AK300_TRANSACTION_OUTPUTS		2
#define AK300_TRANSACTION_FINISH		3

static MD5_CTX context;

static unsigned char *bit1 = record_data + 0;
static unsigned char *bit2 = record_data + 1;
static unsigned char *bit3 = record_data + 2;
static unsigned char *bit4 = record_data + 3;
static unsigned char *bit5 = record_data + 4;

static TERMINAL dummy_terminal = { 0, 0, 0, 0, NULL, NULL };

SESSION *data_session_open()
{
	SESSION *session = (SESSION *)malloc(sizeof(SESSION));

	memset(session, 0, sizeof(SESSION));
	memcpy(session->SessionKey, AK300_DEFAULT_KEY, 16);								

	session->nCommandState	= AK300_COMMAND_STATE_LEN_LO;
	session->ptr			= (unsigned char *)&session->cmd;
	session->terminal		= &dummy_terminal;

	return session;
}

void data_session_close(SESSION *session)
{
	TERMINAL *terminal = (TERMINAL *)session->terminal;

	if (terminal->session == session) {
		terminal->session = NULL;
		add_event(terminal, RECORD_EVENT_TERMINAL_OFFLINE);
		api_log_printf("[AK305] Connection closed, terminal_id=%u\r\n", terminal->id);
	}

	if (session->f != NULL) {
		fclose(session->f);
		session->f = NULL;
	}

	api_log_printf("[AK305] Closing session 0x%08X\r\n", session);

	free(session);
}

int data_session_timer(SESSION *session, char **p, size_t *l)
{
	data_session_close(session);

	*l = 0;

	return SESSION_COMPLETE;
}

static void AK305_SendCommand(SESSION *session, unsigned short nPayloadLen) 
{
	session->cmd.header.size		= sizeof(AK300_COMMAND_HEADER) + nPayloadLen;
	
	if ((session->cmd.header.size - sizeof(session->cmd.header.size)) % 8)
		session->cmd.header.size += (8 - (session->cmd.header.size - sizeof(session->cmd.header.size)) % 8);

	session->cmd.header.protocol_id	= AK300_PROTOCOL;
	session->cmd.header.crc			= 0x0000;

	session->cmd.header.crc			= crcbuf(AK300_POLYNOM, (unsigned char *)&session->cmd, session->cmd.header.size);

	api_log_printf("[AK305] Send command #%u, terminal_id=%u\r\n", session->cmd.header.command_id, ((TERMINAL *)session->terminal)->id);

	TEAStreamEncrypt(((unsigned char *)&session->cmd) + 2, session->cmd.header.size - sizeof(session->cmd.header.size), session->SessionKey);

	api_send_tcp(session, (unsigned char *)&session->cmd, session->cmd.header.size);
}

int data_session_data(SESSION *session, unsigned char **p, size_t *l)
{
	unsigned char *data = *p;
	BLOB_RECORD_305 *config;
	TERMINAL *terminal = (TERMINAL *)session->terminal;

	while (*l > 0) {

		switch (session->nCommandState) {

		default:
		case AK300_COMMAND_STATE_LEN_LO:

			session->ptr = (unsigned char *)&session->cmd;

			*session->ptr++ = *data++;

			session->nCommandState = AK300_COMMAND_STATE_LEN_HI;
		
			(*l)--;

			if (*l == 0)
				return AK300_TIMEOUT;

		case AK300_COMMAND_STATE_LEN_HI:
		
			*session->ptr++ = *data++;

			if (session->cmd.header.size > sizeof(AK300COMMAND)) {
				api_log_printf("[AK305] Command too long: 0x%04X, terminal_id=%u\r\n", session->cmd.header.size, terminal->id);
				*l = 0;
				return SESSION_COMPLETE;
			}

			session->nCommandState = AK300_COMMAND_STATE_DATA;
			session->nDataBytesReceived = 2;

			(*l)--;

			if (*l == 0)
				return AK300_TIMEOUT;

		case AK300_COMMAND_STATE_DATA:

			while ((*l > 0)&&(session->nDataBytesReceived != session->cmd.header.size)) {
				session->nDataBytesReceived++;
				*session->ptr++ = *data++;
				(*l)--;
			}

			if (session->nDataBytesReceived != session->cmd.header.size) 
				return AK300_TIMEOUT;
		}

		session->nCommandState = AK300_COMMAND_STATE_LEN_LO;

		TEAStreamDecrypt(((unsigned char *)&session->cmd) + 2, 
			session->cmd.header.size - sizeof(session->cmd.header.size), session->SessionKey);

		unsigned short crc = session->cmd.header.crc;

		session->cmd.header.crc = 0;
		if (crc != crcbuf(AK300_POLYNOM, (unsigned char *)&session->cmd, session->cmd.header.size)) {
			api_log_printf("[AK305] Received Command with invalid CRC, terminal_id=%u\r\n", terminal->id);
			*l = 0;
			data_session_close(session);
			return SESSION_COMPLETE;
		}

		api_log_printf("[AK305] Received command #%u, terminal_id=%u\r\n", session->cmd.header.command_id, terminal->id);

		if ((session->terminal == &dummy_terminal)&&(session->cmd.header.command_id != AK300_CMD_NEGOTIATE)) {
			api_log_printf("[AK305] Terminal not authorized, terminal_id=%u\r\n", terminal->id);
			*l = 0;
			data_session_close(session);
			return SESSION_COMPLETE;
		}

		switch (session->cmd.header.command_id) {
	
		case AK300_CMD_NEGOTIATE:

			if (session->nNegState == 0) {

				session->cmd.payload.CmdNegotiate[15] = '\0';
				memcpy(session->dev_id, session->cmd.payload.CmdNegotiate, 16);

				unsigned char dev_id[8];

				dev_id[0] = ((session->cmd.payload.CmdNegotiate[0]  - '0') << 4) | (session->cmd.payload.CmdNegotiate[1]  - '0');
				dev_id[1] = ((session->cmd.payload.CmdNegotiate[2]  - '0') << 4) | (session->cmd.payload.CmdNegotiate[3]  - '0');
				dev_id[2] = ((session->cmd.payload.CmdNegotiate[4]  - '0') << 4) | (session->cmd.payload.CmdNegotiate[5]  - '0');
				dev_id[3] = ((session->cmd.payload.CmdNegotiate[6]  - '0') << 4) | (session->cmd.payload.CmdNegotiate[7]  - '0');
				dev_id[4] = ((session->cmd.payload.CmdNegotiate[8]  - '0') << 4) | (session->cmd.payload.CmdNegotiate[9]  - '0');
				dev_id[5] = ((session->cmd.payload.CmdNegotiate[10] - '0') << 4) | (session->cmd.payload.CmdNegotiate[11] - '0');
				dev_id[6] = ((session->cmd.payload.CmdNegotiate[12] - '0') << 4) | (session->cmd.payload.CmdNegotiate[13] - '0');
				dev_id[7] = ((session->cmd.payload.CmdNegotiate[14] - '0') << 4);

				std::map<uint64_t, TERMINAL>::iterator it = terminals.find(*(uint64_t *)dev_id);

				if (it == terminals.end()) {

					api_log_printf("[AK305] Unknown terminal [%s]\r\n", session->cmd.payload.CmdNegotiate);

					*l = 0;

					data_session_close(session);

					return SESSION_COMPLETE;
				}

				terminal = &it->second;
				session->terminal = terminal;
				terminal->session = session;

				session->cmd.header.command_id = AK300_CMD_NEGOTIATE;

				for (int i = 0; i < 16; i++) 
					session->cmd.payload.CmdNegotiate[i] = rand();

				memcpy(session->buffer, session->cmd.payload.CmdNegotiate, 16);

				AK305_SendCommand(session, sizeof(session->cmd.payload.CmdNegotiate));

				session->nNegState = 1;

				return AK300_AUTH_TIMEOUT;
			}
			else {
				unsigned char password_hash[16];
				unsigned char response[16];

				MD5Init(&context);
				MD5Update(&context, session->dev_id, 15);
				MD5Final(password_hash, &context);

				tea_enc((unsigned int *)&session->buffer[0], (unsigned int *)password_hash);
				tea_enc((unsigned int *)&session->buffer[8], (unsigned int *)password_hash);
	
				MD5Init(&context);
				MD5Update(&context, session->buffer, 16);
				MD5Final(response, &context);

				if (memcmp(session->cmd.payload.CmdNegotiate, response, 16) != 0) {

					api_log_printf("[AK305] Negotiation failed. Phase 'Authentication', terminal_id=%u\r\n", terminal->id);

					*l = 0;

					data_session_close(session);

					return SESSION_COMPLETE;
				}

				for (int i = 0; i < 16; i++) 
					session->cmd.payload.CmdNegotiate[i] = rand();

				MD5Init(&context);
				MD5Update(&context, session->cmd.payload.CmdNegotiate, 16);
				MD5Final(session->buffer, &context);

				tea_enc((unsigned int *)&session->cmd.payload.CmdNegotiate[0], (unsigned int *)password_hash);
				tea_enc((unsigned int *)&session->cmd.payload.CmdNegotiate[8], (unsigned int *)password_hash);

				AK305_SendCommand(session, sizeof(session->cmd.payload.CmdNegotiate));

				memcpy(session->SessionKey, session->buffer, 16);

				api_log_printf("[AK305] Terminal Authorized [%s], terminal_id=%u\r\n", session->dev_id, terminal->id);
				add_event(terminal, RECORD_EVENT_TERMINAL_ONLINE);

				session->cmd.header.command_id = AK300_CMD_SETTIME;
				session->cmd.payload.CmdSetTime.t = (unsigned int)time(NULL);
				AK305_SendCommand(session, sizeof(session->cmd.payload.CmdSetTime));
			}
			
			return AK300_TIMEOUT;

		case AK300_CMD_POSITION: 

			*l = 0;

			if (session->cmd.payload.CmdPosition.pos_count <= 4) {

				bool store_error = false;

				for (int i = 0; i < session->cmd.payload.CmdPosition.pos_count; i++) {

					record->t = session->cmd.payload.CmdPosition.Positions[i].t;

					if (record->t >= terminal->last_command_time) {

						if (record->t < now + 600) {

							unsigned char	flags1			= terminal->flags1 & 0xF0;
							unsigned char	flags2			= terminal->flags2 & 0xFE;

							int				latitude		= htonl(session->cmd.payload.CmdPosition.Positions[i].latitude);
							int				longitude		= htonl(session->cmd.payload.CmdPosition.Positions[i].longitude);
							unsigned short	speed			= (htons(session->cmd.payload.CmdPosition.Positions[i].speed) * 3600) / 10000;
							unsigned short	altitude		= htons(session->cmd.payload.CmdPosition.Positions[i].altitude) / 100;
							unsigned short	adc1			= session->cmd.payload.CmdPosition.Positions[i].ai1 * 10;
							unsigned short	adc2			= session->cmd.payload.CmdPosition.Positions[i].ai2 * 10;
							unsigned short	adc3			= session->cmd.payload.CmdPosition.Positions[i].ai3 * 10;
							unsigned short	input1_counter	= session->cmd.payload.CmdPosition.Positions[i].di1;
							unsigned int	input1_injector	= session->cmd.payload.CmdPosition.Positions[i].di1;
							unsigned short	input2_counter	= session->cmd.payload.CmdPosition.Positions[i].di2;
							unsigned short	input3_counter	= session->cmd.payload.CmdPosition.Positions[i].di3;
							unsigned short	input4_counter	= session->cmd.payload.CmdPosition.Positions[i].di4;

							if (session->cmd.payload.CmdPosition.Positions[i].event_id == 0x201)
								flags1 |= RECORD_FLAG1_MOVE;
							if (session->cmd.payload.CmdPosition.Positions[i].event_id == 0x202)
								flags1 &= ~RECORD_FLAG1_MOVE;
							if (session->cmd.payload.CmdPosition.Positions[i].event_id == 0x101)
								flags1 |= RECORD_FLAG1_IGNITION;
							if (session->cmd.payload.CmdPosition.Positions[i].event_id == 0x102)
								flags1 &= ~RECORD_FLAG1_IGNITION;
							if (session->cmd.payload.CmdPosition.Positions[i].event_id == 0x0006)
								flags1 |= RECORD_FLAG1_COG_9;
							if (session->cmd.payload.CmdPosition.Positions[i].event_id == 0x0005)
								flags1 &= ~RECORD_FLAG1_COG_9;
							if (session->cmd.payload.CmdPosition.Positions[i].event_id == 0x0004)
								flags1 &= ~RECORD_FLAG1_COG_9;
							if (session->cmd.payload.CmdPosition.Positions[i].event_id == 0x0400)
								flags2 |= RECORD_FLAG2_DI1;
							if (session->cmd.payload.CmdPosition.Positions[i].event_id == 0x0401)
								flags2 |= RECORD_FLAG2_DI2;
							if (session->cmd.payload.CmdPosition.Positions[i].event_id == 0x0402)
								flags2 |= RECORD_FLAG2_DI3;
							if (session->cmd.payload.CmdPosition.Positions[i].event_id == 0x0403)
								flags2 |= RECORD_FLAG2_DI4;
							if (session->cmd.payload.CmdPosition.Positions[i].event_id == 0x0440)
								flags2 &= ~RECORD_FLAG2_DI1;
							if (session->cmd.payload.CmdPosition.Positions[i].event_id == 0x0441)
								flags2 &= ~RECORD_FLAG2_DI2;
							if (session->cmd.payload.CmdPosition.Positions[i].event_id == 0x0442)
								flags2 &= ~RECORD_FLAG2_DI3;
							if (session->cmd.payload.CmdPosition.Positions[i].event_id == 0x0443)
								flags2 &= ~RECORD_FLAG2_DI4;

							*bit1 = RECORD_BIT1_FLAGS | RECORD_BIT_MORE;
							*bit2 = RECORD_BIT2_ADC1 | RECORD_BIT2_ADC2 | RECORD_BIT_MORE;
							*bit3 = RECORD_BIT3_COUNTER1 | RECORD_BIT3_COUNTER2 | RECORD_BIT3_COUNTER3 | RECORD_BIT3_COUNTER4 | RECORD_BIT_MORE;
							*bit4 = RECORD_BIT_MORE;
							*bit5 = RECORD_BIT5_INJECTOR;

							if ((flags1 & RECORD_FLAG1_COG_9)&&(latitude != 0)&&(longitude != 0)) {
								*bit1 |= RECORD_BIT1_NAV | RECORD_BIT1_ALT;
							}

							if (flags1 & RECORD_FLAG1_COG_9) {
		
								if (speed & 0x0100)
									flags1 |= RECORD_FLAG1_SPEED_9;

								if (speed & 0x0200)
									flags1 |= RECORD_FLAG1_SPEED_10;

								if (speed & 0x0400)
									flags1 |= RECORD_FLAG1_SPEED_11;
							}

							if (flags2 != 0)
								flags1 |= RECORD_FLAG1_MORE;

							unsigned char *dst = bit5 + 1;

							*dst++ = flags1;
							if (flags1 & RECORD_FLAG1_MORE)
								*dst++ = flags2;

							if (*bit1 & RECORD_BIT1_NAV) {
								*(int *)dst = latitude;
								dst += 4;
								*(int *)dst = longitude;
								dst += 4;
								*dst++ = speed & 0xFF;
							}

							if (*bit1 & RECORD_BIT1_ALT) {
								*(short *)dst = altitude;
								dst += 2;
							}

							*(unsigned short *)dst = adc1;
							dst += 2;

							*(unsigned short *)dst = adc2;
							dst += 2;

							*(unsigned short *)dst = input1_counter;
							dst += 2;
							*(unsigned short *)dst = input2_counter;
							dst += 2;
							*(unsigned short *)dst = input3_counter;
							dst += 2;
							*(unsigned short *)dst = input4_counter;
							dst += 2;
							*(unsigned int *)dst = input1_injector;
							dst += 4;

							record->size = dst - record_buffer;

							if (api_storage_add_record_to_stream(terminal->object->stream, record, record->size) == 0) {

								terminal->last_command_time = record->t;
								terminal->flags1 = flags1;
								terminal->flags2 = flags2;
							}
							else {
								store_error = true;
								break;
							}
						}
						else {
							api_log_printf("[AK305] Record's time is too far in future, ignoring (this %08X, now %08X), terminal_id=%u\r\n", record->t, now, terminal->id);
						}
					}
					else {
						api_log_printf("[AK305] Record already received, ignoring (this %08X, last %08X), terminal_id=%u\r\n", record->t, terminal->last_command_time, terminal->id);
					}
				}		

				if (store_error == false) {
					session->cmd.header.command_id = AK300_CMD_ACK;	
					AK305_SendCommand(session, 0);
				}				
			}

			break;
		
		case AK300_CMD_VERSION:

			config = (BLOB_RECORD_305 *)terminal->object->module_data;

			if ((config->actual_fw_ver != session->cmd.payload.CmdVersion.nVer)||(config->info[0] == 0)) {
				config->actual_fw_ver = session->cmd.payload.CmdVersion.nVer;
				sprintf((char *)config->info, "{\"fw\":%u}", config->actual_fw_ver);
				api_db_update_object_blob(terminal->object);
			}

			if ((config->requested_fw_ver != 0)&&(config->actual_fw_ver != config->requested_fw_ver)) {

				api_log_printf("[AK305] Firmware upgrade required, actual fw #%u, requested fw #%u, terminal_id=%u\r\n", config->actual_fw_ver, config->requested_fw_ver, terminal->id);			

				session->cmd.header.command_id = AK300_CMD_FIRMWARE;
				AK305_SendCommand(session, 0);
			}

			break;

		case AK300_CMD_KEEPALIVE:

			session->cmd.header.command_id = AK300_CMD_ALIVECONFIRM;
			AK305_SendCommand(session, sizeof(session->cmd.payload.CmdAliveConfirm));

			config = (BLOB_RECORD_305 *)terminal->object->module_data;

			if ((config->requested_fw_ver != 0)&&(config->actual_fw_ver != config->requested_fw_ver)) {

				api_log_printf("[AK305] Firmware upgrade required, actual fw #%u, requested fw #%u, terminal_id=%u\r\n", config->actual_fw_ver, config->requested_fw_ver, terminal->id);			

				session->cmd.header.command_id = AK300_CMD_FIRMWARE;
				AK305_SendCommand(session, sizeof(session->cmd.payload.CmdFirmware));

				break;
			}

			if (config->need_profile) {


				api_log_printf("[AK305] Apply Profile, transaction start, terminal_id=%u\r\n", terminal->id);
			
				session->cmd.header.command_id		= AK300_CMD_OPTION;
      			session->cmd.payload.CmdOption.code	= AK300_OPTION_TRANSACTION;
      			session->cmd.payload.CmdOption.Option.Transaction.code = 0;
			
				AK305_SendCommand(session, sizeof(session->cmd.payload.CmdOption.Option.Transaction) + 1);

				session->nTransactionStatus = AK300_TRANSACTION_COMBO;
	
				break;
			}

			break;

		case AK300_CMD_PROFILE:

			config = (BLOB_RECORD_305 *)terminal->object->module_data;

			api_log_printf("[AK305] Profile report, timestamp = %08X, terminal_id=%u\r\n", session->cmd.payload.CmdProfile.t, terminal->id);

			if (session->cmd.payload.CmdProfile.t != config->timestamp) {

				api_log_printf("[AK305] Apply Profile, transaction start, terminal_id=%u\r\n", terminal->id);
			
				session->cmd.header.command_id		= AK300_CMD_OPTION;
				session->cmd.payload.CmdOption.code	= AK300_OPTION_TRANSACTION;
				session->cmd.payload.CmdOption.Option.Transaction.code = 0;
			
				AK305_SendCommand(session, sizeof(session->cmd.payload.CmdOption.Option.Transaction) + 1);

				session->nTransactionStatus = AK300_TRANSACTION_COMBO;

				break;
			}

			break;

		case AK300_CMD_ACK:

			config = (BLOB_RECORD_305 *)terminal->object->module_data;

			switch (session->nTransactionStatus) {
			case AK300_TRANSACTION_COMBO:

				api_log_printf("[AK305] Transaction acked, sending combo, terminal_id=%u\r\n", session, terminal->id);

				memset(&session->cmd.payload.CmdOption.Option.ak300_combo, 0, sizeof(session->cmd.payload.CmdOption.Option.ak300_combo));

				session->cmd.payload.CmdOption.Option.ak300_combo.DriveInterval			= config->DriveInterval;
				session->cmd.payload.CmdOption.Option.ak300_combo.DrivePPP				= config->DrivePPP;
				session->cmd.payload.CmdOption.Option.ak300_combo.ParkInterval			= config->ParkInterval * 60;
				session->cmd.payload.CmdOption.Option.ak300_combo.ParkPPP				= config->ParkPPP;
				session->cmd.payload.CmdOption.Option.ak300_combo.SpeakerLevel			= config->nCLVL;
				session->cmd.payload.CmdOption.Option.ak300_combo.MicLevel				= config->nCMIC;
				session->cmd.payload.CmdOption.Option.ak300_combo.EchoModel				= config->EchoModel;
				session->cmd.payload.CmdOption.Option.ak300_combo.EchoLevel				= config->EchoLevel;
				session->cmd.payload.CmdOption.Option.ak300_combo.EchoPatterns			= config->EchoPatterns;
				session->cmd.payload.CmdOption.Option.ak300_combo.AutoAnswer			= config->bAutoAnswer;
				session->cmd.payload.CmdOption.Option.ak300_combo.StopMethod			= 0;
				session->cmd.payload.CmdOption.Option.ak300_combo.nMinStopTimeToDetect	= 0;
				session->cmd.payload.CmdOption.Option.ak300_combo.bStaticNavDisabled	= config->bDisableStatic;

				memcpy(session->cmd.payload.CmdOption.Option.ak300_combo.Phonebook[0], config->Phone, sizeof(config->Phone));

				session->cmd.header.command_id		= AK300_CMD_OPTION;
				session->cmd.payload.CmdOption.code	= AK300_OPTION_AK300COMBO;

				AK305_SendCommand(session, sizeof(session->cmd.payload.CmdOption.Option.ak300_combo) + 1);

				session->nTransactionStatus = AK300_TRANSACTION_INPUTS;

				break;

			case AK300_TRANSACTION_INPUTS:

				api_log_printf("[AK305] Combo acked, sending inputs, terminal_id=%u\r\n", session, terminal->id);

				memset(&session->cmd.payload.CmdOption.Option.ak300_inputs, 0, sizeof(session->cmd.payload.CmdOption.Option.ak300_inputs));

				if (strlen(config->Phone) < sizeof(session->cmd.payload.CmdOption.Option.ak300_inputs.DialNumber[0]))
					strcpy(session->cmd.payload.CmdOption.Option.ak300_inputs.DialNumber[0], config->Phone);
				if (strlen(config->Phone) < sizeof(session->cmd.payload.CmdOption.Option.ak300_inputs.DialNumber[1]))
					strcpy(session->cmd.payload.CmdOption.Option.ak300_inputs.DialNumber[1], config->Phone);
				if (strlen(config->Phone) < sizeof(session->cmd.payload.CmdOption.Option.ak300_inputs.DialNumber[2]))
					strcpy(session->cmd.payload.CmdOption.Option.ak300_inputs.DialNumber[2], config->Phone);
				if (strlen(config->Phone) < sizeof(session->cmd.payload.CmdOption.Option.ak300_inputs.DialNumber[3]))
					strcpy(session->cmd.payload.CmdOption.Option.ak300_inputs.DialNumber[3], config->Phone);

				switch (config->Input1Mode & 0x0F) {
				// Ignition
				case 1:
					session->cmd.payload.CmdOption.Option.ak300_inputs.Choice[0] = '1';
					break;
				// Discrete
				case 3:
					session->cmd.payload.CmdOption.Option.ak300_inputs.Choice[0] = (config->Input1SMSOnActive[0] != '\0') ? '5' : '7';
					break;
				// Pulse counter
				case 4:
					session->cmd.payload.CmdOption.Option.ak300_inputs.Choice[0] = 112;
					break;
				// Alarm
				case 10:
					session->cmd.payload.CmdOption.Option.ak300_inputs.Choice[0] = '2';
					break;
				// Injector
				case 9:
					session->cmd.payload.CmdOption.Option.ak300_inputs.Choice[0] = 'i';
					break;
				default:
					session->cmd.payload.CmdOption.Option.ak300_inputs.Choice[0] = '0';
				}

				if (strlen(config->Input1SMSOnActive) < sizeof(session->cmd.payload.CmdOption.Option.ak300_inputs.SMSText[0]))
					strcpy(session->cmd.payload.CmdOption.Option.ak300_inputs.SMSText[0], config->Input1SMSOnActive);

				switch (config->Input2Mode & 0x0F) {
				// Ignition
				case 1:
					session->cmd.payload.CmdOption.Option.ak300_inputs.Choice[1] = '1';
					break;
				// Discrete
				case 3:
					session->cmd.payload.CmdOption.Option.ak300_inputs.Choice[1] = (config->Input2SMSOnActive[1] != '\0') ? '5' : '8';
					break;
				// Pulse counter
				case 4:
					session->cmd.payload.CmdOption.Option.ak300_inputs.Choice[1] = 112;
					break;
				// Alarm
				case 10:
					session->cmd.payload.CmdOption.Option.ak300_inputs.Choice[1] = '2';
					break;
				default:
					session->cmd.payload.CmdOption.Option.ak300_inputs.Choice[1] = '0';
				}

				if (strlen(config->Input2SMSOnActive) < sizeof(session->cmd.payload.CmdOption.Option.ak300_inputs.SMSText[1]))
					strcpy(session->cmd.payload.CmdOption.Option.ak300_inputs.SMSText[1], config->Input2SMSOnActive);

				switch (config->Input3Mode & 0x0F) {
				// Ignition
				case 1:
					session->cmd.payload.CmdOption.Option.ak300_inputs.Choice[2] = '1';
					break;
				// Discrete
				case 3:
					session->cmd.payload.CmdOption.Option.ak300_inputs.Choice[2] = (config->Input3SMSOnActive[0] != '\0') ? '5' : '9';
					break;
				// Pulse counter
				case 4:
					session->cmd.payload.CmdOption.Option.ak300_inputs.Choice[2] = 112;
					break;
				// Alarm
				case 10:
					session->cmd.payload.CmdOption.Option.ak300_inputs.Choice[2] = '2';
					break;
				default:
					session->cmd.payload.CmdOption.Option.ak300_inputs.Choice[2] = '0';
				}

				if (strlen(config->Input3SMSOnActive) < sizeof(session->cmd.payload.CmdOption.Option.ak300_inputs.SMSText[2]))
					strcpy(session->cmd.payload.CmdOption.Option.ak300_inputs.SMSText[2], config->Input3SMSOnActive);

				switch (config->Input4Mode & 0x0F) {
				// Ignition
				case 1:
					session->cmd.payload.CmdOption.Option.ak300_inputs.Choice[3] = '1';
					break;
				// Discrete
				case 3:
					session->cmd.payload.CmdOption.Option.ak300_inputs.Choice[3] = (config->Input4SMSOnActive[0] != '\0') ? '5' : 'a';
					break;
				// Pulse counter
				case 4:
					session->cmd.payload.CmdOption.Option.ak300_inputs.Choice[3] = 112;
					break;
				// Alarm
				case 10:
					session->cmd.payload.CmdOption.Option.ak300_inputs.Choice[3] = '2';
					break;
				default:
					session->cmd.payload.CmdOption.Option.ak300_inputs.Choice[3] = '0';
				}

				if (strlen(config->Input4SMSOnActive) < sizeof(session->cmd.payload.CmdOption.Option.ak300_inputs.SMSText[3]))
					strcpy(session->cmd.payload.CmdOption.Option.ak300_inputs.SMSText[3], config->Input4SMSOnActive);

				session->cmd.header.command_id		= AK300_CMD_OPTION;
				session->cmd.payload.CmdOption.code	= AK300_OPTION_AK300INPUTS;

				AK305_SendCommand(session, sizeof(session->cmd.payload.CmdOption.Option.ak300_inputs) + 1);

				session->nTransactionStatus = AK300_TRANSACTION_OUTPUTS;

				break;

			case AK300_TRANSACTION_OUTPUTS:

				api_log_printf("[AK305] Inputs acked, sending outputs, terminal_id=%u\r\n", terminal->id);

				memset(&session->cmd.payload.CmdOption.Option.ak300_outputs, 0, sizeof(session->cmd.payload.CmdOption.Option.ak300_outputs));

				session->cmd.payload.CmdOption.Option.ak300_outputs.Choice[0] = 49;
				if (strlen(config->sms_a1) < sizeof(session->cmd.payload.CmdOption.Option.ak300_outputs.SMSA[0]))
					strcpy(session->cmd.payload.CmdOption.Option.ak300_outputs.SMSA[0], config->sms_a1);
				if (strlen(config->sms_d1) < sizeof(session->cmd.payload.CmdOption.Option.ak300_outputs.SMSD[0]))
					strcpy(session->cmd.payload.CmdOption.Option.ak300_outputs.SMSD[0], config->sms_d1);

				session->cmd.payload.CmdOption.Option.ak300_outputs.Choice[1] = 49;
				if (strlen(config->sms_a2) < sizeof(session->cmd.payload.CmdOption.Option.ak300_outputs.SMSA[1]))
					strcpy(session->cmd.payload.CmdOption.Option.ak300_outputs.SMSA[1], config->sms_a2);
				if (strlen(config->sms_d2) < sizeof(session->cmd.payload.CmdOption.Option.ak300_outputs.SMSD[1]))
					strcpy(session->cmd.payload.CmdOption.Option.ak300_outputs.SMSD[1], config->sms_d2);

				session->cmd.payload.CmdOption.Option.ak300_outputs.Choice[2] = 49;
				if (strlen(config->sms_a3) < sizeof(session->cmd.payload.CmdOption.Option.ak300_outputs.SMSA[2]))
					strcpy(session->cmd.payload.CmdOption.Option.ak300_outputs.SMSA[2], config->sms_a3);
				if (strlen(config->sms_d3) < sizeof(session->cmd.payload.CmdOption.Option.ak300_outputs.SMSD[2]))
					strcpy(session->cmd.payload.CmdOption.Option.ak300_outputs.SMSD[2], config->sms_d3);

				session->cmd.payload.CmdOption.Option.ak300_outputs.Choice[3] = 49;
				if (strlen(config->sms_a4) < sizeof(session->cmd.payload.CmdOption.Option.ak300_outputs.SMSA[3]))
					strcpy(session->cmd.payload.CmdOption.Option.ak300_outputs.SMSA[3], config->sms_a4);
				if (strlen(config->sms_d4) < sizeof(session->cmd.payload.CmdOption.Option.ak300_outputs.SMSD[3]))
					strcpy(session->cmd.payload.CmdOption.Option.ak300_outputs.SMSD[3], config->sms_d4);

				session->cmd.header.command_id		= AK300_CMD_OPTION;
				session->cmd.payload.CmdOption.code	= AK300_OPTION_AK300OUTPUTS;

				AK305_SendCommand(session, sizeof(session->cmd.payload.CmdOption.Option.ak300_outputs) + 1);

				session->nTransactionStatus = AK300_TRANSACTION_FINISH;

				break;

			case AK300_TRANSACTION_FINISH:

				api_log_printf("[AK305] Outputs acked, transaction commit, terminal_id=%u\r\n", terminal->id);

				session->cmd.header.command_id		= AK300_CMD_OPTION;
	      		session->cmd.payload.CmdOption.code = AK300_OPTION_TRANSACTION;
	      		session->cmd.payload.CmdOption.Option.Transaction.code = 1;
				session->cmd.payload.CmdOption.Option.Transaction.t = (int)config->timestamp;
			
				AK305_SendCommand(session, sizeof(session->cmd.payload.CmdOption.Option.Transaction) + 1);

				config->need_profile = 0;
				api_db_update_object_blob(terminal->object);

				session->nTransactionStatus = 0;
			
				break;
			}

			break;

		case AK300_CMD_REASON:

			api_log_printf("[AK305] Restart Reason 0x%08X, terminal_id=%u\r\n", session->cmd.payload.CmdReason.nReason, terminal->id);
		
			add_event(terminal, RECORD_EVENT_REASON + session->cmd.payload.CmdReason.nReason);

			break;

		default:               

			api_log_printf("[AK305] Unknown command #%u, terminal_id=%u\r\n", session->cmd.header.command_id, terminal->id);

			break;
		}
	}

	*l = 0;

	return AK300_TIMEOUT;
}

