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

#include "thread.h"
#include "retranslator.h"
#include "api.h"

#include "../../core/cross.h"

#pragma pack(push, 1)

typedef struct tagRRCB
{
	char rrcb[6];

	unsigned short	len;

	unsigned short	NUPE;
	int				Time;
	unsigned char	E;
	unsigned char	SatDel;
	unsigned char	LonDegrees;
	unsigned char	LatDegrees;
	unsigned short	LonMinutes;
	unsigned short	LatMinutes;
	unsigned char	Speed;
	unsigned char	Course;
	short			Track;
	unsigned char	AnIn[4];
	unsigned char	AnHi;
	unsigned char	Alt;
	unsigned char	DiOut;
	unsigned char	StatMess;

	char			star;
	char			cs[2];
	char			r;
	char			n;
} RRCB;

#pragma pack(pop)

typedef struct tagCONTEXT
{
	fd_set	fdReadSet;
	fd_set	fdWriteSet;
	int max_fd;
} ENUMCONTEXT;

static RRCB rrcb;

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
			api_log_printf("[GRANIT V3] getaddrinfo failed for %s:%u\r\n", pRetranslator->host.c_str(), pRetranslator->port);
			break;
		}

		for (p = psAddrInfo; p; p = p->ai_next) {
			
			pRetranslator->sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);

			if (pRetranslator->sock < 0) {		
				api_log_printf("[GRANIT V3] errno #%d on creating socket\r\n", errno);
				continue;
			}

			status = 1;

#ifdef _MSC_VER
			ioctlsocket(pRetranslator->sock, FIONBIO, &status);
#else			
			fcntl(pRetranslator->sock, F_SETFL, O_NONBLOCK);
#endif
			api_log_printf("[GRANIT V3] socket #%d connecting to %s:%u\r\n", pRetranslator->sock, pRetranslator->host.c_str(), pRetranslator->port);

			status = connect(pRetranslator->sock, p->ai_addr, p->ai_addrlen);

			if (status == 0) {
				
				pRetranslator->status = RETRANSLATOR_STATUS_CONNECTED;
				pRetranslator->timeout = 0;
			
				api_log_printf("[GRANIT V3] socket #%d connected immidiatly\r\n");
				
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
					api_log_printf("[GRANIT V3] error #%d on connecting to %s:%u\r\n", error, pRetranslator->host.c_str(), pRetranslator->port);
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

			api_log_printf("[GRANIT V3] socket #%d connect timeout, closing\r\n", pRetranslator->sock);

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

			api_log_printf("[GRANIT V3] socket #%d ack timeout, closing\r\n", pRetranslator->sock);

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

		api_log_printf("[GRANIT V3] socket #%d is readble\r\n", pRetranslator->sock);
	
		for (;;) {
			
			char buf[1024];
		
			int status = recv(pRetranslator->sock, buf, sizeof(buf), 0);

			if (status <= 0) {

				api_log_printf("[GRANIT V3] socket #%d is closed\r\n", pRetranslator->sock);
				closesocket(pRetranslator->sock);
				pRetranslator->sock = -1;
				pRetranslator->status = RETRANSLATOR_STATUS_INIT;

				return;
			}

			if (status > 0) {

				api_log_printf("[GRANIT V3] Received #%d bytes from socket #%d\r\n", status, pRetranslator->sock);

				if (status == 21) {

					const char bbugrc[10] = { 'B', 'B', '+', 'U', 'G', 'R', 'C', '~', 0x06, 0x00 };

					if (memcmp(buf, bbugrc, 10) == 0) {

						spinlock_lock(&pRetranslator->spinlock);
						pRetranslator->records_queue.pop();
						spinlock_unlock(&pRetranslator->spinlock);

						pRetranslator->status = RETRANSLATOR_STATUS_CONNECTED;
						pRetranslator->timeout = 0;
					}
				}
			}

			break;
		}
	}

	if (FD_ISSET(pRetranslator->sock, &pContext->fdWriteSet)) {

		api_log_printf("[GRANIT V3] socket #%d is writible\r\n", pRetranslator->sock);
		
		switch (pRetranslator->status) {

		case RETRANSLATOR_STATUS_INIT:

			api_log_printf("[GRANIT V3] ERROR, socket #%d is in intial state\r\n", pRetranslator->sock);
			break;

		case RETRANSLATOR_STATUS_CONNECTING:

			api_log_printf("[GRANIT V3] socket #%d connected\r\n", pRetranslator->sock);
			pRetranslator->status = RETRANSLATOR_STATUS_CONNECTED;

		case RETRANSLATOR_STATUS_CONNECTED:

			spinlock_lock(&pRetranslator->spinlock);
			
			if (!pRetranslator->records_queue.empty()) {

				RETRANSLATOR_RECORD rr = pRetranslator->records_queue.front();

				spinlock_unlock(&pRetranslator->spinlock);

				rrcb.NUPE = pRetranslator->nupe;
				rrcb.Time = rr.t;
				rrcb.E = 0;

				if ((rr.latitude != 0)&&(rr.longitude != 0)) {

					rrcb.E |= 0x80;
			
					if (rr.cog & 0x100)
						rrcb.E |= 0x40;
					if (rr.longitude > 0)
						rrcb.E |= 0x20;
					if (rr.latitude > 0)
						rrcb.E |= 0x10;

					rrcb.SatDel = 0;

					double intpart;

					double arg = (double)rr.longitude / 10000000;
					double fractpart = modf(arg, &intpart);
					rrcb.LonDegrees = (char)intpart;
					rrcb.LonMinutes = (unsigned short)(fractpart * 60000);

					arg = (double)rr.latitude / 10000000;
					fractpart = modf(arg, &intpart);
					rrcb.LatDegrees = (char)intpart;
					rrcb.LatMinutes = (unsigned short)(fractpart * 60000);

					rrcb.Speed = (unsigned char)((double)rr.speed / 10 * 0.539956803);

					rrcb.Course = rr.cog & 0xFF;
				}
				else {
					rrcb.LatDegrees = 0;
					rrcb.LatMinutes = 0;
					rrcb.LonDegrees = 0;
					rrcb.LonDegrees = 0;
					rrcb.Speed		= 0;
					rrcb.Course		= 0;
				}

				unsigned char *ptr = (unsigned char *)&rrcb;			
				unsigned char *end = ptr + sizeof(rrcb) - 5;

				unsigned char cs = 0;

				while (ptr != end) {
					cs ^= *ptr++;
				}

				rrcb.cs[0] = (cs >> 4) > 9 ? (cs >> 4) + 55 : (cs >> 4) + 48;

				rrcb.cs[1] = (cs & 0xf) > 9 ? (cs & 0xf) + 55 : (cs & 0xf) + 48;

				send(pRetranslator->sock, (char *)&rrcb, sizeof(rrcb), 0);

				api_log_printf("[GRANIT V3] socket #%d send record\r\n", pRetranslator->sock);

				pRetranslator->status = RETRANSLATOR_STATUS_WAITACK;
				pRetranslator->timeout = now + 30;

			}
			else {
				spinlock_unlock(&pRetranslator->spinlock);
			}

			break;

		case RETRANSLATOR_STATUS_WAITACK:

			api_log_printf("[GRANIT V3] ERROR, socket #%d is in waitack state\r\n", pRetranslator->sock);
			break;

		}
	}
}

void thread()
{
	memset(&rrcb, 0, sizeof(rrcb));
	memcpy(rrcb.rrcb, "+RRCB~", 6);
	rrcb.len = 26;
	rrcb.star = '*';
	rrcb.r = '\r';
	rrcb.n = '\n';

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

			api_log_printf("[GRANIT V3] select error #%u\r\n", error);

			if (error != 9)
				break;
		}

		retranslators_enum(enumerator_handler, &ctx);
	}
}

// End

