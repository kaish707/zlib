
// zlib_compression_routines.cpp
// 2017 Feb 12


// http://www.zlib.net/zlib_how.html

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "zlib.h"
#include "common/util/directory_iterator.h"
#include <stdint.h>
#include <sys/stat.h>
#include "zlib_compression_routines.h"

#if defined(MSDOS) || defined(OS2) || defined(_WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#include <Windows.h>
#  define SET_BINARY_MODE(file) _setmode(_fileno(file), O_BINARY)
#else
#  define SET_BINARY_MODE(file)
#endif

#if defined(_MSC_VER) & (_MSC_VER>1400)
#pragma warning (disable:4996)
#ifdef _WIN64
#pragma warning (disable:4267)  // '=': conversion from 'size_t' to 'uInt', possible loss of data
#endif
#endif


#ifdef __cplusplus
extern "C"{
#endif

typedef struct SUserDataForDirCompress
{
	SCompressList	list;
	uint16_t		numberOfItems;
	uint16_t		headerSize;
	uint16_t		offsetToTakeRoot;
	uint16_t		lastFolderNum;
}SUserDataForDirCompress;

static int CallbackForCompressToFile(const void*a_buffer, int a_bufLen, void*a_userData);
static int DirectoryIterator(const char* a_dir, const FIND_DATAA* a_file_info, void* a_user, int a_isDir);


int ZlibCompressBufferToCallback(
	z_stream* a_strm, int a_flush,
	void* a_out, int a_outBufferSize,
	typeCompressCallback a_clbk,void* a_userData)
{
	int ret2, retZlib;

	/* run deflate() on input until output buffer not full, finish
	compression if all of source has been read in */
	do {

		a_strm->avail_out = a_outBufferSize;
		a_strm->next_out = (Bytef*)a_out;

		retZlib = deflate(a_strm, a_flush);		/* no bad return value */
		assert(retZlib != Z_STREAM_ERROR);		/* state not clobbered */

		ret2=(*a_clbk)(a_out,a_outBufferSize-a_strm->avail_out, a_userData);
		if(ret2){return ret2;}

	} while (a_strm->avail_out == 0);
	assert(a_strm->avail_in == 0);     /* all input will be used */
	return retZlib;
}


int ZlibCompressBufferToFile(
	z_stream* a_strm, int a_flush,
	void* a_out, int a_outBufferSize,
	FILE *a_dest)
{
	return ZlibCompressBufferToCallback(a_strm,a_flush,a_out,a_outBufferSize,&CallbackForCompressToFile,a_dest);
}


/* Compress from file source to file dest until EOF on source.
def() returns Z_OK on success, Z_MEM_ERROR if memory could not be
allocated for processing, Z_STREAM_ERROR if an invalid compression
level is supplied, Z_VERSION_ERROR if the version of zlib.h and the
version of the library linked do not match, or Z_ERRNO if there is
an error reading or writing the files. */
int ZlibCompressFileRawEx(
	z_stream* a_strm,
	FILE * a_source, FILE * a_dest,
	void* a_in, int a_inBufferSize,
	void* a_out, int a_outBufferSize,
	int a_nFlushInTheEnd)
{
	int ret=Z_OK, flush, isFileof=0;

	/* compress until end of file */
	do {
		a_strm->avail_in = fread(a_in, 1, a_inBufferSize, a_source);
		if (ferror(a_source)) {return Z_ERRNO;}
		isFileof = feof(a_source);
		flush = (a_nFlushInTheEnd&&isFileof) ? Z_FINISH : Z_NO_FLUSH;
		a_strm->next_in = (Bytef*)a_in;

		ret = ZlibCompressBufferToFile(a_strm, flush,a_out,a_outBufferSize,a_dest);

	} while (!isFileof);
	if(a_nFlushInTheEnd){assert(ret == Z_STREAM_END);}        /* stream will be complete */
	
	return Z_OK;
}



/* Compress from file source to file dest until EOF on source.
def() returns Z_OK on success, Z_MEM_ERROR if memory could not be
allocated for processing, Z_STREAM_ERROR if an invalid compression
level is supplied, Z_VERSION_ERROR if the version of zlib.h and the
version of the library linked do not match, or Z_ERRNO if there is
an error reading or writing the files. */
int ZlibCompressFileRaw(FILE * a_source, FILE * a_dest,int a_nCompressionLeel)
{
	z_stream strm;
	int nReturn =Z_OK;
	unsigned char in[DEF_CHUNK_SIZE];
	unsigned char out[DEF_CHUNK_SIZE];

	/* allocate deflate state */
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	nReturn = deflateInit(&strm, a_nCompressionLeel);
	if (nReturn != Z_OK){return nReturn;}

	nReturn=ZlibCompressFileRawEx(&strm,a_source,a_dest,in, DEF_CHUNK_SIZE,out, DEF_CHUNK_SIZE,1);

	(void)deflateEnd(&strm);
	return Z_OK;
}


int ZlibCompressFolderEx(const SCompressList* a_list, uint16_t a_headerSize, uint16_t a_numberOfItems, FILE *a_dest, int a_level)
{
	uint8_t *pCurrent2;
	SCompressDecompressHeader *pHeader2 = NULL;
	SFileItemList *pItem;
	z_stream strm;
	int nReturn = -1;
	uint16_t unLenFromItem;
	unsigned char in[DEF_CHUNK_SIZE];
	unsigned char out[DEF_CHUNK_SIZE];

	/* allocate deflate state */
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	nReturn = deflateInit(&strm, a_level);
	if (nReturn != Z_OK)
		return nReturn;

	pHeader2 = ZlibCreateCompressDecompressHeader(a_headerSize, CompressedContentDirectory, a_numberOfItems);
	if(!pHeader2){return ENOMEM;}

	pCurrent2 = ((uint8_t*)pHeader2)+ sizeof(SCompressDecompressHeader);

	pItem = a_list->first;
	while(pItem){
		unLenFromItem = LEN_FROM_ITEM(pItem->item);
		memcpy(pCurrent2,pItem->item,unLenFromItem);
		pCurrent2 += unLenFromItem;
		pItem = pItem->next;
	}

	strm.avail_in = a_headerSize;
	strm.next_in = (Bytef*)pHeader2;
	nReturn=ZlibCompressBufferToFile(&strm,0,out, DEF_CHUNK_SIZE,a_dest);
	if(nReturn){goto returnPoint;}

	pItem = a_list->first;
	while (pItem) {
		if(pItem->file){
			strm.avail_in = DEF_CHUNK_SIZE;
			nReturn = ZlibCompressFileRawEx(&strm,pItem->file,a_dest,in, DEF_CHUNK_SIZE,out, DEF_CHUNK_SIZE,0);
			if (nReturn) { goto returnPoint; }
		}
		pItem = pItem->next;
	}

	strm.avail_in = LAST_STRING_IN_THE_FILE_LEN;
	strm.next_in = (Bytef*)LAST_STRING_IN_THE_FILE;
	nReturn = ZlibCompressBufferToFile(&strm,1, out, DEF_CHUNK_SIZE, a_dest);
	if (nReturn) { goto returnPoint; }

	nReturn = 0;
returnPoint:

	if (pHeader2) { DestroyCompressDecompressHeader(pHeader2); }
	(void)deflateEnd(&strm); // if not inited then this line will not run

	return nReturn;
}



int ZlibCompressFolder(const char* a_directoryPath, FILE *a_dest, int a_level)
{
	SFileItemList *pItem,*pItemNext;
	SUserDataForDirCompress aData;
	int nReturn;
	int nSubDirs = 1;

	memset(&aData, 0, sizeof(SUserDataForDirCompress));

	aData.offsetToTakeRoot = (uint16_t)strlen(a_directoryPath)+1;
	nReturn=IterateOverDirectoryFiles(a_directoryPath, DirectoryIterator, &aData, &nSubDirs);

	if(nReturn){goto returnPoint;}

	aData.headerSize += sizeof(SCompressDecompressHeader);
	nReturn = ZlibCompressFolderEx(&aData.list, aData.headerSize, aData.numberOfItems, a_dest, a_level);

returnPoint:

	pItem = aData.list.first;
	while(pItem){
		pItemNext = pItem->next;
		if(pItem->file){fclose(pItem->file);}
		if(pItem->item){free(pItem->item);}
		free(pItem);
		pItem = pItemNext;
	}

	return nReturn;
}


/* /////////////////////////////////////////////////////////////////////////////////////////////////// */
SFileItemList*	ZlibCreateListItemCompress(const char* a_cpcFileName,uint16_t a_strLen, int a_isDir, const char* a_fullPath)
{
	SFileItemList * pItem=NULL;
	struct stat fStat;
	uint32_t fileSize;
	uint16_t strLenPlus1= a_strLen+1,strLenPlus1Normalized;

	pItem = (SFileItemList*)malloc(sizeof(SFileItemList));
	if(!pItem){ return pItem;}
	pItem->file = NULL;
	pItem->next = NULL;
	
	strLenPlus1Normalized = ((strLenPlus1 %8)==0)?strLenPlus1 :(((strLenPlus1 /8)+1)*8);
	pItem->item = (SFileItem*)malloc(sizeof(SFileItem) + strLenPlus1Normalized);
	if (!pItem->item) { free(pItem); return NULL; }

	pItem->item->fileNameLen = strLenPlus1Normalized;
	pItem->item->folderNum = 0;
	memcpy(ITEM_NAME(pItem->item), a_cpcFileName, a_strLen);
	*(ITEM_NAME(pItem->item)+ a_strLen)=0;

	if (!a_isDir) {
		fileSize = FIND_FILE_SIZE_LATER;
		if(a_fullPath){
			pItem->file = fopen(a_fullPath, "rb");
			if (!pItem->file) {  goto finishPoint; }
			if (fstat(fileno(pItem->file), &fStat)) { goto finishPoint; }
			fileSize = (uint32_t)fStat.st_size;
		}
	}
	else { fileSize = 0; }

finishPoint:
	pItem->item->fileSize = fileSize;

	return pItem;
}


static int DirectoryIterator(const char* a_dir, const FIND_DATAA* a_file_info, void* a_user, int a_isDir)
{
	const char* cpcFileName;
	SUserDataForDirCompress* pData = (SUserDataForDirCompress*)a_user;
	SFileItemList* pItem = NULL;
	int nReturn = -1;
	uint16_t strLen;
	char vcStrFilePath[MAX_PATH];

	_snprintf(vcStrFilePath,MAX_PATH, "%s\\%s", a_dir, a_file_info->cFileName);
	cpcFileName = vcStrFilePath + pData->offsetToTakeRoot;
	strLen = (uint16_t)strlen(cpcFileName);

	pItem=ZlibCreateListItemCompress(cpcFileName,strLen,a_isDir,vcStrFilePath);
	if(!pItem){nReturn= -ENOMEM;goto returnPoint;}
	pItem->item->folderNum = pData->lastFolderNum;

	if(pData->list.last){pData->list.last->next=pItem;}
	else /*if(!pData->first)*/{ pData->list.first = pItem;}
	pData->list.last = pItem;

	++pData->numberOfItems;
	pData->headerSize += LEN_FROM_ITEM(pItem->item);
	if(a_isDir){++pData->lastFolderNum;}
	
	nReturn = 0;
returnPoint:

	if(nReturn<0){
		if(pItem->file){fclose(pItem->file);}
		if(pItem->item){free(pItem->item);}
	}

	return nReturn;
}


static int CallbackForCompressToFile(const void*a_buffer, int a_bufLen, void*a_userData)
{
	FILE* fpOutFile = (FILE*)a_userData;

	if (fwrite(a_buffer,1, a_bufLen,fpOutFile) != a_bufLen || ferror(fpOutFile)) {
		return Z_ERRNO;
	}

	return 0;
}


#ifdef __cplusplus
}
#endif

