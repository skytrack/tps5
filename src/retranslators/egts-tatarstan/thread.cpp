//******************************************************************************
//
// File Name : thread.cpp
// Author    : Skytrack ltd - Copyright (C) 2015
//
// This code is property of Skytrack company
//
//******************************************************************************

void thread();
static unsigned int stop_signal;

#ifndef _MSC_VER

#include <pthread.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

pthread_t hThread;

void *thread_proc(void *param)
{
	thread();

	return NULL;
}
 
void thread_start()
{
	stop_signal = 0;

	pthread_create(&hThread, NULL, thread_proc, NULL);
}

void thread_stop()
{
	stop_signal = 1;

	pthread_join(hThread, NULL);
}

#else

#include <winsock2.h>
#include <Ws2tcpip.h>

HANDLE hThread;

DWORD WINAPI thread_proc(LPVOID lpThreadParameter)
{
	thread();

	return 0;
}

void thread_start()
{
	stop_signal = 0;

	hThread = CreateThread(NULL, 0, thread_proc, NULL, 0, NULL);
}

void thread_stop()
{
	stop_signal = 1;

	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);
}

#endif

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include "crc8.c"
#include "crc16.c"

#include "thread.h"
#include "retranslator.h"
#include "api.h"

#include "../../core/cross.h"
#include "../../core/record.h"

#pragma pack(push, 1)

#pragma pack(1)

typedef struct _tagTATARPACKET
{
	unsigned char	version;
	unsigned char	securityKeyId;
	unsigned char	packet_flags;
	unsigned char	packet_length;
	unsigned char	encoding;
	unsigned short	dataLength;
	unsigned short	packet_id;
	unsigned char	type;
	//unsigned short	peerAddress;
	//unsigned short	recipientAddress;
	//unsigned char	timeToLive;
	unsigned char	hcs;

	unsigned short	record_length;
	unsigned short	record_num;
	unsigned char	record_flags;
	unsigned int	record_oid;
	//unsigned int	eventId;
	//unsigned int	timestamp;
	unsigned char	record_sst;
	unsigned char	record_rst;

	unsigned char	subrecord_type;
	unsigned short	subrecord_length;

	unsigned int	sr_pos_data_timestamp;
	unsigned int	sr_pos_data_latitude;
	unsigned int	sr_pos_data_longitude;
	unsigned char	sr_pos_data_flags;
	unsigned short	sr_pos_data_speed;
	unsigned char	sr_pos_data_direction;
	unsigned char 	sr_pos_data_odometer[3];
	unsigned char	sr_pos_data_digitalInputs;
	unsigned char	sr_pos_data_source;
	//unsigned char	sr_pos_data_altitude[3];
	//unsigned short	sr_pos_data_sourceData;

	unsigned short	data_crc;

} TATARPACKET;

typedef struct _tagTATACKACKET
{
	unsigned char	version;
	unsigned char	securityKeyId;
	unsigned char	packet_flags;
	unsigned char	packet_length;
	unsigned char	encoding;
	unsigned short	dataLength;
	unsigned short	packet_id;
	unsigned char	type;
	unsigned char	hcs;

	unsigned short	record_id;
	unsigned char	result;

	unsigned char	data[1024];

} TATACKACKET;

#pragma pack(pop)

typedef struct tagCONTEXT
{
	fd_set	fdReadSet;
	fd_set	fdWriteSet;
	int max_fd;
} ENUMCONTEXT;

static TATARPACKET TatarPacket;
static TATACKACKET AckPacket;


static void enumerator_select(RETRANSLATOR *pRetranslator, void *ctx)
{
	unsigned long		status;
	char				port[12];
	struct addrinfo		sHints, *psAddrInfo, *p;

	ENUMCONTEXT *pContext = (ENUMCONTEXT *)ctx;
	
	time_t now = time(NULL);

	switch (pRetranslator->status) {

	case RETRANSLATOR_STATUS_INIT:

		memset(&sHints, 0, sizeof(struct addrinfo));

		sHints.ai_family   = PF_UNSPEC;
		sHints.ai_socktype = SOCK_STREAM;
		sHints.ai_protocol = IPPROTO_TCP;
		sHints.ai_flags    = AI_PASSIVE;
		
		sprintf(port, "%u", pRetranslator->port);

		status = getaddrinfo(pRetranslator->host.c_str(), port, &sHints, &psAddrInfo);

		if (status != 0) {
			api_log_printf("[EGTS-TATARSTAN] getaddrinfo failed for %s:%u\r\n", pRetranslator->host.c_str(), pRetranslator->port);
			break;
		}

		for (p = psAddrInfo; p; p = p->ai_next) {
			
			pRetranslator->sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);

			if (pRetranslator->sock < 0) {		
				api_log_printf("[EGTS-TATARSTAN] errno #%d on creating socket\r\n", errno);
				continue;
			}

			status = 1;

#ifdef _MSC_VER
			ioctlsocket(pRetranslator->sock, FIONBIO, &status);
#else			
			fcntl(pRetranslator->sock, F_SETFL, O_NONBLOCK);
#endif
			api_log_printf("[EGTS-TATARSTAN] socket #%d connecting to %s:%u\r\n", pRetranslator->sock, pRetranslator->host.c_str(), pRetranslator->port);

			status = connect(pRetranslator->sock, p->ai_addr, p->ai_addrlen);

			if (status == 0) {
				
				pRetranslator->status = RETRANSLATOR_STATUS_CONNECTED;
				pRetranslator->timeout = 0;
				pRetranslator->packet_id = 0;
			
				api_log_printf("[EGTS-TATARSTAN] socket #%d connected immidiatly\r\n");
				
				break;
			}
			else {

				int error;

#ifdef _MSC_VER
				error = WSAGetLastError();
				if (error == WSAEWOULDBLOCK)
					pRetranslator->status = RETRANSLATOR_STATUS_CONNECTING;
#else
				error = errno;
				if (error == EINPROGRESS)
					pRetranslator->status = RETRANSLATOR_STATUS_CONNECTING;
#endif
				if (pRetranslator->status != RETRANSLATOR_STATUS_CONNECTING) {
					api_log_printf("[EGTS-TATARSTAN] error #%d on connecting to %s:%u\r\n", error, pRetranslator->host.c_str(), pRetranslator->port);
					closesocket(pRetranslator->sock);
					pRetranslator->sock = -1;
				}
				else {
					pRetranslator->timeout = now + 60;
				}

				break;
			}
		}

		freeaddrinfo(psAddrInfo);

		break;

	case RETRANSLATOR_STATUS_CONNECTING:

		if (pRetranslator->timeout <= now) {

			api_log_printf("[EGTS-TATARSTAN] socket #%d connect timeout, closing\r\n", pRetranslator->sock);

			pRetranslator->status = RETRANSLATOR_STATUS_INIT;
			closesocket(pRetranslator->sock);
			pRetranslator->sock = -1;

			break;
		}

		FD_SET(pRetranslator->sock, &pContext->fdReadSet);
		FD_SET(pRetranslator->sock, &pContext->fdWriteSet);

		if (pRetranslator->sock > pContext->max_fd)
			pContext->max_fd = pRetranslator->sock;

		break;

	case RETRANSLATOR_STATUS_CONNECTED:

		FD_SET(pRetranslator->sock, &pContext->fdReadSet);

		spinlock_lock(&pRetranslator->spinlock);

		if (!pRetranslator->records_queue.empty()) {
			FD_SET(pRetranslator->sock, &pContext->fdWriteSet);
		}

		spinlock_unlock(&pRetranslator->spinlock);

		if (pRetranslator->sock > pContext->max_fd)
			pContext->max_fd = pRetranslator->sock;

		break;

	case RETRANSLATOR_STATUS_WAITACK:

		if (pRetranslator->timeout <= now) {

			api_log_printf("[EGTS-TATARSTAN] socket #%d ack timeout, closing\r\n", pRetranslator->sock);

			pRetranslator->status = RETRANSLATOR_STATUS_INIT;
			closesocket(pRetranslator->sock);
			pRetranslator->sock = -1;
			
			break;
		}

		FD_SET(pRetranslator->sock, &pContext->fdReadSet);

		if (pRetranslator->sock > pContext->max_fd)
			pContext->max_fd = pRetranslator->sock;
	}
}

static void enumerator_handler(RETRANSLATOR *pRetranslator, void *ctx)
{
	if (pRetranslator->sock == -1)
		return;

	ENUMCONTEXT *pContext = (ENUMCONTEXT *)ctx;

	time_t now = time(NULL);

	if (FD_ISSET(pRetranslator->sock, &pContext->fdReadSet)) {

		api_log_printf("[EGTS-TATARSTAN] socket #%d is readble\r\n", pRetranslator->sock);
	
		for (;;) {
			
			int status = recv(pRetranslator->sock, (char *)&AckPacket, sizeof(AckPacket), 0);

			if (status <= 0) {

				api_log_printf("[EGTS-TATARSTAN] socket #%d is closed\r\n", pRetranslator->sock);
				closesocket(pRetranslator->sock);
				pRetranslator->sock = -1;
				pRetranslator->status = RETRANSLATOR_STATUS_INIT;

				return;
			}

			if (status > 0) {

				api_log_printf("[EGTS-TATARSTAN] Received #%d bytes from socket #%d\r\n", status, pRetranslator->sock);

				if (AckPacket.hcs != CRC8((unsigned char *)&AckPacket, 10)) {						
					api_log_printf("[EGTS-TATARSTAN] CRC8 invalid\r\n");
					break;
				}

				if (*((unsigned short *)((unsigned char *)&AckPacket.record_id + AckPacket.dataLength)) != Crc16((unsigned char *)&AckPacket.record_id, AckPacket.dataLength)) {
					api_log_printf("[EGTS-TATARSTAN] CRC16 invalid\r\n");
					break;
				}

				if (AckPacket.record_id != pRetranslator->packet_id) {
					api_log_printf("[EGTS-TATARSTAN] Wrong record_id\r\n");
					break;
				}

				if (AckPacket.result != 0) {
					api_log_printf("[EGTS-TATARSTAN] Wrong result code\r\n");
					break;
				}

				spinlock_lock(&pRetranslator->spinlock);
				pRetranslator->records_queue.pop();
				spinlock_unlock(&pRetranslator->spinlock);

				pRetranslator->status = RETRANSLATOR_STATUS_CONNECTED;
				pRetranslator->timeout = 0;
				pRetranslator->packet_id++;
			}

			break;
		}
	}

	if (FD_ISSET(pRetranslator->sock, &pContext->fdWriteSet)) {

		api_log_printf("[EGTS-TATARSTAN] socket #%d is writible\r\n", pRetranslator->sock);
		
		switch (pRetranslator->status) {

		case RETRANSLATOR_STATUS_INIT:

			api_log_printf("[EGTS-TATARSTAN] ERROR, socket #%d is in intial state\r\n", pRetranslator->sock);
			break;

		case RETRANSLATOR_STATUS_CONNECTING:

			api_log_printf("[EGTS-TATARSTAN] socket #%d connected\r\n", pRetranslator->sock);
			pRetranslator->packet_id = 0;
			pRetranslator->status = RETRANSLATOR_STATUS_CONNECTED;

		case RETRANSLATOR_STATUS_CONNECTED:

			spinlock_lock(&pRetranslator->spinlock);
			
			if (!pRetranslator->records_queue.empty()) {

				RETRANSLATOR_RECORD rr = pRetranslator->records_queue.front();

				spinlock_unlock(&pRetranslator->spinlock);

				TatarPacket.version			= 0x01;
				TatarPacket.securityKeyId	= 0x01;
				TatarPacket.packet_flags	= 0x01;
				TatarPacket.packet_length	= 0x0b;
				TatarPacket.encoding		= 0x00;
				TatarPacket.dataLength		= 35;
				TatarPacket.packet_id		= pRetranslator->packet_id;
				TatarPacket.type			= 1; // EGTS_PT_APPDATA

				TatarPacket.record_length	= 24;
				TatarPacket.record_num		= pRetranslator->packet_id;
				TatarPacket.record_flags	= 0x09;  // OID – (Object Identifier) | PRIORITY 01
				TatarPacket.record_oid		= pRetranslator->oid; // Last 8 digits of imei
				TatarPacket.record_sst		= 0x02; // EGTS_TELEDATA_SERVICE
				TatarPacket.record_rst		= 0x02; // EGTS_TELEDATA_SERVICE

				TatarPacket.subrecord_type	= 0x10; // EGTS_SR_POS_DATA
				TatarPacket.subrecord_length	= 0x15;

				TatarPacket.sr_pos_data_timestamp	= rr.t - 1262304000;

				float lat = (float)rr.latitude / 10000000;
				float lon = (float)rr.longitude / 10000000;

				TatarPacket.sr_pos_data_latitude	= (unsigned int)((fabs(lat) / 90) * 0xFFFFFFFF);
				TatarPacket.sr_pos_data_longitude	= (unsigned int)((fabs(lon) / 180) * 0xFFFFFFFF);

				TatarPacket.sr_pos_data_flags		= 0;

				if ((rr.latitude != 0)&&(rr.longitude != 0))
					TatarPacket.sr_pos_data_flags	|= 0x01;

				if (rr.flags1 & RECORD_FLAG1_MOVE)
					TatarPacket.sr_pos_data_flags	|= 0x10;

				if (lon < 0)
					TatarPacket.sr_pos_data_flags	|= 0x40;
				if (lat < 0)
					TatarPacket.sr_pos_data_flags	|= 0x20;

				TatarPacket.sr_pos_data_speed		= rr.speed & 0x3FFF;

				TatarPacket.sr_pos_data_direction	= (rr.cog & 0xFF);
				if (rr.cog > 255)
					TatarPacket.sr_pos_data_speed	|= 0x8000;
				
				TatarPacket.sr_pos_data_odometer[0]	= 0xcf;
				TatarPacket.sr_pos_data_odometer[1]	= 0xf6;
				TatarPacket.sr_pos_data_odometer[2]	= 0x27;
				TatarPacket.sr_pos_data_digitalInputs	= rr.flags2 & 0xF0;
				TatarPacket.sr_pos_data_source			= (rr.flags1 & RECORD_FLAG1_IGNITION) ? 0 : 5;

				TatarPacket.hcs = CRC8((unsigned char *)&TatarPacket, 10);
				TatarPacket.data_crc = Crc16((unsigned char *)&TatarPacket.record_length, TatarPacket.dataLength);

				send(pRetranslator->sock, (char *)&TatarPacket, sizeof(TatarPacket), 0);

				api_log_printf("[EGTS-TATARSTAN] socket #%d send record\r\n", pRetranslator->sock);

				pRetranslator->status = RETRANSLATOR_STATUS_WAITACK;
				pRetranslator->timeout = now + 30;

			}
			else {
				spinlock_unlock(&pRetranslator->spinlock);
			}

			break;

		case RETRANSLATOR_STATUS_WAITACK:

			api_log_printf("[EGTS-TATARSTAN] ERROR, socket #%d is in waitack state\r\n", pRetranslator->sock);
			break;

		}
	}
}

void thread()
{
	while (stop_signal == 0) {

		ENUMCONTEXT ctx;

		FD_ZERO(&ctx.fdReadSet);
		FD_ZERO(&ctx.fdWriteSet);

		ctx.max_fd = 0;

		retranslators_enum(enumerator_select, &ctx);

		if (ctx.max_fd == 0) {
#ifdef _MSC_VER
			Sleep(1000);
#else
			sleep(1);
#endif
			continue;
		}

		struct timeval tv;
		tv.tv_sec	= 1;
		tv.tv_usec	= 0;

		int status = select(ctx.max_fd + 1, &ctx.fdReadSet, &ctx.fdWriteSet, NULL, &tv);
	
		if (status == -1) {

			int error;
#ifdef _MSC_VER
			error = WSAGetLastError();
#else
			error = errno;
#endif

			api_log_printf("[EGTS-TATARSTAN] select error #%u\r\n", error);

			if (error != 9)
				break;
		}

		retranslators_enum(enumerator_handler, &ctx);
	}
}

// End

