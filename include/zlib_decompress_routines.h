
// zlib_decompress_routines.h
// to include ->  #include "zlib_decompress_routines.h"
// 2018 Feb 12

#ifndef __zlib_decompress_routines_h__
#define __zlib_decompress_routines_h__

#include <zlib_compress_decompress_common.h>


#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#else
#endif

// return 0, continues, non 0 stops
#ifndef typeDecompressCallback_defined
typedef int(*typeDecompressCallback)(const void*buffer, int bufLen, void*userData);
#define typeDecompressCallback_defined
#endif

int ZlibDecompressBufferToCallback(
	z_stream* a_strm,
	void* a_out, int a_outBufferSize,
	typeDecompressCallback a_clbk, void* a_userData);
int ZlibDecompressBufferToFile(
	z_stream* a_strm,
	void* a_out, int a_outBufferSize,
	FILE *a_dest);
int ZlibDecompressFile(FILE *source, FILE *dest);
int ZlibDecompressFolder(FILE *a_source, const char* a_outDirectoryPath);
int ZlibDecompressFromWeb(const char *a_webUri, const char* a_outDirectoryPath);

#ifdef _WIN32
int ZlibBurnImageFromWeb(const char *a_cpcUrl, HANDLE a_drive, __int64 a_nDiskSize);
#else
#endif

#if 0
int ZlibDecompressWebToCallback(
	z_stream* a_strm,
	HINTERNET a_source,
	void* a_in, int a_inBufferSize,
	void* a_out, int a_outBufferSize,
	typeDecompressCallback a_clbk, void* a_userData);
#endif

#ifdef __cplusplus
}
#endif


#endif  // #ifndef __zlib_decompress_routines_h__
