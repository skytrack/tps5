//******************************************************************************
//
// File Name : cross.h
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#ifndef _CROSS_H

#define _CROSS_H

typedef int (*ENUMFOLDER_CALLBACK)(const char *lpszFileName);

#include <string>

#ifdef _MSC_VER
#include <windows.h>
#include <io.h>
#define close _close
#define write _write
#define read _read
#define open _open
#define localtime_r(a,b) localtime_s(b, a);
#define gmtime_r(a,b) gmtime_s(b, a);
#define strerror_r(errno,buf,len) strerror_s(buf,len,errno)
#define O_WRONLY _O_WRONLY
#define O_APPEND _O_APPEND
#define O_CREATE _O_CREATE
#define PMODE (_S_IREAD  | _S_IWRITE)
#define O_SYNC 0
#define O_DIRECT 0

#define snprintf c99_snprintf

inline int c99_vsnprintf(char* str, size_t size, const char* format, va_list ap)
{
    int count = -1;

    if (size != 0)
        count = _vsnprintf_s(str, size, _TRUNCATE, format, ap);
    if (count == -1)
        count = _vscprintf(format, ap);

    return count;
}

inline int c99_snprintf(char* str, size_t size, const char* format, ...)
{
    int count;
    va_list ap;

    va_start(ap, format);
    count = c99_vsnprintf(str, size, format, ap);
    va_end(ap);

    return count;
}

static int fsync (int fd)
{
	HANDLE h = (HANDLE) _get_osfhandle (fd);
	DWORD err;

	if (h == INVALID_HANDLE_VALUE)
	{
		errno = EBADF;
		return -1;
	}

	if (!FlushFileBuffers (h))
	{
		/* Translate some Windows errors into rough approximations of Unix
		* errors.  MSDN is useless as usual - in this case it doesn't
		* document the full range of errors.
		*/
		err = GetLastError ();
		switch (err)
		{
		case ERROR_ACCESS_DENIED:
			/* For a read-only handle, fsync should succeed, even though we have
			no way to sync the access-time changes.  */
			return 0;

		/* eg. Trying to fsync a tty. */
		case ERROR_INVALID_HANDLE:
			errno = EINVAL;
			break;

		default:
			errno = EIO;
		}
		return -1;
	}

	return 0;
}

static int scan_folder(const char *lpszStartFolder, ENUMFOLDER_CALLBACK pCallBack)
{
	WIN32_FIND_DATAA ffd;
	HANDLE hFind;

	std::string szFindPattern = lpszStartFolder;
	szFindPattern += "\\*";

	hFind = FindFirstFileA(szFindPattern.c_str(), &ffd);

	if (hFind == INVALID_HANDLE_VALUE) {
		return -1;
	} 
   
	do {
		
		szFindPattern = lpszStartFolder;
		szFindPattern += "\\";
		szFindPattern += ffd.cFileName;

		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			if ((ffd.cFileName[0] != '.')||(ffd.cFileName[1] != '\0')&&
				((ffd.cFileName[0] != '.')||(ffd.cFileName[1] != '.')||(ffd.cFileName[2] != '\0'))) {

				if (scan_folder(szFindPattern.c_str(), pCallBack) != 0)
					return -1;
			}
		}
		else {
			pCallBack(szFindPattern.c_str());
		}

	} while (FindNextFileA(hFind, &ffd) != 0);

	FindClose(hFind);

	return 0;
}

static time_t timegm(struct tm * a_tm)
{
    time_t ltime = mktime(a_tm);
    struct tm tm_val;
    gmtime_s(&tm_val, &ltime);
    int offset = (tm_val.tm_hour - a_tm->tm_hour);
    if (offset > 12)
    {
        offset = 24 - offset;
    }
    time_t utc = mktime(a_tm) - offset * 3600;
    return utc;
}

#define RTLD_NOW 0
#define dlopen(a, b) LoadLibraryA(a)
#define dlsym(a, b) GetProcAddress(a, b);
static const char *dlerror() { return NULL; }
#define dlclose(a) FreeLibrary(a)
#define _SC_OPEN_MAX 1000
#define sysconf(a) (a)

#else
#define PMODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define _O_BINARY 0

#include <dirent.h>
#include <unistd.h> 

#define closesocket close

static int scan_folder(const char *lpszStartFolder, ENUMFOLDER_CALLBACK pCallBack)
{
	DIR *dp; 
	struct dirent *ep;      

	std::string s;

	dp = opendir(lpszStartFolder);  

	if (dp != NULL) { 
	
		while ((ep = readdir (dp)) != NULL) {
		
			if (strcmp(ep->d_name, ".")&&strcmp(ep->d_name, "..")) {

				s = lpszStartFolder;
				s += "/";
				s += ep->d_name;

				pCallBack(s.c_str());
			}
		}
		
		closedir(dp); 
	}
}

#endif

#endif

// End
