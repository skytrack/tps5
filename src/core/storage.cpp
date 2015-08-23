//******************************************************************************
//
// File Name : storage.cpp
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <algorithm>
#include <omp.h>

#include "cross.h"
#include "core.h"
#include "likely.h"
#include "storage.h"
#include "log.h"
#include "spinlock.h"
#include "record.h"
#include "module.h"

static char data_dir[2048];
static char patches_file[2048];
static int data_commit_type;
static int data_file_mode;
static unsigned long data_buffer_len;

static char *data_buffer = NULL;
static int  data_file    = -1;

static char *data_buffer_current;
static int  data_bytes_left;

static int  data_file_oflag, data_mday;
static FILE *patch = NULL;

typedef struct tag_PATCH_RECORD {
	unsigned short id;
	unsigned int t;
	char field[32];
	char value[8];
} PATCH_RECORD;

typedef struct tag_STORAGE_STREAM
{
	int id;
	// Размер буфера в байтах
	size_t capacity;
	// Число записей в буфере
	size_t count;
	// Размер записей в буфере в байтах
	size_t len;
	// Число отсортированных записей в начале буфера
	size_t sorted_count;
	// Размер отсортированных записей в байтах
	size_t sorted_len;
	// Указатель на буфер
	unsigned char *buffer;	
	unsigned char *data;	
	unsigned int base_time;
	// Объект для синхронизации и блокировки
	spinlock_t spinlock;

	void *retranslator_module;
	void *retranslator;

	STREAM_INFO info;

} STORAGE_STREAM;

static STORAGE_STREAM *storage[USHRT_MAX + 1];

int storage_init(const char *dir, int commit_type, int file_mode, size_t buffer_len)
{
	char data_path[2048];
	char error[2048];
	struct tm tms;
	time_t now;

	if (dir != NULL) {
		data_dir[sizeof(data_dir) - 1] = '\0';
		strncpy(data_dir, dir, sizeof(data_dir) - 1);
	}
	else 
		*data_dir = '\0';

	data_commit_type = commit_type;
	data_file_mode = file_mode;

	data_buffer_len = buffer_len;

	data_buffer = (char *)malloc(data_buffer_len);
	
	if (unlikely(data_buffer == NULL)) {
		printf("Unable to allocate data buffer\r\n");
		return -1;
	}

	data_file_oflag = O_WRONLY | O_APPEND | O_CREAT | _O_BINARY;
                
	if (data_file_mode == STORAGE_FILE_MODE_OSYNC)
		data_file_oflag |= O_SYNC;

	if (data_file_mode == STORAGE_FILE_MODE_ODIRECT)
		data_file_oflag |= O_DIRECT;

	now = time(NULL);
	localtime_r(&now, &tms);

	data_mday = tms.tm_mday;

	snprintf(data_path, sizeof(data_path), "%s/%04u%02u%02u.dat", data_dir, tms.tm_year + 1900, tms.tm_mon + 1, tms.tm_mday);

	data_file = open(data_path, data_file_oflag, PMODE);

	if (unlikely(data_file == -1)) {

		int prev_errno = errno;

		if (likely(strerror_r(errno, error, sizeof(error)) == 0))
			log_printf("Failed to open data file '%s': %s [%d]\r\n", data_path, error, errno);
		else
			log_printf("Failed to open data file '%s', errno %d\r\n", data_path, prev_errno);

		return -1;
	}

	data_buffer_current = data_buffer;
	data_bytes_left = data_buffer_len;

	memset(storage, 0, sizeof(storage));

	strcpy(patches_file, data_dir);
	strcat(patches_file, "/patches.dat");

	patch = fopen(patches_file, "a+b");

	return 0;
}

void storage_cleanup()
{
	for (size_t i = 0; i < USHRT_MAX; i++) {
		if (storage[i] != NULL) {
			if (storage[i]->data != NULL) {
				free(storage[i]->data);
			}
			free(storage[i]);
		}
	}

	storage_1sec_timer();

	if (data_buffer != NULL) {
		free(data_buffer);
		data_buffer = NULL;
	}

	if (data_file != -1) {
		close(data_file);
		data_file = -1;
	}

	if (patch != NULL) {
		fclose(patch);
	}
}

void *storage_create_stream(int id, size_t capacity)
{
	STORAGE_STREAM *stream = (STORAGE_STREAM *)malloc(sizeof(STORAGE_STREAM));

	memset(stream, 0, sizeof(STORAGE_STREAM));

	stream->data = (unsigned char *)malloc(capacity);

	if (stream->data == NULL) {
		free(stream);
		return NULL;
	}

	stream->buffer = stream->data + 2;
	stream->id = id;
	stream->capacity = capacity;
	stream->len = 0;
	stream->count = 0;
	stream->sorted_count = 0;
	stream->sorted_len = 0;
	
	spinlock_init(&stream->spinlock);

	stream->base_time = 0;

	storage[id] = stream;

	return stream;
}

void storage_destroy_stream(void *s)
{
	STORAGE_STREAM *stream = (STORAGE_STREAM *)s;	

	if (stream->data != NULL) {
		free(stream->data);
		stream->data = NULL;
	}

	storage[stream->id] = NULL;

	free(stream);
}

void *storage_get_stream_by_id(int id)
{
	return storage[id];
}

void storage_lock_stream(void *s)
{
	STORAGE_STREAM *stream = (STORAGE_STREAM *)s;	

	spinlock_lock(&stream->spinlock);
}

void storage_unlock_stream(void *s)
{
	STORAGE_STREAM *stream = (STORAGE_STREAM *)s;

	spinlock_unlock(&stream->spinlock);
}

STORAGE_RECORD_HEADER *storage_get_stream_first_record(void *s)
{
	STORAGE_STREAM *stream = (STORAGE_STREAM *)s;

	if (stream->len == 0)
		return NULL;

	return (STORAGE_RECORD_HEADER *)stream->buffer;
}

size_t storage_get_stream_records_count(void *s)
{
	STORAGE_STREAM *stream = (STORAGE_STREAM *)s;

	return stream->count;
}

static void storage_update_info(STORAGE_STREAM *stream, STORAGE_RECORD_HEADER *rh, size_t len, bool bulk_load)
{
	RETRANSLATOR_RECORD rr;

	unsigned char *bits = (unsigned char *)rh + sizeof(STORAGE_RECORD_HEADER);
	unsigned char *ptr = bits;

	while (*ptr & RECORD_BIT_MORE)
		ptr++;

	ptr++;

	if (bits[0] & RECORD_BIT1_EVENT) {
		unsigned short event_id = *(unsigned short *)ptr;

		if (event_id == RECORD_EVENT_TERMINAL_ONLINE)
			stream->info.online = 1;
		else
		if (event_id == RECORD_EVENT_TERMINAL_OFFLINE)
			stream->info.online = 0;
		ptr += 2;
	}

	if (bits[0] & RECORD_BIT1_FLAGS) {

		rr.t		= rh->t;
		rr.flags1	= *ptr++;
		rr.flags2	= (rr.flags1 & RECORD_FLAG1_MORE) ? *ptr++ : 0; 

		if (stream->info.last_flags_time < rh->t) {
		
			stream->info.last_flags_time = rh->t;

			stream->info.last_flags1 = rr.flags1; 
			stream->info.last_flags2 = rr.flags2; 
		}
	}

	if (bits[0] & RECORD_BIT1_NAV) {

		rr.latitude = *(int *)ptr; ptr += 4;
		rr.longitude = *(int *)ptr; ptr += 4;
		rr.speed = *ptr++;

		if (rr.flags1 & RECORD_FLAG1_SPEED_9)
			rr.speed |= 0x0100;
		if (rr.flags1 & RECORD_FLAG1_SPEED_10)
			rr.speed |= 0x0200;
		if (rr.flags1 & RECORD_FLAG1_SPEED_11)
			rr.speed |= 0x0400;

		if (stream->info.last_nav_time < rh->t) {
		
			stream->info.last_nav_time = rr.t;

			stream->info.last_latitude = rr.latitude;
			stream->info.last_longitude = rr.longitude;
			stream->info.last_speed = rr.speed;
		}
	}
	else {
		rr.latitude = 0;
		rr.longitude = 0;
		rr.speed = 0;
	}

	if (bits[0] & RECORD_BIT1_ALT) {

		rr.altitude = *(short *)ptr;
		ptr += 2;

		if (stream->info.last_nav_time <= rh->t) {
		
			stream->info.last_altitude = rr.altitude;
		}

		ptr += 2;
	}
	else
		rr.altitude = 0;

	if (bits[0] & RECORD_BIT1_COG) {

		rr.cog = *ptr++;

		if (rr.flags1 & RECORD_FLAG1_COG_9)
			rr.cog |= 0x0100;

		if (stream->info.last_nav_time <= rh->t) {
		
			stream->info.last_cog = rr.cog;
		}
	}
	else {
		rr.cog = 0;
	}
	
	if (bits[0] & RECORD_BIT1_RS485_1) {

		if (stream->info.last_lls1_time < rh->t) {

			stream->info.last_lls1_time = rh->t;

			stream->info.last_rs485_1 = *(unsigned short *)ptr;
		}

		ptr += 2;
	}

	if (bits[0] & RECORD_BIT1_RS485_2) {

		if (stream->info.last_lls2_time < rh->t) {

			stream->info.last_lls2_time = rh->t;

			stream->info.last_rs485_2 = *(unsigned short *)ptr;
		}

		ptr += 2;
	}

	if (bits[0] & RECORD_BIT_MORE) {

		if (bits[1] & RECORD_BIT2_ADC1) {
	
			if (stream->info.last_adc1_time < rh->t) {

				stream->info.last_adc1_time = rh->t;

				stream->info.last_adc1 = *(unsigned short *)ptr;
			}

			ptr += 2;
		}
		if (bits[1] & RECORD_BIT2_ADC2) {
	
			if (stream->info.last_adc2_time < rh->t) {

				stream->info.last_adc2_time = rh->t;

				stream->info.last_adc2 = *(unsigned short *)ptr;
			}

			ptr += 2;
		}
		if (bits[1] & RECORD_BIT2_ADC3) {
	
			if (stream->info.last_adc3_time < rh->t) {

				stream->info.last_adc3_time = rh->t;

				stream->info.last_adc3 = *(unsigned short *)ptr;
			}

			ptr += 2;
		}
		if (bits[1] & RECORD_BIT2_FREQUENCY1) {
	
			if (stream->info.last_freq1_time < rh->t) {

				stream->info.last_freq1_time = rh->t;

				stream->info.last_freq1 = *(unsigned short *)ptr;
			}

			ptr += 2;
		}	
		if (bits[1] & RECORD_BIT2_FREQUENCY2) {
	
			if (stream->info.last_freq2_time < rh->t) {

				stream->info.last_freq2_time = rh->t;

				stream->info.last_freq2 = *(unsigned short *)ptr;
			}

			ptr += 2;
		}	
		if (bits[1] & RECORD_BIT2_FREQUENCY3) {
	
			if (stream->info.last_freq3_time < rh->t) {

				stream->info.last_freq3_time = rh->t;

				stream->info.last_freq3 = *(unsigned short *)ptr;
			}

			ptr += 2;
		}	
		if (bits[1] & RECORD_BIT2_FREQUENCY4) {
	
			if (stream->info.last_freq4_time < rh->t) {

				stream->info.last_freq4_time = rh->t;

				stream->info.last_freq4 = *(unsigned short *)ptr;
			}

			ptr += 2;
		}	
	}

	if ((stream->retranslator != NULL)&&(stream->retranslator_module != NULL)&&(!bulk_load)) {
		((MODULE *)stream->retranslator_module)->add_record_to_retranslator(stream->retranslator, &rr);
	}

}

int storage_add_record_to_stream(void *s, STORAGE_RECORD_HEADER *rh, size_t len)
{
	STORAGE_STREAM *stream = (STORAGE_STREAM *)s;

	if (stream->base_time > rh->t)
		return 0;

	if (stream->len + len >= stream->capacity - 2) {
		log_printf("[STORAGE] No room for %u\r\n", stream->id);
		return 0;
	}

	storage_lock_stream(stream);

	unsigned char *dst_ptr = stream->buffer + stream->len;
	memcpy(dst_ptr, rh, len);

	unsigned short tmp = *(unsigned short *)(dst_ptr - 2);
	*(unsigned short *)(dst_ptr - 2) = stream->id;

	stream->len += len;
	stream->count++;

	if (likely(data_commit_type == STORAGE_COMMIT_TYPE_1)) {
		memcpy(data_buffer_current, dst_ptr - 2, len + 2);
		data_buffer_current += len;
		data_bytes_left -= len;
	}
	else
	if (data_commit_type == STORAGE_COMMIT_TYPE_2) {

		write(data_file, dst_ptr - 2, len + 2);
		fsync(data_file);
	}
	else {
		write(data_file, dst_ptr - 2, len + 2);
	}

	*(unsigned short *)(dst_ptr - 2) = tmp;

	storage_update_info(stream, rh, rh->size, false);

	storage_unlock_stream(stream);

	return 0;
}

void storage_trim_stream(void *s, unsigned int base_time)
{
	STORAGE_STREAM *stream = (STORAGE_STREAM *)s;

	size_t records = 0;
	unsigned char *src_ptr = stream->buffer;

	while ((src_ptr < stream->buffer + stream->sorted_len)&&(((STORAGE_RECORD_HEADER *)src_ptr)->t <= base_time)) {
		src_ptr += ((STORAGE_RECORD_HEADER *)src_ptr)->size;
		records++;
	}

	if (src_ptr != stream->buffer) {

		stream->count -= records;
		stream->len -= src_ptr - stream->buffer;
		
		memcpy(stream->buffer, src_ptr, stream->len);

		stream->sorted_count = stream->count;
		stream->sorted_len = stream->len;
	}

	stream->base_time = base_time;
}

void storage_drop_first_records(void *s, unsigned int count)
{
	STORAGE_STREAM *stream = (STORAGE_STREAM *)s;

	size_t records = 0;
	unsigned char *src_ptr = stream->buffer;

	while (src_ptr < stream->buffer + stream->sorted_len) {
		src_ptr += ((STORAGE_RECORD_HEADER *)src_ptr)->size;
		records++;

		if (records == count)
			break;
	}

	if (src_ptr != stream->buffer) {

		stream->count -= records;
		stream->len -= src_ptr - stream->buffer;
		
		memcpy(stream->buffer, src_ptr, stream->len);

		stream->sorted_count = stream->count;
		stream->sorted_len = stream->len;
	}
}

void storage_sort_stream(void *s, void *sort_buffer, std::vector<STORAGE_SORT_ITEM> *si)
{
	STORAGE_STREAM *stream = (STORAGE_STREAM *)s;

	if (stream->count == stream->sorted_count)
		return;

	// Число неотсортированных записей
	size_t size = stream->count - stream->sorted_count;

	si->clear();
	si->reserve(size);

	// Указатель на первую неотсортированную запись
	unsigned char *src_ptr = stream->buffer + stream->sorted_len;

	// Строю упрощенный массив из неотсортированных элементов
	// В упрощенном массиве элементы короткие и они одного размера, поэтому их быстрее сортировать и проще переставлять
	for (size_t i = 0; i < size; i++) {
		STORAGE_SORT_ITEM s;
		s.t = ((STORAGE_RECORD_HEADER *)src_ptr)->t;
		s.rh = (STORAGE_RECORD_HEADER *)src_ptr;
		si->push_back(s);
		src_ptr += ((STORAGE_RECORD_HEADER *)src_ptr)->size;
	}

	// Сортирую
	std::sort(si->begin(), si->end());

	// Заполняю сортировочный буфер новыми элементами уже в отсортированном порядке
	unsigned char *sort_ptr = (unsigned char *)sort_buffer;

	size_t len = 0;
	for (size_t i = 0; i < size; i++) {
		memcpy(sort_ptr, si->at(i).rh, si->at(i).rh->size);
		si->at(i).sort_rh = (STORAGE_RECORD_HEADER *)sort_ptr;
		si->at(i).prior_len = len;
		len += si->at(i).rh->size;
		sort_ptr += si->at(i).rh->size;
	}

	// Прохожу по отсортированной части буфера и нахожу места куда надо вставлять новые элементы

	src_ptr = stream->buffer;

	unsigned char *prev_ptr;
	char all_in_one_place = 1;

	for (size_t i = 0; i < size; i++) {
	
		while (src_ptr < stream->buffer + stream->sorted_len) {

			STORAGE_RECORD_HEADER *rh = (STORAGE_RECORD_HEADER *)src_ptr;

			// Как только нахожу старый элемент больше нового - новый надо ставить перед ним
			if (rh->t > si->at(i).t)
				break;

			src_ptr += ((STORAGE_RECORD_HEADER *)src_ptr)->size;
		}

		// Переменная all_in_one_place будет 1 если все элементы надо вставить в одно и то же место
		if ((i > 0)&&(prev_ptr != src_ptr))
			all_in_one_place = 0;

		si->at(i).dst = src_ptr;
		prev_ptr = src_ptr;
	}
		
	// Если все новые элементы надо поместить в одно место основного буфера
	if (all_in_one_place) {

		// Если основной буфер пуст или все элементы сортировочного буфера больше основного буфера - копирую сортировочный буфер в конец основного буфера
		if (src_ptr == stream->buffer + stream->sorted_len) {

			memcpy(stream->buffer + stream->sorted_len, sort_buffer, stream->len - stream->sorted_len);
		}
		else {

			// Иначе открываю окно в основном буфере
			memcpy(si->at(0).dst + (stream->len - stream->sorted_len), si->at(0).dst, stream->sorted_len - (si->at(0).dst - stream->buffer));
			// Копирую сортировочный буфер в окно в основном буфере
			memcpy(si->at(0).dst, sort_buffer, stream->len - stream->sorted_len);
		}
	}
	else {

		// Ищу в сортировочном буфере точку разрыва
		// Элементы слева от точки разрыва должны быть помещены внутри основного буфера
		// Элементы начиная с точки разрыва должны быть добавлены в конец основного буфера

		size_t delimeter = size;

		for (size_t i = 0; i < size; i++) {
			
			if (si->at(i).dst == stream->buffer + stream->sorted_len) {
				delimeter = i;
				break;
			}
		}

		size_t bytes_moved = 0;

		unsigned char *buffer_end = stream->buffer + stream->sorted_len;

		for (int i = delimeter - 1; i >= 0; ) {

			size_t inner_data_len = si->at(i).sort_rh->size;

			// Смотрю сколько элементов подряд встают в одну позицию
			int j = i - 1;

			while ((j >= 0)&&(si->at(i).dst == si->at(j).dst)) {
				inner_data_len += si->at(j).sort_rh->size;
				j--;
			}

			// Открываю окно в основном буфере под размер идущих подряд элементов в сортировочном буфере
			unsigned char *window_begin = si->at(i).dst;
			size_t window_size = inner_data_len + si->at(j + 1).prior_len;

			memcpy(window_begin + window_size, window_begin, buffer_end - window_begin);

			bytes_moved += buffer_end - window_begin;

			// Заполняю окно реальными элементами
			memcpy(si->at(j + 1).dst + si->at(j + 1).prior_len, si->at(j + 1).sort_rh, inner_data_len);

			buffer_end = window_begin;

			i = j;
		}

		// Элементы сортировочного буфера начиная с разделителя становятся продолжением основного буфера

		if (delimeter < size) {

			unsigned char *dst_ptr = stream->buffer + stream->sorted_len;

			if (delimeter > 0) 
				dst_ptr += si->at(delimeter).prior_len;

			memcpy(dst_ptr, si->at(delimeter).sort_rh, sort_ptr - (unsigned char *)si->at(delimeter).sort_rh);
		}
	}

	// Завершение
	stream->sorted_count = stream->count;
	stream->sorted_len = stream->len;
}

STREAM_INFO *storage_get_stream_info(void *s)
{
	STORAGE_STREAM *stream = (STORAGE_STREAM *)s;

	return &stream->info;
}

void storage_update_record(unsigned short id, STORAGE_RECORD_HEADER *rh, const char *field, void *value) 
{
	unsigned char field_no = 0;
	size_t len = strlen(field);
	switch (len) {
	case 5:
		if (memcmp(field, "speed", 5) == 0) {
			field_no = 255;
			break;
		}
	case 7:
		if (memcmp(field, "rs485_1", 7) == 0) {
			field_no = 1;
			break;
		}
		if (memcmp(field, "rs485_2", 7) == 0) {
			field_no = 2;
			break;
		}
		if (memcmp(field, "analog1", 7) == 0) {
			field_no = 3;
			break;
		}
		if (memcmp(field, "analog2", 7) == 0) {
			field_no = 4;
			break;
		}
		if (memcmp(field, "analog3", 7) == 0) {
			field_no = 5;
			break;
		}
		if (memcmp(field, "analog4", 7) == 0) {
			field_no = 6;
			break;
		}
		if (memcmp(field, "analog5", 7) == 0) {
			field_no = 7;
			break;
		}
		if (memcmp(field, "analog6", 7) == 0) {
			field_no = 8;
			break;
		}
		if (memcmp(field, "analog7", 7) == 0) {
			field_no = 9;
			break;
		}
		if (memcmp(field, "analog8", 7) == 0) {
			field_no = 10;
			break;
		}
		break;

	case 10:
		if (memcmp(field, "frequency1", 10) == 0) {
			field_no = 11;
			break;
		}
		if (memcmp(field, "frequency2", 10) == 0) {
			field_no = 12;
			break;
		}
		if (memcmp(field, "frequency3", 10) == 0) {
			field_no = 13;
			break;
		}
		if (memcmp(field, "frequency4", 10) == 0) {
			field_no = 14;
			break;
		}
		if (memcmp(field, "frequency5", 10) == 0) {
			field_no = 15;
			break;
		}
		if (memcmp(field, "frequency6", 10) == 0) {
			field_no = 16;
			break;
		}
		if (memcmp(field, "frequency7", 10) == 0) {
			field_no = 17;
			break;
		}
		if (memcmp(field, "frequency8", 10) == 0) {
			field_no = 18;
			break;
		}
		break;
	}

	if (field_no == 0) {
		return;
	}

	if (id != 0) {

		PATCH_RECORD pr;

		pr.id = id;
		pr.t = rh->t;
		memcpy(pr.field, field, len + 1);
		memcpy(pr.value, value, 4);

		if (patch != NULL) {
			fwrite(&pr, sizeof(pr), 1, patch);
			fflush(patch);
		}
	}

	unsigned char *bits = (unsigned char *)rh + sizeof(STORAGE_RECORD_HEADER);
	unsigned char *ptr = bits;

	while (*ptr & RECORD_BIT_MORE)
		ptr++;

	ptr++;

	if (bits[0] & RECORD_BIT1_EVENT)
		ptr += 2;

	if (bits[0] & RECORD_BIT1_FLAGS) {

		if (field_no == 255)
			*ptr |= (RECORD_FLAG1_SPEED_9 | RECORD_FLAG1_SPEED_10 | RECORD_FLAG1_SPEED_11);

		if (*ptr & RECORD_FLAG1_MORE)
			ptr += 2;
		else
			ptr++;
	}

	if (bits[0] & RECORD_BIT1_NAV) {
		if (field_no == 255) {
			*(ptr + 8) = 0xFF;
			return;
		}
		ptr += 9;
	}

	if (bits[0] & RECORD_BIT1_ALT)
		ptr += 2;

	if (bits[0] & RECORD_BIT1_COG)
		ptr++;
	
	if (bits[0] & RECORD_BIT1_RS485_1) {

		if (field_no == 1) {
			*(unsigned short *)ptr = *(unsigned short *)value;
			return;
		}

		ptr += 2;
	}

	if (bits[0] & RECORD_BIT1_RS485_2) {

		if (field_no == 2) {
			*(unsigned short *)ptr = *(unsigned short *)value;
			return;
		}

		ptr += 2;
	}

	if (bits[0] & RECORD_BIT_MORE) {

		if (bits[1] & RECORD_BIT2_ADC1) {
	
			if (field_no == 3) {
				*(unsigned short *)ptr = *(unsigned short *)value;
				return;
			}

			ptr += 2;
		}
		if (bits[1] & RECORD_BIT2_ADC2) {
	
			if (field_no == 4) {
				*(unsigned short *)ptr = *(unsigned short *)value;
				return;
			}

			ptr += 2;
		}
		if (bits[1] & RECORD_BIT2_ADC3) {
	
			if (field_no == 5) {
				*(unsigned short *)ptr = *(unsigned short *)value;
				return;
			}

			ptr += 2;
		}
		if (bits[1] & RECORD_BIT2_FREQUENCY1) {
	
			if (field_no == 11) {
				*(unsigned short *)ptr = *(unsigned short *)value;
				return;
			}

			ptr += 2;
		}	
		if (bits[1] & RECORD_BIT2_FREQUENCY2) {
	
			if (field_no == 12) {
				*(unsigned short *)ptr = *(unsigned short *)value;
				return;
			}

			ptr += 2;
		}	
		if (bits[1] & RECORD_BIT2_FREQUENCY3) {
	
			if (field_no == 13) {
				*(unsigned short *)ptr = *(unsigned short *)value;
				return;
			}

			ptr += 2;
		}	
		if (bits[1] & RECORD_BIT2_FREQUENCY4) {
	
			if (field_no == 14) {
				*(unsigned short *)ptr = *(unsigned short *)value;
				return;
			}

			ptr += 2;
		}	

		if (bits[1] & RECORD_BIT_MORE) {

			if (bits[2] & RECORD_BIT3_VCC)
				ptr += 2;	
			if (bits[2] & RECORD_BIT3_SAT_NO)
				ptr++;

			if (bits[2] & RECORD_BIT3_ADC4) {
	
				if (field_no == 6) {
					*(unsigned short *)ptr = *(unsigned short *)value;
					return;
				}

				ptr += 2;
			}

			if (bits[2] & RECORD_BIT3_COUNTER1)
				ptr += 2;
			if (bits[2] & RECORD_BIT3_COUNTER2)
				ptr += 2;
			if (bits[2] & RECORD_BIT3_COUNTER3)
				ptr += 2;
			if (bits[2] & RECORD_BIT3_COUNTER4)	
				ptr += 2;

			if (bits[2] & RECORD_BIT_MORE) {
	
				if (bits[3] & RECORD_BIT4_RS232_1)
					ptr += 2;
				if (bits[3] & RECORD_BIT4_RS232_2)
					ptr += 2;
				if (bits[3] & RECORD_BIT4_ODOMETER)
					ptr += 4;
				if (bits[3] & RECORD_BIT4_FREQUENCY5) {
	
					if (field_no == 15) {
						*(unsigned short *)ptr = *(unsigned short *)value;
						return;
					}

					ptr += 2;
				}	
				if (bits[3] & RECORD_BIT4_FREQUENCY6) {
	
					if (field_no == 16) {
						*(unsigned short *)ptr = *(unsigned short *)value;
						return;
					}

					ptr += 2;
				}	
				if (bits[3] & RECORD_BIT4_FREQUENCY7) {
	
					if (field_no == 17) {
						*(unsigned short *)ptr = *(unsigned short *)value;
						return;
					}

					ptr += 2;
				}	
				if (bits[3] & RECORD_BIT4_FREQUENCY8) {
	
					if (field_no == 18) {
						*(unsigned short *)ptr = *(unsigned short *)value;
						return;
					}

					ptr += 2;
				}				
			}
		}
	}
}

int storage_1sec_timer()
{
	char data_path[2048];
	time_t now;
	struct tm tms;

	now = time(NULL);
	localtime_r(&now, &tms);

	if (likely(data_commit_type == STORAGE_COMMIT_TYPE_1)) {

		write(data_file, data_buffer, data_buffer_len - data_bytes_left);
		fsync(data_file);

		data_buffer_current = data_buffer;
		data_bytes_left = data_buffer_len;
	}
	else
	if (data_commit_type == STORAGE_COMMIT_TYPE_3) {
		fsync(data_file);
	}

	if (unlikely(data_mday != tms.tm_mday)) {

		close(data_file);

		data_mday = tms.tm_mday;

		snprintf(data_path, sizeof(data_path), "%s/%04u%02u%02u.dat", data_dir, tms.tm_year + 1900, tms.tm_mon + 1, tms.tm_mday);

		data_file = open(data_path, data_file_oflag, PMODE);

		if (unlikely(data_file == -1)) {

			return -1;
		}
	}

	return 0;
}

std::vector<std::string> files;

int enum_callback(const char *file_name)
{
	if ((strlen(file_name) < 11) || (memcmp(file_name + strlen(file_name) - 11, "patches.dat", 11)))
		files.push_back(file_name);

	return 0;
}

int load_file(const char *file_name, void *sort_buffer, std::vector<STORAGE_SORT_ITEM> *si)
{
	FILE *f = fopen(file_name, "rb");
		
	if (f) {

		log_printf("[STORAGE] Loading file '%s'\r\n", file_name);

		size_t records_count = 0;

		size_t bytes_read;

		unsigned char state = 0;
		unsigned short bytes_handled = 0;

		unsigned char record[4096];
		unsigned short *stream_id = (unsigned short *)record;
		unsigned short *len = (unsigned short *)(record + 2);

		unsigned char *file_buffer = (unsigned char *)malloc(1024 * 1024);

		if (file_buffer == NULL) {
			log_printf("[STORAGE] Unable to allocate read buffer\r\n");
			fclose(f);
			return 0;
		}

		while ((bytes_read = fread(file_buffer, 1, sizeof(file_buffer), f)) > 0) {

			unsigned char *last_byte = file_buffer + bytes_read;

			for (unsigned char *ptr = file_buffer; ptr < last_byte; ptr++)  {

				switch (state) {
				case 0:
					record[bytes_handled] = *ptr;
					bytes_handled++;
					if (bytes_handled == 4) {
						state = 1;
						bytes_handled = 2;
					}
					break;

				case 1:
					if (*len != 0) {
						record[bytes_handled + 2] = *ptr;
						bytes_handled++;
					}


					if (bytes_handled == *len) {

						STORAGE_RECORD_HEADER *rh = (STORAGE_RECORD_HEADER *)(record + 2);

						STORAGE_STREAM *stream = (STORAGE_STREAM *)storage[*stream_id];
			
						if (stream) {

							storage_lock_stream(stream);

							if (stream->base_time < rh->t) {

								if (stream->len + rh->size >= stream->capacity - 2) {
									storage_sort_stream(stream, sort_buffer, si);
									storage_drop_first_records(stream, 1000);
								}

								unsigned char *dst_ptr = stream->buffer + stream->len;
								memcpy(dst_ptr, rh, rh->size);

								storage_update_info(stream, rh, rh->size, true);

								stream->len += rh->size;
								stream->count++;

								records_count++;
							}

							storage_unlock_stream(stream);
						}

						bytes_handled = 0;
						state = 0;
					}
				}
			} 
		}

		free(file_buffer);

		log_printf("[STORAGE] Loaded %u records\r\n", records_count);
		fclose(f);
	}

	return 0;
}

void storage_load(unsigned int t)
{
	for (size_t i = 0; i < USHRT_MAX; i++)
		if (storage[i] != NULL)
			storage[i]->base_time = t;

	scan_folder(data_dir, enum_callback);

	size_t files_count = files.size();

	#pragma omp parallel
	{
		std::vector<STORAGE_SORT_ITEM> ssi;

		unsigned char *sort_buffer = (unsigned char *)malloc(DEFAULT_STREAM_SIZE);

		#pragma omp for
		for (int i = 0; i < files_count; i++) {
			if (bContinueToWork)
				load_file(files[i].c_str(), sort_buffer, &ssi);
		}

		free(sort_buffer);
	}

	if (bContinueToWork == 0)
		return;

	STORAGE_STREAM **pStreams = (STORAGE_STREAM **)malloc(USHRT_MAX * sizeof(STORAGE_STREAM *));

	size_t count = 0;

	for (size_t i = 0; i < USHRT_MAX; i++) {
		if (storage[i] != NULL) {
			pStreams[count] = storage[i];
			count++;
		}
	}

	#pragma omp parallel
	{
		std::vector<STORAGE_SORT_ITEM> ssi;

		unsigned char *sort_buffer = (unsigned char *)malloc(DEFAULT_STREAM_SIZE);

		#pragma omp for
		for (int i = 0; i < count; i++) {
			if (bContinueToWork)
				storage_sort_stream(pStreams[i], sort_buffer, &ssi);
		}

		free(sort_buffer);
	}

	free(pStreams);

	for (size_t i = 0; i < USHRT_MAX; i++) {
		if (storage[i] != NULL) {
			storage[i]->info.online = 0;
		}
	}

	if (patch != NULL) {

		fseek(patch, 0, SEEK_SET);

		PATCH_RECORD pr;

		while (fread(&pr, 1, sizeof(pr), patch) > 0) {

			void *stream = storage_get_stream_by_id(pr.id);
			if (stream != NULL) {

				STORAGE_RECORD_HEADER *rh = storage_get_stream_first_record(stream);

				if (rh != NULL) {

					size_t records_count = storage_get_stream_records_count(stream);
	
					for (size_t i = 0; i < records_count; i++) {

						if (rh->t == pr.t) {
							storage_update_record(0, rh, pr.field, &pr.value);
						}

						rh = (STORAGE_RECORD_HEADER *)(((unsigned char *)rh) + rh->size);
					}
				}
			}
		}
	}
}

void storage_set_retranslator(void *s, void *module, void *retranslator)
{
	STORAGE_STREAM *stream = (STORAGE_STREAM *)s;

	storage_lock_stream(stream);

	stream->retranslator_module = module;
	stream->retranslator = retranslator;

	storage_unlock_stream(stream);
}

// End
