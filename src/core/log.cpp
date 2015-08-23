//******************************************************************************
//
// File Name	: log.cpp
// Author	: Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h> 
#include <time.h> 

#include "log.h"
#include "likely.h"
#include "spinlock.h"
#include "cross.h"

// ini settings
static char	log_dir[2048];
static int	log_commit_type;
static int	log_file_mode;
static size_t	log_buffer_len;

static char	*log_buffer;
static int	log_file;

static char	*log_buffer_current;
static size_t	log_bytes_left;

static int	log_file_oflag, log_mday;

static char	log_now_string[13];

static char	*log_buffer_plus_11;
static size_t	log_buffer_len_minus_11;

static spinlock_t spinlock;

int log_printf(const char * __format, ... )
{
	int result = 0;

	spinlock_lock(&spinlock);

	for (;;) {
	
		if (likely(log_commit_type == LOG_COMMIT_TYPE_1)) {

			if (unlikely(log_bytes_left < 11)) {
				result = -1;
				break;
			}

			memcpy(log_buffer_current, log_now_string, 11);

			log_buffer_current += 11;
			log_bytes_left -= 11;

			va_list args;
			va_start(args,__format);
			size_t ret_status = vsnprintf(log_buffer_current, log_bytes_left, __format, args);
			va_end(args);

			if (likely((ret_status >= 0)&&(ret_status < log_bytes_left))) {
				log_buffer_current += ret_status;
				log_bytes_left -= ret_status;

				result = 0;
			
				break;
			}
			else {
				log_buffer_current -= 11;
				log_bytes_left += 11;
				
				result = -1;

				break;
			}
		}
		else
		if (log_commit_type == LOG_COMMIT_TYPE_2) {

			memcpy(log_buffer, log_now_string, 11);

			va_list args;
			va_start(args,__format);
			size_t ret_status = vsnprintf(log_buffer_plus_11, log_buffer_len_minus_11, __format, args);
			va_end(args);

			if (likely((ret_status >= 0)&&(ret_status < log_buffer_len_minus_11))) {

				write(log_file, log_buffer, 11 + ret_status);
				fsync(log_file);

				result = 0;

				break;
			}
			else {

				result =-1;
			
				break;
			}
		}
		else {
			memcpy(log_buffer, log_now_string, 11);

			va_list args;
			va_start(args,__format);
			size_t ret_status = vsnprintf(log_buffer_plus_11, log_buffer_len_minus_11, __format, args);
			va_end(args);

			if (likely((ret_status >= 0)&&(ret_status < log_buffer_len_minus_11))) {
	
				write(log_file, log_buffer, 11 + ret_status);

				result = 0;	

				break;
			}
			else {
				result = -1;

				break;
			}
		}
	}

	spinlock_unlock(&spinlock);

	return result;
}

int log_1sec_timer()
{
	char log_path[2048];
	time_t now;
	struct tm tms;

	now = time(NULL);
	localtime_r(&now, &tms);

	int zero = 0;

	spinlock_lock(&spinlock);

	sprintf(log_now_string, "[%02u:%02u:%02u]  ", tms.tm_hour, tms.tm_min, tms.tm_sec);

	if (likely(log_commit_type == LOG_COMMIT_TYPE_1)) {

		write(log_file, log_buffer, log_buffer_len - log_bytes_left);
		fsync(log_file);

		log_buffer_current = log_buffer;
		log_bytes_left = log_buffer_len;
	}
	else
	if (log_commit_type == LOG_COMMIT_TYPE_3) {
		fsync(log_file);
	}

	if (unlikely(log_mday != tms.tm_mday)) {

		close(log_file);

		log_mday = tms.tm_mday;

		snprintf(log_path, sizeof(log_path), "%s/%04u%02u%02u.txt", log_dir, tms.tm_year + 1900, tms.tm_mon + 1, tms.tm_mday);

		log_file = open(log_path, log_file_oflag, PMODE);

		if (unlikely(log_file == -1)) {

			spinlock_unlock(&spinlock);

			return -1;
		}
	}

	spinlock_unlock(&spinlock);

	return 0;
}

int log_init(const char *dir, int commit_type, int file_mode, size_t buffer_len)
{
	char log_path[2048];
	char error[2048];
	struct tm tms;
	time_t now;

	if (dir != NULL) {
		log_dir[sizeof(log_dir) - 1] = '\0';
		strncpy(log_dir, dir, sizeof(log_dir) - 1);
	}
	else
		*log_dir = '\0';

	log_buffer_len = buffer_len;
	log_commit_type = commit_type;
	log_file_mode = file_mode;	

	if (log_buffer_len == 0)
		log_buffer_len = 4096;

	log_buffer = (char *)malloc(log_buffer_len);
	
	if (unlikely(log_buffer == NULL)) {
		printf("Unable to allocate log buffer\r\n");
		return -1;
	}

	log_file_oflag = O_WRONLY | O_APPEND | O_CREAT;
                
	if (log_file_mode == LOG_FILE_MODE_OSYNC)
		log_file_oflag |= O_SYNC;

	if (log_file_mode == LOG_FILE_MODE_ODIRECT)
		log_file_oflag |= O_DIRECT;

	now = time(NULL);
	localtime_r(&now, &tms);

	log_mday = tms.tm_mday;

	snprintf(log_path, sizeof(log_path), "%s/%04u%02u%02u.txt", log_dir, tms.tm_year + 1900, tms.tm_mon + 1, tms.tm_mday);

	log_file = open(log_path, log_file_oflag, PMODE);

	if (unlikely(log_file == -1)) {

		int prev_errno = errno;

		if (likely(strerror_r(errno, error, sizeof(error)) == 0))
			printf("Failed to open log file '%s': %s [%d]\r\n", log_path, error, errno);
		else
			printf("Failed to open log file '%s', errno %d\r\n", log_path, prev_errno);

		return -1;
	}

	log_buffer_current = log_buffer;
	log_bytes_left = log_buffer_len;

	log_buffer_plus_11 = log_buffer + 11;
	log_buffer_len_minus_11 = log_buffer_len - 11;

	spinlock_init(&spinlock);

	log_1sec_timer();

	return 0;
}

void log_commit()
{
	log_1sec_timer();
}

int log_cleanup()
{
	if (log_buffer != NULL) {
		free(log_buffer);
		log_buffer = NULL;
	}

	if (log_file != -1) {
		close(log_file);
		log_file = -1;
	}

	return 0;
}

// End
