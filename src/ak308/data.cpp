//******************************************************************************
//
// File Name : data.cpp
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#include <string.h>
#include <time.h>
#include <limits.h>
#include "api.h"
#include "crc16.h"
#include "config.h"
#include "common.h"
#include "json.h"
#include "../core/record.h"

static AK308COMMAND *cmd;

static unsigned char *bit1 = record_data + 0;
static unsigned char *bit2 = record_data + 1;
static unsigned char *bit3 = record_data + 2;
static unsigned char *bit4 = record_data + 3;
static unsigned char *bit5 = record_data + 4;
static unsigned char *bit6 = record_data + 5;

void construct_command(TERMINAL *terminal, size_t payload_len) 
{
	terminal->tx_cmd.header.size	= sizeof(AK308_COMMAND_HEADER) + payload_len;
	terminal->tx_cmd.header.seqNo	= terminal->next_tx_seq_no++;
	terminal->tx_cmd.header.crc		= 0x0000;
	terminal->tx_cmd.header.crc		= crcbuf(AK308_POLYNOM, (unsigned char *)&terminal->tx_cmd, terminal->tx_cmd.header.size);

	terminal->send_attempt			= 0;
	terminal->ack_timeout			= now + send_attempt_intervals[0];

	api_log_printf("[AK308] Send command %u, seq_no: %u, terminal_id=%u\r\n", terminal->tx_cmd.header.command_id, terminal->tx_cmd.header.seqNo, terminal->id);
}

static inline void construct_ack(TERMINAL *terminal)
{
	cmd->payload.CmdAck.seqNo	= cmd->header.seqNo;

	cmd->header.size			= sizeof(AK308_COMMAND_HEADER) + sizeof(AK308_ACK_PAYLOAD);
	cmd->header.command_id		= AK308_CMD_ACK;
	cmd->header.seqNo			= terminal->next_tx_seq_no;
	cmd->header.crc				= 0x0000;
	cmd->header.crc				= crcbuf(AK308_POLYNOM, (unsigned char *)cmd, sizeof(AK308_COMMAND_HEADER) + sizeof(AK308_ACK_PAYLOAD));

	api_log_printf("[AK308] Send command 8, seq_no: %u, terminal_id=%u\r\n", cmd->header.seqNo, terminal->id);
}

static inline void construct_nack(TERMINAL *terminal)
{
	cmd->payload.CmdNack.seqNo	= cmd->header.seqNo;

	cmd->header.size			= sizeof(AK308_COMMAND_HEADER) + sizeof(AK308_NACK_PAYLOAD);
	cmd->header.command_id		= AK308_CMD_NACK;
	cmd->header.seqNo			= terminal->next_tx_seq_no;
	cmd->header.crc				= 0x0000;
	cmd->header.crc				= crcbuf(AK308_POLYNOM, (unsigned char *)cmd, sizeof(AK308_COMMAND_HEADER) + sizeof(AK308_NACK_PAYLOAD));

	api_log_printf("[AK308] Send command 9, seq_no: %u, terminal_id=%u\r\n", cmd->header.seqNo, terminal->id);
}

void construct_profile(TERMINAL *terminal)
{
	BLOB_RECORD *config = (BLOB_RECORD *)terminal->object->module_data;

	memset(&terminal->tx_cmd.payload.CmdConfig, 0, sizeof(terminal->tx_cmd.payload.CmdConfig));

	/*****************************************************************************************************/
	/*** COMMON ***/
	/*****************************************************************************************************/

	terminal->tx_cmd.payload.CmdConfig.CurrentTime			= (unsigned int)now;
	terminal->tx_cmd.payload.CmdConfig.ProfileTimestamp		= (unsigned int)config->timestamp;

	terminal->tx_cmd.payload.CmdConfig.wakeup				= config->wakeup;

	terminal->tx_cmd.payload.CmdConfig.vcc_threshold		= config->vcc_threshold;
	terminal->tx_cmd.payload.CmdConfig.vcc_active_time		= config->vcc_active_time;
	terminal->tx_cmd.payload.CmdConfig.vcc_inactive_time	= config->vcc_inactive_time;
	
	memcpy(terminal->tx_cmd.payload.CmdConfig.vcc_sms_on_active, config->vcc_sms_on_active, 16);
	memcpy(terminal->tx_cmd.payload.CmdConfig.vcc_sms_on_inactive, config->vcc_sms_on_inactive, 16);

	terminal->tx_cmd.payload.CmdConfig.accell_sense			= config->accell_sense;
	terminal->tx_cmd.payload.CmdConfig.accell_active_time	= config->accell_active_time;
	terminal->tx_cmd.payload.CmdConfig.accell_inactive_time	= config->accell_inactive_time;

	memcpy(terminal->tx_cmd.payload.CmdConfig.accell_sms_on_active, config->accell_sms_on_active, 16);
	memcpy(terminal->tx_cmd.payload.CmdConfig.accell_sms_on_inactive, config->accell_sms_on_inactive, 16);

	terminal->tx_cmd.payload.CmdConfig.lls_period			= config->fuel_period;
	terminal->tx_cmd.payload.CmdConfig.lls_threshold		= config->fuel_threshold;

	terminal->tx_cmd.payload.CmdConfig.io_mode = config->port;

	memcpy(terminal->tx_cmd.payload.CmdConfig.Phone, config->Phone, 32);

	terminal->tx_cmd.payload.CmdConfig.mask	= 0;

	if (config->altitude)
		terminal->tx_cmd.payload.CmdConfig.mask |= AK308_MASK_ALTITUDE;
	if (config->cog)
		terminal->tx_cmd.payload.CmdConfig.mask |= AK308_MASK_COG;
	if ((config->vcc_expanded)&&(config->vcc_transmit))
		terminal->tx_cmd.payload.CmdConfig.mask |= AK308_MASK_VCC;
	if ((config->accell_expanded)&&(config->accell_transmit))
		terminal->tx_cmd.payload.CmdConfig.mask |= AK308_MASK_ACCELL;
	if (config->port == AK308_IO_MODE_485)
		terminal->tx_cmd.payload.CmdConfig.mask |= AK308_MASK_LLS1 | AK308_MASK_LLS2;

	terminal->tx_cmd.payload.CmdConfig.Input1Flags = 0;
	terminal->tx_cmd.payload.CmdConfig.Input2Flags = 0;

	/*****************************************************************************************************/
	/*** MODES ***/
	/*****************************************************************************************************/

	/* Drive Mode Settings */
	terminal->tx_cmd.payload.CmdConfig.DriveInterval	= config->DriveInterval;
	terminal->tx_cmd.payload.CmdConfig.DrivePPP			= config->DrivePPP;
	terminal->tx_cmd.payload.CmdConfig.DriveFlags		= 0;

	/* Sleep Mode Settings */
	if (config->wait_expanded) {
		terminal->tx_cmd.payload.CmdConfig.SleepDelayInterval	= config->SleepDelayInterval;
		terminal->tx_cmd.payload.CmdConfig.SleepWakeupInterval	= config->SleepWakeupInterval;
		terminal->tx_cmd.payload.CmdConfig.SleepFlags			= 0;
		if (config->wait_after_fuel)
			terminal->tx_cmd.payload.CmdConfig.SleepFlags |= AK308_MODE_FLAG_WAIT_FOR_STABLE_LLS;
		if (config->wait_to_active_fuel)
			terminal->tx_cmd.payload.CmdConfig.SleepFlags |= AK308_MODE_FLAG_SWITCH_TO_ACTIVE_ON_LLS;
	}
	else {
		terminal->tx_cmd.payload.CmdConfig.SleepDelayInterval = 0;
	}

	/* Sleep Deep Mode Settings */
	if (config->sleep_expanded) {
		terminal->tx_cmd.payload.CmdConfig.SleepDeepDelayInterval		= config->SleepDeepDelayInterval;
		terminal->tx_cmd.payload.CmdConfig.SleepDeepWakeupInterval		= config->SleepDeepWakeupInterval;
		terminal->tx_cmd.payload.CmdConfig.SleepDeepMaxGPSSearchTime	= config->SleepDeepMaxGPSSearchTime;
		terminal->tx_cmd.payload.CmdConfig.SleepDeepMaxGSMSearchTime	= config->SleepDeepMaxGSMSearchTime;
		terminal->tx_cmd.payload.CmdConfig.SleepDeepFlags				= 0;
		if (config->sleep_after_fuel)
			terminal->tx_cmd.payload.CmdConfig.SleepFlags |= AK308_MODE_FLAG_WAIT_FOR_STABLE_LLS;
		if (config->sleep_to_active_fuel)
			terminal->tx_cmd.payload.CmdConfig.SleepFlags |= AK308_MODE_FLAG_SWITCH_TO_ACTIVE_ON_LLS;
	}
	else {
		terminal->tx_cmd.payload.CmdConfig.SleepDeepDelayInterval = 0;
	}

	/*****************************************************************************************************/
	/*** INPUT 1 ***/
	/*****************************************************************************************************/

	if (config->wakeup == AK308_SOURCE_INPUT1) {
		terminal->tx_cmd.payload.CmdConfig.Input1Mode = AK308_INPUT_MODE_DISCRETE;
		terminal->tx_cmd.payload.CmdConfig.Input1ActiveInterval		= config->wakeup_active_interval;
		terminal->tx_cmd.payload.CmdConfig.Input1InactiveInterval	= config->wakeup_inactive_interval;
		memcpy(terminal->tx_cmd.payload.CmdConfig.Input1SMSOnActive,	config->wakeup_sms_on_active, 16);
		memcpy(terminal->tx_cmd.payload.CmdConfig.Input1SMSOnInactive,	config->wakeup_sms_on_inactive, 16);
	}
	else
	if (config->input1_expanded) {

		terminal->tx_cmd.payload.CmdConfig.Input1ActiveInterval		= config->Input1ActiveInterval;
		terminal->tx_cmd.payload.CmdConfig.Input1InactiveInterval	= config->Input1InactiveInterval;
		memcpy(terminal->tx_cmd.payload.CmdConfig.Input1SMSOnActive,	config->Input1SMSOnActive, 16);
		memcpy(terminal->tx_cmd.payload.CmdConfig.Input1SMSOnInactive,	config->Input1SMSOnInactive, 16);

		switch (config->Input1Mode) {
		default:
		/* Unconnected */
		case 0:
			terminal->tx_cmd.payload.CmdConfig.Input1Mode = AK308_INPUT_MODE_UNCONNECTED;
			break;
		/* Discrete sensor */
		case 1:
			terminal->tx_cmd.payload.CmdConfig.Input1Mode = AK308_INPUT_MODE_DISCRETE;
			break;
		/* Pulse Counter */
		case 2:
			terminal->tx_cmd.payload.CmdConfig.Input1Mode = AK308_INPUT_MODE_COUNTER;
			terminal->tx_cmd.payload.CmdConfig.mask |= AK308_MASK_INPUT1COUNTER;
			break;
		/* Frequency */
		case 3:
			terminal->tx_cmd.payload.CmdConfig.Input1Mode = AK308_INPUT_MODE_FREQUENCY;
			terminal->tx_cmd.payload.CmdConfig.mask |= AK308_MASK_INPUT1FREQUENCY;
			break;
		/* LLS */
		case 4:
			terminal->tx_cmd.payload.CmdConfig.Input1Mode = AK308_INPUT_MODE_FREQUENCY;
			terminal->tx_cmd.payload.CmdConfig.Input1Flags = AK308_INPUT_FLAG_LLS;
			terminal->tx_cmd.payload.CmdConfig.mask |= AK308_MASK_INPUT1FREQUENCY;
			break;
		/* RS485 */
		case 5:
			terminal->tx_cmd.payload.CmdConfig.Input1Mode = AK308_INPUT_MODE_RS485_B;
			terminal->tx_cmd.payload.CmdConfig.Input1Flags = AK308_INPUT_FLAG_LLS;
			terminal->tx_cmd.payload.CmdConfig.mask |= AK308_MASK_LLS1;
			break;
		}
	}
	else
	{
		terminal->tx_cmd.payload.CmdConfig.Input1Mode = AK308_INPUT_MODE_UNCONNECTED;
	}


	/*****************************************************************************************************/
	/*** INPUT 2 ***/
	/*****************************************************************************************************/

	if (config->wakeup == AK308_SOURCE_INPUT2) {
		terminal->tx_cmd.payload.CmdConfig.Input2Mode = AK308_INPUT_MODE_DISCRETE;
		terminal->tx_cmd.payload.CmdConfig.Input2ActiveInterval		= config->wakeup_active_interval;
		terminal->tx_cmd.payload.CmdConfig.Input2InactiveInterval	= config->wakeup_inactive_interval;
		memcpy(terminal->tx_cmd.payload.CmdConfig.Input2SMSOnActive,	config->wakeup_sms_on_active, 16);
		memcpy(terminal->tx_cmd.payload.CmdConfig.Input2SMSOnInactive,	config->wakeup_sms_on_inactive, 16);
	}
	else
	if (config->input2_expanded) {
	
		terminal->tx_cmd.payload.CmdConfig.Input2ActiveInterval		= config->Input2ActiveInterval;
		terminal->tx_cmd.payload.CmdConfig.Input2InactiveInterval	= config->Input2InactiveInterval;
		memcpy(terminal->tx_cmd.payload.CmdConfig.Input2SMSOnActive,	config->Input2SMSOnActive, 16);
		memcpy(terminal->tx_cmd.payload.CmdConfig.Input2SMSOnInactive,	config->Input2SMSOnInactive, 16);

		switch (config->Input2Mode) {
		default:
		/* Unconnected */
		case 0:
			terminal->tx_cmd.payload.CmdConfig.Input2Mode = AK308_INPUT_MODE_UNCONNECTED;
			break;
		case 1:
			terminal->tx_cmd.payload.CmdConfig.Input2Mode = AK308_INPUT_MODE_DISCRETE;
			break;
		/* Pulse Counter */
		case 2:
			terminal->tx_cmd.payload.CmdConfig.Input2Mode = AK308_INPUT_MODE_COUNTER;
			terminal->tx_cmd.payload.CmdConfig.mask |= AK308_MASK_INPUT2COUNTER;
			break;
		/* Frequency */
		case 3:
			terminal->tx_cmd.payload.CmdConfig.Input2Mode = AK308_INPUT_MODE_FREQUENCY;
			terminal->tx_cmd.payload.CmdConfig.mask |= AK308_MASK_INPUT2FREQUENCY;
			break;
		/* LLS */
		case 4:
			terminal->tx_cmd.payload.CmdConfig.Input2Mode = AK308_INPUT_MODE_FREQUENCY;
			terminal->tx_cmd.payload.CmdConfig.Input2Flags = AK308_INPUT_FLAG_LLS;
			terminal->tx_cmd.payload.CmdConfig.mask |= AK308_MASK_INPUT2FREQUENCY;
			break;
		/* RS485 */
		case 5:
			terminal->tx_cmd.payload.CmdConfig.Input2Mode = AK308_INPUT_MODE_RS485_B;
			terminal->tx_cmd.payload.CmdConfig.Input2Flags = AK308_INPUT_FLAG_LLS;
			terminal->tx_cmd.payload.CmdConfig.mask |= AK308_MASK_LLS2;
			break;
		}
	}
	else {
		terminal->tx_cmd.payload.CmdConfig.Input2Mode = AK308_INPUT_MODE_UNCONNECTED;
	}

	terminal->tx_cmd.header.command_id = AK308_CMD_CONFIG;
	construct_command(terminal, sizeof(terminal->tx_cmd.payload.CmdConfig));
}

static void cp1251_to_utf8(char *out, const char *in) {
    static const int table[128] = {                    
        0x82D0,0x83D0,0x9A80E2,0x93D1,0x9E80E2,0xA680E2,0xA080E2,0xA180E2,
        0xAC82E2,0xB080E2,0x89D0,0xB980E2,0x8AD0,0x8CD0,0x8BD0,0x8FD0,    
        0x92D1,0x9880E2,0x9980E2,0x9C80E2,0x9D80E2,0xA280E2,0x9380E2,0x9480E2,
        0,0xA284E2,0x99D1,0xBA80E2,0x9AD1,0x9CD1,0x9BD1,0x9FD1,               
        0xA0C2,0x8ED0,0x9ED1,0x88D0,0xA4C2,0x90D2,0xA6C2,0xA7C2,              
        0x81D0,0xA9C2,0x84D0,0xABC2,0xACC2,0xADC2,0xAEC2,0x87D0,              
        0xB0C2,0xB1C2,0x86D0,0x96D1,0x91D2,0xB5C2,0xB6C2,0xB7C2,              
        0x91D1,0x9684E2,0x94D1,0xBBC2,0x98D1,0x85D0,0x95D1,0x97D1,            
        0x90D0,0x91D0,0x92D0,0x93D0,0x94D0,0x95D0,0x96D0,0x97D0,
        0x98D0,0x99D0,0x9AD0,0x9BD0,0x9CD0,0x9DD0,0x9ED0,0x9FD0,
        0xA0D0,0xA1D0,0xA2D0,0xA3D0,0xA4D0,0xA5D0,0xA6D0,0xA7D0,
        0xA8D0,0xA9D0,0xAAD0,0xABD0,0xACD0,0xADD0,0xAED0,0xAFD0,
        0xB0D0,0xB1D0,0xB2D0,0xB3D0,0xB4D0,0xB5D0,0xB6D0,0xB7D0,
        0xB8D0,0xB9D0,0xBAD0,0xBBD0,0xBCD0,0xBDD0,0xBED0,0xBFD0,
        0x80D1,0x81D1,0x82D1,0x83D1,0x84D1,0x85D1,0x86D1,0x87D1,
        0x88D1,0x89D1,0x8AD1,0x8BD1,0x8CD1,0x8DD1,0x8ED1,0x8FD1
    };
    while (*in)
        if (*in & 0x80) {
            int v = table[(int)(0x7f & *in++)];
            if (!v)
                continue;
            *out++ = (char)v;
            *out++ = (char)(v >> 8);
            if (v >>= 16)
                *out++ = (char)v;
        }
        else
            *out++ = *in++;
    *out = 0;
}

int data(unsigned char **p, size_t *l, void *ctx, size_t ctx_len)
{
	unsigned char *ptr;
	cmd = (AK308COMMAND *)*p;
	bool store_error;

	if (*l < sizeof(AK308_COMMAND_HEADER)) {
		*l = 0;
		return 0;
	}
	
	unsigned short crc = cmd->header.crc;
		
	cmd->header.crc = 0x0000;
		
	if (crc != crcbuf(AK308_POLYNOM, *p, *l)) {

		api_log_printf("[AK308] Invalid CRC\r\n");

		*l = 0;

		return 0;
	}

	TERMINAL *terminal = NULL;

	ptr = id;

	for (size_t i = 0; i < terminals.size(); i++, ptr += 8) {

		if (memcmp(ptr, cmd->header.imei, 8) == 0) {
			terminal = &terminals[i];
			break;
		}
	}

	if (terminal == NULL) {

		char imei[16];

		imei[0] = (cmd->header.imei[0] >> 4) + '0';
		imei[1] = (cmd->header.imei[0] & 0x0F) + '0';
		imei[2] = (cmd->header.imei[1] >> 4) + '0';
		imei[3] = (cmd->header.imei[1] & 0x0F) + '0';
		imei[4] = (cmd->header.imei[2] >> 4) + '0';
		imei[5] = (cmd->header.imei[2] & 0x0F) + '0';
		imei[6] = (cmd->header.imei[3] >> 4) + '0';
		imei[7] = (cmd->header.imei[3] & 0x0F) + '0';
		imei[8] = (cmd->header.imei[4] >> 4) + '0';
		imei[9] = (cmd->header.imei[4] & 0x0F) + '0';
		imei[10] = (cmd->header.imei[5] >> 4) + '0';
		imei[11] = (cmd->header.imei[5] & 0x0F) + '0';
		imei[12] = (cmd->header.imei[6] >> 4) + '0';
		imei[13] = (cmd->header.imei[6] & 0x0F) + '0';
		imei[14] = (cmd->header.imei[7] >> 4) + '0';
		imei[15] = '\0';

		api_log_printf("[AK308] Unknown terminal [%s]\r\n", imei);

		cmd->payload.CmdNack.seqNo	= cmd->header.seqNo;

		cmd->header.size			= sizeof(AK308_COMMAND_HEADER) + sizeof(AK308_NACK_PAYLOAD);
		cmd->header.command_id		= AK308_CMD_NACK;
		cmd->header.seqNo			= 1;
		cmd->header.crc				= 0x0000;
		cmd->header.crc				= crcbuf(AK308_POLYNOM, (unsigned char *)cmd, sizeof(AK308_COMMAND_HEADER) + sizeof(AK308_NACK_PAYLOAD));

		api_log_printf("[AK308] Send command 9, seq_no: 1, terminal_id=unknown\r\n");

		*p = (unsigned char *)cmd;
		*l = sizeof(AK308_COMMAND_HEADER) + sizeof(AK308_NACK_PAYLOAD);
		
		return 0;
	}

	BLOB_RECORD *config = (BLOB_RECORD *)terminal->object->module_data;

	if (ctx_len <= sizeof(terminal->context))
		memcpy(terminal->context, ctx, ctx_len);

	terminal->session_timeout = now + 300;
	terminal->last_rx_seq_no = cmd->header.seqNo;

	*l = 0;

	if ((cmd->header.seqNo == 1)&&(cmd->header.command_id == AK308_CMD_INFO)) {

		terminal->last_rx_seq_no	= 1;
		terminal->next_tx_seq_no	= 1;
		terminal->ack_timeout		= UINT_MAX;
		terminal->online			= true;

		char imei[16];

		imei[0] = (cmd->header.imei[0] >> 4) + '0';
		imei[1] = (cmd->header.imei[0] & 0x0F) + '0';
		imei[2] = (cmd->header.imei[1] >> 4) + '0';
		imei[3] = (cmd->header.imei[1] & 0x0F) + '0';
		imei[4] = (cmd->header.imei[2] >> 4) + '0';
		imei[5] = (cmd->header.imei[2] & 0x0F) + '0';
		imei[6] = (cmd->header.imei[3] >> 4) + '0';
		imei[7] = (cmd->header.imei[3] & 0x0F) + '0';
		imei[8] = (cmd->header.imei[4] >> 4) + '0';
		imei[9] = (cmd->header.imei[4] & 0x0F) + '0';
		imei[10] = (cmd->header.imei[5] >> 4) + '0';
		imei[11] = (cmd->header.imei[5] & 0x0F) + '0';
		imei[12] = (cmd->header.imei[6] >> 4) + '0';
		imei[13] = (cmd->header.imei[6] & 0x0F) + '0';
		imei[14] = (cmd->header.imei[7] >> 4) + '0';
		imei[15] = '\0';

		api_log_printf("[AK308] Terminal Authorized [%s], terminal_id=%u\r\n", imei, terminal->id);

		/* Validate szBalance field */
		size_t i;
		for (i = 0; i < sizeof(cmd->payload.CmdInfo.szBalance); i++) {
			if (cmd->payload.CmdInfo.szBalance[i] < ' ') {
				if (cmd->payload.CmdInfo.szBalance[i] == '\0')
					break;
				cmd->payload.CmdInfo.szBalance[0] = '\0';
				break;
			}
		}

		if (i == sizeof(cmd->payload.CmdInfo.szBalance))
			cmd->payload.CmdInfo.szBalance[0] = '\0';

		char my_buf[512];

		cp1251_to_utf8(my_buf, (char *)cmd->payload.CmdInfo.szBalance);
		
		size_t balance_len = strlen(my_buf);

		/* At this point szBalance either valid or blank */

		if ((balance_len > 0)||(config->actual_fw_ver != cmd->payload.CmdInfo.nVer)||(config->info[0] == 0)) {

			unsigned char *ptr = config->info;
			
			*ptr++ = '{';
			
			ptr = json_add_uint(ptr, "fw", config->actual_fw_ver);

			*ptr++ = ',';

			ptr = json_add_string(ptr, "balance", my_buf, balance_len);

			*ptr++ = '}';
			*ptr++ = '\0';

			config->actual_fw_ver = cmd->payload.CmdInfo.nVer;

			api_db_update_object_blob(terminal->object);
		}

		if ((config->requested_fw_ver != 0)&&(config->actual_fw_ver != config->requested_fw_ver)) {
			api_log_printf("[AK308] Request fw: %u, Actual fw: %u\r\n", config->requested_fw_ver, config->actual_fw_ver);
			construct_ack(terminal);
			api_send_udp((unsigned char *)cmd, sizeof(AK308_COMMAND_HEADER) + sizeof(AK308_ACK_PAYLOAD), ctx);
			terminal->tx_cmd.header.command_id = AK308_CMD_FIRMWARE;
			construct_command(terminal, 0);
		}
		else
		if( (cmd->payload.CmdInfo.tProfile == 0) || (cmd->payload.CmdInfo.tProfile != config->timestamp)) {
			construct_ack(terminal);
			api_send_udp((unsigned char *)cmd, sizeof(AK308_COMMAND_HEADER) + sizeof(AK308_ACK_PAYLOAD), ctx);
			api_log_printf("[AK308] Profile time in database: %08X, Profile time on device: %08X, terminal_id=%u\r\n", (unsigned int)config->timestamp, cmd->payload.CmdInfo.tProfile, terminal->id);
			construct_profile(terminal);
		}
		else {
			construct_ack(terminal);
			api_send_udp((unsigned char *)cmd, sizeof(AK308_COMMAND_HEADER) + sizeof(AK308_ACK_PAYLOAD), ctx);
			terminal->tx_cmd.header.command_id		= AK308_CMD_SETTIME;
			terminal->tx_cmd.payload.CmdSetTime.t	= (unsigned int)now;
			construct_command(terminal, sizeof(terminal->tx_cmd.payload.CmdSetTime));
		}

		if (terminal->object->stream == NULL) {
			api_log_printf("[AK308] Unable to find stream for terminal_id=%d\r\n", terminal->id);

			construct_nack(terminal);

			*p = (unsigned char *)cmd;
			*l = sizeof(AK308_COMMAND_HEADER) + sizeof(AK308_NACK_PAYLOAD);

			return 0;
		}
		else {
		
			add_event(terminal, RECORD_EVENT_TERMINAL_ONLINE);

			if (cmd->payload.CmdInfo.nReason)
				add_event(terminal, RECORD_EVENT_REASON + cmd->payload.CmdInfo.nReason);
		}

		*p = (unsigned char *)&terminal->tx_cmd;
		*l = terminal->tx_cmd.header.size;

		return 0;
	}
	
	if (terminal->online == false) {

		api_log_printf("[AK308] Terminal's session is closed, terminal_id=%u\r\n", terminal->id);

		construct_nack(terminal);

		*p = (unsigned char *)cmd;
		*l = sizeof(AK308_COMMAND_HEADER) + sizeof(AK308_NACK_PAYLOAD);

		return 0;
	}

	api_log_printf("[AK308] Received command #%u, seq_no: %u, terminal_id=%u\r\n", cmd->header.command_id, cmd->header.seqNo, terminal->id);

	switch(cmd->header.command_id) {
	case AK308_CMD_CONFIG:

		add_event(terminal, RECORD_EVENT_TERMINAL_PROFILE);

		construct_ack(terminal);
		api_send_udp((unsigned char *)cmd, sizeof(AK308_COMMAND_HEADER) + sizeof(AK308_ACK_PAYLOAD), ctx);

		construct_profile(terminal);
		*p = (unsigned char *)&terminal->tx_cmd;
		*l = terminal->tx_cmd.header.size;

		return 0;

	case AK308_CMD_ACK:

		if (cmd->payload.CmdAck.seqNo == terminal->tx_cmd.header.seqNo) {

			terminal->ack_timeout = UINT_MAX;

			if (terminal->tx_cmd.header.command_id == AK308_CMD_CONFIG) {
				config->need_profile = false;
				api_db_update_object_blob(terminal->object);
			}
		}

		return 0;

	case AK308_CMD_NACK:

		close_connection(terminal);
		
		return 0;

	case AK308_CMD_POSITION:

		ptr = cmd->payload.buffer;

		if (*ptr++ > 4)
			break;

		store_error = false;

		for (size_t i = 0; i < *cmd->payload.buffer; i++) {

			unsigned char *dst;

			unsigned short mask = *(unsigned int *)ptr; ptr+=2;

			record->t = *(unsigned int *)ptr; ptr+=4;

			int latitude  = *((int *)ptr); ptr+=4;
			int longitude = *((int *)ptr); ptr+=4;
			unsigned short speed = ((*((unsigned short *)ptr))*3600)/10000; ptr+=2;

			unsigned short flags = *(unsigned short *)ptr; ptr+=2;

			int altitude;
			unsigned int input1_counter;
			unsigned int input1_frequency;
			unsigned int input2_counter;
			unsigned int input2_frequency;
			unsigned int cog;
			unsigned int vcc;
			unsigned short lls1;
			unsigned short lls2;
			unsigned short accell;
			unsigned char record_flags1 = 0;
			unsigned char record_flags2 = 0;

			if (speed & 0x0100)
				record_flags1 |= RECORD_FLAG1_SPEED_9;

			if (speed & 0x0200)
				record_flags1 |= RECORD_FLAG1_SPEED_10;

			if (speed & 0x0400)
				record_flags1 |= RECORD_FLAG1_SPEED_11;

			if (flags & AK308_POINT_FLAG_DI1) {
				record_flags2 |= RECORD_FLAG2_DI1;
				record_flags1 |= RECORD_FLAG1_MORE;
			}

			if (flags & AK308_POINT_FLAG_DI2) {
				record_flags2 |= RECORD_FLAG2_DI2;
				record_flags1 |= RECORD_FLAG1_MORE;
			}

			if ((flags & AK308_POINT_FLAG_VCC)&&(config->vcc_as_di)) {
				record_flags2 |= RECORD_FLAG2_DI3;
				record_flags1 |= RECORD_FLAG1_MORE;
			}

			if ((flags & AK308_POINT_FLAG_ACCEL)&&(config->accell_as_di)) {
				record_flags2 |= RECORD_FLAG2_DI4;
				record_flags1 |= RECORD_FLAG1_MORE;
			}

			if (flags & AK308_POINT_FLAG_SLEEP) {
				record_flags2 |= RECORD_FLAG2_WAIT;
				record_flags1 |= RECORD_FLAG1_MORE;
			}
			
			if (flags & AK308_POINT_FLAG_SLEEPDEEP) {
				record_flags2 |= RECORD_FLAG2_SLEEP;
				record_flags1 |= RECORD_FLAG1_MORE;
			}

			*bit1 = 0;
			*bit2 = 0;
			*bit3 = 0;
			*bit4 = 0;
			*bit5 = 0;
			*bit6 = 0;

			*bit1 |= RECORD_BIT1_FLAGS;
			dst = bit2;

			if ((flags & AK308_POINT_FLAG_NAVVALID)&&(latitude != 0)&&(longitude != 0))			
				*bit1 |= RECORD_BIT1_NAV;				
		
			if (mask & AK308_MASK_ALTITUDE) {
				if (*bit1 & RECORD_BIT1_NAV) {
					*bit1 |= RECORD_BIT1_ALT;
					altitude = *(short *)ptr; 
				}
				ptr += 2;
			}
			
			if (mask & AK308_MASK_COG) {
				if (*bit1 & RECORD_BIT1_NAV) {
					*bit1 |= RECORD_BIT1_COG;
					cog = *(unsigned short *)ptr; 

					if (cog > 255)
						record_flags1 |= RECORD_FLAG1_COG_9;
				}
				ptr += 2;
			}

			if (mask & AK308_MASK_VCC) {

				vcc = *(unsigned short *)ptr; 
				ptr += 2;

				*bit1 |= RECORD_BIT_MORE;
				*bit2 |= RECORD_BIT_MORE;
				*bit3 |= RECORD_BIT3_VCC;

				dst = bit4;
			}

			if (mask & AK308_MASK_INPUT1COUNTER) {
				input1_counter = *(unsigned int *)ptr; ptr += 4;
				*bit1 |= RECORD_BIT_MORE;
				*bit2 |= RECORD_BIT_MORE;
				*bit3 |= RECORD_BIT3_COUNTER1;
				dst = bit4;
			}

			if (mask & AK308_MASK_INPUT1FREQUENCY) {
				input1_frequency = *(unsigned short *)ptr; ptr += 2;
				*bit1 |= RECORD_BIT_MORE;
				*bit2 |= RECORD_BIT2_FREQUENCY1;
				if (dst < bit3)
					dst = bit3;
			}

			if (mask & AK308_MASK_INPUT2COUNTER) {
				input2_counter = *(unsigned int *)ptr; ptr += 4;
				*bit1 |= RECORD_BIT_MORE;
				*bit2 |= RECORD_BIT2_FREQUENCY1;
				*bit3 |= RECORD_BIT3_COUNTER2;
				dst = bit4;
			}

			if (mask & AK308_MASK_INPUT2FREQUENCY) {
				input2_frequency = *(unsigned short *)ptr; ptr += 2;
				*bit1 |= RECORD_BIT_MORE;
				*bit2 |= RECORD_BIT2_FREQUENCY2;
				if (dst < bit3)
					dst = bit3;
			}

			if (mask & AK308_MASK_LLS1) {
				lls1 = *(unsigned short *)ptr; ptr += 2;
				*bit1 |= RECORD_BIT1_RS485_1;
			}

			if (mask & AK308_MASK_LLS2) {
				lls2 = *(unsigned short *)ptr; ptr += 2;
				*bit1 |= RECORD_BIT1_RS485_2;
			}

			if (mask & AK308_MASK_ACCELL) {
				accell = *(unsigned short *)ptr; ptr += 2;
				*bit1 |= RECORD_BIT_MORE;
				*bit2 |= RECORD_BIT_MORE;
				*bit3 |= RECORD_BIT_MORE;
				*bit4 |= RECORD_BIT_MORE;
				*bit5 |= RECORD_BIT_MORE;
				*bit6 |= RECORD_BIT6_ACCELL;
				dst = bit6 + 1;
			}

			if (*bit1 & RECORD_BIT1_FLAGS) {
				*dst++ = record_flags1;
				if (record_flags1 & RECORD_FLAG1_MORE)
					*dst++ = record_flags2;
			}

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

			if (*bit1 & RECORD_BIT1_COG) {
				*dst++ = cog;
			}

			if (*bit1 & RECORD_BIT1_RS485_1) {
				*(unsigned short *)dst = lls1;
				dst += 2;
			}

			if (*bit1 & RECORD_BIT1_RS485_2) {
				*(unsigned short *)dst = lls2;
				dst += 2;
			}

			if (*bit1 & RECORD_BIT_MORE) {

				if (*bit2 & RECORD_BIT2_FREQUENCY1) {
					*(unsigned short *)dst = (input1_frequency) ? (10000000 / input1_frequency) : 0;
					dst += 2;
				}

				if (*bit2 & RECORD_BIT2_FREQUENCY2) {
					*(unsigned short *)dst = (input2_frequency) ? (10000000 / input2_frequency) : 0;
					dst += 2;
				}
			
				if (*bit2 & RECORD_BIT_MORE) {

					if (*bit3 & RECORD_BIT3_VCC) {
						*(unsigned short *)dst = vcc;
						dst += 2;
					}
					if (*bit3 & RECORD_BIT3_COUNTER1) {
						*(unsigned short *)dst = input1_counter;
						dst += 2;
					}
					if (*bit3 & RECORD_BIT3_COUNTER2) {
						*(unsigned short *)dst = input2_counter;
						dst += 2;
					}
					if (*bit3 & RECORD_BIT_MORE) {
						if (*bit4 & RECORD_BIT_MORE) {
							if (*bit5 & RECORD_BIT_MORE) {
								if (*bit6 & RECORD_BIT6_ACCELL) {
									*(unsigned short *)dst = accell;
									dst += 2;
								}
							}
						}
					}
				}
			}

			record->size = dst - record_buffer;

			if (record->t > terminal->last_command_time) {

				if (record->t < now + 600) {

					if (api_storage_add_record_to_stream(terminal->object->stream, record, record->size) == 0) {

						terminal->last_command_time = record->t;
					}
					else {
						store_error = true;
						break;
					}
				}
				else {
					api_log_printf("[AK308] Record's time is too far in future, ignoring (this %08X, now %08X)\r\n", record->t, now);
				}
			}
			else {
				api_log_printf("[AK308] Record already received, ignoring (this %08X, last %08X)\r\n", record->t, terminal->last_command_time);
			}
		}		

		if (store_error == false) {
			construct_ack(terminal);
			*p = (unsigned char *)cmd;
			*l = sizeof(AK308_COMMAND_HEADER) + sizeof(AK308_ACK_PAYLOAD);
		}

		return 0;

	default:               
		return 0;
	}

	return 0;
}
