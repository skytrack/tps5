//******************************************************************************
//
// File Name : core.cpp
// Author    : Skytrack ltd - Copyright (C) 2013
//
// This code is property of Skytrack company
//
//******************************************************************************

#include <stdlib.h>
#include <stdio.h>
#include <string.h> 
#include <fcntl.h>
#include <sys/types.h> 
#include <sys/stat.h>
#include <errno.h> 
#include <signal.h> 
#include <time.h>
#include <limits.h>
#include <vector>
#include <string>
#include <queue>
#include <list>
#include <map>
#include <limits.h>
#include "jparse.h"

#ifndef _MSC_VER
#include <pthread.h>
#include <dlfcn.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/event.h> 
#include <sys/un.h>
#include <sys/uio.h>
#include <pwd.h>
#include <grp.h>
#include "cross.h"

#define MODULE_HANDLE void *

#else
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include "cross.h"
#include "kqueue.h"

#define socklen_t int
#define MODULE_HANDLE HMODULE

#endif

#include "core.h"
#include "log.h"
#include "config.h"
#include "storage.h"
#include "module.h"
#include "db.h"
#include "likely.h"
#include "spinlock.h"

#ifdef _MSC_VER
char config[2048] = "c:\\Projects\\tps5\\tps5.ini";
#else
char config[2048] = "/home/tps4/tps5/tps5.ini";
#endif

volatile sig_atomic_t bContinueToWork = 1;

static int    kq;
static struct kevent *inqueue = NULL;
static int    inqueue_size;
static struct kevent *outqueue = NULL;
static int    outqueue_size;
static struct kevent *event;

static spinlock_t spinlock;

		std::list<SESSION_HANDLERS *> sessions;

void ExitSignalHandler(int a) 
{
	bContinueToWork = 0;
}

// ini settings
static std::string modules_dir;
static char database[2048] = { 0 };
static char user[33]       = { 0 };
static char grp[33]      = { 0 };
static std::string pid;
static std::string fuel;
static std::string password = "0000";

static size_t module;

static std::vector<MODULE> arr_modules;

static char error[8192];

static char host[NI_MAXHOST];
static char port[NI_MAXSERV];
struct sockaddr_storage remote;
static socklen_t remote_len;
static char buffer[1024 * 256];
static size_t buffer_len;

static int status;
static size_t inqueue_count = 0;

static char	log_dir[2048]    = { 0 };
static int	log_commit_type  = LOG_COMMIT_TYPE_2;
static int	log_file_mode    = LOG_FILE_MODE_REGULAR;
static size_t	log_buffer_len   = 0;

static char	data_dir[2048]   = { 0 };
static int	data_commit_type = STORAGE_COMMIT_TYPE_2;
static int	data_file_mode   = STORAGE_FILE_MODE_REGULAR;
static size_t	data_buffer_len  = 0;

static MODULE_HANDLE fuel_handle = NULL;
static FUEL_PROCESS fuel_proc_fuel_process = NULL;

typedef struct udp_ctx
{
	int sock;
	struct sockaddr_storage remote;
	socklen_t remote_len;
} UDP_CONTEXT;

UDP_CONTEXT udp_context;

static int config_handler(const char *name, const char *value)
{
	char *p;
	if (strcmp(name, "fuel") == 0) {
		fuel = value;
	}
	else
	if (strcmp(name, "password") == 0) {
		password = value;
	}
	else
	if (strcmp(name, "user") == 0) {
		user[sizeof(user) - 1] = '\0';
		strncpy(user, value, sizeof(user) - 1);
	}
	else
	if (strcmp(name, "group") == 0) {
		grp[sizeof(grp) - 1] = '\0';
		strncpy(grp, value, sizeof(grp) - 1);
	}
	else
	if (strcmp(name, "pid") == 0) {
		pid = value;
	}
	else
	if (strcmp(name, "modules_dir") == 0) {
		modules_dir = value;
	}
	else
	if (strcmp(name, "data_dir") == 0) {
		data_dir[sizeof(data_dir) - 1] = '\0';
		strncpy(data_dir, value, sizeof(data_dir) - 1);
	}
	else
	if (strcmp(name, "log_dir") == 0) {
		log_dir[sizeof(log_dir) - 1] = '\0';
		strncpy(log_dir, value, sizeof(log_dir) - 1);
	}
	else
	if (strcmp(name, "database") == 0) {
		database[sizeof(database) - 1] = '\0';
		strncpy(database, value, sizeof(database) - 1);
	}
	else
	if (strcmp(name, "data_commit_type") == 0) {
		if (strcmp(value, "1") == 0)
			data_commit_type = STORAGE_COMMIT_TYPE_1;
		else
		if (strcmp(value, "2") == 0)
			data_commit_type = STORAGE_COMMIT_TYPE_2;
		else
		if (strcmp(value, "3") == 0)
			data_commit_type = STORAGE_COMMIT_TYPE_3;
		else {
			printf("[CONFIG] data_commit_type can be 1, 2 or 3\r\n");

			return 0;
		}
	}
	else
	if (strcmp(name, "data_file_mode") == 0) {
		if (strcmp(value, "0") == 0)
			data_file_mode = 0;
		else
		if (strcmp(value, "O_SYNC") == 0)
			data_file_mode = LOG_FILE_MODE_OSYNC;
		else
		if (strcmp(value, "O_DIRECT") == 0)
			data_file_mode = LOG_FILE_MODE_ODIRECT;
		else {
			printf("[CONFIG] data_file_mode can be 0, O_SYNC or O_DIRECT\r\n");

			return 0;
		}
	}
	else
	if (strcmp(name, "data_buffer_len") == 0) {
		
		errno = 0;

		unsigned long i = strtoul(value, &p, 10);

		if ((i == 0)&&(p == value)) {

			printf("data_buffer_len has invalid value\r\n");

			return 0;
		}

		if ((i == ULONG_MAX)&&(errno == ERANGE)) {

			printf("data_buffer_len is out of range\r\n");

			return 0;
		}

		if (*p != '\0') {
			switch (*p) {
			case 'b':
			case 'B':
				break;
			case 'm':
			case 'M':
				i *= 1024 * 1024;
				break;
			case 'g':
			case 'G':
				i *= 1024 * 1024 * 1024;
				break;
			default:
				printf("[CONFIG] data_buffer_len has invalid value\r\n");
				
				return 0;
			}
		}

		data_buffer_len = i;
	}
	else
	if (strcmp(name, "log_buffer_len") == 0) {
		
		errno = 0;

		unsigned long i = strtoul(value, &p, 10);

		if ((i == 0)&&(p == value)) {

			printf("[CONFIG] log_buffer_len has invalid value\r\n");

			return 0;
		}

		if ((i == ULONG_MAX)&&(errno == ERANGE)) {

			printf("[CONFIG] log_buffer_len is out of range\r\n");

			return 0;
		}

		if (*p != '\0') {
			switch (*p) {
			case 'b':
			case 'B':
				break;
			case 'k':
			case 'K':
				i *= 1024;
				break;
			case 'm':
			case 'M':
				i *= 1024 * 1024;
				break;
			case 'g':
			case 'G':
				i *= 1024 * 1024 * 1024;
				break;
			default:
				printf("[CONFIG] log_buffer_len has invalid value\r\n");

				return 0;
			}
		}

		log_buffer_len = i;
	}
	else
	if (strcmp(name, "log_commit_type") == 0) {
		if (strcmp(value, "1") == 0)
			log_commit_type = LOG_COMMIT_TYPE_1;
		else
		if (strcmp(value, "2") == 0)
			log_commit_type = LOG_COMMIT_TYPE_2;
		else
		if (strcmp(value, "3") == 0)
			log_commit_type = LOG_COMMIT_TYPE_3;
		else {
			printf("[CONFIG] log_commit_type can be 1, 2 or 3\r\n");

			return 0;
		}
	}
	else
	if (strcmp(name, "log_file_mode") == 0) {
		if (strcmp(value, "0") == 0)
			log_file_mode = 0;
		else
		if (strcmp(value, "O_SYNC") == 0)
			log_file_mode = LOG_FILE_MODE_OSYNC;
		else
		if (strcmp(value, "O_DIRECT") == 0)
			log_file_mode = LOG_FILE_MODE_ODIRECT;
		else {
			printf("[CONFIG] log_file_mode can be 0, O_SYNC or O_DIRECT\r\n");

			return 0;
		}
	}
	else {
		printf("[CONFIG] Unknown parameter `%s`\r\n", name);

		return -3;
	}

	return 1;
}

static int modules_handler(const char *name, const char *value)
{
	MODULE m;

	m.name = name;
	m.path = value;

	arr_modules.push_back(m);

	return 1;
}

static int module_handler(const char *name, const char *value)
{
	CONFIG_OPTION co;

	co.name = name;
	co.value = value;

	arr_modules[module].set_var(MODULE_VAR_OPTION, &co);

	return 1;
}

static int listen_tcp(void *tag, const char *host, const char *_port, const SESSION_HANDLERS *handlers)
{
	struct kevent event;
	struct addrinfo sHints, *psAddrInfo, *p;

	char addr[NI_MAXHOST];
	char port[NI_MAXSERV];

	memset(&sHints, 0, sizeof(struct addrinfo));

	sHints.ai_family   = PF_UNSPEC;
	sHints.ai_socktype = SOCK_STREAM;
	sHints.ai_protocol = IPPROTO_TCP;
	sHints.ai_flags    = AI_PASSIVE;
		
	const char *address = ((*host == '*')||(*host == '\0')) ? NULL : host;

	int status = getaddrinfo(address, _port, &sHints, &psAddrInfo);

	if (status != 0) {

		log_printf("[CORE] getaddrinfo failed for %s:%s\r\n", host, port);

		return -1;
	}

	for (p = psAddrInfo; p; p = p->ai_next) {
			
		addr[sizeof(host) - 1] = '\0';
		port[sizeof(port) - 1] = '\0';

		if (getnameinfo(p->ai_addr, p->ai_addrlen, addr, sizeof(addr), port, sizeof(port), NI_NUMERICHOST | NI_NUMERICSERV) != 0) {

			*addr = '\0';
			*port = '\0';
		}
        
		int sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);

		if (sock < 0) {

			log_printf("[CORE] Unable to create socket, errno - %d\r\n", errno);

			continue;
		}

		status = 1;
		setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&status, sizeof(status));

#ifndef _MSC_VER
		status = fcntl(sock, F_SETFL, O_NONBLOCK);
#endif
		status = bind(sock, p->ai_addr, p->ai_addrlen);

		if (status == -1) {
			
			log_printf("[CORE] Unable to bind socket to '%s':'%s', errno - %d\r\n", addr, port, errno);

			closesocket(sock);

			continue;
		}

		status = listen(sock, 128);

		if (status == -1) {
					
			log_printf("[CORE] Unable to listen socket to '%s':'%s', errno - %d\r\n", addr, port, errno);

			closesocket(sock);

			continue;
		}

		log_printf("[CORE] Binded to TCP '%s':'%s', socket #%d\r\n", addr, port, sock);

		intptr_t data = (intptr_t)handlers;
		data++;
		EV_SET(&event, sock, EVFILT_READ, EV_ADD, 0, 0, (void *)data); 

		int status = kevent(kq, &event, 1, NULL, 0, NULL);

		if (status == -1) {

			log_printf("[CORE] errno #%d on adding socket #%d to kqueue\r\n", errno, sock);

			closesocket(sock);

			continue;
		}

		log_printf("[CORE] socket #%d added to kqueue\r\n", sock);
	}
		
	return 0;
}

static int listen_udp(void *tag, const char *host, const char *_port, UDP_DATA udp_data)
{
	struct kevent event;
	struct addrinfo sHints, *psAddrInfo, *p;

	char addr[NI_MAXHOST];
	char port[NI_MAXSERV];

	memset(&sHints, 0, sizeof(struct addrinfo));

	sHints.ai_family   = PF_UNSPEC;
	sHints.ai_socktype = SOCK_DGRAM;
	sHints.ai_protocol = IPPROTO_UDP;
	sHints.ai_flags    = AI_PASSIVE;
		
	const char *address = ((*host == '*')||(*host == '\0')) ? NULL : host;

	int status = getaddrinfo(address, _port, &sHints, &psAddrInfo);

	if (status != 0) {

		log_printf("[CORE] getaddrinfo failed for %s:%s\r\n", host, port);

		return -1;
	}

	for (p = psAddrInfo; p; p = p->ai_next) {
			
		addr[sizeof(host) - 1] = '\0';
		port[sizeof(port) - 1] = '\0';

		if (getnameinfo(p->ai_addr, p->ai_addrlen, addr, sizeof(addr), port, sizeof(port), NI_NUMERICHOST | NI_NUMERICSERV) != 0) {

			*addr = '\0';
			*port = '\0';
		}
        
        	int sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);

		if (sock < 0) {

			log_printf("[CORE] Unable to create socket, errno - %d\r\n", errno);

			continue;
		}

		status = 1;
		setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&status, sizeof(status));

		status = bind(sock, p->ai_addr, p->ai_addrlen);

		if (status == -1) {
			
			log_printf("[CORE] Unable to bind socket to '%s':'%s', errno - %d\r\n", addr, port, errno);

			closesocket(sock);

			continue;
		}

		log_printf("[CORE] Binded to UDP '%s':'%s', socket #%d\r\n", addr, port, sock);

		intptr_t data = (intptr_t)udp_data;
		data |= 3;
		EV_SET(&event, sock, EVFILT_READ, EV_ADD, 0, 0, (void *)data); 

       	int status = kevent(kq, &event, 1, NULL, 0, NULL);

		if (status == -1) {

			log_printf("[CORE] errno #%d on adding socket #%d to kqueue\r\n", errno, sock);

			closesocket(sock);

			continue;
       	}

		log_printf("[CORE] socket #%d added to kqueue\r\n", sock);
	}
		
	return 0;
}

int send_tcp(SESSION_HANDLERS *handlers, unsigned char *p, size_t l)
{
	log_printf("[CORE] Send %u bytes to socket #%u\r\n", l, handlers->sock);

#ifndef _MSC_VER
	fcntl(handlers->sock, F_SETFL, fcntl(handlers->sock, F_GETFL, 0) & ~O_NONBLOCK);
#endif

	int res = send(handlers->sock, (char *)p, l, 0);

#ifndef _MSC_VER
		status = fcntl(handlers->sock, F_SETFL, O_NONBLOCK);
#endif

	return res;
}

int send_udp(unsigned char *p, size_t l, UDP_CONTEXT *ctx)
{
	return sendto(ctx->sock, (char *)p, l, 0, (struct sockaddr *)&ctx->remote, ctx->remote_len);
}

int close_tcp(SESSION_HANDLERS *handlers)
{
	log_printf("[CORE] Socket #%d closed\r\n", handlers->sock);
	EV_SET(&inqueue[inqueue_count++], handlers->sock, EVFILT_TIMER, EV_DELETE, 0, 0, NULL); 
	EV_SET(&inqueue[inqueue_count++], handlers->sock, EVFILT_READ, EV_DELETE, 0, 0, NULL); 
	spinlock_lock(&spinlock);
	sessions.remove(handlers);
	spinlock_unlock(&spinlock);
	handlers->close(handlers);
	return closesocket(handlers->sock);
}

int dummy_timer()
{
	return 0;
}

MODULE *get_device_module(int device_type)
{
	for (size_t i = 0; i < arr_modules.size(); i++)
		if ((arr_modules[i].family == MODULE_FAMILY_DEVICE)&&(*arr_modules[i].terminal_type == device_type))
			return &arr_modules[i];

	return NULL;
}

MODULE *get_retranslator_module(int retranslator_type)
{
	for (size_t i = 0; i < arr_modules.size(); i++)
		if ((arr_modules[i].family == MODULE_FAMILY_RETRANSLATOR)&&(*arr_modules[i].protocol_id == retranslator_type))
			return &arr_modules[i];

	return NULL;
}

int enum_modules(MODULE_FAMILY family, ENUM_MODULES_CALLBACK callback, void *ctx)
{
	for (size_t i = 0; i < arr_modules.size(); i++)
		if ((family == MODULE_FAMILY_ANY)||(arr_modules[i].family == family))
			if (callback(&arr_modules[i], ctx))
				return -1;
	return 0;
}

void demonize(const char *pidFile) 
{
#ifndef _MSC_VER
	int  rc; 
	int  maxfd; 
	int  fd; 
	char s[32]; 

	rc = fork(); 
	if (rc < 0)  {
    		printf("first-time fork() error\n");
		exit(1);
	}
	if (rc > 0) 
		exit(0); /* End parent process. */ 

	rc = setsid(); 
	if (rc == -1) {
    		printf("unable to set sid\n");
		exit(1);
	}

	rc = fork(); 
	if (rc < 0) {
    		printf("first-time fork() error\n");
		exit(1);
	}
	if (rc > 0) 
		exit(0); /* End parent process. */ 

	chdir("/");

	if (pidFile != NULL) { 
		sprintf(s, "%u\n", getpid()); 
		fd = open(pidFile, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR ); 
		if (fd == -1) {
			printf("daemon(): unable to create pid-file\n"); 
			exit(1);
		}
		if (write(fd, s, strlen(s)) == -1) {
			printf("daemon(): unable to write pid-file\n"); 
			exit(1);
		}
		close(fd); 
	} 

	maxfd = sysconf(_SC_OPEN_MAX); 
	for (fd = maxfd - 1; fd >= 0; fd--) 
		close(fd); /* Ignore errors. */ 

	fd = open("/dev/null", O_RDWR); /* stdin  - file handle 0. */ 
	dup(fd);                        /* stdout - file handle 1. */ 
	dup(fd);                        /* stderr - file handle 2. */ 

	umask(0);
#endif
}

size_t fuel_process(int *time, float *lat, float *lng, unsigned short *speed, float *fuel, size_t data_length, 
					int t_from, int t_to,
					float fill_threshold, float drain_threshold, size_t max_rate, size_t filter_length,
					unsigned char *buffer, size_t bytes_left, unsigned int flags)
{
	*buffer++ = '[';
	*buffer++ = ']';

	return 2;
}

void bubble_sort(unsigned char *arr, size_t arr_size)
{
	for (size_t i = 0; i < arr_size - 1; i++) {

		bool changed = false;
		size_t min_index = i;

		for (size_t j = i; j < arr_size - 1 - i; j++) {

			if (arr[j] > arr[j + 1]) {

				unsigned char t = arr[j];
				arr[j] = arr[j + 1];
				arr[j + 1] = t;

				changed = true;
			}

			if (arr[min_index] > arr[j])
				min_index = j;
		}

		if (changed == false)
			break;

		if (min_index != i) {

			unsigned char t = arr[min_index];
			arr[min_index] = arr[i];
			arr[i] = t;
		}
	}
}

void shake_sort(unsigned char *arr, size_t arr_size)
{
	size_t left = 0;
	size_t right = arr_size - 1;

	while (left < right) {

		for (size_t i = right; i > left; i--) {
			if (arr[i] < arr[i - 1]) {
				unsigned char t = arr[i];
				arr[i] = arr[i - 1];
				arr[i - 1] = t;
			}
		}

		left++;

		for (size_t i = left; i < right; i++) {
			if (arr[i] > arr[i + 1]) {
				unsigned char t = arr[i];
				arr[i] = arr[i + 1];
				arr[i + 1] = t;
			}
		}

		right--;
	}
}

void quick_sort(unsigned char *arr, size_t arr_size)
{
	unsigned char base = arr[arr_size / 2];

	size_t l = 0;
	size_t r = arr_size - 1;

	for (;l < r;) {

		while (arr[l] < base)
			l++;

		while (arr[r] > base)
			r--;

		if (l >= r)
			break;

		unsigned char t = arr[l];
		arr[l] = arr[r];
		arr[r] = t;
		l++;
		r--;
	}

	if (l > 1)
		quick_sort(arr, l);
	if (arr_size - l > 1)
		quick_sort(arr + l, arr_size - l);
}

void insert_sort(unsigned char *arr, size_t arr_size)
{
	for (size_t k = 1; k < arr_size; k++) {

		size_t current_item = k;

		while ((current_item >= 1)&&(arr[current_item] < arr[current_item - 1])) {
			unsigned char t = arr[current_item];
			arr[current_item] = arr[current_item - 1];
			arr[current_item - 1] = t;

			current_item--;
		}
	}
}

void merge_sort(unsigned char *arr, size_t arr_size)
{
	if (arr_size < 2)
		return;

	if ((arr_size == 2)&&(arr[0] > arr[1])) {
		unsigned char t = arr[0];
		arr[0] = arr[1];
		arr[1] = t;
		return;
	}

	size_t middle = arr_size / 2;

	unsigned char *left = arr;
	unsigned char *left_end = left + middle;

	merge_sort(left, middle);

	unsigned char *right = arr + middle;
	unsigned char *right_end = right + arr_size - middle;

	merge_sort(right, arr_size - middle);

	unsigned char *buf = (unsigned char *)malloc(arr_size);
	unsigned char *ptr = buf;

	while ((left < left_end)&&(right < right_end)) {
		if (*left <= *right)
			*ptr++ = *left++;
		else
			*ptr++ = *right++;
	}

	while (left < left_end)
		*ptr++ = *left++;

	while (right < right_end)
		*ptr++ = *right++;

	memcpy(arr, buf, arr_size);

	free(buf);
}

int main(int argc, char* argv[]) 
{ 
	int status;
/*
	const size_t arr_size = 50000;

	unsigned char orig_arr[arr_size] = { 5,4,3,2,1};
	unsigned char arr[arr_size];

	srand (time(NULL));

	for (int i = 0; i < arr_size; i++) {
		orig_arr[i] = rand();
	}

	memcpy(arr, orig_arr, sizeof(arr));
	quick_sort(arr, arr_size);

	for (int i = 1; i < arr_size; i++) {
		if (arr[i] < arr[i - 1]) {
			printf("ERROR\r\n");
			break;
		}
	}

	memcpy(arr, orig_arr, sizeof(arr));
	merge_sort(arr, arr_size);

	for (int i = 1; i < arr_size; i++) {
		if (arr[i] < arr[i - 1]) {
			printf("ERROR\r\n");
			break;
		}
	}

	return 0;
	*/
#ifndef _MSC_VER
	signal(SIGPIPE, SIG_IGN);
#endif
	signal(SIGTERM, ExitSignalHandler);
	signal(SIGINT, ExitSignalHandler);

	/*********************************************************************/
	/* validate ini file                                                 */
	/*********************************************************************/

	if (argc > 1)
		strncpy(config, argv[1], sizeof(config) - 1);

	if (config_validate(config) < 0)
		return -1;

	/*********************************************************************/
	/* load core settings                                                */
	/*********************************************************************/

	if (config_read_section(config, "core", config_handler) < 0)
		return -1;

	/*********************************************************************/
	/* init log subsystem                                                */
	/*********************************************************************/

	if (pid.length() > 0)
		demonize(pid.c_str());

	if (log_init(log_dir, log_commit_type, log_file_mode, log_buffer_len) < 0)
		return -1;

	jparse_init();

	if (db_open(database, password.c_str()))
		return -1;

#ifdef _MSC_VER
	WSADATA wsaData;
	if (WSAStartup(0x0101, &wsaData))  {
		log_printf("winsock not bi initialized !\r\n");
		WSACleanup();
	}
#endif

	log_printf("[CORE] Start\r\n");

	for (;;) {

		/*********************************************************************/
		/* disk subsystem                                                    */
		/*********************************************************************/

		if (storage_init(data_dir, data_commit_type, data_file_mode, data_buffer_len) < 0)
			break;

		/*********************************************************************/
		/* read modules list from ini file                                   */
		/*********************************************************************/

		if (config_read_section(config, "modules", modules_handler) < 0)
			break;

		/*********************************************************************/
		/* load each module                                                  */
		/*********************************************************************/

		for (size_t i = 0; i < arr_modules.size(); i++) {

			std::string module_path = modules_dir + "/" + arr_modules[i].path;

			arr_modules[i].handle = dlopen(module_path.c_str(), RTLD_NOW);

			if (arr_modules[i].handle == NULL) {
			
				const char *ptr = dlerror();

				if (ptr)
					log_printf("[CORE] Loading module '%s' `%s` failed with error: %s\r\n", arr_modules[i].name.c_str(), arr_modules[i].path.c_str(), ptr);
				else
					log_printf("[CORE] Loading module '%s' `%s` failed with unknown error\r\n", arr_modules[i].name.c_str(), arr_modules[i].path.c_str());

				continue;
			}

			arr_modules[i].get_var = (GETVAR)dlsym(arr_modules[i].handle, "get_var");

			if (arr_modules[i].get_var == NULL) {
	
				const char *ptr = dlerror();

				if (ptr)
					log_printf("[CORE] Loading module '%s' `%s` failed with error: %s", arr_modules[i].name.c_str(), arr_modules[i].path.c_str(), ptr);
				else
					log_printf("[CORE] Loading module '%s' `%s` failed with error: Unable to locate function 'get_var'", arr_modules[i].name.c_str(), arr_modules[i].path.c_str());

				continue;
			}

			arr_modules[i].set_var = (SETVAR)dlsym(arr_modules[i].handle, "set_var");

			if (arr_modules[i].set_var == NULL) {
	
				const char *ptr = dlerror();

				if (ptr)
					log_printf("[CORE] Loading module '%s' `%s` failed with error: %s", arr_modules[i].name.c_str(), arr_modules[i].path.c_str(), ptr);
				else
					log_printf("[CORE] Loading module '%s' `%s` failed with error: Unable to locate function 'set_var'", arr_modules[i].name.c_str(), arr_modules[i].path.c_str());

				continue;
			}

			arr_modules[i].start = (START)dlsym(arr_modules[i].handle, "start");

			if (arr_modules[i].start == NULL) {
	
				const char *ptr = dlerror();

				if (ptr)
					log_printf("[CORE] Loading module '%s' `%s` failed with error: %s", arr_modules[i].name.c_str(), arr_modules[i].path.c_str(), ptr);
				else
					log_printf("[CORE] Loading module '%s' `%s` failed with error: Unable to locate function 'start'", arr_modules[i].name.c_str(), arr_modules[i].path.c_str());

				continue;
			}

			arr_modules[i].stop = (STOP)dlsym(arr_modules[i].handle, "stop");

			if (arr_modules[i].stop == NULL) {
	
				const char *ptr = dlerror();

				if (ptr)
					log_printf("[CORE] Loading module '%s' `%s` failed with error: %s", arr_modules[i].name.c_str(), arr_modules[i].path.c_str(), ptr);
				else
					log_printf("[CORE] Loading module '%s' `%s` failed with error: Unable to locate function 'stop'", arr_modules[i].name.c_str(), arr_modules[i].path.c_str());

				continue;
			}

			log_printf("[CORE] Module '%s' loaded\r\n", arr_modules[i].name.c_str());
		}


		/*********************************************************************/
		/* delete incomplete/invalid modules                                 */
		/*********************************************************************/

		for (size_t i = 0; i < arr_modules.size(); i++) {

			if ((arr_modules[i].handle == NULL) || (arr_modules[i].start == NULL) ||
			    (arr_modules[i].stop == NULL) || (arr_modules[i].get_var == NULL) ||
			    (arr_modules[i].set_var == NULL)) {

			    arr_modules.erase(arr_modules.begin() + i);

			    i = -1;
			}
		}

		/*********************************************************************/
		/* FUEL MODULE                                                       */
		/*********************************************************************/

		if (!fuel.empty()) {

			std::string module_path = modules_dir + "/" + fuel;

			fuel_handle = dlopen(module_path.c_str(), RTLD_NOW);

			if (fuel_handle == NULL) {
			
				const char *ptr = dlerror();

				if (ptr)
					log_printf("[CORE] Loading module 'FUEL' `%s` failed with error: %s\r\n", module_path.c_str(), ptr);
				else
					log_printf("[CORE] Loading module 'FUEL' `%s` failed with unknown error\r\n", module_path.c_str());
			}
			else {

				fuel_proc_fuel_process = (FUEL_PROCESS)dlsym(fuel_handle, "fuel_process");

				if (fuel_proc_fuel_process == NULL) {
	
					const char *ptr = dlerror();

					if (ptr)
						log_printf("[CORE] Loading module 'FUEL' `%s` failed with error: %s\r\n", module_path.c_str(), ptr);
					else
						log_printf("[CORE] Loading module 'FUEL' `%s` failed with error: Unable to locate function 'fuel_process'\r\n", module_path.c_str());
				}
			}
		}

		/*********************************************************************/
		/* init modules                                                      */
		/*********************************************************************/

		for (module = 0; module < arr_modules.size(); module++) {

			arr_modules[module].set_var(MODULE_VAR_FUNC_LOG, (void *)log_printf);
			arr_modules[module].set_var(MODULE_VAR_FUNC_LISTEN_TCP, (void *)listen_tcp);
			arr_modules[module].set_var(MODULE_VAR_FUNC_LISTEN_UDP, (void *)listen_udp);
			arr_modules[module].set_var(MODULE_VAR_FUNC_SEND_TCP, (void *)send_tcp);
			arr_modules[module].set_var(MODULE_VAR_FUNC_SEND_UDP, (void *)send_udp);
			arr_modules[module].set_var(MODULE_VAR_FUNC_CLOSE_TCP, (void *)close_tcp);
			arr_modules[module].set_var(MODULE_VAR_TAG, (void *)&arr_modules[module]);

			arr_modules[module].set_var(MODULE_VAR_FUNC_STORAGE_CREATE_STREAM, (void *)storage_create_stream);
			arr_modules[module].set_var(MODULE_VAR_FUNC_STORAGE_DESTROY_STREAM, (void *)storage_destroy_stream);
			arr_modules[module].set_var(MODULE_VAR_FUNC_STORAGE_GET_STREAM_BY_ID, (void *)storage_get_stream_by_id);
			arr_modules[module].set_var(MODULE_VAR_FUNC_STORAGE_LOCK_STREAM, (void *)storage_lock_stream);
			arr_modules[module].set_var(MODULE_VAR_FUNC_STORAGE_UNLOCK_STREAM, (void *)storage_unlock_stream);
			arr_modules[module].set_var(MODULE_VAR_FUNC_STORAGE_ADD_RECORD_TO_STREAM, (void *)storage_add_record_to_stream);
			arr_modules[module].set_var(MODULE_VAR_FUNC_STORAGE_UPDATE_RECORD, (void *)storage_update_record);
			arr_modules[module].set_var(MODULE_VAR_FUNC_STORAGE_GET_STREAM_FIRST_RECORD, (void *)storage_get_stream_first_record);
			arr_modules[module].set_var(MODULE_VAR_FUNC_STORAGE_GET_STREAM_RECORDS_COUNT, (void *)storage_get_stream_records_count);
			arr_modules[module].set_var(MODULE_VAR_FUNC_STORAGE_SORT_STREAM, (void *)storage_sort_stream);
			arr_modules[module].set_var(MODULE_VAR_FUNC_STORAGE_TRIM_STREAM, (void *)storage_trim_stream);
			arr_modules[module].set_var(MODULE_VAR_FUNC_STORAGE_GET_STREAM_INFO, (void *)storage_get_stream_info);

			arr_modules[module].set_var(MODULE_VAR_FUNC_DB_GET_OBJECT, (void *)db_get_object);
			arr_modules[module].set_var(MODULE_VAR_FUNC_DB_PUT_OBJECT, (void *)db_put_object);
			arr_modules[module].set_var(MODULE_VAR_FUNC_DB_DELETE_OBJECT, (void *)db_delete_object);
			arr_modules[module].set_var(MODULE_VAR_FUNC_DB_UPDATE_OBJECT, (void *)db_update_object);
			arr_modules[module].set_var(MODULE_VAR_FUNC_DB_UPDATE_OBJECT_BLOB, (void *)db_update_object_module_data);
			arr_modules[module].set_var(MODULE_VAR_FUNC_DB_CHANGE_OBJECT_PARENT, (void *)db_change_object_parent);
			arr_modules[module].set_var(MODULE_VAR_FUNC_DB_MOVE_OBJECT, (void *)db_move_object);

			arr_modules[module].set_var(MODULE_VAR_FUNC_DB_ENUM_OBJECTS, (void *)db_enum_objects);
			arr_modules[module].set_var(MODULE_VAR_FUNC_DB_GET_ERROR, (void *)db_get_error);

			arr_modules[module].set_var(MODULE_VAR_FUNC_ENUM_MODULES, (void *)enum_modules);
			arr_modules[module].set_var(MODULE_VAR_FUNC_GET_DEVICE_MODULE, (void *)get_device_module);

			arr_modules[module].get_var(MODULE_VAR_FUNC_TIMER, (void **)&arr_modules[module].timer);
			if (arr_modules[module].timer == NULL)
				arr_modules[module].timer = dummy_timer;

			arr_modules[module].get_var(MODULE_VAR_FAMILY, (void **)&arr_modules[module].family);

			arr_modules[module].set_var(MODULE_VAR_FUNC_FUEL_PROCESS, (fuel_proc_fuel_process) ? (void *)fuel_proc_fuel_process : (void *)fuel_process);

			if (arr_modules[module].family == MODULE_FAMILY_DEVICE) {
				arr_modules[module].get_var(MODULE_VAR_TERMINAL_TYPE, (void **)&arr_modules[module].terminal_type);
				arr_modules[module].get_var(MODULE_VAR_FUNC_CONFIG_GET_JSON, (void **)&arr_modules[module].config_get_json);
				arr_modules[module].get_var(MODULE_VAR_FUNC_CONFIG_PUT_JSON, (void **)&arr_modules[module].config_put_json);
				arr_modules[module].get_var(MODULE_VAR_FUNC_GET_ERROR, (void **)&arr_modules[module].config_get_error);
				arr_modules[module].get_var(MODULE_VAR_FUNC_GET_DEVICE_CAPS, (void **)&arr_modules[module].config_get_device_caps);
				arr_modules[module].get_var(MODULE_VAR_FUNC_ON_OBJECT_CREATE, (void **)&arr_modules[module].on_object_create);
				arr_modules[module].get_var(MODULE_VAR_FUNC_ON_OBJECT_REMOVE, (void **)&arr_modules[module].on_object_remove);
				arr_modules[module].get_var(MODULE_VAR_FUNC_ON_OBJECT_UPDATE, (void **)&arr_modules[module].on_object_update);
				arr_modules[module].get_var(MODULE_VAR_FUNC_ON_TIMER, (void **)&arr_modules[module].timer);
				arr_modules[module].get_var(MODULE_VAR_FUNC_GET_INFO, (void **)&arr_modules[module].get_info);
			}
			else
			if (arr_modules[module].family == MODULE_FAMILY_RETRANSLATOR) {
				arr_modules[module].get_var(MODULE_VAR_FUNC_CREATE_RETRANSLATOR, (void **)&arr_modules[module].create_retranslator);
				arr_modules[module].get_var(MODULE_VAR_FUNC_DESTROY_RETRANSLATOR, (void **)&arr_modules[module].destroy_retranslator);
				arr_modules[module].get_var(MODULE_VAR_FUNC_ADD_RECORD_TO_RETRANSLATOR, (void **)&arr_modules[module].add_record_to_retranslator);
				arr_modules[module].get_var(MODULE_VAR_PROTOCOL_ID, (void **)&arr_modules[module].protocol_id);
			}

			if (config_read_section(config, arr_modules[module].name.c_str(), module_handler) < 0)
				break;
		}

		if (db_load())
			break;

		/*********************************************************************/
		/* Create kqueue                                                     */
		/*********************************************************************/

		kq = kqueue();

		if (kq == -1) {
	
			log_printf("[CORE] errno #%d on creating kqueue\r\n", errno);
		
			break;
		}

		/*********************************************************************/
		/* Create kqueue input array: worst case 1 timer + maximum sockets  */
		/*********************************************************************/

		inqueue_size = 1 + sysconf(_SC_OPEN_MAX);

		inqueue = (struct kevent *)malloc(inqueue_size * sizeof(struct kevent));
       	
		if (inqueue == NULL) {
	
			log_printf("[CORE] Unable to allocate kqueue input array\r\n");
       		
			break;
		}

		/*********************************************************************/
		/* Create kqueue output array: worst case 1 timer + maximum sockets  */
		/*********************************************************************/

		outqueue_size = 1 + sysconf(_SC_OPEN_MAX);

		outqueue = (struct kevent *)malloc(outqueue_size * sizeof(struct kevent));
       	
		if (outqueue == NULL) {
	
			log_printf("[CORE] Unable to allocate kqueue output array\r\n");
       		
			break;
		}

		/*********************************************************************/
		/* Register 1000ms timer in kqueue                                   */
		/*********************************************************************/

		EV_SET(inqueue, 1, EVFILT_TIMER, EV_ADD, 0, 999, 0); 
		status = kevent(kq, inqueue, 1, NULL, 0, NULL);

		if (status == -1) {

			log_printf("[CORE] errno #%d on adding timer to kqueue\r\n", errno);

			break;
		}

		storage_create_stream(1, 32 * 1024 * 1024);

		storage_load(0);

		/*********************************************************************/
		/* start modules                                                     */
		/*********************************************************************/

		for (size_t i = 0; i < arr_modules.size(); i++) {
			arr_modules[i].start();
		}

		/*********************************************************************/
		/* switch group and user                                             */
		/*********************************************************************/

#ifndef _MSC_VER
		if (strcmp(grp, "*")) {
			
			struct group *gr;
   
			if (!(gr = getgrnam(grp))) {
				log_printf("Couldn't get group id for group '%s'", grp);
			}
			else {
				if (setgid(gr->gr_gid)) {
					log_printf("Couldn't switch to group '%s', errno = %d", grp, errno);
				}
			}
			endgrent();
		}

		if (strcmp(user, "*")) {
		
			struct passwd *pw;
   
			if (!(pw = getpwnam(user))) {
				log_printf("Couldn't get user id for user '%s'", user);
			}   	
			else {
				if (setuid(pw->pw_uid)) {
					log_printf("Couldn't switch to user '%s', errno = %d", user, errno);
				}
			}
			endpwent();
		}
#endif

		//--------------------------------------------------------------
		// main loop                                                         
		//--------------------------------------------------------------

		log_printf("[CORE] Run\r\n");

		inqueue_count = 0;

		std::map<SESSION_HANDLERS *, SESSION_DATA> reserv;
		spinlock_init(&spinlock);

		while (bContinueToWork) {

			//log_printf("[CORE] enter kevent %u\r\n", inqueue_count);
			int events_count = kevent(kq, inqueue, inqueue_count, outqueue, outqueue_size, NULL);
			//log_printf("[CORE] kevent done %u\r\n", events_count);

			if (events_count == -1) {
		
				if (errno != EINTR)
					log_printf("[CORE] errno #%d on kevent\r\n", errno);

				break;
			}

			event = outqueue;
			inqueue_count = 0;

			reserv.clear();

			spinlock_lock(&spinlock);
			
			log_printf("[CORE] snapshot\r\n", errno);
			for (std::list<SESSION_HANDLERS *>::const_iterator iterator = sessions.begin(), end = sessions.end(); iterator != end; ++iterator) {
				reserv[*iterator] = (*iterator)->data;
			}					
			
			spinlock_unlock(&spinlock);

			for (int iEvent = 0; iEvent < events_count; iEvent++, event++) {

				spinlock_lock(&spinlock);

				for (std::list<SESSION_HANDLERS *>::const_iterator iterator = sessions.begin(), end = sessions.end(); iterator != end; ++iterator) {
					if ((*iterator)->data != reserv[*iterator])
						if (reserv[*iterator] != 0)
							log_printf("[CORE] corrupt #%u, was 0x%08X, now 0x%08X\r\n", event->ident, reserv[*iterator], (*iterator)->data);
				}					

				log_printf("[CORE] snapshot\r\n", errno);
	
				reserv.clear();

				for (std::list<SESSION_HANDLERS *>::const_iterator iterator = sessions.begin(), end = sessions.end(); iterator != end; ++iterator) {
					reserv[*iterator] = (*iterator)->data;
				}					

				spinlock_unlock(&spinlock);
				
				log_printf("[CORE] event %u\r\n", iEvent);
				log_printf("[CORE] data %08X\r\n", (intptr_t)event->udata);
				
				intptr_t data = (intptr_t)event->udata;

				bool closed = false;
       				
				//----------------------------------------------
				// Primary TCP socket
				//----------------------------------------------
       			if (likely((data & 0x03) == 0x01)) {
       					
       				data--;

					SESSION_HANDLERS *handlers = (SESSION_HANDLERS *)data;

					//----------------------------------------------
					// Accept
					//----------------------------------------------
					while (event->data--) {

						remote_len = sizeof(remote);

						int sock = accept(event->ident, (struct sockaddr*)&remote, &remote_len);

						if (unlikely(sock == -1)) {
							log_printf("[CORE] Accept failed, errno - %d\r\n", errno);
							continue;
						}

						if (likely(getnameinfo((struct sockaddr*)&remote, remote_len, host, sizeof(host), port, sizeof(port), NI_NUMERICHOST | NI_NUMERICSERV) == 0))
							log_printf("[CORE] Accepted connection on socket #%u from '%s':'%s', socket: #%d\r\n", event->ident, host, port, sock);
						else
							log_printf("[CORE] Accepted connection, socket: #%d\r\n", sock);
				
						void *session = handlers->open();

						if (session == NULL) {
							log_printf("[CORE] Failed to start session\r\n");
							closesocket(sock);
							continue;
						}

						*(SESSION_HANDLERS *)session = *handlers;
						((SESSION_HANDLERS *)session)->sock = sock;
						((SESSION_HANDLERS *)session)->buffer = NULL;
						((SESSION_HANDLERS *)session)->buffer_size = 0;

						spinlock_lock(&spinlock);
						sessions.push_back((SESSION_HANDLERS *)session);
						spinlock_unlock(&spinlock);

						intptr_t d = (intptr_t)session;
						
						d += 2;

						log_printf("[CORE] Session address: %08X\r\n", d);
						
						EV_SET(&inqueue[inqueue_count++], sock, EVFILT_READ, EV_ADD, 0, 0, (void *)d); 
						EV_SET(&inqueue[inqueue_count++], sock, EVFILT_TIMER, EV_ADD, 0, 14999, (void *)d); 
					}
				}
				else 

				//----------------------------------------------
				// Session TCP socket
				//----------------------------------------------
				if (likely((data & 0x03) == 0x02)) {

       				data -= 2;

					SESSION_HANDLERS *handlers = (SESSION_HANDLERS *)data;

					//----------------------------------------------
					// Socket timer
					//----------------------------------------------

					if (unlikely(event->filter == EVFILT_TIMER)) {

						log_printf("[CORE] Timer for socket #%d\r\n", event->ident);

						char *ptr = buffer;

						buffer_len = sizeof(buffer);
						int res = handlers->timer(handlers, &ptr, &buffer_len);

						if (buffer_len != 0) {

							log_printf("[CORE] Sending %u bytes to #%u\r\n", buffer_len, event->ident);

							int sent = send(event->ident, ptr, buffer_len, 0);

							if ((sent >= 0)&&(sent != buffer_len)&&(handlers->buffer == NULL)) {
								EV_SET(&inqueue[inqueue_count++], event->ident, EVFILT_WRITE, EV_ADD, 0, 0, (SESSION_HANDLERS *)(data + 2));
								handlers->buffer_size = buffer_len - sent;
								handlers->buffer = (unsigned char *)malloc(handlers->buffer_size);
								memcpy(handlers->buffer, ptr + sent, handlers->buffer_size);
								log_printf("[CORE] Not fully sent (%u of %u), buffer %08X\r\n", sent, buffer_len, handlers->buffer);
							}
						}
							
						if (res == SESSION_COMPLETE) {
							
							spinlock_lock(&spinlock);
							sessions.remove(handlers);
							spinlock_unlock(&spinlock);

							if (handlers->buffer != NULL) {
								free(handlers->buffer);
								handlers->buffer = NULL;
							}
							
							log_printf("[CORE] Closing socket #%d\r\n", event->ident);
							closesocket(event->ident);
							EV_SET(&inqueue[inqueue_count++], event->ident, EVFILT_TIMER, EV_DELETE, 0, 0, NULL); 
							EV_SET(&inqueue[inqueue_count++], event->ident, EVFILT_READ, EV_DELETE, 0, 0, NULL); 
							EV_SET(&inqueue[inqueue_count++], event->ident, EVFILT_WRITE, EV_DELETE, 0, 0, NULL); 
							closed = true;
						}
						else {
							EV_SET(&inqueue[inqueue_count++], event->ident, EVFILT_TIMER, EV_DELETE, 0, 0, NULL); 
							EV_SET(&inqueue[inqueue_count++], event->ident, EVFILT_TIMER, EV_ADD, 0, res * 1000 - 1, (SESSION_HANDLERS *)(data + 2)); 
							log_printf("[CORE] Timeout for socket #%d to %u\r\n", event->ident, res);
						}
					}
					else
					if (unlikely(event->filter == EVFILT_WRITE)) {
								
						if ((event->flags & EV_EOF) == 0) {

							log_printf("[CORE] Countinue send %u bytes to #%u\r\n", handlers->buffer_size, event->ident);

							int sent = send(event->ident, (char *)handlers->buffer, handlers->buffer_size, 0);

							if (sent >= 0) {

								if (sent != handlers->buffer_size) {
									log_printf("[CORE] Not fully sent (%u of %u)\r\n", sent, handlers->buffer_size);
									memcpy(handlers->buffer, handlers->buffer + sent, handlers->buffer_size - sent);
									handlers->buffer_size -= sent;
								}
								else {
									log_printf("[CORE] Send done, buffer %08X\r\n", handlers->buffer);
									free(handlers->buffer);
									handlers->buffer = NULL;
									EV_SET(&inqueue[inqueue_count++], event->ident, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
								}
							}
						}
					}
					else {

						//----------------------------------------------
						// Socket has data
						//----------------------------------------------
						
						if (likely(event->data > 0)) {

							log_printf("[CORE] Handlers address: %08X, %08X\r\n", data, handlers->data);

							int recv_len = recv(event->ident, buffer, sizeof(buffer), 0);

							if (recv_len <= 0) {
								event->flags |= EV_EOF;						
							}
							else {

								buffer_len = recv_len;

								log_printf("[CORE] Received %u bytes from socket #%d\r\n", buffer_len, event->ident);

								char *ptr = buffer;

								int res = handlers->data(handlers, &ptr, &buffer_len);
	
								if (buffer_len > 0) {

									log_printf("[CORE] Sending %u bytes to #%u\r\n", buffer_len, event->ident);

									int sent = send(event->ident, ptr, buffer_len, 0);

									if ((sent >= 0)&&(sent != buffer_len)&&(handlers->buffer == NULL)) {
										EV_SET(&inqueue[inqueue_count++], event->ident, EVFILT_WRITE, EV_ADD, 0, 0, (SESSION_HANDLERS *)(data + 2));
										handlers->buffer_size = buffer_len - sent;
										handlers->buffer = (unsigned char *)malloc(handlers->buffer_size);
										log_printf("[CORE] Not fully sent (%u of %u)\r\n", sent, handlers->buffer_size);
										memcpy(handlers->buffer, ptr + sent, handlers->buffer_size);
									}
								}
	
								if (res == SESSION_COMPLETE) {

									spinlock_lock(&spinlock);
									sessions.remove(handlers);
									spinlock_unlock(&spinlock);

									if (handlers->buffer != NULL) {
										free(handlers->buffer);
										handlers->buffer = NULL;
									}

									log_printf("[CORE] Closing socket #%d\r\n", event->ident);
									closesocket(event->ident);
									EV_SET(&inqueue[inqueue_count++], event->ident, EVFILT_TIMER, EV_DELETE, 0, 0, NULL); 
									EV_SET(&inqueue[inqueue_count++], event->ident, EVFILT_READ, EV_DELETE, 0, 0, NULL); 
									EV_SET(&inqueue[inqueue_count++], event->ident, EVFILT_WRITE, EV_DELETE, 0, 0, NULL); 
									closed = true;
								}
								else {
									EV_SET(&inqueue[inqueue_count++], event->ident, EVFILT_TIMER, EV_DELETE, 0, 0, NULL); 
									EV_SET(&inqueue[inqueue_count++], event->ident, EVFILT_TIMER, EV_ADD, 0, res * 1000 - 1, (SESSION_HANDLERS *)(data + 2)); 
									log_printf("[CORE] Timeout for socket #%d to %u\r\n", event->ident, res);
								}
							}
						}
					
						//----------------------------------------------
						// Socket is closed
						//----------------------------------------------
						if ((event->flags & EV_EOF)&&(closed == false)) {
							
							log_printf("[CORE] Connection with socket #%d interrupted %08X\r\n", event->ident, handlers->buffer);

							if (handlers->buffer != NULL) {
								free(handlers->buffer);
								handlers->buffer = NULL;
							}
														
							closesocket(event->ident);

							EV_SET(&inqueue[inqueue_count++], event->ident, EVFILT_TIMER, EV_DELETE, 0, 0, NULL); 
							EV_SET(&inqueue[inqueue_count++], event->ident, EVFILT_READ, EV_DELETE, 0, 0, NULL); 
							EV_SET(&inqueue[inqueue_count++], event->ident, EVFILT_WRITE, EV_DELETE, 0, 0, NULL); 
							
							spinlock_lock(&spinlock);
							sessions.remove(handlers);
							spinlock_unlock(&spinlock);

							handlers->close(handlers);
						}
					}
				}
				else
				//----------------------------------------------
				// UDP socket
				//----------------------------------------------
				if (likely((data & 0x03) == 0x03)) {

					UDP_DATA handler = (UDP_DATA)(data - 3);

					udp_context.sock = event->ident;
					udp_context.remote_len = sizeof(udp_context.remote);
					
					buffer_len = recvfrom(event->ident, buffer, sizeof(buffer), 0, (struct sockaddr *)&udp_context.remote, &udp_context.remote_len);

					if (buffer_len != SIZE_MAX) {
	
						if (1) {
						
							char addr[NI_MAXHOST];
							char port[NI_MAXSERV];

							addr[sizeof(host) - 1] = '\0';
							port[sizeof(port) - 1] = '\0';

							if (getnameinfo((struct sockaddr *)&udp_context.remote, udp_context.remote_len, addr, sizeof(addr), port, sizeof(port), NI_NUMERICHOST | NI_NUMERICSERV) != 0) {

								*addr = '\0';
								*port = '\0';
							}

							log_printf("[CORE] Received datagram (%u bytes) from socket #%d, %s:%s\r\n", buffer_len, event->ident, addr, port);
						}

						char *ptr = buffer;
							
						int res = handler((unsigned char **)&ptr, &buffer_len, &udp_context, sizeof(udp_context));
	
						if (buffer_len != 0) {
							int res = sendto(event->ident, ptr, buffer_len, 0, (struct sockaddr *)&udp_context.remote, udp_context.remote_len);
							log_printf("[CORE] Send reply with %u bytes, result code: #%d\r\n", buffer_len, res);
						}
					}
				}
				else

				//----------------------------------------------
				// 1000ms timer                              
				//----------------------------------------------
	       	
	       			if (event->filter == EVFILT_TIMER) {
       		
        				if (event->ident == 1) {
               
						log_1sec_timer();
						storage_1sec_timer();

						for (size_t i = 0; i < arr_modules.size(); i++) {
							arr_modules[i].timer();
						}
					}
				}

		
			}

			spinlock_lock(&spinlock);

			for (std::list<SESSION_HANDLERS *>::const_iterator iterator = sessions.begin(), end = sessions.end(); iterator != end; ++iterator) {
				if ((*iterator)->data != reserv[*iterator])
					if (reserv[*iterator] != 0)
						log_printf("[CORE] corrupt2 #%u, was 0x%08X, now 0x%08X\r\n", event->ident, reserv[*iterator], (*iterator)->data);
			}					

			spinlock_unlock(&spinlock);
        }

        break;
	}

	/*********************************************************************/
	/* stop and unload modules                                           */
	/*********************************************************************/

	for (size_t i = 0; i < arr_modules.size(); i++) {
		arr_modules[i].stop();
		dlclose(arr_modules[i].handle);
	}

	if (fuel_handle) {
		dlclose(fuel_handle);
	}
	/*********************************************************************/
	/* cleanup                                                           */
	/*********************************************************************/

	if (outqueue != NULL)
		free(outqueue);

	if (inqueue != NULL)
		free(inqueue);

	storage_cleanup();

	db_close();

	jparse_destroy();

#ifdef _MSC_VER
	WSACleanup();
#endif

	log_printf("[CORE] Stop\r\n");

	log_cleanup();
}
