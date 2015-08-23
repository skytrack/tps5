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
#include "../../core/record.h"

#define DORTRANS_FRAMETAG1			0x00
#define DORTRANS_FRAMETAG2			0x01
#define DORTRANS_FRAMELEN			0x02
#define DORTRANS_FRAMERES			0x03
#define DORTRANS_FRAMEBODY			0x04
#define DORTRANS_FRAMECRC			0x05

#define DORTRANS_PACKLEN			0x04
#define DORTRANS_PACKNUM			0x05
#define DORTRANS_PACKTYPE			0x06
#define DORTRANS_PACKRES			0x07
#define DORTRANS_PACKBODY			0x08
#define DORTRANS_PACKCRC			0x09

#define DORTRANS_MAX_RECORDS		32

#define DORTRANS_STATUS_AUTH		1
#define DORTRANS_STATUS_ONLINE		2
#define DORTRANS_STATUS_WAITACK		3

#pragma pack(push, 1)

typedef struct _tagDORPACKETHEADER	
{
	unsigned int	pack_len;
	unsigned int	pack_num;
	unsigned short	pack_type;
	unsigned char	pack_reserved[2];
} DORPACKETHEADER;

typedef struct _tagDORTRANSAUTH	
{
	unsigned char	frame_tag[2];
	unsigned int	frame_len;
	unsigned char	frame_reserved[6];

	unsigned int	pack_len;
	unsigned int	pack_num;
	unsigned short	pack_type;
	unsigned char	pack_reserved[2];

	unsigned char	auth_code[16];

	unsigned char	frame_crc;
} DORTRANSAUTH;

typedef struct _tagDORTRANSPING	
{
	unsigned char	frame_tag[2];
	unsigned int	frame_len;
	unsigned char	frame_reserved[6];

	unsigned int	pack_len;
	unsigned int	pack_num;
	unsigned short	pack_type;
	unsigned char	pack_reserved[2];

	unsigned char	frame_crc;
} DORTRANSPING;

typedef struct _tagDORTRANSACK
{
	unsigned char	frame_tag[2];
	unsigned int	frame_len;
	unsigned char	frame_reserved[6];

	unsigned int	pack_len;
	unsigned int	pack_num;
	unsigned short	pack_type;
	unsigned char	pack_reserved[2];

	unsigned int	pack_ack_num;

	unsigned char	frame_crc;
} DORTRANSACK;

typedef struct _tagDORTRANSNAVPACK
{
	unsigned int	pack_len;
	unsigned int	pack_num;
	unsigned short	pack_type;
	unsigned char	pack_reserved[2];

	unsigned int	terminal_id;
	unsigned short	terminal_type;
	unsigned int	arrive_time;
	unsigned int	t;
	unsigned char	flags;
	unsigned int	lat;
	unsigned int	lon;
	unsigned short	speed;
	unsigned short	cog;
	short		alt;
	unsigned char	nsat;
	unsigned int	mileage;
	unsigned char	flags2;
	unsigned char	csq;
	unsigned char	reserved[6];
} DORTRANSNAVPACK;

typedef struct _tagDORTRANSNAV
{
	unsigned char	frame_tag[2];
	unsigned int	frame_len;
	unsigned char	frame_reserved[6];

	DORTRANSNAVPACK	p[DORTRANS_MAX_RECORDS];

	unsigned char	frame_crc;
} DORTRANSNAV;

#pragma pack(pop)

typedef struct tagCONTEXT
{
	fd_set	fdReadSet;
	fd_set	fdWriteSet;
	int max_fd;
} ENUMCONTEXT;

static void enumerator_select(RETRANSLATOR *pRetranslator, void *ctx)
{
	unsigned long		status;
	char				port[12];
	struct addrinfo		sHints, *psAddrInfo, *p;

	ENUMCONTEXT *pContext = (ENUMCONTEXT *)ctx;
	
	time_t now = time(NULL);

	switch (pRetranslator->connection_status) {

	case RETRANSLATOR_STATUS_INIT:

		memset(&sHints, 0, sizeof(struct addrinfo));

		sHints.ai_family   = PF_UNSPEC;
		sHints.ai_socktype = SOCK_STREAM;
		sHints.ai_protocol = IPPROTO_TCP;
		sHints.ai_flags    = AI_PASSIVE;
		
		sprintf(port, "%u", pRetranslator->port);

		status = getaddrinfo(pRetranslator->host.c_str(), port, &sHints, &psAddrInfo);

		if (status != 0) {
			api_log_printf("[DORTRANS] getaddrinfo failed for %s:%u\r\n", pRetranslator->host.c_str(), pRetranslator->port);
			break;
		}

		for (p = psAddrInfo; p; p = p->ai_next) {
			
			pRetranslator->sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);

			if (pRetranslator->sock < 0) {		
				api_log_printf("[DORTRANS] errno #%d on creating socket\r\n", errno);
				continue;
			}

			status = 1;

#ifdef _MSC_VER
			ioctlsocket(pRetranslator->sock, FIONBIO, &status);
#else			
			fcntl(pRetranslator->sock, F_SETFL, O_NONBLOCK);
#endif
			api_log_printf("[DORTRANS] socket #%d connecting to %s:%u\r\n", pRetranslator->sock, pRetranslator->host.c_str(), pRetranslator->port);

			status = connect(pRetranslator->sock, p->ai_addr, p->ai_addrlen);

			if (status == 0) {
				
				pRetranslator->connection_status	= RETRANSLATOR_STATUS_CONNECTED;
				pRetranslator->timeout				= 0;
				pRetranslator->status				= DORTRANS_STATUS_AUTH;
			
				api_log_printf("[DORTRANS] socket #%d connected immidiatly\r\n");
				
				break;
			}
			else {

				int error;

#ifdef _MSC_VER
				error = WSAGetLastError();
				if (error == WSAEWOULDBLOCK)
					pRetranslator->connection_status = RETRANSLATOR_STATUS_CONNECTING;
#else
				error = errno;
				if (error == EINPROGRESS)
					pRetranslator->connection_status = RETRANSLATOR_STATUS_CONNECTING;
#endif
				if (pRetranslator->connection_status != RETRANSLATOR_STATUS_CONNECTING) {
					api_log_printf("[DORTRANS] error #%d on connecting to %s:%u\r\n", error, pRetranslator->host.c_str(), pRetranslator->port);
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

			api_log_printf("[DORTRANS] socket #%d connect timeout, closing\r\n", pRetranslator->sock);

			pRetranslator->connection_status = RETRANSLATOR_STATUS_INIT;
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

		switch (pRetranslator->status) {
		case DORTRANS_STATUS_WAITACK:

			if ((pRetranslator->timeout != 0)&&(pRetranslator->timeout <= now)) {

				api_log_printf("[DORTRANS] socket #%d ack timeout, closing\r\n", pRetranslator->sock);

				pRetranslator->connection_status = RETRANSLATOR_STATUS_INIT;
				closesocket(pRetranslator->sock);
				pRetranslator->sock = -1;
			
				break;
			}

			if (pRetranslator->sock > pContext->max_fd)
				pContext->max_fd = pRetranslator->sock;

			break;

		case DORTRANS_STATUS_ONLINE:
		
			spinlock_lock(&pRetranslator->spinlock);

			if (!pRetranslator->records_list.empty()) {
				FD_SET(pRetranslator->sock, &pContext->fdWriteSet);
			}

			spinlock_unlock(&pRetranslator->spinlock);

			if (pRetranslator->sock > pContext->max_fd)
				pContext->max_fd = pRetranslator->sock;

			if (pRetranslator->last_send + 120 < now) {

				DORTRANSPING dp;

				memset(&dp, 0, sizeof(dp));

				dp.frame_tag[0]	= '~';
				dp.frame_tag[1]	= '~';

				dp.frame_len	= sizeof(DORTRANSPING);

				dp.pack_len		= sizeof(dp.pack_len) + sizeof(dp.pack_num) + sizeof(dp.pack_type) + sizeof(dp.pack_reserved);
				dp.pack_num		= pRetranslator->pack_num++;
				dp.pack_type	= 10;

				unsigned char *uptr = (unsigned char *)&dp;
				dp.frame_crc = 0x00;
				for (int j = 0; j < sizeof(dp) - 1; j++)
					dp.frame_crc ^= *uptr++;			

				api_log_printf("[DORTRANS] Send ping packet, socket #%d\r\n", pRetranslator->sock);

				send(pRetranslator->sock, (char *)&dp, sizeof(dp), 0);

				pRetranslator->last_send = now;
			}

			break;
		}
	}
}

static void enumerator_handler(RETRANSLATOR *pRetranslator, void *ctx)
{
	DORTRANSAUTH	ap;
	DORTRANSACK		ackp;
	DORTRANSNAV		np;

	unsigned char *uptr;
	size_t nNumRecord;

	if (pRetranslator->sock == -1)
		return;

	ENUMCONTEXT *pContext = (ENUMCONTEXT *)ctx;

	time_t now = time(NULL);

	if (FD_ISSET(pRetranslator->sock, &pContext->fdReadSet)) {

#ifdef VERBOSE
		api_log_printf("[DORTRANS] socket #%d is readble\r\n", pRetranslator->sock);
#endif	
		unsigned char buf[8192];

		int status = recv(pRetranslator->sock, (char *)buf, sizeof(buf), 0);

		if (status <= 0) {

			api_log_printf("[DORTRANS] socket #%d is closed\r\n", pRetranslator->sock);
			closesocket(pRetranslator->sock);
			pRetranslator->sock = -1;
			pRetranslator->connection_status = RETRANSLATOR_STATUS_INIT;

			return;
		}

		for (int i = 0; i < status; i++) {

			unsigned char ch = buf[i];

			switch (pRetranslator->frame_state) {

			default:
			case DORTRANS_FRAMETAG1:

				if (ch == '~') {
					pRetranslator->frame_state = DORTRANS_FRAMETAG2;

#ifdef VERBOSE
					api_log_printf("[DORTRANS] Tilda 1, socket #%d\r\n", pRetranslator->sock);
#endif
				}

				break;

			case DORTRANS_FRAMETAG2:

				if (ch == '~') {
					pRetranslator->frame_state = DORTRANS_FRAMELEN;
					pRetranslator->frame_bytes_received = 2;
					pRetranslator->frame_crc = 0;
					pRetranslator->frame_len = 0;
#ifdef VERBOSE
					api_log_printf("[DORTRANS] Tilda 2, socket #%d\r\n", pRetranslator->sock);
#endif
				}
				else {
					pRetranslator->frame_state = DORTRANS_FRAMETAG1; 
				}

				break;

			case DORTRANS_FRAMELEN:

				pRetranslator->frame_crc ^= ch;

				*(((unsigned char *)&pRetranslator->frame_len) + (pRetranslator->frame_bytes_received - 2)) = ch;
				pRetranslator->frame_bytes_received++;

				if (pRetranslator->frame_bytes_received == 6) {
#ifdef VERBOSE
					api_log_printf("[DORTRANS] Frame len %u, socket #%d\r\n", pRetranslator->frame_len, pRetranslator->sock);
#endif
					pRetranslator->frame_state = DORTRANS_FRAMERES;
				}

				break;

			case DORTRANS_FRAMERES:

				pRetranslator->frame_crc ^= ch;

				pRetranslator->frame_bytes_received++;

				if (pRetranslator->frame_bytes_received == 12) {

					pRetranslator->frame_state = (pRetranslator->frame_len > 0) ? DORTRANS_FRAMEBODY : DORTRANS_FRAMECRC;
				}

				break;

			case DORTRANS_FRAMEBODY:

				pRetranslator->frame_crc ^= ch;

				pRetranslator->frame_body[pRetranslator->frame_bytes_received - 12] = ch;

				pRetranslator->frame_bytes_received++;

				if (pRetranslator->frame_bytes_received == pRetranslator->frame_len - 1) {
					pRetranslator->frame_state = DORTRANS_FRAMECRC;
				}
				break;

			case DORTRANS_FRAMECRC:
#ifdef VERBOSE
				api_log_printf("[DORTRANS] Frame crc 0x%02X, actual crc: 0x%02X, socket #%d\r\n", ch & 0xFF, pRetranslator->frame_crc & 0xFF, pRetranslator->sock);
#endif				
				pRetranslator->frame_state = DORTRANS_FRAMETAG1;

				if (ch == pRetranslator->frame_crc) {

					unsigned char *ptr = pRetranslator->frame_body;

					while (ptr < &pRetranslator->frame_body[pRetranslator->frame_bytes_received - 12]) {

						DORPACKETHEADER *ph = (DORPACKETHEADER *)ptr;

						ptr += ph->pack_len;

#ifdef VERBOSE
						api_log_printf("[DORTRANS] Packet type %u, socket #%d\r\n", ph->pack_type, pRetranslator->sock);
#endif
						size_t acks_count;
						unsigned int *acks;

						switch (ph->pack_type) {
						case 101:
							if (*(((unsigned char *)ph) + sizeof(DORPACKETHEADER)) == 0) {
								pRetranslator->status = DORTRANS_STATUS_ONLINE;
								pRetranslator->timeout = 0;
			
								RETRANSLATOR_RECORD rr;

								pRetranslator->records_list.insert(pRetranslator->records_list.begin(), rr);
							}
							else {
								api_log_printf("[DORTRANS] socket #%d auth failed, closing\r\n", pRetranslator->sock);

								pRetranslator->connection_status = RETRANSLATOR_STATUS_INIT;
								closesocket(pRetranslator->sock);
								pRetranslator->sock = -1;
							}

							break;

						case 0:

							acks_count = (ph->pack_len - sizeof(DORPACKETHEADER)) / 4;

							acks = (unsigned int *)((unsigned char *)ph + sizeof(DORPACKETHEADER));

							for (size_t i = 0; i < acks_count; i++) {
								
								for (std::list<RETRANSLATOR_RECORD>::iterator record = pRetranslator->records_list.begin(); record != pRetranslator->records_list.end(); record++) {

									RETRANSLATOR_RECORD &rr = *record;

									if (rr.id == *acks) {
										pRetranslator->records_list.erase(record);
										break;
									}
								}

								acks++;
							}

							pRetranslator->status = DORTRANS_STATUS_ONLINE;
							pRetranslator->timeout = 0;

							api_log_printf("[DORTRANS] %u packets acked, socket #%d\r\n", acks_count, pRetranslator->sock);

							break;

						case 1:
							break;

						default:

							memset(&ackp, 0, sizeof(ackp));

							ackp.frame_tag[0]	= '~';
							ackp.frame_tag[1]	= '~';

							ackp.frame_len		= sizeof(ackp);

							ackp.pack_len		= sizeof(ackp.pack_len) + sizeof(ackp.pack_num) + sizeof(ackp.pack_type) + sizeof(ackp.pack_reserved) + sizeof(ackp.pack_ack_num);
							ackp.pack_num		= pRetranslator->pack_num++;
							ackp.pack_type		= 0;
							ackp.pack_ack_num	= ph->pack_num;

							uptr = (unsigned char *)&ackp;
							ackp.frame_crc = 0x00;
							for (int j = 0; j < sizeof(ackp) - 1; j++)
								ackp.frame_crc ^= *uptr++;			

							api_log_printf("[DORTRANS] Send ack packet, socket #%d\r\n", pRetranslator->sock);

							send(pRetranslator->sock, (char *)&ackp, sizeof(ackp), 0);
						}
					}
				}

				break;
			}
		}
	}

	if (FD_ISSET(pRetranslator->sock, &pContext->fdWriteSet)) {

#ifdef VERBOSE
		api_log_printf("[DORTRANS] socket #%d is writible\r\n", pRetranslator->sock);
#endif		
		switch (pRetranslator->connection_status) {

		case RETRANSLATOR_STATUS_INIT:

			api_log_printf("[DORTRANS] ERROR, socket #%d is in intial state\r\n", pRetranslator->sock);
			break;

		case RETRANSLATOR_STATUS_CONNECTING:

			api_log_printf("[DORTRANS] socket #%d connected\r\n", pRetranslator->sock);

			pRetranslator->connection_status	= RETRANSLATOR_STATUS_CONNECTED;
			pRetranslator->status				= DORTRANS_STATUS_AUTH;
			pRetranslator->timeout				= now + 30;
			pRetranslator->last_send			= now + 120;
			pRetranslator->pack_num				= 0;

		case RETRANSLATOR_STATUS_CONNECTED:

			switch (pRetranslator->status) {
			case DORTRANS_STATUS_AUTH:
			
				memset(&ap, 0, sizeof(ap));

				ap.frame_tag[0] = '~';
				ap.frame_tag[1] = '~';

				ap.frame_len = sizeof(ap);

				ap.pack_len = sizeof(ap.pack_len) + sizeof(ap.pack_num) + sizeof(ap.pack_type) + sizeof(ap.pack_reserved) + sizeof(ap.auth_code);
				ap.pack_num = pRetranslator->pack_num++;
				ap.pack_type = 1;

				ap.auth_code[0] = 0x57;
				ap.auth_code[1] = 0x61;
				ap.auth_code[2] = 0x1A;
				ap.auth_code[3] = 0xA3;
				ap.auth_code[4] = 0xB6;
				ap.auth_code[5] = 0x4A;
				ap.auth_code[6] = 0xB4;
				ap.auth_code[7] = 0x4B;
				ap.auth_code[8] = 0x80;
				ap.auth_code[9] = 0x47;
				ap.auth_code[10] = 0x8B;
				ap.auth_code[11] = 0x4B;
				ap.auth_code[12] = 0xAD;
				ap.auth_code[13] = 0xC0;
				ap.auth_code[14] = 0xD4;
				ap.auth_code[15] = 0x93;

				uptr = (unsigned char *)&ap;
				ap.frame_crc = 0x00;
				for (size_t i = 0; i < sizeof(ap) - 1; i++)
					ap.frame_crc ^= *uptr++;			

				send(pRetranslator->sock, (char *)&ap, sizeof(ap), 0);

				api_log_printf("[DORTRANS] socket #%d send auth request\r\n", pRetranslator->sock);
			
				pRetranslator->status = DORTRANS_STATUS_WAITACK;
				pRetranslator->timeout = now + 30;
				pRetranslator->last_send = now;

				break;

			case DORTRANS_STATUS_ONLINE:
			
				spinlock_lock(&pRetranslator->spinlock);
				
				if (pRetranslator->records_list.empty()) {
					spinlock_unlock(&pRetranslator->spinlock);
					break;
				}

				nNumRecord = 0;

				for (std::list<RETRANSLATOR_RECORD>::iterator record = pRetranslator->records_list.begin(); record != pRetranslator->records_list.end(); record++) {

					RETRANSLATOR_RECORD &rr = *record;

					np.p[nNumRecord].terminal_id	= rr.nupe;
					np.p[nNumRecord].terminal_type	= 733;
					np.p[nNumRecord].arrive_time	= rr.t;
					np.p[nNumRecord].t				= rr.t;
					np.p[nNumRecord].flags			= 0x60;
					np.p[nNumRecord].lat			= rr.latitude;
					np.p[nNumRecord].lon			= rr.longitude;
					np.p[nNumRecord].speed			= rr.speed / 10;
					np.p[nNumRecord].cog			= rr.cog;
					np.p[nNumRecord].alt			= rr.altitude;
					np.p[nNumRecord].nsat			= 3;
					np.p[nNumRecord].mileage		= 0;
					np.p[nNumRecord].flags2			= 0;
					np.p[nNumRecord].csq			= 21;

					if ((rr.latitude != 0)&&(rr.longitude != 0))
						np.p[nNumRecord].flags |= 0x80;

					if (rr.flags1 & RECORD_FLAG1_IGNITION)
						np.p[nNumRecord].flags |= 0x02;

					rr.id = pRetranslator->pack_num + nNumRecord;

					nNumRecord++;

					if (nNumRecord == DORTRANS_MAX_RECORDS)
						break;
				}

				spinlock_unlock(&pRetranslator->spinlock);

				if (nNumRecord > 0) {

					np.frame_tag[0]	= '~';
					np.frame_tag[1]	= '~';

					np.frame_len	= 13;

					for (size_t i = 0; i < nNumRecord; i++) {

						np.p[i].pack_len	= sizeof(np.p[i]);
						np.p[i].pack_num	= pRetranslator->pack_num++;
						np.p[i].pack_type	= 2;
					
						np.frame_len += np.p[i].pack_len;
					}

					uptr = (unsigned char *)&np;
					np.frame_crc = 0x00;
					for (size_t i = 0; i < np.frame_len - 1; i++)
						np.frame_crc ^= *uptr++;			

					send(pRetranslator->sock, (char *)&np, np.frame_len - 1, 0);
					send(pRetranslator->sock, (char *)&np.frame_crc, 1, 0);

					pRetranslator->status		= DORTRANS_STATUS_WAITACK;
					pRetranslator->timeout		= now + 30;
					pRetranslator->last_send	= now;

					api_log_printf("[DORTRANS] Send %u records, socket #%d\r\n", nNumRecord, pRetranslator->sock);
				}
			}
	
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

			api_log_printf("[DORTRANS] select error #%u\r\n", error);

			if (error != 9)
				break;
		}

		retranslators_enum(enumerator_handler, &ctx);
	}
}

// End

