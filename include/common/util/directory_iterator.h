
// directory_iterator.h
// to include    ->  #include "util/directory_iterator.h"
// created on 2018 Feb 11

#ifndef __directory_iterator_h__
#define __directory_iterator_h__

#ifdef __cplusplus
extern "C" {
#endif

#define _STOP_FOR_CUR_DIR_	-2018

#ifdef _WIN32
#define _FILE_FROM_PATH_B_(_char_ptr_,__path__)	(  ( (_char_ptr_)=strrchr((__path__),'\\') ) ? (((_char_ptr_))+1) : (__path__)   )
typedef struct _WIN32_FIND_DATAA	FIND_DATAA;
#else
#define _FILE_FROM_PATH_B_(_char_ptr_,__path__)	(  ( (_char_ptr_)=strrchr((__path__),'/') ) ? (((_char_ptr_))+1) : (__path__)   )
#endif

// non 0-means stop
typedef int(*TypeIterFunc)(const char* dir, const FIND_DATAA* file_info, void*user,int isDir);

int IterateOverDirectoryFiles(const char* directory, TypeIterFunc callback, void* ud, int* subDirs);

#ifdef __cplusplus
}
#endif

#endif  // #ifndef __directory_iterator_h__
