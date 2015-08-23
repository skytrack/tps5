//******************************************************************************
//
// File Name : update.cpp
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#include <string.h>
#include <stdio.h>
#include "api.h"
#include "common.h"
#include "crc16.h"
#include "config.h"
#include "../core/record.h"

#define TFTP_MAX_BLOCK_SIZE	512

#define TFTP_OPCODE_READ	1
#define TFTP_OPCODE_WRITE	2
#define TFTP_OPCODE_DATA	3
#define TFTP_OPCODE_ACK		4
#define TFTP_OPCODE_ERROR	5

static std::string file_name;

int update(unsigned char **p, size_t *l, void *ctx, size_t ctx_len)
{
	unsigned char *buffer = *p;

	if ((buffer[0] == 0x00)&&(buffer[1] == TFTP_OPCODE_READ)) {
		
		TERMINAL *terminal = NULL;

		unsigned char dev_id[8];

		dev_id[0] = ((buffer[2]  - '0') << 4) | (buffer[3]  - '0');
		dev_id[1] = ((buffer[4]  - '0') << 4) | (buffer[5]  - '0');
		dev_id[2] = ((buffer[6]  - '0') << 4) | (buffer[7]  - '0');
		dev_id[3] = ((buffer[8]  - '0') << 4) | (buffer[9]  - '0');
		dev_id[4] = ((buffer[10] - '0') << 4) | (buffer[11]  - '0');
		dev_id[5] = ((buffer[12] - '0') << 4) | (buffer[13] - '0');
		dev_id[6] = ((buffer[14] - '0') << 4) | (buffer[15] - '0');
		dev_id[7] = ((buffer[16] - '0') << 4);

		unsigned char *ptr = id;

		for (size_t i = 0; i < terminals.size(); i++, ptr += 8) {

			if (memcmp(ptr, dev_id, 8) == 0) {
				terminal = &terminals[i];
				break;
			}
		}

		if (terminal == NULL) {

			buffer[17] = '\0';
			api_log_printf("[AK308] Unknown terminal [%s]\r\n", &buffer[2]);

			unsigned char *ptr = *p;

			*ptr++ = 0x00;
			*ptr++ = TFTP_OPCODE_ERROR;
			*ptr++ = 0x01;
			*ptr++ = 0x00;

			memcpy(ptr, "Unknown terminal", 17);

			*l = 21;

			return 0;
		}

		BLOB_RECORD *config = (BLOB_RECORD *)terminal->object->module_data;

		if (terminal->object->module_data_size != sizeof(BLOB_RECORD)) {
			api_log_printf("[AK308] Invalid config record, terminal_id=%d\r\n", terminal->id);
			*l = 0;
			return 0;
		}

		terminal->online = true;

		add_event(terminal, RECORD_EVENT_TERMINAL_ONLINE);
		add_event(terminal, RECORD_EVENT_TERMINAL_UPDATE_START);

		api_log_printf("[AK308] Terminal authorized for update, terminal_id=%u\r\n", terminal->id);

		file_name = fw_dir;
		file_name += "/";

		if (memcmp(config->fw, "default", 7)) {
			file_name += config->fw;
		}
		else {
			char fw[64];
			sprintf(fw, "fw%04u.308", config->actual_fw_ver);
			file_name += fw;
		}

		terminal->f = fopen(file_name.c_str(), "rb");
		if (terminal->f) {

			terminal->tx_cmd.header.size = fread(&terminal->tx_cmd.payload.buffer[4], 
						1, TFTP_MAX_BLOCK_SIZE, terminal->f) + 4;
									
			terminal->tx_cmd.payload.buffer[0] = 0x00;
			terminal->tx_cmd.payload.buffer[1] = TFTP_OPCODE_DATA;
			terminal->tx_cmd.payload.buffer[2] = 0x00;
			terminal->tx_cmd.payload.buffer[3] = 0x01;
							
			api_log_printf("[AK308] Send frame 1, size %u, terminal_id=%u\r\n", terminal->tx_cmd.header.size, terminal->id);

			terminal->ack_timeout = now + send_attempt_intervals[0];
			terminal->tx_cmd.header.seqNo = 1;
			terminal->send_attempt = 0;
			terminal->session_timeout = now + 300;

			if (ctx_len <= sizeof(terminal->context))
				memcpy(terminal->context, ctx, ctx_len);

			*p = terminal->tx_cmd.payload.buffer;
			*l = terminal->tx_cmd.header.size;
		}
		else {
			api_log_printf("[AK308] Unable to open file [%s], terminal_id=%u\r\n", file_name.c_str(), terminal->id);

			terminal->tx_cmd.payload.buffer[0] = 0x00;
			terminal->tx_cmd.payload.buffer[1] = TFTP_OPCODE_ERROR;
			terminal->tx_cmd.payload.buffer[2] = 0x01;
			terminal->tx_cmd.payload.buffer[3] = 0x00;
			terminal->tx_cmd.payload.buffer[4] = 0x00;

			memcpy(&terminal->tx_cmd.payload.buffer[4], "File Not Found", 15);

			*p = terminal->tx_cmd.payload.buffer;
			*l = 19;

			add_event(terminal, RECORD_EVENT_TERMINAL_UPDATE_FAILED);

			close_connection(terminal);
		}

		return 0;
	}
	else
	if ((buffer[0] == 0x00)&&(buffer[1] == TFTP_OPCODE_ACK)&&(*l >= 4)) {

		TERMINAL *terminal = NULL;

		for (size_t i = 0; i < terminals.size(); i++) {

			if (memcmp(ctx, terminals[i].context, ctx_len) == 0) {
				terminal = &terminals[i];
				break;
			}
		}

		if (terminal == NULL) {
			*l = 0;
			return 0;
		}

		if (terminal->tx_cmd.header.seqNo == buffer[3]) {

			api_log_printf("[AK308] Frame acked, terminal_id=%u\r\n", terminal->id);

			if ((terminal->tx_cmd.header.size < 512)||(terminal->tx_cmd.header.seqNo == 88)) {

				api_log_printf("[AK308] Transfer done, terminal_id=%u\r\n", terminal->id);

				add_event(terminal, RECORD_EVENT_TERMINAL_UPDATE_DONE);

				close_connection(terminal);
			}
			else {

				terminal->tx_cmd.header.size = fread(&terminal->tx_cmd.payload.buffer[4], 
							1, TFTP_MAX_BLOCK_SIZE, terminal->f) + 4;

				terminal->tx_cmd.header.seqNo++;

				terminal->tx_cmd.payload.buffer[0] = 0x00;
				terminal->tx_cmd.payload.buffer[1] = TFTP_OPCODE_DATA;
				terminal->tx_cmd.payload.buffer[2] = (terminal->tx_cmd.header.seqNo >> 8) & 0xFF;
				terminal->tx_cmd.payload.buffer[3] = terminal->tx_cmd.header.seqNo & 0xFF;
	
				api_log_printf("[AK308] Send frame %u, size %u, terminal_id=%u\r\n", terminal->tx_cmd.header.seqNo, terminal->tx_cmd.header.size, terminal->id);

				terminal->ack_timeout = now + send_attempt_intervals[0];
				terminal->send_attempt = 0;
				terminal->session_timeout = now + 300;

				*p = terminal->tx_cmd.payload.buffer;
				*l = terminal->tx_cmd.header.size;
			}
		}

		return 0;
	}

	*l = 0;

	return 0;
}
