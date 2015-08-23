//******************************************************************************
//
// File Name	: log.h
// Author	: Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#ifndef _LOG_H

#define _LOG_H

/*******************************************************************************/
/*  онстанты дл€ *_commit_type */
/*******************************************************************************/

// write и flush производ€тс€ раз в секунду
// ѕри любом падении софта или всей системы последн€€ секунда тер€етс€
// —амый быстрый и неустойчивый метод
#define LOG_COMMIT_TYPE_1		0x01

// write и flush производ€тс€ дл€ каждой поступившей записи
// —амый устойчивый и медленный метод
#define LOG_COMMIT_TYPE_2		0x02

// write производитс€ дл€ каждой поступившей записи
// flush производитс€ раз в секунду
// ѕри падении софта ничего не тер€етс€, при падении системы или пропадании 
// питани€ тер€етс€ последн€€ секунда
#define LOG_COMMIT_TYPE_3		0x03


/*******************************************************************************/
/*  онстанты дл€ *_file_mode */
/*******************************************************************************/

#define LOG_FILE_MODE_REGULAR	0x00

// The file is opened for synchronous I/O. Any writes on the resulting file 
// descriptor will block the calling process until the data has been physically 
// written to the underlying hardware
#define LOG_FILE_MODE_OSYNC	0x01

// Try to minimize cache effects of the I/O to and from this file. 
// In general this will degrade performance, but it is useful in special 
// situations, such as when applications do their own caching. File I/O is done 
// directly to/from user-space buffers. The O_DIRECT flag on its own makes an 
// effort to transfer data synchronously, but does not give the guarantees of the 
// O_SYNC flag that data and necessary metadata are transferred. To guarantee 
// synchronous I/O, O_SYNC must be used in addition to O_DIRECT
#define LOG_FILE_MODE_ODIRECT	0x02

int log_printf(const char * __format, ... );
int log_1sec_timer();
int log_init(const char *dir = NULL, int commit_type = LOG_COMMIT_TYPE_2, int file_mode = LOG_FILE_MODE_REGULAR, size_t buffer_len = 4096);
int log_cleanup();
void log_commit();

#endif

// End
