//******************************************************************************
//
// File Name : dirscan.h
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#include <string.h>
#include <string>
#include "dirscan.h"

#ifdef _MSC_VER

#include <windows.h>

int dirscan(const char *dir_path, DIRSCAN_CALLBACK callback, void *ctx)
{
	WIN32_FIND_DATAA ffd;
	HANDLE hFind;
	char path[MAX_PATH];

	size_t len = strlen(dir_path);
	memcpy(path, dir_path, len);

	path[len + 0] = '\\';
	path[len + 1] = '*';
	path[len + 2] = '\0';

	hFind = FindFirstFileA(path, &ffd);

	if (hFind == INVALID_HANDLE_VALUE) {
		return -1;
	} 
   
	do {
		if ((ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
			if (callback(ffd.cFileName, ctx)) {
				FindClose(hFind);
				return -1;
			}
		}
	} while (FindNextFileA(hFind, &ffd) != 0);

	FindClose(hFind);

	return 0;
}

#else

#include <dirent.h>

int dirscan(const char *dir_path, DIRSCAN_CALLBACK callback, void *ctx)
{
	DIR *dp; 
	struct dirent *ep;      

	dp = opendir(dir_path);  

	if (dp != NULL) { 
	
		while ((ep = readdir (dp)) != NULL) {
		
			if (strcmp(ep->d_name, ".")&&strcmp(ep->d_name, "..")) {

				if (callback(ep->d_name, ctx)) {
					closedir(dp); 
					return -1;
				}
			}
		}
		
		closedir(dp); 
	}
}

#endif
