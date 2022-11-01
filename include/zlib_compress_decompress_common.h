
// zlib_compress_decompress_common.h
// to include  ->  #include "zlib_compress_decompress_common.h"
// 2018 Feb 12

#ifndef __zlib_compress_decompress_common_h__
#define __zlib_compress_decompress_common_h__

#include <stdint.h>
#include <zlib.h>
#include <stdio.h>

#define ZLIB_COMPR_DECOMPR_VERSION	1

#define SIZE_OF_MAIN_HEADER			32  // B

#define LAST_STRING_IN_THE_FILE		"End"
#define LAST_STRING_IN_THE_FILE_LEN	4

#define DEF_CHUNK_SIZE				16384


#ifdef __cplusplus
extern "C"{
#endif

#ifdef _WIN32
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#else
#endif

enum TypeOfCompressedContent{CompressedContentDirectory,CompressedContentFile};

// padding is 8
typedef struct SFileItem
{
	uint32_t fileSize;
	uint16_t folderNum, fileNameLen/* increased to 8* */;
}SFileItem;

typedef struct SCompressDecompressHeader
{
	uint32_t version;
	uint32_t wholeHeaderSizeInBytes;
	uint32_t typeOfCompressedContent;
	uint32_t numberOfItems;
	uint32_t vReserved[4];
}SCompressDecompressHeader;


#define LEN_FROM_ITEM(_item)	(  sizeof(SFileItem)+(_item)->fileNameLen    )
#define ITEM_NAME(_item)		(  ((char*)(_item))+sizeof(SFileItem)  )

SCompressDecompressHeader* ZlibCreateCompressDecompressHeader(uint32_t headerSize, uint32_t typeOfCompressedContent, uint32_t numberOfItems);
SCompressDecompressHeader* ZlibCreateAndCopyComprDecomprHeader(const SCompressDecompressHeader* orgin, int a_nAll);
void DestroyCompressDecompressHeader(SCompressDecompressHeader* header);


#ifdef __cplusplus
}
#endif


#endif  // #ifndef __zlib_compress_decompress_common_h__
