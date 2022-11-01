
// zlib_compress_decompress_common.h
// to include  ->  #include "zlib_compress_decompress_common.h"
// 2018 Feb 12

#ifndef qt_zlib_compress_decompress_common_hpp
#define qt_zlib_compress_decompress_common_hpp


#include <zlib.h>
#include <stdint.h>
#include <stddef.h>
#include <QString>

struct SRawCompressHeader{
	uint16_t	endian;
	union{
		struct{
			uint16_t	isDirectory : 1;
			uint16_t	isEndianDiscovered : 1;
			uint16_t	reserved16bitwise01 : 14;
		}bits;
		uint16_t	allBits;
	}stats;
	uint32_t	itemsHeaderSizeOrFileSize;
	uint64_t	reserved64bit01;
};

struct SRawCompressedFileItem{
	uint16_t	itemPathLengthPlus1;
	union{
		struct{
			uint16_t	isDirectory : 1;
			uint16_t	shouldRootDirBeAdded : 1;
			uint16_t	reserved16bitwise01 : 14;
		}bits;
		uint16_t	allBits;
	}stats;
	uint32_t	fileSize ;
};

namespace qtcore{

struct SCompressedFileItem{
	SRawCompressedFileItem	raw;
	QString					itemFullPath;
};

}  // namespace qt{

#define ITEM_FILE_NAME_FROM_ITEM_PTR(_itemPtr)		reinterpret_cast<const wchar_t*>( reinterpret_cast<const char*>(_itemPtr) + sizeof(SRawCompressedFileItem) )
#define ITEM_TOTAL_LENGTH_FROM_ITEM_PTR(_itemPtr)	( ((sizeof(SRawCompressedFileItem)+static_cast<size_t>(sizeof(wchar_t)*(_itemPtr)->itemPathLengthPlus1)+7)/8)*8 )
#define NULL_ITEM_FROM_PATH(_itemPath)				{{0,{{0,0,0}},0},(_itemPath)}
#define INIT_CD_MAIN_HEADER(_isDir)					{1,{{(_isDir)?1:0,0,0}},0,0}


#endif  // #ifndef qt_zlib_compress_decompress_common_hpp
