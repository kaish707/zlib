
// http://www.zlib.net/zlib_how.html

#ifdef _MSC_VER
#pragma comment (lib,"zlib.lib")
#endif

#include "zlib_decompress_routines.h"
#include "zlib_compression_routines.h"

#include <stdio.h>
#include <string.h>
#include <util/common_hashtbl.hpp>

#if defined(MSDOS) || defined(OS2) || defined(_WIN32) || defined(__CYGWIN__)
#include <conio.h>
#include <Windows.h>
#  define SET_BINARY_MODE(file) _setmode(_fileno(file), O_BINARY)
#else
#  define SET_BINARY_MODE(file)
#endif

#if defined(_MSC_VER) & (_MSC_VER>1400)
#pragma warning (disable:4996)
#endif

#if 0

#include "util/directory_iterator.h"
static int DirectoryIterator(const char* a_dir, const FIND_DATAA* a_file_info, void* a_user, int a_isDir)
{
	if(a_isDir){printf("dir:   ");}
	else { printf("file:  "); }

	printf("%s\n",a_file_info->cFileName);
	return 0;
}

#endif

static int CompressBasedOnConfig(const char* a_configFileName, FILE *a_dest, int a_level);

/* compress or decompress from stdin to stdout */
int main(int argc, char **argv)
{

#if 0
	int nSubDirs = 0;
	if(argc<2){
		return 1;
	}

	IterateOverDirectoryFiles(argv[1], DirectoryIterator, NULL, &nSubDirs);

	return 0;

#endif



	FILE *fpIn=NULL, *fpOut=NULL;
	int ret=1;

	if(argc<4){
		fprintf(stderr,"Provide the mode (c,d) and the input and output files names!\n");
		goto returnPoint;
	}

	

	if(strcmp("cf",argv[1])==0){
		fpOut = fopen(argv[3], "wb");
		if (!fpOut) { goto returnPoint; }
		fpIn = fopen(argv[2],"rb");
		if (!fpIn) { goto returnPoint; }
		ret = ZlibCompressFileRaw(fpIn,fpOut,Z_DEFAULT_COMPRESSION);
	}
	else if (strcmp("df", argv[1]) == 0){
		fpOut = fopen(argv[3], "wb");
		if (!fpOut) { goto returnPoint; }
		fpIn = fopen(argv[2],"rb");
		if (!fpIn) { goto returnPoint; }
		ret = ZlibDecompressFile(fpIn,fpOut);
	}
	else if (strcmp("cd", argv[1]) == 0) {
		fpOut = fopen(argv[3], "wb");
		if (!fpOut) { goto returnPoint; }
		ret = ZlibCompressFolder(argv[2], fpOut, Z_DEFAULT_COMPRESSION);
	}
	else if (strcmp("dd", argv[1]) == 0) {
		fpIn = fopen(argv[2], "rb");
		if (!fpIn) { goto returnPoint; }
		ret=ZlibDecompressFolder(fpIn,argv[3]);
	}
	else if(strcmp("cbf", argv[1]) == 0){
		fpOut = fopen(argv[3], "wb");
		if (!fpOut) { goto returnPoint; }
		ret = CompressBasedOnConfig(argv[2], fpOut, Z_DEFAULT_COMPRESSION);
	}
	else{
		fprintf(stderr,"wrong option!\n");
		goto returnPoint;
	}

	ret = 0;
returnPoint:

	if (fpIn) { fclose(fpIn); }
	if (fpOut) { fclose(fpOut); }

	printf("return=%d\n",ret);
	//if ((ret != Z_OK)&&(ret!=0))zerr(ret);

#if defined(_WIN32) & !defined(_M_ARM)
	printf("Press any key to exit!"); fflush(stdout);
	_getch();
	printf("\n");
#endif

	return ret;
}



static int PrepareListFromFile(
	SCompressList* a_list, uint16_t* a_pHeaderSize, uint16_t* a_numberOfItems,const char* a_configFileName);


static int CompressBasedOnConfig(const char* a_configFileName, FILE *a_dest, int a_level)
{
	SFileItemList *pItem, *pItemTemp;
	SCompressList aList;
	int nReturn = -1;
	uint16_t headerSize, numberOfItems;

	if(PrepareListFromFile(&aList,&headerSize,&numberOfItems,a_configFileName)){
		goto returnPoint;
	}

	headerSize += sizeof(SCompressDecompressHeader);
	nReturn=ZlibCompressFolderEx(&aList,headerSize,numberOfItems,a_dest,a_level);

returnPoint:

	pItem = aList.first;

	while(pItem){
		pItemTemp = pItem->next;
		if(pItem->file){fclose(pItem->file);}
		if(pItem->item){free(pItem->item);}
		free(pItem);
		pItem = pItemTemp->next;
	}

	return nReturn;

	//GetPrintProcessorDirectory()

}


static int PrepareListFromFile(SCompressList* a_list,uint16_t* a_pHeaderSize, uint16_t* a_numberOfItems, const char* a_configFileName)
{	
	static const size_t scunLINE_LEN_MIN1 = 4 * MAX_PATH - 1;
	static const char svcTermStr[] = {' ','\t'};
	static const char svcTermStr2[] = { ' ','\t','\n' };
	
	char *pcFilePath, *pcTargetPath, *pcTemp, *pcFileName, *pcSubDirs, *pcNext;
	SFileItemList* pItem;
	FILE* fpFile = fopen(a_configFileName, "r");
	common::HashTbl<bool> aHashDirs;
	size_t unIndexOff;
	size_t unFilePathLen, unTargetPathLen;
	int nReturn = -1;
	uint16_t fileNameLen;
	char vcLine[scunLINE_LEN_MIN1 +1];
	char vcOriginal;
	bool bFindResult;

	Init_SCompressList(a_list);
	*a_pHeaderSize = 0;
	*a_numberOfItems = 0;

	if(!fpFile){goto returnPoint;}

	while (fgets(vcLine, scunLINE_LEN_MIN1, fpFile) != NULL){
		if (vcLine[0] == '#') continue;
		
		unIndexOff=strspn(vcLine, svcTermStr);  // skip all tabs and spaces
		if((vcLine[unIndexOff]=='\n')||(vcLine[unIndexOff]==0)){continue;}
		pcFilePath = vcLine + unIndexOff;
		
		unFilePathLen=strcspn(pcFilePath,svcTermStr);  // find next tab or space
		if((pcFilePath[unIndexOff]=='\n')||(pcFilePath[unIndexOff]==0)){continue;}
		pcTemp = pcFilePath + unFilePathLen;
		*pcTemp++ = 0;

		unIndexOff=strspn(pcTemp, svcTermStr); // skip all tabs and spaces
		if((pcTemp[unIndexOff]=='\n')||(pcTemp[unIndexOff]==0)){continue;}
		pcTargetPath = pcTemp + unIndexOff;

		unTargetPathLen = strcspn(pcTargetPath, svcTermStr2);  // find next tab or space
		if(pcTargetPath[unTargetPathLen-1]=='.'){
			pcFileName = strrchr(pcFilePath, '/');
			if(pcFileName){++pcFileName;}
			else{
				pcFileName = strrchr(pcFilePath, '\\');
				if(pcFileName){++pcFileName;}
				else{pcFileName=pcFilePath;}
			}
			strcpy(&pcTargetPath[unTargetPathLen-1], pcFileName);
			unTargetPathLen = strlen(pcTargetPath);
		}
		else{
			pcTargetPath[unTargetPathLen] = 0;
		}

		pcTemp = strchr(pcTargetPath, '/');
		if (pcTemp) { vcOriginal = '/'; }
		else{ pcTemp = strchr(pcTargetPath, '\\');if(pcTemp){vcOriginal = '\\';} }
		pcSubDirs = pcTargetPath;

		while(pcTemp){
			*pcTemp = 0;
			//fileNameLen = (uint16_t)strlen(pcSubDirs);
			//if(!aHashDirs.FindEntry(pcSubDirs,fileNameLen,&bFindResult)){
			fileNameLen = (uint16_t)strlen(pcTargetPath);
			if (!aHashDirs.FindEntry(pcTargetPath, fileNameLen, &bFindResult)) {
				pItem=ZlibCreateListItemCompress(pcSubDirs, fileNameLen,1,NULL);
				if(!pItem){goto returnPoint;}
				*a_pHeaderSize += LEN_FROM_ITEM(pItem->item);
				++(*a_numberOfItems);
				if(a_list->last){a_list->last->next=pItem;a_list->last=pItem;}
				else {a_list->first=a_list->last=pItem;}
				aHashDirs.AddEntry(pcSubDirs, fileNameLen,true);
			}
			*pcTemp = vcOriginal;
			
			pcNext = pcTemp + 1;
			pcTemp = strchr(pcNext, '/');
			if (pcTemp) { vcOriginal = '/'; }
			else { pcTemp = strchr(pcNext, '\\'); if (pcTemp) { vcOriginal = '\\'; } }
			pcSubDirs = pcNext;
		}

		pItem=ZlibCreateListItemCompress(pcTargetPath, (uint16_t)unTargetPathLen,0, pcFilePath);
		if (!pItem) { goto returnPoint; }
		*a_pHeaderSize += LEN_FROM_ITEM(pItem->item);
		++(*a_numberOfItems);
		if (a_list->last) { a_list->last->next = pItem; a_list->last = pItem; }
		else { a_list->first = a_list->last = pItem; }

	}  // while (fgets(vcLine, scunLINE_LEN_MIN1, fpFile) != NULL){

	nReturn = 0;
returnPoint:

	if(fpFile){fclose(fpFile);}

	return nReturn;
}











#if 0
/* report a zlib or i/o error */
void zerr(int ret)
{
	fputs("zpipe: ", stderr);
	switch (ret) {
	case Z_ERRNO:
		if (ferror(stdin))
			fputs("error reading stdin\n", stderr);
		if (ferror(stdout))
			fputs("error writing stdout\n", stderr);
		break;
	case Z_STREAM_ERROR:
		fputs("invalid compression level\n", stderr);
		break;
	case Z_DATA_ERROR:
		fputs("invalid or incomplete deflate data\n", stderr);
		break;
	case Z_MEM_ERROR:
		fputs("out of memory\n", stderr);
		break;
	case Z_VERSION_ERROR:
		fputs("zlib version mismatch!\n", stderr);
	}
}
#endif
