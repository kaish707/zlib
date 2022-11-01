
/*
 *	File: director_iterator_win.cpp
 *
 *	Created on: 2017 Feb 11
 *	Author    : Davit Kalantaryan (Email: davit.kalantaryan@desy.de)
 *
 *  This file implements all functions connected to posix threading
 *		1)
 *
 *
 */

#include "common/util/directory_iterator.h"
#include <string.h>
#include <Windows.h>
#include <stdio.h>


#ifdef _MSC_VER
#if(_MSC_VER >= 1400)
#pragma warning(disable : 4996)
#endif
#endif



#ifdef __cplusplus
extern "C" {
#endif

int IterateOverDirectoryFiles(const char* a_sourceDirectory, TypeIterFunc a_callback,void* a_ud,int* a_bSubDirs)
{
	HANDLE				hFile;                       // Handle to directory
	WIN32_FIND_DATAA	FileInformation;             // File information
	int					nReturn, nIsDir;
	char				vcStrPattern[MAX_PATH];
	char				vcStrFilePath[MAX_PATH];

	if (!(GetFileAttributesA(a_sourceDirectory)&FILE_ATTRIBUTE_DIRECTORY)){return -1;}

	//strPattern = sourceDirectory + "\\*.*";
	_snprintf(vcStrPattern,MAX_PATH,"%s\\*.*",a_sourceDirectory);
	
	hFile = ::FindFirstFileA(vcStrPattern, &FileInformation);
	if (hFile != INVALID_HANDLE_VALUE){
		do{
			if(FileInformation.cFileName[0] == '.'){
				if ((FileInformation.cFileName[1] == 0)||( (FileInformation.cFileName[1]== '.')&&(FileInformation.cFileName[2]==0) )){continue;}
			}
			if (FileInformation.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY){nIsDir=1;}
			else { nIsDir = 0; }

			nReturn = (*a_callback)(a_sourceDirectory, &FileInformation, a_ud,nIsDir);
			if (nReturn < 0){ if(nReturn==_STOP_FOR_CUR_DIR_){return 0;}return nReturn; }

			if (nIsDir&&(*a_bSubDirs)){
				//strFilePath = sourceDirectory + "\\" + FileInformation.cFileName;//strTargetFilePath
				_snprintf(vcStrFilePath, MAX_PATH,"%s\\%s",a_sourceDirectory, FileInformation.cFileName);
				int iRC = IterateOverDirectoryFiles(vcStrFilePath, a_callback, a_ud,a_bSubDirs);
				if (iRC){ return iRC; }
			}  // if (nIsDir&&(*a_bSubDirs)){

		} while (::FindNextFileA(hFile, &FileInformation) == TRUE);

		// Close handle
		::FindClose(hFile);
	}

	return 0;
}

#ifdef __cplusplus
}
#endif
