
// zlib_compress_decompress_common.cpp
// 2018 Feb 12

#include "zlib_compress_decompress_common.h"
#include <stdlib.h>
#include <search.h>
#include <memory.h>

#ifdef __cplusplus
extern "C" {
#endif

SCompressDecompressHeader* ZlibCreateCompressDecompressHeader(uint32_t a_headerSize, uint32_t a_typeOfCompressedContent,uint32_t a_numberOfItems)
{
	SCompressDecompressHeader* pCompressDecompressHeader = (SCompressDecompressHeader*)malloc(a_headerSize);
	if (!pCompressDecompressHeader) { return pCompressDecompressHeader; }
	pCompressDecompressHeader->version = ZLIB_COMPR_DECOMPR_VERSION;
	pCompressDecompressHeader->numberOfItems = a_numberOfItems;
	pCompressDecompressHeader->wholeHeaderSizeInBytes = a_headerSize;
	pCompressDecompressHeader->typeOfCompressedContent = a_typeOfCompressedContent;
	return pCompressDecompressHeader;
}


SCompressDecompressHeader* ZlibCreateAndCopyComprDecomprHeader(const SCompressDecompressHeader* a_orgin, int a_nAll)
{
	SCompressDecompressHeader* pCompressDecompressHeader = (SCompressDecompressHeader*)malloc(a_orgin->wholeHeaderSizeInBytes);
	size_t unSize = a_nAll ? a_orgin->wholeHeaderSizeInBytes : sizeof(SCompressDecompressHeader);

	if (!pCompressDecompressHeader) { return pCompressDecompressHeader; }
	memcpy(pCompressDecompressHeader, a_orgin, unSize);
	return pCompressDecompressHeader;
}


void DestroyCompressDecompressHeader(SCompressDecompressHeader* a_header)
{
	free(a_header);
}


#ifdef __cplusplus
}
#endif
