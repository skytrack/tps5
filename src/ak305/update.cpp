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
#include <limits.h>
#include <time.h>
#include "api.h"
#include "config.h"
#include "common.h"
#include "ak305.h"
#include "update.h"
#include "../core/record.h"

#define AK300_COMMAND_STATE_LEN_LO		0
#define AK300_COMMAND_STATE_LEN_HI      1
#define AK300_COMMAND_STATE_DATA        2
#define AK300_COMMAND_STATE_UPDATEAUTH	3

static std::string file_name;
static unsigned char frame_buffer[512];

static TERMINAL dummy_terminal = { 0, 0, 0, 0, NULL, NULL };

SESSION *update_session_open()
{
	SESSION *session = (SESSION *)malloc(sizeof(SESSION));

	memset(session, 0, sizeof(SESSION));
	
	session->ptr			= session->buffer;
	session->nCommandState	= AK300_COMMAND_STATE_UPDATEAUTH;
	session->terminal		= &dummy_terminal;

	return session;
}

void update_session_close(SESSION *session)
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

	free(session);
}

int update_session_timer(SESSION *session, char **p, size_t *l)
{
	update_session_close(session);

	*l = 0;

	return SESSION_COMPLETE;
}

int update_session_data(SESSION *session, unsigned char **p, size_t *l)
{
	char hi, lo, ch;
	int frameSize;
	TERMINAL *terminal;
	unsigned char *data = *p;

	switch (session->nCommandState) {

	default:
	case AK300_COMMAND_STATE_UPDATEAUTH:

		while ((session->nDataBytesReceived != 15)&&(*l > 0)) {
			*session->ptr++ = *data++;
			session->nDataBytesReceived++;
			(*l)--;
		}

		if (session->nDataBytesReceived == 15) {

			unsigned char dev_id[8];

			session->buffer[15] = '\0';

			dev_id[0] = ((session->buffer[0]  - '0') << 4) | (session->buffer[1]  - '0');
			dev_id[1] = ((session->buffer[2]  - '0') << 4) | (session->buffer[3]  - '0');
			dev_id[2] = ((session->buffer[4]  - '0') << 4) | (session->buffer[5]  - '0');
			dev_id[3] = ((session->buffer[6]  - '0') << 4) | (session->buffer[7]  - '0');
			dev_id[4] = ((session->buffer[8]  - '0') << 4) | (session->buffer[9]  - '0');
			dev_id[5] = ((session->buffer[10] - '0') << 4) | (session->buffer[11] - '0');
			dev_id[6] = ((session->buffer[12] - '0') << 4) | (session->buffer[13] - '0');
			dev_id[7] = ((session->buffer[14] - '0') << 4);

			std::map<uint64_t, TERMINAL>::iterator it = terminals.find(*(uint64_t *)dev_id);

			if (it == terminals.end()) {

				session->cmd.payload.CmdNegotiate[15] = '\0';

				api_log_printf("[AK305] Unknown terminal [%s]\r\n", session->buffer);

				*l = 0;

				update_session_close(session);

				return SESSION_COMPLETE;
			}

			terminal = &it->second; 
			session->terminal = terminal;
			terminal->session = session;

			api_log_printf("[AK305] Terminal authorized for update, terminal_id=%u\r\n", terminal->id);
			add_event(terminal, RECORD_EVENT_TERMINAL_ONLINE);

			BLOB_RECORD_305 *config = (BLOB_RECORD_305 *)terminal->object->module_data;

			file_name = fw_dir;
			file_name += "/";

			if (memcmp(config->fw, "default", 7)) {
				file_name += config->fw;
			}
			else {
				char fw[64];
				sprintf(fw, "fw%04u.305", config->actual_fw_ver);
				file_name += fw;
			}

			session->f = fopen(file_name.c_str(), "rb");
			if (session->f) {
				add_event(terminal, RECORD_EVENT_TERMINAL_UPDATE_START);
			}
			else {

				api_log_printf("[AK305] Unable to open file [%s], terminal_id=%u\r\n", file_name.c_str(), terminal->id);

				add_event(terminal, RECORD_EVENT_TERMINAL_UPDATE_FAILED);

				update_session_close(session);

				*l = 0;

				return SESSION_COMPLETE;
			}
		}
		else {
			return 120;
		}

	case AK300_COMMAND_STATE_DATA:
			
		terminal = (TERMINAL *)session->terminal;

		if (session->nCommandState != AK300_COMMAND_STATE_UPDATEAUTH) {

			if ((*data != 0x11)||(*l != 1)) {
				api_log_printf("[AK305] FW Update Failed, terminal_id=%u\r\n", terminal->id);
				add_event(terminal, RECORD_EVENT_TERMINAL_UPDATE_FAILED);
		   		break;
			}
			else {
				api_log_printf("[AK305] Frame acked, terminal_id=%u\r\n", terminal->id);
			}
		}

		ch = fgetc(session->f);

		if (ch == EOF) {
			
			api_log_printf("[AK305] FW Update Done, terminal_id=%u\r\n", terminal->id);

			add_event(terminal, RECORD_EVENT_TERMINAL_UPDATE_DONE);

			update_session_close(session);

			*l = 0;

			return SESSION_COMPLETE;
		}

		hi = (ch-((ch < 'A')?'0':('A'-10)))*16;
		ch = fgetc(session->f);
		hi += (ch-((ch < 'A')?'0':('A'-10)));
		ch = fgetc(session->f);
		lo = (ch-((ch < 'A')?'0':('A'-10)))*16;
		ch = fgetc(session->f);
		lo += (ch-((ch < 'A')?'0':('A'-10)));

		frameSize = ((hi << 8) | lo) + 2;

		api_log_printf("[AK305] Frame Size %d, terminal_id=%u\r\n", frameSize, terminal->id);

		frame_buffer[0] = hi;
		frame_buffer[1] = lo;

		for (int i = 0; i < frameSize - 2; i++) {
			ch = fgetc(session->f);
			hi = (ch-((ch < 'A')?'0':('A'-10)))*16;
			ch = fgetc(session->f);
			hi += (ch-((ch < 'A')?'0':('A'-10)));
			frame_buffer[i + 2] = hi;
		}

		session->nCommandState = AK300_COMMAND_STATE_DATA;

		*p = frame_buffer;
		*l = frameSize;

		return 120;
	}

	update_session_close(session);

	*l = 0;

	return SESSION_COMPLETE;
}

