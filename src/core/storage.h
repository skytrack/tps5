//******************************************************************************
//
// File Name : storage.h
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#ifndef _STORAGE_H

#define _STORAGE_H

#include <stdlib.h>

/*******************************************************************************/
/*  онстанты дл€ *_commit_type */
/*******************************************************************************/

// write и flush производ€тс€ раз в секунду
// ѕри любом падении софта или всей системы последн€€ секунда тер€етс€
// —амый быстрый и неустойчивый метод
#define STORAGE_COMMIT_TYPE_1		0x00

// write и flush производ€тс€ дл€ каждой поступившей записи
// —амый устойчивый и медленный метод
#define STORAGE_COMMIT_TYPE_2		0x01

// write производитс€ дл€ каждой поступившей записи
// flush производитс€ раз в секунду
// ѕри падении софта ничего не тер€етс€, при падении системы или пропадании 
// питани€ тер€етс€ последн€€ секунда
#define STORAGE_COMMIT_TYPE_3		0x02


/*******************************************************************************/
/*  онстанты дл€ *_file_mode */
/*******************************************************************************/

#define STORAGE_FILE_MODE_REGULAR	0x00

// The file is opened for synchronous I/O. Any writes on the resulting file 
// descriptor will block the calling process until the data has been physically 
// written to the underlying hardware
#define STORAGE_FILE_MODE_OSYNC		0x01

// Try to minimize cache effects of the I/O to and from this file. 
// In general this will degrade performance, but it is useful in special 
// situations, such as when applications do their own caching. File I/O is done 
// directly to/from user-space buffers. The O_DIRECT flag on its own makes an 
// effort to transfer data synchronously, but does not give the guarantees of the 
// O_SYNC flag that data and necessary metadata are transferred. To guarantee 
// synchronous I/O, O_SYNC must be used in addition to O_DIRECT
#define STORAGE_FILE_MODE_ODIRECT	0x02

#include <vector>

#pragma pack(push,1)

#ifdef _MSC_VER
#define DEFAULT_STREAM_SIZE (1024 * 1024 / 2)
#else
#define DEFAULT_STREAM_SIZE (32 * 1024 * 1024)
#endif

typedef struct tag_STORAGE_RECORD_HEADER
{
	unsigned short size;
	unsigned int t;
} STORAGE_RECORD_HEADER;

#pragma pack(pop)

typedef struct tag_STORAGE_SORT_ITEM
{
	unsigned int t;
	STORAGE_RECORD_HEADER *rh;
	STORAGE_RECORD_HEADER *sort_rh;
	unsigned char *dst;
	size_t prior_len;

	bool operator < (const struct tag_STORAGE_SORT_ITEM& a) const
	{
		return (t < a.t);
	};

} STORAGE_SORT_ITEM;

typedef struct tag_STREAM_INFO
{
	unsigned int	last_nav_time;
	unsigned int	last_latitude;
	unsigned int	last_longitude;
	unsigned short	last_speed;
	unsigned short	last_cog;
	unsigned int	last_altitude;

	unsigned int	last_lls1_time;
	unsigned short	last_rs485_1;

	unsigned int	last_lls2_time;
	unsigned short	last_rs485_2;

	unsigned int	last_freq1_time;
	unsigned short	last_freq1;

	unsigned int	last_freq2_time;
	unsigned short	last_freq2;

	unsigned int	last_freq3_time;
	unsigned short	last_freq3;

	unsigned int	last_freq4_time;
	unsigned short	last_freq4;

	unsigned int	last_freq5_time;
	unsigned short	last_freq5;

	unsigned int	last_freq6_time;
	unsigned short	last_freq6;

	unsigned int	last_freq7_time;
	unsigned short	last_freq7;

	unsigned int	last_freq8_time;
	unsigned short	last_freq8;

	unsigned int	last_flags_time;
	unsigned char	last_flags1;
	unsigned char	last_flags2;

	unsigned int	last_adc1_time;
	unsigned short	last_adc1;
	unsigned int	last_adc2_time;
	unsigned short	last_adc2;
	unsigned int	last_adc3_time;
	unsigned short	last_adc3;

	unsigned char	online;
} STREAM_INFO;

int storage_init(const char *dir = NULL, int commit_type = STORAGE_COMMIT_TYPE_2, int file_mode = STORAGE_FILE_MODE_REGULAR, size_t buffer_len = 1024 * 1024);
void storage_cleanup();

void *storage_create_stream(int id, size_t capacity);
void storage_destroy_stream(void *s);
void *storage_get_stream_by_id(int id);

void storage_lock_stream(void *s);
void storage_unlock_stream(void *s);

int storage_add_record_to_stream(void *s, STORAGE_RECORD_HEADER *rh, size_t len);
STORAGE_RECORD_HEADER *storage_get_stream_first_record(void *s);
size_t storage_get_stream_records_count(void *s);
void storage_update_record(unsigned short id, STORAGE_RECORD_HEADER *rh, const char *field, void *value);

void storage_sort_stream(void *s, void *sort_buffer, std::vector<STORAGE_SORT_ITEM> *si);
void storage_trim_stream(void *s, unsigned int base_time);
STREAM_INFO *storage_get_stream_info(void *s);
void storage_set_retranslator(void *s, void *module, void *retranslator);

int storage_1sec_timer();
void storage_load(unsigned int t);

#endif

// End
