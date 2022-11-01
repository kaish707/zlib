
// zlib_first_include.h
// to include ->  #include "zlib_first_include.h"
// created on 2018 Feb 11

#ifndef zlib_first_include_h
#define zlib_first_include_h

#ifdef _WIN32
#include <io.h>
#define write _write
#else
#include <sys/types.h>
#include <unistd.h>
#endif


#if defined(_MSC_VER) & (_MSC_VER>1400)
#pragma warning (disable:4996)

#ifdef _WIN64
#pragma warning (disable:4267)  // '=': conversion from 'size_t' to 'unsigned int', possible loss of data
#endif

#endif


#endif  // #ifndef zlib_first_include_h
