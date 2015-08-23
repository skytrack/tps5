//******************************************************************************
//
// File Name : thread_pool.cpp
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#include <queue>
#include <vector>
#include <stdlib.h>
#include <string.h>
#include <png.h>
#include <zlib.h>

#include "api.h"
#include "thread_pool.h"
#include "response.h"
#include "distance.h"
#include "isearch.h"
#include "../core/record.h"
#include "../core/cross.h"
#include "../core/almalloc.h"
#include "pdf.h"

extern void *searcher;
extern std::string font_file_name;

static std::queue<THREAD_POOL_JOB> job_queue;
static THREAD_POOL_JOB job;

static const unsigned char not_enough_memory[] = "Not enough memory";

#define BUFFER_SIZE (32 * 1024 * 1024)

typedef struct png_write
{
	unsigned char *ptr;
	size_t bytes_left;
} PNG_WRITE;

void png_send_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
	PNG_WRITE *pw = (PNG_WRITE *)png_get_io_ptr(png_ptr); 

	if (length <= pw->bytes_left) {
		memcpy(pw->ptr, data, length);
		pw->ptr += length;		
		pw->bytes_left -= length;
	}
}

static int job_report_history_png(THREAD_POOL_JOB *job, unsigned char *buffer, size_t buffer_size, std::vector<STORAGE_SORT_ITEM> *si)
{	
	const char headers_ka[] = "HTTP/1.1 200 OK\r\n"
					"Content-Type: image/png\r\n"
					"Server: attiny2313\r\n"
					"Connection: Keep-alive\r\n"					
					"Content-Length:              \r\n\r\n";

	const char headers_close[] = "HTTP/1.1 200 OK\r\n"
					"Content-Type: image/png\r\n"
					"Server: attiny2313\r\n"
					"Connection: Close\r\n"					
					"Content-Length:              \r\n\r\n";

	static unsigned char response_empty_ka[] = "HTTP/1.1 200 OK\r\n"
					"Content-Type: image/png\r\n"
					"Server: attiny2313\r\n"
					"Connection: Keep-alive\r\n"					
					"Content-Length: 119\r\n\r\n"
					"\x89\x50\x4E\x47\x0D\x0A\x1A\x0A\x00\x00\x00\x0D\x49\x48\x44\x52\x00\x00\x00\x01\x00\x00\x00\x01\x08\x02\x00\x00\x00\x90\x77\x53\xDE\x00\x00\x00\x01\x73\x52\x47\x42\x00\xAE\xCE\x1C\xE9\x00\x00\x00\x04\x67\x41\x4D\x41\x00\x00\xB1\x8F\x0B\xFC\x61\x05\x00\x00\x00\x09\x70\x48\x59\x73\x00\x00\x0E\xC3\x00\x00\x0E\xC3\x01\xC7\x6F\xA8\x64\x00\x00\x00\x0C\x49\x44\x41\x54\x18\x57\x63\xF8\xFF\xFF\x3F\x00\x05\xFE\x02\xFE\xA7\x35\x81\x84\x00\x00\x00\x00\x49\x45\x4E\x44\xAE\x42\x60\x82";

	static unsigned char response_empty_close[] = "HTTP/1.1 200 OK\r\n"
					"Content-Type: image/png\r\n"
					"Server: attiny2313\r\n"
					"Connection: Close\r\n"					
					"Content-Length: 119\r\n\r\n"
					"\x89\x50\x4E\x47\x0D\x0A\x1A\x0A\x00\x00\x00\x0D\x49\x48\x44\x52\x00\x00\x00\x01\x00\x00\x00\x01\x08\x02\x00\x00\x00\x90\x77\x53\xDE\x00\x00\x00\x01\x73\x52\x47\x42\x00\xAE\xCE\x1C\xE9\x00\x00\x00\x04\x67\x41\x4D\x41\x00\x00\xB1\x8F\x0B\xFC\x61\x05\x00\x00\x00\x09\x70\x48\x59\x73\x00\x00\x0E\xC3\x00\x00\x0E\xC3\x01\xC7\x6F\xA8\x64\x00\x00\x00\x0C\x49\x44\x41\x54\x18\x57\x63\xF8\xFF\xFF\x3F\x00\x05\xFE\x02\xFE\xA7\x35\x81\x84\x00\x00\x00\x00\x49\x45\x4E\x44\xAE\x42\x60\x82";

	#define LINE_WIDTH 1024

	void *stream = api_storage_get_stream_by_id(job->terminal_id);

	if (stream != NULL) {

		api_storage_lock_stream(stream);

		api_storage_sort_stream(stream, buffer, si);

		STORAGE_RECORD_HEADER *rh = api_storage_get_stream_first_record(stream);

		if ((rh != NULL)&&(rh->t < job->tTo)) {

			size_t records_count = api_storage_get_stream_records_count(stream);
	
			STORAGE_RECORD_HEADER *first_rh = NULL;
			STORAGE_RECORD_HEADER *last_rh = NULL;

			size_t i;

			for (i = 0; i < records_count; i++) {

				if (rh->t >= job->tFrom) {

					first_rh = rh;

					for (; i < records_count; i++) {
						if (rh->t < job->tTo) {
							last_rh = rh;
						}
						else
							break;

						rh = (STORAGE_RECORD_HEADER *)(((unsigned char *)rh) + rh->size);
					}
					break;
				}

				rh = (STORAGE_RECORD_HEADER *)(((unsigned char *)rh) + rh->size);
			}

			if ((first_rh != NULL)&&(first_rh < last_rh)) {

				size_t data_size = (char *)last_rh - (char *)first_rh + last_rh->size;

				char last_line[LINE_WIDTH];

				size_t nLinesCount = data_size / LINE_WIDTH;

				if (data_size % LINE_WIDTH)
					nLinesCount++;

				if (nLinesCount < (buffer_size / sizeof (png_byte *))) {

					png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
					if (png_ptr == NULL) {
						api_storage_unlock_stream(stream);
						return 500;
					}
    
					png_infop info_ptr = png_create_info_struct(png_ptr);
					if (info_ptr == NULL) {
						png_destroy_write_struct(&png_ptr, &info_ptr);
						api_storage_unlock_stream(stream);
						return 500;
					}
    
					png_byte **row_pointers = (png_byte **)buffer;

					if (nLinesCount == 1) {
						row_pointers[0] = (png_byte *)first_rh;
					}
					else {

						for (size_t iLine = 0; iLine < nLinesCount - 1; iLine++)
							row_pointers[iLine] = (png_byte *)((char *)first_rh + iLine * LINE_WIDTH);

						memset(last_line, 0xCD, LINE_WIDTH);
						memcpy(last_line, (char *)first_rh + (nLinesCount - 1) * LINE_WIDTH, data_size - (nLinesCount - 1) * LINE_WIDTH);
						row_pointers[nLinesCount - 1] = (png_byte *)last_line;
					}
		
					PNG_WRITE pw;
					png_set_write_fn(png_ptr, &pw, png_send_data, NULL);

					unsigned char *response = buffer + nLinesCount * sizeof(png_byte *);

					pw.bytes_left = buffer_size - (response - buffer);

					if (job->session->zero_init.keep_alive) {

						if (pw.bytes_left >= (sizeof(headers_ka) - 1)) {

							memcpy(response, headers_ka, sizeof(headers_ka) - 1);
							pw.ptr = response + sizeof(headers_ka) - 1;
							pw.bytes_left -= (sizeof(headers_ka) - 1);
						}
						else {
							png_destroy_write_struct(&png_ptr, &info_ptr);
							api_storage_unlock_stream(stream);
							return 500;
						}
					}
					else {

						if (pw.bytes_left >= (sizeof(headers_close) - 1)) {

							memcpy(response, headers_close, sizeof(headers_close) - 1);
							pw.ptr = response + sizeof(headers_close) - 1;
							pw.bytes_left -= (sizeof(headers_close) - 1);
						}
						else {
							png_destroy_write_struct(&png_ptr, &info_ptr);
							api_storage_unlock_stream(stream);
							return 500;
						}
					}

					unsigned char *content_length_ptr = pw.ptr - 4;

					/* Set up error handling. */
					if (setjmp(png_jmpbuf(png_ptr))) {
						png_destroy_write_struct(&png_ptr, &info_ptr);
						api_storage_unlock_stream(stream);
						return 500;
					}

					/* Set image attributes. */
					png_set_IHDR (png_ptr, info_ptr, (nLinesCount == 1) ? data_size : LINE_WIDTH, nLinesCount, 8, PNG_COLOR_TYPE_GRAY, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
       
					png_set_rows(png_ptr, info_ptr, row_pointers);

					png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
				
					api_storage_unlock_stream(stream);

					png_destroy_write_struct(&png_ptr, &info_ptr);

					size_t content_length = pw.ptr - (content_length_ptr + 4);
					do {
						*--content_length_ptr = '0' + content_length % 10;
						content_length /= 10;
					} while (content_length != 0);

					api_send_tcp(job->session, response, pw.ptr - response);

					return 0;
				}
			}
		}

		api_storage_unlock_stream(stream);
	}

	if (job->session->zero_init.keep_alive > 0)
		api_send_tcp(job->session, response_empty_ka, sizeof(response_empty_ka) - 1);
	else
		api_send_tcp(job->session, response_empty_close, sizeof(response_empty_close) - 1);

	return 0;
}

static int job_report_history_bin(THREAD_POOL_JOB *job, unsigned char *buffer, size_t buffer_size, std::vector<STORAGE_SORT_ITEM> *si)
{	
	const char headers_ka[] = "HTTP/1.1 200 OK\r\n"
					"Content-Type: application/octet-stream\r\n"
					"Server: attiny2313\r\n"
					"Connection: Keep-alive\r\n"	
					"Content-Encoding: gzip\r\n"
					"Content-Length:              \r\n\r\n"
					"\37\213\10\0\0\0\0\0\0\377";

	const char headers_close[] = "HTTP/1.1 200 OK\r\n"
					"Content-Type: application/octet-stream\r\n"
					"Server: attiny2313\r\n"
					"Connection: Close\r\n"	
					"Content-Encoding: gzip\r\n"
					"Content-Length:              \r\n\r\n"
					"\37\213\10\0\0\0\0\0\0\377";

	static unsigned char response_empty_ka[] = "HTTP/1.1 200 OK\r\n"
					"Content-Type: application/octet-stream\r\n"
					"Server: attiny2313\r\n"
					"Connection: Keep-alive\r\n"					
					"Content-Length: 0\r\n\r\n";

	static unsigned char response_empty_close[] = "HTTP/1.1 200 OK\r\n"
					"Content-Type: application/octet-stream\r\n"
					"Server: attiny2313\r\n"
					"Connection: Close\r\n"					
					"Content-Length: 0\r\n\r\n";

	#define LINE_WIDTH 1024

	void *stream = api_storage_get_stream_by_id(job->terminal_id);

	if (stream != NULL) {

		api_storage_lock_stream(stream);

		api_storage_sort_stream(stream, buffer, si);

		STORAGE_RECORD_HEADER *rh = api_storage_get_stream_first_record(stream);

		if ((rh != NULL)&&(rh->t < job->tTo)) {

			size_t records_count = api_storage_get_stream_records_count(stream);
	
			STORAGE_RECORD_HEADER *first_rh = NULL;
			STORAGE_RECORD_HEADER *last_rh = NULL;

			size_t i;

			for (i = 0; i < records_count; i++) {

				if (rh->t >= job->tFrom) {

					first_rh = rh;

					for (; i < records_count; i++) {
						if (rh->t < job->tTo) {
							last_rh = rh;
						}
						else
							break;

						rh = (STORAGE_RECORD_HEADER *)(((unsigned char *)rh) + rh->size);
					}
					break;
				}

				rh = (STORAGE_RECORD_HEADER *)(((unsigned char *)rh) + rh->size);
			}

			if ((first_rh != NULL)&&(first_rh < last_rh)) {

				size_t data_size = (char *)last_rh - (char *)first_rh + last_rh->size;
				size_t bytes_left = buffer_size;
				unsigned char *ptr = buffer;

				if (job->session->zero_init.keep_alive) {

					if (bytes_left >= (sizeof(headers_ka) - 1)) {

						memcpy(ptr, headers_ka, sizeof(headers_ka) - 1);
						ptr += sizeof(headers_ka) - 1;
						bytes_left -= (sizeof(headers_ka) - 1);
					}
					else {
						return 500;
					}
				}
				else {

					if (bytes_left >= (sizeof(headers_close) - 1)) {

						memcpy(ptr, headers_close, sizeof(headers_close) - 1);
						ptr += sizeof(headers_close) - 1;
						bytes_left -= (sizeof(headers_close) - 1);
					}
					else {
						return 500;
					}
				}

				unsigned char *content_length_ptr = ptr - 14;
				unsigned char *gzip_begin = ptr;

				z_stream zs;

				zs.zalloc    = (alloc_func)0;
				zs.zfree     = (free_func)0;
				zs.opaque    = (voidpf)0;

				zs.next_in   = (Byte*)first_rh;
				zs.avail_in  = data_size;

				zs.next_out  = (Byte*)gzip_begin;
				zs.avail_out = bytes_left;

				if (deflateInit2(&zs, 9, Z_DEFLATED, -MAX_WBITS, 8, Z_DEFAULT_STRATEGY) != Z_OK) {
					api_log_printf("[HTTP] deflateInit2 error\r\n");
					deflateEnd(&zs);
					return 500;
				}

				if (deflate(&zs, Z_FINISH) != Z_STREAM_END) {
					api_log_printf("[HTTP] deflate error\r\n");
					deflateEnd(&zs);
					return 500;
				}

				deflateEnd(&zs);

				ptr += zs.total_out;

				bytes_left -= zs.total_out;

				if (bytes_left < 8)
					return 500;

				*(unsigned int *)ptr = crc32(crc32(0, NULL, 0), (Byte*)first_rh, data_size);
				ptr += 4;
				*(unsigned int *)ptr = data_size;
				ptr += 4;

				api_storage_unlock_stream(stream);

				size_t content_length = ptr - gzip_begin + 10;

				do {
					*--content_length_ptr = '0' + content_length % 10;
					content_length /= 10;
				} while (content_length != 0);

				api_send_tcp(job->session, buffer, ptr - buffer);

				return 0;
			}
		}

		api_storage_unlock_stream(stream);
	}

	if (job->session->zero_init.keep_alive > 0)
		api_send_tcp(job->session, response_empty_ka, sizeof(response_empty_ka) - 1);
	else
		api_send_tcp(job->session, response_empty_close, sizeof(response_empty_close) - 1);

	return 0;
}

static int job_report_mileage(THREAD_POOL_JOB *job, unsigned char *buffer, size_t buffer_size, std::vector<STORAGE_SORT_ITEM> *si)
{
	void *stream = api_storage_get_stream_by_id(job->terminal_id);

	if (stream == NULL) {

		if (job->session->zero_init.keep_alive > 0)
			api_send_tcp(job->session, response_success_empty_array_ka, response_success_empty_array_ka_length);
		else
			api_send_tcp(job->session, response_success_empty_array_close, response_success_empty_array_close_length);

		return 0;
	}

	api_storage_lock_stream(stream);
		
	api_storage_sort_stream(stream, buffer, si);

	size_t records_count	= api_storage_get_stream_records_count(stream);
	unsigned char *response = buffer;
	float *pInitialLat		= (float *)(buffer + buffer_size / 4);
	float *pInitialLng		= (float *)(buffer + buffer_size / 2);
	size_t bytes_left		= buffer_size / 4;

	size_t content_length;
	unsigned char *content_length_ptr;

	unsigned char *ptr = response_success_object(response, &bytes_left, job->session->zero_init.keep_alive > 0, &content_length_ptr, &content_length);

	if (ptr == NULL) {
		api_storage_unlock_stream(stream);
		return 500;
	}
		
	unsigned char *object_start = ptr;

	if (bytes_left < 1) {
		api_storage_unlock_stream(stream);
		return 500;
	}

	*ptr++ = '[';
	bytes_left--;

	STORAGE_RECORD_HEADER *rh = api_storage_get_stream_first_record(stream);

	// ≈сли поток не пуст
	if (rh != NULL) {
			
		unsigned int ignition_flag	= 0;
		unsigned int engine_flag	= 0;
		unsigned int move_flag		= 0;

		if (job->ignition) {
			ignition_flag = RECORD_FLAG1_IGNITION;
		}
		else
		if ((job->ignition_source > 0)&&(job->ignition_source <= 5)) {
			ignition_flag = ((0x80 >> (job->ignition_source - 1))) << 8;
		}

		if (job->engine) {
			engine_flag = RECORD_FLAG1_ENGINE;
		}
		else
		if ((job->engine_source <= 5)&&(job->engine_source <= 5)) {
			engine_flag = ((0x80 >> (job->engine_source - 1))) << 8;
		}

		if (job->move) {
			move_flag = RECORD_FLAG1_MOVE;
		}
		else
		if ((job->move_source >= 0)&&(job->move_source <= 5)) {
			move_flag = ((0x80 >> (job->move_source - 1))) << 8;
		}

		if (move_flag == 0)
			move_flag = engine_flag;

		if (move_flag == 0)
			move_flag = ignition_flag;

		// Ќахожу в потоке первую точку больше или равной t_from
		size_t i;
		for (i = 0; i < records_count; i++) {

			if (rh->t >= job->tFrom)
				break;
			
			rh = (STORAGE_RECORD_HEADER *)(((unsigned char *)rh) + rh->size);
		}

		// ≈сли така€ точка найдена и она меньше tTo - есть данные внутри интервала запроса
		if ((i < records_count)&&(rh->t < job->tTo)) {

			struct tm tms;
			time_t t = job->tFrom;
			localtime_r(&t, &tms);

			tms.tm_min = 00;
			tms.tm_sec = 00;

			unsigned int first_hour = mktime(&tms);

			t = job->tTo;
			localtime_r(&t, &tms);

			tms.tm_min = 00;
			tms.tm_sec = 00;

			unsigned int last_hour = mktime(&tms);

			unsigned int this_hour = first_hour;

			float *pLat = pInitialLat;
			float *pLng = pInitialLng;

			float *pMaxLat = pLng;

			while (this_hour <= job->tTo) {

				unsigned int this_hour_end = this_hour + 3599;

				if (this_hour_end > job->tTo)
					this_hour_end = job->tTo;

				if (rh->t > this_hour_end) {
					this_hour += 3600;
					continue;
				}

				for (; i < records_count; i++) {

					unsigned char *bits = (unsigned char *)rh + sizeof(STORAGE_RECORD_HEADER);
					unsigned char *ptr = bits;

					while (*ptr & RECORD_BIT_MORE)
						ptr++;

					ptr++;

					if (bits[0] & RECORD_BIT1_EVENT)
						ptr += 2;

					unsigned short flags = 0;

					if (bits[0] & RECORD_BIT1_FLAGS) {
						flags = *ptr;
						if (flags & RECORD_FLAG1_MORE) {
							
							ptr++;
							flags |= (*ptr) << 8;

							while (*ptr & RECORD_FLAG1_MORE)
								ptr++;
						}
						ptr++;
					}

					if (bits[0] & RECORD_BIT1_NAV) {

						if ((*(ptr + 8) != 0xFF)||((flags & 0x0E) != 0x0E)) {

							if ((pLat == pInitialLat)||(move_flag == 0)||((flags & move_flag) > 0)) {
								int iLat = *(int *)ptr; ptr += 4;
								int iLng = *(int *)ptr;
									
								*pLat++ = iLat / 10000000.0f;
								*pLng++ = iLng / 10000000.0f;

								if (pLat == pMaxLat)
									break;

								if (rh->t >= this_hour_end) {
									printf("%u\r\n", rh->t);
									fflush(stdout);
									break;
								}
							}
						}
					}

					rh = (STORAGE_RECORD_HEADER *)(((unsigned char *)rh) + rh->size);
				}

				size_t points_count = pLat - pInitialLat;

				if (points_count >= 2) {
					
					float distance = GetDistanceST(pInitialLat, pInitialLng, points_count);

					if (distance > 0) {

						if (bytes_left < 46) {
							api_storage_unlock_stream(stream);
							return 500;
						}

						memcpy(ptr, "{\"hour\":             ", 20);
						ptr += 20;
						bytes_left -= 20;

						unsigned char *num_ptr = ptr;

						unsigned int hour = (this_hour > job->tFrom) ? this_hour : job->tFrom;
						do {
							*--num_ptr = '0' + hour % 10;
							hour /= 10;
						} while (hour != 0);

						memcpy(ptr, ",\"mileage\":             },", 26);
						ptr += 26;
						bytes_left -= 46;

						num_ptr = ptr - 2;

						unsigned int mileage = (unsigned int)distance;
						do {
							*--num_ptr = '0' + mileage % 10;
							mileage /= 10;
						} while (mileage != 0);
					}

					pLat = pInitialLat;
					pLng = pInitialLng;
				}

				this_hour += 3600;
			}
		}
	}
					
	api_storage_unlock_stream(stream);

	if (*(ptr - 1) == ',') {
		ptr--;
		bytes_left++;
	}

	if (bytes_left < 2) {
		api_storage_unlock_stream(stream);
		return 500;
	}

	*ptr++ = ']';
	*ptr++ = '}';

	content_length += ptr - object_start;

	do {
		*--content_length_ptr = '0' + content_length % 10;
		content_length /= 10;
	} while (content_length != 0);

	api_send_tcp(job->session, response, ptr - response);

	return 0;
}

static int job_report_flag(THREAD_POOL_JOB *job, unsigned char *buffer, size_t buffer_size, std::vector<STORAGE_SORT_ITEM> *si, unsigned int flag)
{
	void *stream = api_storage_get_stream_by_id(job->terminal_id);

	if (stream != NULL) {

		api_storage_lock_stream(stream);
		
		api_storage_sort_stream(stream, buffer, si);

		size_t records_count = api_storage_get_stream_records_count(stream);

		STORAGE_RECORD_HEADER *rh = api_storage_get_stream_first_record(stream);

		// ≈сли поток не пуст
		if ((rh != NULL)&&(rh->t <= job->tTo)) {

			unsigned char *content_length_ptr;
			size_t content_length;
			size_t bytes_left					= buffer_size / 4;
			unsigned char *response				= buffer;
			float *pInitialLat					= (float *)(buffer + buffer_size / 4);
			float *pInitialLng					= (float *)(buffer + buffer_size / 2);
			float *pMaxLat						= pInitialLng;

			unsigned char *ptr = response_success_object(response, &bytes_left, job->session->zero_init.keep_alive > 0, &content_length_ptr, &content_length);

			if (ptr == NULL) {
				api_storage_unlock_stream(stream);
				return 500;
			}
		
			unsigned char *object_start = ptr;
			unsigned short prev_bit = USHRT_MAX;

			*ptr++ = '[';

			// Ќахожу в потоке первую точку больше или равной t_from
			size_t i;
			for (i = 0; i < records_count; i++) {

				if (rh->t >= job->tFrom)
					break;

				unsigned char *bits = (unsigned char *)rh + sizeof(STORAGE_RECORD_HEADER);

				unsigned char *p = bits;

				while (*p & RECORD_BIT_MORE)
					p++;

				p++;

				if (*bits & RECORD_BIT1_EVENT)
					p += 2;

				unsigned short flags = 0;

				if (*bits & RECORD_BIT1_FLAGS) {				

					flags = *p;

					if (flags & RECORD_FLAG1_MORE)
						flags |= (*(p + 1) << 8);

					while (*p & RECORD_FLAG1_MORE)
						p++;

					p++;
				}

				if (*bits & RECORD_BIT1_FLAGS)
					prev_bit = flags & flag;
						
				rh = (STORAGE_RECORD_HEADER *)(((unsigned char *)rh) + rh->size);
			}

			float *pLat = pInitialLat;
			float *pLng = pInitialLng;
			int lat = 0;
			int lng = 0;

			for (; i < records_count; i++) {

				if (rh->t > job->tTo)
					break;

				unsigned char *bits = (unsigned char *)rh + sizeof(STORAGE_RECORD_HEADER);

				unsigned char *p = bits;

				while (*p & RECORD_BIT_MORE)
					p++;

				p++;

				if (*bits & RECORD_BIT1_EVENT)
					p += 2;

				unsigned short flags = 0;

				if (*bits & RECORD_BIT1_FLAGS) {				

					flags = *p;

					if (flags & RECORD_FLAG1_MORE)
						flags |= (*(p + 1) << 8);

					while (*p & RECORD_FLAG1_MORE)
						p++;

					p++;
				}

				if (*bits & RECORD_BIT1_NAV) {

					if ((*(p + 8) != 0xFF)||((flags & 0x0E) != 0x0E)) {
				
						lat = *(int *)p;
						p += 4;
						lng = *(int *)p;

						if (pLat != pMaxLat) {
							*pLat++ = lat / 10000000.0f;
							*pLng++ = lng / 10000000.0f;
						}
					}
				}

				if (*bits & RECORD_BIT1_FLAGS) {				

					if ((flags & flag) != prev_bit) {

						prev_bit = flags & flag;

						size_t points_count = pLat - pInitialLat;

						float distance = 0;

						if (points_count >= 2)
							distance = GetDistanceST(pInitialLat, pInitialLng, points_count);

						pLat = pInitialLat;
						pLng = pInitialLng;

						if (*bits & RECORD_BIT1_NAV) {

							*pLat++ = lat / 10000000.0f;
							*pLng++ = lng / 10000000.0f;

							for (;;) {

#ifndef _MSC_VER
								if (searcher != NULL) {
	
									const char *searchResult = searchNear(searcher, *(pLng - 1), *(pLat - 1), 10000);

									if (searchResult != NULL) {

										ptr += sprintf((char *)ptr, "{\"time\":%u,\"status\":%s,\"lat\":%d,\"lng\":%d,\"distance\":%u, \"address\":%s},", rh->t, prev_bit ? "true" : "false", lat, lng, (unsigned int)distance, searchResult);

										break;
									}
								}

#endif
								ptr += sprintf((char *)ptr, "{\"time\":%u,\"status\":%s,\"lat\":%d,\"lng\":%d,\"distance\":%u},", rh->t, prev_bit ? "true" : "false", lat, lng, (unsigned int)distance);

								break;
							}
						}
						else {
							ptr += sprintf((char *)ptr, "{\"time\":%u,\"status\":%s,\"distance\":%u},", rh->t, prev_bit ? "true" : "false", (unsigned int)distance);
						}
					}
				}

				rh = (STORAGE_RECORD_HEADER *)(((unsigned char *)rh) + rh->size);
			}

			api_storage_unlock_stream(stream);

			if (*(ptr - 1) == ',')
				ptr--;

			*ptr++ = ']';
			*ptr++ = '}';

			content_length += ptr - object_start;

			do {
				*--content_length_ptr = '0' + content_length % 10;
				content_length /= 10;
			} while (content_length != 0);

			api_send_tcp(job->session, response, ptr - response);

			return 0;
		}

		api_storage_unlock_stream(stream);
	}

	if (job->session->zero_init.keep_alive > 0)
		api_send_tcp(job->session, response_success_empty_array_ka, response_success_empty_array_ka_length);
	else
		api_send_tcp(job->session, response_success_empty_array_close, response_success_empty_array_close_length);

	return 0;
}

static int job_report_park_with_engine(THREAD_POOL_JOB *job, unsigned char *buffer, size_t buffer_size, std::vector<STORAGE_SORT_ITEM> *si)
{
	void *stream = api_storage_get_stream_by_id(job->terminal_id);

	if (stream != NULL) {

		api_storage_lock_stream(stream);
		
		api_storage_sort_stream(stream, buffer, si);

		size_t records_count = api_storage_get_stream_records_count(stream);

		STORAGE_RECORD_HEADER *rh = api_storage_get_stream_first_record(stream);

		unsigned int ignition_flag	= 0;
		unsigned int engine_flag	= 0;
		unsigned int move_flag		= 0;

		if (job->ignition) {
			ignition_flag = RECORD_FLAG1_IGNITION;
		}
		else
		if (job->ignition_source <= 5) {
			ignition_flag = ((0x80 >> (job->ignition_source - 1))) << 8;
		}

		if (job->engine) {
			engine_flag = RECORD_FLAG1_ENGINE;
		}
		else
		if (job->engine_source <= 5) {
			engine_flag = ((0x80 >> (job->engine_source - 1))) << 8;
		}

		if (job->move) {
			move_flag = RECORD_FLAG1_MOVE;
		}
		else
		if (job->engine_source <= 5) {
			move_flag = ((0x80 >> (job->move_source - 1))) << 8;
		}

		if (engine_flag == 0)
			engine_flag = ignition_flag;

		if ((engine_flag != 0) && (move_flag != 0)) {

			bool prev_engine	= false;
			bool prev_move		= false;

			// ≈сли поток не пуст
			if ((rh != NULL)&&(rh->t <= job->tTo)) {
			
				unsigned char *content_length_ptr;
				size_t content_length;
				size_t bytes_left		= buffer_size;
				unsigned char *response = buffer;

				unsigned char *ptr = response_success_object(response, &bytes_left, job->session->zero_init.keep_alive > 0, &content_length_ptr, &content_length);

				if (ptr == NULL) {
					api_storage_unlock_stream(stream);
					return 500;
				}
		
				unsigned char *object_start = ptr;

				*ptr++ = '[';

				bool prev_park_with_engine = false;

				// Ќахожу в потоке первую точку больше или равной t_from
				size_t i;
				for (i = 0; i < records_count; i++) {

					if (rh->t >= job->tFrom)
						break;

					unsigned char *bits = (unsigned char *)rh + sizeof(STORAGE_RECORD_HEADER);

					unsigned char *p = bits;

					while (*p & RECORD_BIT_MORE)
						p++;

					p++;

					if (*bits & RECORD_BIT1_EVENT)
						p += 2;

					unsigned short flags = 0;

					if (*bits & RECORD_BIT1_FLAGS) {				

						flags = *p;

						if (flags & RECORD_FLAG1_MORE)
							flags |= (*(p + 1) << 8);

						prev_park_with_engine = (((flags & engine_flag) > 0)&&((flags & move_flag) == 0));
					}

					rh = (STORAGE_RECORD_HEADER *)(((unsigned char *)rh) + rh->size);
				}

				bool park_with_engine = prev_park_with_engine;

				int park_start;
				bool nav_exists = false;
				int lat;
				int lng;

				if (park_with_engine)
					park_start = job->tFrom;

				for (; i < records_count; i++) {

					if (rh->t > job->tTo)
						break;

					unsigned char *bits = (unsigned char *)rh + sizeof(STORAGE_RECORD_HEADER);

					unsigned char *p = bits;

					while (*p & RECORD_BIT_MORE)
						p++;

					p++;

					if (*bits & RECORD_BIT1_EVENT)
						p += 2;

					unsigned short flags = 0;

					if (*bits & RECORD_BIT1_FLAGS) {				

						flags = *p;

						if (flags & RECORD_FLAG1_MORE)
							flags |= (*(p + 1) << 8);

						while (*p & RECORD_FLAG1_MORE)
							p++;
						p++;

						park_with_engine = ((flags & engine_flag) > 0)&&((flags & move_flag) == 0);

						if ((prev_park_with_engine == false)&&(park_with_engine == true)) {
							nav_exists = false;
						}
					}

					if ((*bits & RECORD_BIT1_NAV)&&(park_with_engine)&&(nav_exists == false)) {

						if ((*(p + 8) != 0xFF)||((flags & 0x0E) != 0x0E)) {
							lat = *(int *)p;
							p += 4;
							lng = *(int *)p;

							nav_exists = true;
						}
					}

					if (*bits & RECORD_BIT1_FLAGS) {				

						if (park_with_engine != prev_park_with_engine) {

							if (park_with_engine) {
								park_start = rh->t;
							}
							else {

								if (nav_exists) {

									for (;;) {

#ifndef _MSC_VER
										if (searcher != NULL) {
	
											const char *searchResult = searchNear(searcher, lng / 10000000.0f, lat / 10000000.0f, 10000);

											if (searchResult != NULL) {

												ptr += sprintf((char *)ptr, "{\"from\":%u,\"to\":%u,\"lat\":%d,\"lng\":%d,\"address\":%s},", park_start, rh->t, lat, lng, searchResult);

												break;
											}
										}
#endif
										ptr += sprintf((char *)ptr, "{\"from\":%u,\"to\":%u,\"lat\":%d,\"lng\":%d},", park_start, rh->t, lat, lng);

										break;
									}
								}
								else {
									ptr += sprintf((char *)ptr, "{\"from\":%u,\"to\":%u},", park_start, rh->t);
								}
							}
							prev_park_with_engine = park_with_engine;
						}
					}

					rh = (STORAGE_RECORD_HEADER *)(((unsigned char *)rh) + rh->size);
				}

				api_storage_unlock_stream(stream);

				if (park_with_engine) {

					int now = (int)time(NULL);

					int tTo = (job->tTo > now) ? now : job->tTo;

					if (nav_exists) {

						for (;;) {

#ifndef _MSC_VER
							if (searcher != NULL) {

								const char *searchResult = searchNear(searcher, lng / 10000000.0f, lat / 10000000.0f, 10000);

								if (searchResult != NULL) {

									ptr += sprintf((char *)ptr, "{\"from\":%u,\"to\":%u,\"lat\":%d,\"lng\":%d,\"address\":%s},", park_start, tTo, lat, lng, searchResult);

									break;
								}
							}
#endif
							ptr += sprintf((char *)ptr, "{\"from\":%u,\"to\":%u,\"lat\":%d,\"lng\":%d},", park_start, tTo, lat, lng);

							break;
						}
					}
					else {
						ptr += sprintf((char *)ptr, "{\"from\":%u,\"to\":%u},", park_start, tTo);
					}
				}

				if (*(ptr - 1) == ',')
					ptr--;

				*ptr++ = ']';
				*ptr++ = '}';

				content_length += ptr - object_start;

				do {
					*--content_length_ptr = '0' + content_length % 10;
					content_length /= 10;
				} while (content_length != 0);

				api_send_tcp(job->session, response, ptr - response);

				return 0;
			}
		}

		api_storage_unlock_stream(stream);
	}					
	
	if (job->session->zero_init.keep_alive > 0)
		api_send_tcp(job->session, response_success_empty_array_ka, response_success_empty_array_ka_length);
	else
		api_send_tcp(job->session, response_success_empty_array_close, response_success_empty_array_close_length);

	return 0;
}

static int job_report_fuel(THREAD_POOL_JOB *job, unsigned char *buffer, size_t buffer_size, std::vector<STORAGE_SORT_ITEM> *si)
{
	void *stream = api_storage_get_stream_by_id(job->terminal_id);

	if (stream != NULL) {

		api_storage_lock_stream(stream);
		
		api_storage_sort_stream(stream, buffer, si);

		STORAGE_RECORD_HEADER *rh = api_storage_get_stream_first_record(stream);

		// ≈сли поток не пуст
		if ((rh != NULL)&&(rh->t <= job->tTo)) {

			if (job->lls_table_left.size() >= 2) {

				FUEL_TABLE_RECORD *prev_ftr = &job->lls_table_left[job->lls_table_left.size() - 1];

				for (size_t i = job->lls_table_left.size() - 1; i--;) {
					FUEL_TABLE_RECORD *ftr = &job->lls_table_left[i];
					ftr->d = (prev_ftr->fuel_value - ftr->fuel_value) / (prev_ftr->sensor_value - ftr->sensor_value);
					prev_ftr = ftr;
				}

				job->lls_table_left[job->lls_table_left.size() - 1].d = job->lls_table_left[job->lls_table_left.size() - 2].d;
			}

			if (job->lls_table_right.size() >= 2) {

				FUEL_TABLE_RECORD *prev_ftr = &job->lls_table_right[job->lls_table_right.size() - 1];

				for (size_t i = job->lls_table_right.size() - 1; i--;) {
					FUEL_TABLE_RECORD *ftr = &job->lls_table_right[i];
					ftr->d = (prev_ftr->fuel_value - ftr->fuel_value) / (prev_ftr->sensor_value - ftr->sensor_value);
					prev_ftr = ftr;
				}

				job->lls_table_right[job->lls_table_right.size() - 1].d = job->lls_table_right[job->lls_table_right.size() - 2].d;
			}

			unsigned int ignition_flag	= 0;
			unsigned int engine_flag	= 0;
			unsigned int move_flag		= 0;

			if (job->ignition) {
				ignition_flag = RECORD_FLAG1_IGNITION;
			}
			else
			if (job->ignition_source <= 5) {
				ignition_flag = ((0x80 >> (job->ignition_source - 1))) << 8;
			}

			if (job->engine) {
				engine_flag = RECORD_FLAG1_ENGINE;
			}
			else
			if (job->engine_source <= 5) {
				engine_flag = ((0x80 >> (job->engine_source - 1))) << 8;
			}

			if (job->move) {
				move_flag = RECORD_FLAG1_MOVE;
			}
			else
			if (job->engine_source <= 5) {
				move_flag = ((0x80 >> (job->move_source - 1))) << 8;
			}

			if (engine_flag == 0)
				engine_flag = ignition_flag;

			size_t records_count = api_storage_get_stream_records_count(stream);
		
			STORAGE_RECORD_HEADER *rh_analys_start = rh;
			int analys_index = 0;

			float *pInitLat				= (float *)(buffer + buffer_size / 8);
			float *pInitLng				= (float *)(buffer + (buffer_size / 8) * 2);
			float *pInitFuel			= (float *)(buffer + (buffer_size / 8) * 3);
			int *pInitTime				= (int *)(buffer + (buffer_size / 8) * 4);
			unsigned short *pInitSpeed	= (unsigned short *)(buffer + (buffer_size / 8) * 5);
			float *pInitDistanceLat		= (float *)(buffer + (buffer_size / 8) * 6);
			float *pInitDistanceLng		= (float *)(buffer + (buffer_size / 8) * 7);

			float *pLat				= pInitLat;
			float *pLng				= pInitLng;
			float *pFuel			= pInitFuel;
			int *pTime				= pInitTime;
			unsigned short *pSpeed	= pInitSpeed;
			float *pMaxLat			= pLng;


			float *pDistanceLat		= pInitDistanceLat;
			float *pDistanceLng		= pInitDistanceLng;
			float *pMaxDistanceLat	= pDistanceLng;
			bool distance_done		= false;

			float fuel_at_start		= -1;
			float fuel_at_end		= -1;
			float prevFuel			= -1;

			size_t i, k;
			
			unsigned short prev_flags		= 0;
			unsigned int moto_seconds		= 0;
			unsigned int moto_start			= 0;
			STORAGE_RECORD_HEADER *prev_rh	= NULL;

			uint64_t injector = 0;

			// Ќахожу в потоке первую точку больше или равной t_from
			for (size_t i = 0; i < records_count; i++) {

				if (i > 1000) {
					rh_analys_start = (STORAGE_RECORD_HEADER *)(((unsigned char *)rh_analys_start) + rh_analys_start->size);
					analys_index = i - 1000;
				}

				if (rh->t >= job->tFrom)
					break;
			
				unsigned char *bits = (unsigned char *)rh + sizeof(STORAGE_RECORD_HEADER);

				if (*bits & RECORD_BIT1_FLAGS)
					prev_rh = rh;

				rh = (STORAGE_RECORD_HEADER *)(((unsigned char *)rh) + rh->size);
			}

			if (prev_rh != NULL) {

				unsigned char *bits = (unsigned char *)prev_rh + sizeof(STORAGE_RECORD_HEADER);

				unsigned char *p = bits;

				while (*p & RECORD_BIT_MORE)
					p++;

				p++;

				if (*bits & RECORD_BIT1_EVENT)
					p += 2;

				unsigned short flags = 0;

				flags = *p;

				if (flags & RECORD_FLAG1_MORE)
					flags |= (*(p + 1) << 8);

				prev_flags = flags & engine_flag;

				if (prev_flags > 0)
					moto_start = job->tFrom;
			}

			for (rh = rh_analys_start, k = 0, i = analys_index; (i < records_count)&&(k < 1000); rh = (STORAGE_RECORD_HEADER *)(((unsigned char *)rh) + rh->size), i++) {

				if (pLat == pMaxLat) {
					api_storage_unlock_stream(stream);
					return 500;
				}

				if (rh->t >= job->tTo)
					k++;

				unsigned short speed = 0;

				float lls_left = -1, lls_right = -1;
				float lat = 0, lng = 0;

				unsigned short A[8] = { USHRT_MAX, USHRT_MAX, USHRT_MAX, USHRT_MAX, USHRT_MAX, USHRT_MAX, USHRT_MAX, USHRT_MAX };
				unsigned short F[8] = { USHRT_MAX, USHRT_MAX, USHRT_MAX, USHRT_MAX, USHRT_MAX, USHRT_MAX, USHRT_MAX, USHRT_MAX };
				unsigned short RS485[2] = { USHRT_MAX, USHRT_MAX };
				unsigned short RS232[2] = { USHRT_MAX, USHRT_MAX };

				unsigned char *bits = (unsigned char *)rh + sizeof(STORAGE_RECORD_HEADER);

				unsigned char *p = bits;

				while (*p & RECORD_BIT_MORE)
					p++;

				p++;

				if (*bits & RECORD_BIT1_EVENT)
					p += 2;

				unsigned short flags = 0;

				if (*bits & RECORD_BIT1_FLAGS) {				

					flags = *p;

					if (flags & RECORD_FLAG1_MORE)
						flags |= (*(p + 1) << 8);

					while (*p & RECORD_FLAG1_MORE)
						p++;

					if (job->ignition) {
						if (flags & RECORD_FLAG1_IGNITION) {
							speed |= 0x8000;
						}
					}
					else 
					if ((job->ignition_source <= 5)&&((flags >> 8) & (0x80 >> (job->ignition_source - 1)))) {
						speed |= 0x8000;
					}
					
					if (job->move) {
						if (flags & RECORD_FLAG1_MOVE) {
							speed |= 0x4000;
						}
					}
					else 
					if ((job->move_source <= 5)&&((flags >> 8) & (0x80 >> (job->move_source - 1)))) {
						speed |= 0x4000;
					}

					p++;

					if ((rh->t >= job->tFrom)&&(rh->t <= job->tTo)) {
						if ((flags & engine_flag) != prev_flags) {

							if (prev_flags > 0) {
								moto_seconds += rh->t - moto_start;
							}
							else {
								moto_start = rh->t;
							}

							prev_flags = flags & engine_flag;
						}
					}
				}

				if (*bits & RECORD_BIT1_NAV) {

					if ((*(p + 8) != 0xFF)||((flags & 0x0E) != 0x0E)) {

						lat = (*(int *)p) / 10000000.0f;
						p += 4;
						lng = (*(int *)p) / 10000000.0f;
						p += 4;

						speed |= *p++;

						if (flags & RECORD_FLAG1_SPEED_9)
							speed |= 0x0100;
						if (flags & RECORD_FLAG1_SPEED_10)
							speed |= 0x0200;
						if (flags & RECORD_FLAG1_SPEED_11)
							speed |= 0x0400;

						if ((rh->t >= job->tFrom)&&(distance_done == false)) {

							*pDistanceLat++ = lat;
							*pDistanceLng++ = lng;

							if (rh->t >= job->tTo) {
								distance_done = true;
							}

							if (pDistanceLat == pMaxDistanceLat) {
								api_storage_unlock_stream(stream);
								return 500;
							}
						}
					}
					else {
						p += 9;
					}
				}

				if (*bits & RECORD_BIT1_ALT)
					p += 2;

				if (*bits & RECORD_BIT1_COG)
					p++;

				if (*bits & RECORD_BIT1_RS485_1) {
					RS485[0] = *(unsigned short *)p;
					p += 2;
				}

				if (*bits & RECORD_BIT1_RS485_2) {
					RS485[1] = *(unsigned short *)p;
					p += 2;
				}

				if (*bits & RECORD_BIT_MORE) {
						
					if (bits[1] & RECORD_BIT2_ADC1) {
						A[0] = *(unsigned short *)p;
						p += 2;
					}

					if (bits[1] & RECORD_BIT2_ADC2) {
						A[1] = *(unsigned short *)p;
						p += 2;
					}

					if (bits[1] & RECORD_BIT2_ADC3) {
						A[2] = *(unsigned short *)p;
						p += 2;
					}

					if (bits[1] & RECORD_BIT2_FREQUENCY1) {
						F[0] = *(unsigned short *)p;
						p += 2;
					}

					if (bits[1] & RECORD_BIT2_FREQUENCY2) {
						F[1] = *(unsigned short *)p;
						p += 2;
					}

					if (bits[1] & RECORD_BIT2_FREQUENCY3) {
						F[2] = *(unsigned short *)p;
						p += 2;
					}

					if (bits[1] & RECORD_BIT2_FREQUENCY4) {
						F[3] = *(unsigned short *)p;
						p += 2;
					}

					if (bits[1] & RECORD_BIT_MORE) {

						if (bits[2] & RECORD_BIT3_VCC)
							p += 2;
						if (bits[2] & RECORD_BIT3_SAT_NO)
							p++;
						if (bits[2] & RECORD_BIT3_ADC4)
							p += 2;
						if (bits[2] & RECORD_BIT3_COUNTER1)
							p += 2;
						if (bits[2] & RECORD_BIT3_COUNTER2)
							p += 2;
						if (bits[2] & RECORD_BIT3_COUNTER3)
							p += 2;
						if (bits[2] & RECORD_BIT3_COUNTER4)
							p += 2;

						if (bits[2] & RECORD_BIT_MORE) {

							if (bits[3] & RECORD_BIT4_RS232_1)
								p += 2;
							if (bits[3] & RECORD_BIT4_RS232_2)
								p += 2;
							if (bits[3] & RECORD_BIT4_ODOMETER)
								p += 4;
							if (bits[3] & RECORD_BIT4_FREQUENCY5)
								p += 2;
							if (bits[3] & RECORD_BIT4_FREQUENCY6)
								p += 2;
							if (bits[3] & RECORD_BIT4_FREQUENCY7)
								p += 2;
							if (bits[3] & RECORD_BIT4_FREQUENCY8)
								p += 2;
						
							if (bits[3] & RECORD_BIT_MORE) {
								if (bits[4] & RECORD_BIT5_ADC5)
									p += 2;
								if (bits[4] & RECORD_BIT5_ADC6)
									p += 2;
								if (bits[4] & RECORD_BIT5_COUNTER5)
									p += 2;
								if (bits[4] & RECORD_BIT5_COUNTER6)
									p += 2;
								if (bits[4] & RECORD_BIT5_COUNTER7)
									p += 2;
								if (bits[4] & RECORD_BIT5_COUNTER8)
									p += 2;
								if (bits[4] & RECORD_BIT5_INJECTOR) {
									if ((rh->t >= job->tFrom)&&(distance_done == false))
										injector += (*(unsigned int *)p);
									p += 4;
								}
							}
						}
					}
				}
				
				switch (job->lls_left_source) {
				case FUEL_SOURCE_A1: if (A[0] == USHRT_MAX) break; lls_left = A[0] / 1000.0f; break;
				case FUEL_SOURCE_A2: if (A[1] == USHRT_MAX) break; lls_left = A[1] / 1000.0f; break;
				case FUEL_SOURCE_A3: if (A[2] == USHRT_MAX) break; lls_left = A[2] / 1000.0f; break;
				case FUEL_SOURCE_A4: if (A[3] == USHRT_MAX) break; lls_left = A[3] / 1000.0f; break;
				case FUEL_SOURCE_A5: if (A[4] == USHRT_MAX) break; lls_left = A[4] / 1000.0f; break;
				case FUEL_SOURCE_A6: if (A[5] == USHRT_MAX) break; lls_left = A[5] / 1000.0f; break;
				case FUEL_SOURCE_A7: if (A[6] == USHRT_MAX) break; lls_left = A[6] / 1000.0f; break;
				case FUEL_SOURCE_A8: if (A[7] == USHRT_MAX) break; lls_left = A[7] / 1000.0f; break;
				case FUEL_SOURCE_F1: if (F[0] == USHRT_MAX) break; lls_left = F[0] / 10.0f; break;
				case FUEL_SOURCE_F2: if (F[1] == USHRT_MAX) break; lls_left = F[1] / 10.0f; break;
				case FUEL_SOURCE_F3: if (F[2] == USHRT_MAX) break; lls_left = F[2] / 10.0f; break;
				case FUEL_SOURCE_F4: if (F[3] == USHRT_MAX) break; lls_left = F[3] / 10.0f; break;
				case FUEL_SOURCE_F5: if (F[4] == USHRT_MAX) break; lls_left = F[4] / 10.0f; break;
				case FUEL_SOURCE_F6: if (F[5] == USHRT_MAX) break; lls_left = F[5] / 10.0f; break;
				case FUEL_SOURCE_F7: if (F[6] == USHRT_MAX) break; lls_left = F[6] / 10.0f; break;
				case FUEL_SOURCE_F8: if (F[7] == USHRT_MAX) break; lls_left = F[7] / 10.0f; break;
				case FUEL_SOURCE_RS485_1: if (RS485[0] == USHRT_MAX) break; lls_left = RS485[0]; break;
				case FUEL_SOURCE_RS485_2: if (RS485[1] == USHRT_MAX) break; lls_left = RS485[1]; break;
				case FUEL_SOURCE_RS232_1: if (RS232[0] == USHRT_MAX) break; lls_left = RS232[0]; break;
				case FUEL_SOURCE_RS232_2: if (RS232[1] == USHRT_MAX) break; lls_left = RS232[1]; break;
				}

				switch (job->lls_right_source) {
				case FUEL_SOURCE_A1: if (A[0] == USHRT_MAX) break; lls_right = A[0] / 1000.0f; break;
				case FUEL_SOURCE_A2: if (A[1] == USHRT_MAX) break; lls_right = A[1] / 1000.0f; break;
				case FUEL_SOURCE_A3: if (A[2] == USHRT_MAX) break; lls_right = A[2] / 1000.0f; break;
				case FUEL_SOURCE_A4: if (A[3] == USHRT_MAX) break; lls_right = A[3] / 1000.0f; break;
				case FUEL_SOURCE_A5: if (A[4] == USHRT_MAX) break; lls_right = A[4] / 1000.0f; break;
				case FUEL_SOURCE_A6: if (A[5] == USHRT_MAX) break; lls_right = A[5] / 1000.0f; break;
				case FUEL_SOURCE_A7: if (A[6] == USHRT_MAX) break; lls_right = A[6] / 1000.0f; break;
				case FUEL_SOURCE_A8: if (A[7] == USHRT_MAX) break; lls_right = A[7] / 1000.0f; break;
				case FUEL_SOURCE_F1: if (F[0] == USHRT_MAX) break; lls_right = F[0] / 10.0f; break;
				case FUEL_SOURCE_F2: if (F[1] == USHRT_MAX) break; lls_right = F[1] / 10.0f; break;
				case FUEL_SOURCE_F3: if (F[2] == USHRT_MAX) break; lls_right = F[2] / 10.0f; break;
				case FUEL_SOURCE_F4: if (F[3] == USHRT_MAX) break; lls_right = F[3] / 10.0f; break;
				case FUEL_SOURCE_F5: if (F[4] == USHRT_MAX) break; lls_right = F[4] / 10.0f; break;
				case FUEL_SOURCE_F6: if (F[5] == USHRT_MAX) break; lls_right = F[5] / 10.0f; break;
				case FUEL_SOURCE_F7: if (F[6] == USHRT_MAX) break; lls_right = F[6] / 10.0f; break;
				case FUEL_SOURCE_F8: if (F[7] == USHRT_MAX) break; lls_right = F[7] / 10.0f; break;
				case FUEL_SOURCE_RS485_1: if (RS485[0] == USHRT_MAX) break; lls_right = RS485[0]; break;
				case FUEL_SOURCE_RS485_2: if (RS485[1] == USHRT_MAX) break; lls_right = RS485[1]; break;
				case FUEL_SOURCE_RS232_1: if (RS232[0] == USHRT_MAX) break; lls_right = RS232[0]; break;
				case FUEL_SOURCE_RS232_2: if (RS232[1] == USHRT_MAX) break; lls_right = RS232[1]; break;
				}

				if (job->lls_left_source != 0) {

					if ((lls_left == -1)||(job->lls_table_left.size() < 2))
						continue;

					if (lls_left > job->lls_table_left[job->lls_table_left.size() - 1].sensor_value)
						lls_left = job->lls_table_left[job->lls_table_left.size() - 1].fuel_value;
					else
					if (lls_left < job->lls_table_left[0].sensor_value)
						lls_left = job->lls_table_left[0].fuel_value;
					else {
						for (size_t i = job->lls_table_left.size(); i--;) {
							FUEL_TABLE_RECORD *ftr = &job->lls_table_left[i];
									
							if (lls_left >= ftr->sensor_value) {
								lls_left = ftr->fuel_value + ftr->d * (lls_left - ftr->sensor_value);
								break;
							}
						}
					}
				}
				else lls_left = 0;
	
				if (job->lls_right_source != 0) {

					if ((lls_right == -1)||(job->lls_table_right.size() < 2))
						continue;

					if (lls_right > job->lls_table_right[job->lls_table_right.size() - 1].sensor_value)
						lls_right = job->lls_table_right[job->lls_table_right.size() - 1].fuel_value;
					else
					if (lls_right < job->lls_table_right[0].sensor_value)
						lls_right = job->lls_table_right[0].fuel_value;
					else {
						for (size_t i = job->lls_table_right.size(); i--;) {

							FUEL_TABLE_RECORD *ftr = &job->lls_table_right[i];
									
							if (lls_right >= ftr->sensor_value) {
								lls_right = ftr->fuel_value + ftr->d * (lls_right - ftr->sensor_value);
								break;
							}
						}
					}
				}
				else lls_right = 0;

				if ((rh->t >= job->tFrom)&&(fuel_at_start == -1))
					fuel_at_start = lls_left + lls_right;

				if ((rh->t >= job->tTo)&&(fuel_at_end == -1))
					fuel_at_end = prevFuel;

				*pLat++ = lat;
				*pLng++ = lng;
				*pSpeed++ = speed;
				*pFuel++ = lls_left + lls_right;
				*pTime++ = rh->t;

				if (pLat == pMaxLat) {
					api_storage_unlock_stream(stream);
					return 500;
				}

				prevFuel = lls_left + lls_right;
			}

			api_storage_unlock_stream(stream);

			if (prev_flags > 0) {
				moto_seconds += job->tTo - moto_start;
			}

			if (fuel_at_end == -1)
				fuel_at_end = prevFuel;
	
			int distance_points_count = pDistanceLat - pInitDistanceLat;

			float distance = (distance_points_count > 1) ?  GetDistanceST(pInitDistanceLat, pInitDistanceLng, distance_points_count) : 0;

			unsigned char *content_length_ptr;
			size_t content_length;

			unsigned char *response		= buffer;
			size_t bytes_left			= buffer_size / 8;

			unsigned char *ptr = response_success_object(response, &bytes_left, job->session->zero_init.keep_alive > 0, &content_length_ptr, &content_length);

			if (ptr == NULL)
				return 500;

			unsigned char *object_start = ptr;

			if (bytes_left < 512) {
				return 500;
			}

			if ((fuel_at_start == -1)||(fuel_at_end == -1)||((job->lls_right_source == 0) && (job->lls_left_source == 0))) {
				size_t len = sprintf((char *)ptr, "{\"mileage\":%u,\"moto\":%u,\"injector\":%llu", (unsigned int)distance, moto_seconds, injector);
				ptr += len;
				bytes_left -= len;
			}
			else {
				size_t len = sprintf((char *)ptr, "{\"start\":%f,\"end\":%f,\"mileage\":%u,\"moto\":%u,\"injector\":%llu,\"fills\":", fuel_at_start, fuel_at_end, (unsigned int)distance, moto_seconds, injector);
				ptr += len;
				bytes_left -= len;
				len = api_fuel_process(pInitTime, pInitLat, pInitLng, pInitSpeed, pInitFuel, pLat - pInitLat, job->tFrom, job->tTo, job->fill_threshold, job->drain_threshold, job->max_consumption, job->filter_level, ptr, bytes_left, 3);
				ptr += len;
				bytes_left -= len;
			}

			if (bytes_left < 2) {
				return 500;
			}

			*ptr++ = '}';
			*ptr++ = '}';

			content_length += ptr - object_start;

			do {
				*--content_length_ptr = '0' + content_length % 10;
				content_length /= 10;
			} while (content_length != 0);

			api_send_tcp(job->session, response, ptr - response);

			return 0;
		}

		api_storage_unlock_stream(stream);
	}

	if (job->session->zero_init.keep_alive > 0)
		api_send_tcp(job->session, response_success_empty_object_ka, response_success_empty_object_ka_length);
	else
		api_send_tcp(job->session, response_success_empty_object_close, response_success_empty_object_close_length);

	return 0;
}

static int job_report_activity(THREAD_POOL_JOB *job, unsigned char *buffer, size_t buffer_size, std::vector<STORAGE_SORT_ITEM> *si)
{
	void *stream = api_storage_get_stream_by_id(job->terminal_id);

	if (stream != NULL) {

		api_storage_lock_stream(stream);
		
		api_storage_sort_stream(stream, buffer, si);

		size_t records_count = api_storage_get_stream_records_count(stream);

		STORAGE_RECORD_HEADER *rh = api_storage_get_stream_first_record(stream);

		unsigned char *content_length_ptr;
		size_t content_length;
		size_t bytes_left					= buffer_size;
		unsigned char *response				= buffer;

		unsigned char *ptr = response_success_object(response, &bytes_left, job->session->zero_init.keep_alive > 0, &content_length_ptr, &content_length);

		if (ptr == NULL) {
			api_storage_unlock_stream(stream);
			return 500;
		}
		
		unsigned char *object_start = ptr;

		size_t c = 0;

		// ≈сли поток не пуст
		if ((rh != NULL)&&(rh->t <= job->tTo)) {

			// Ќахожу в потоке первую точку больше или равной t_from
			size_t i;
			for (i = 0; i < records_count; i++) {

				if (rh->t >= job->tFrom)
					break;
						
				rh = (STORAGE_RECORD_HEADER *)(((unsigned char *)rh) + rh->size);
			}

			for (; i < records_count; i++) {

				if (rh->t > job->tTo)
					break;

				c++;

				rh = (STORAGE_RECORD_HEADER *)(((unsigned char *)rh) + rh->size);
			}
		}

		api_storage_unlock_stream(stream);

		ptr += sprintf((char *)ptr, "%u", c);

		*ptr++ = '}';

		content_length += ptr - object_start;

		do {
			*--content_length_ptr = '0' + content_length % 10;
			content_length /= 10;
		} while (content_length != 0);

		api_send_tcp(job->session, response, ptr - response);

		return 0;
	}

	if (job->session->zero_init.keep_alive > 0)
		api_send_tcp(job->session, response_success_empty_array_ka, response_success_empty_array_ka_length);
	else
		api_send_tcp(job->session, response_success_empty_array_close, response_success_empty_array_close_length);

	return 0;
}

static int do_job(THREAD_POOL_JOB *job, unsigned char *buffer, size_t buffer_size, std::vector<STORAGE_SORT_ITEM> *si)
{
	int result;
	unsigned int ignition_flag	= 0;
	unsigned int engine_flag	= 0;
	unsigned int move_flag		= 0;

	switch (job->code) {
	default:
		result = 500;
		break;
	case JOB_CODE_REPORT_MILEAGE:
		result = job_report_mileage(job, buffer, buffer_size, si);
		break;

	case JOB_CODE_REPORT_FUEL:
		result = job_report_fuel(job, buffer, buffer_size, si);
		break;
	case JOB_CODE_REPORT_PARK:
		result = job_report_park_with_engine(job, buffer, buffer_size, si);
		break;
	case JOB_CODE_HISTORY_BIN:
		result = job_report_history_bin(job, buffer, buffer_size, si);
		break;
	case JOB_CODE_HISTORY_PNG:
		result = job_report_history_png(job, buffer, buffer_size, si);
		break;
	case JOB_CODE_REPORT_IGNITION:

		if (job->ignition) {
			ignition_flag = RECORD_FLAG1_IGNITION;
		}
		else
		if ((job->ignition_source > 0)&&(job->ignition_source <= 5)) {
			ignition_flag = ((0x80 >> (job->ignition_source - 1))) << 8;
		}

		result = job_report_flag(job, buffer, buffer_size, si, ignition_flag);

		break;
	case JOB_CODE_REPORT_ENGINE:

		if (job->ignition) {
			ignition_flag = RECORD_FLAG1_IGNITION;
		}
		else
		if ((job->ignition_source > 0)&&(job->ignition_source <= 5)) {
			ignition_flag = ((0x80 >> (job->ignition_source - 1))) << 8;
		}

		if (job->engine) {
			engine_flag = RECORD_FLAG1_ENGINE;
		}
		else
		if ((job->engine_source <= 5)&&(job->engine_source <= 5)) {
			engine_flag = ((0x80 >> (job->engine_source - 1))) << 8;
		}

		result = job_report_flag(job, buffer, buffer_size, si, engine_flag);

		break;
	case JOB_CODE_REPORT_MOVE:

		if (job->ignition) {
			ignition_flag = RECORD_FLAG1_IGNITION;
		}
		else
		if ((job->ignition_source > 0)&&(job->ignition_source <= 5)) {
			ignition_flag = ((0x80 >> (job->ignition_source - 1))) << 8;
		}

		if (job->engine) {
			engine_flag = RECORD_FLAG1_ENGINE;
		}
		else
		if ((job->engine_source <= 5)&&(job->engine_source <= 5)) {
			engine_flag = ((0x80 >> (job->engine_source - 1))) << 8;
		}

		if (job->move) {
			move_flag = RECORD_FLAG1_MOVE;
		}
		else
		if ((job->move_source >= 0)&&(job->move_source <= 5)) {
			move_flag = ((0x80 >> (job->move_source - 1))) << 8;
		}

		if (move_flag == 0)
			move_flag = engine_flag;

		if (move_flag == 0)
			move_flag = ignition_flag;

		result = job_report_flag(job, buffer, buffer_size, si, move_flag);

		break;
	case JOB_CODE_REPORT_DI1:
		result = job_report_flag(job, buffer, buffer_size, si, 0x8000);
		break;
	case JOB_CODE_REPORT_DI2:
		result = job_report_flag(job, buffer, buffer_size, si, 0x4000);
		break;
	case JOB_CODE_REPORT_DI3:
		result = job_report_flag(job, buffer, buffer_size, si, 0x2000);
		break;
	case JOB_CODE_REPORT_DI4:
		result = job_report_flag(job, buffer, buffer_size, si, 0x1000);
		break;
	case JOB_CODE_REPORT_DI5:
		result = job_report_flag(job, buffer, buffer_size, si, 0x0800);
		break;
	case JOB_CODE_REPORT_ACTIVITY:
		result = job_report_activity(job, buffer, buffer_size, si);
		break;
	case JOB_CODE_PDF:
		result = job_report_pdf(job, buffer, buffer_size, font_file_name.c_str());
		free(job->buffer);
		break;
	}

	if (result != 0) {
		size_t bytes_left = buffer_size;
		response_fail_with_message(buffer, &bytes_left, not_enough_memory, sizeof(not_enough_memory) - 1, job->session->zero_init.keep_alive > 0);
		api_send_tcp(job->session, buffer, bytes_left);
	}

	if (job->session->zero_init.keep_alive) {

		memset(&job->session->zero_init, 0, sizeof(job->session->zero_init));

		http_parser_init(&job->session->parser, HTTP_REQUEST);

		job->session->parser.data = job->session;		
	}
	else {
		api_close_tcp(job->session);
	}

	return 0;
}

#ifndef _MSC_VER

#include <pthread.h>
#include <atomic>
#include <unistd.h>
#include <signal.h>

static pthread_mutex_t job_queue_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t job_queue_cond = PTHREAD_COND_INITIALIZER;
 
static std::vector<pthread_t> threads;

static std::atomic_int running_count;

void *thread_pool_proc(void *param)
{
	std::vector<STORAGE_SORT_ITEM> si;

	unsigned char *buffer = (unsigned char *)aligned_malloc(32, BUFFER_SIZE);

	size_t i = (size_t)param;

	running_count++;

	api_log_printf("[TP] Thread %u is running\r\n", i);

	for (;;) {

		pthread_mutex_lock(&job_queue_mutex);
	
		while (job_queue.empty())
			pthread_cond_wait(&job_queue_cond, &job_queue_mutex);

		THREAD_POOL_JOB job = job_queue.front();
		job_queue.pop();

		api_log_printf("[TP] Thread %u got job %d\r\n", i, job.code);

		pthread_mutex_unlock(&job_queue_mutex);
		
		if (job.code == 0)
			break;

		do_job(&job, buffer, BUFFER_SIZE, &si);

		api_log_printf("[TP] Thread %u done\r\n", i);
	}

	api_log_printf("[TP] Thread %u stopped\r\n", i);

	running_count--;

	aligned_free(buffer);

	return NULL;
}

void thread_pool_add_job(THREAD_POOL_JOB *job)
{
	pthread_mutex_lock(&job_queue_mutex);
	
	job_queue.push(*job);
	
	pthread_mutex_unlock(&job_queue_mutex);
	
	pthread_cond_signal(&job_queue_cond);
}
 
void thread_pool_init(size_t num_of_threads)
{
	api_log_printf("[TP] Running threads\r\n");

	sigset_t sigset, oldset;
	sigemptyset(&sigset);
	sigaddset(&sigset, SIGINT);
	pthread_sigmask(SIG_BLOCK, &sigset, &oldset);

	running_count = 0;

	if (running_count.is_lock_free() == false)
		api_log_printf("[TP] Warning, threads counter isn't lock-free\r\n");

	threads.reserve(num_of_threads);

	for (size_t i = 0; i < num_of_threads; i++) 
	{
		pthread_t t;
		pthread_create(&t, NULL, thread_pool_proc, (void *)i);
		threads.push_back(t);
	}

	// Restore the old signal mask only for this thread.
	pthread_sigmask(SIG_SETMASK, &oldset, NULL);

}

void thread_pool_destroy()
{
	api_log_printf("[TP] Terminating threads\r\n");

	job.code = 0;
	for (size_t i = 0; i < threads.size(); i++) 
		thread_pool_add_job(&job);

	for (size_t i = 0; i < threads.size(); i++) 
		pthread_join(threads[i], NULL);
}

#else

static std::vector<HANDLE> threads;

CRITICAL_SECTION CriticalSection;
CONDITION_VARIABLE ConditionVariable;

DWORD WINAPI thread_proc(LPVOID lpThreadParameter)
{
	std::vector<STORAGE_SORT_ITEM> si;

	size_t i = (size_t)lpThreadParameter;

	unsigned char *buffer = (unsigned char *)aligned_malloc(32, BUFFER_SIZE);

	if (buffer == NULL) {
		api_log_printf("[TP] Thread %u failed to start\r\n", i);
		return 0;
	}

	api_log_printf("[TP] Thread %u is running\r\n", i);

	for (;;) {
	
		EnterCriticalSection(&CriticalSection);

		while (job_queue.size() == 0)
            SleepConditionVariableCS(&ConditionVariable, &CriticalSection, INFINITE);

        THREAD_POOL_JOB job = job_queue.front();
        job_queue.pop();

        LeaveCriticalSection(&CriticalSection);

		api_log_printf("[TP] Thread %u got job %d\r\n", i, job.code);

		if (job.code == 0)
			break;

		do_job(&job, buffer, BUFFER_SIZE, &si);

		api_log_printf("[TP] Thread %u done\r\n", i);
	}

	aligned_free(buffer);

	api_log_printf("[TP] Thread %u stopped\r\n", i);

	return 0;
}

void thread_pool_init(size_t num_of_threads)
{
	api_log_printf("[TP] Running threads\r\n");

	threads.reserve(num_of_threads);

	InitializeCriticalSection(&CriticalSection);
	InitializeConditionVariable(&ConditionVariable);

	for (size_t i = 0; i < num_of_threads; i++) {

		HANDLE thread = CreateThread(NULL, 0, thread_proc, (void *)i, 0, NULL);

		threads.push_back(thread);
	}
}

void thread_pool_destroy()
{
	job.code = 0;

	for (size_t i = 0; i < threads.size(); i++)
		thread_pool_add_job(&job);

	for (size_t i = 0; i < threads.size(); i++) {
		WaitForSingleObject(threads[i], INFINITE);
		CloseHandle(threads[i]);
	}

	DeleteCriticalSection(&CriticalSection);
}

void thread_pool_add_job(THREAD_POOL_JOB *job)
{
	EnterCriticalSection(&CriticalSection);

	job_queue.push(*job);

	LeaveCriticalSection(&CriticalSection);

	WakeConditionVariable(&ConditionVariable); 
}

#endif

// End
