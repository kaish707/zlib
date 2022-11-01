
// http://www.zlib.net/zlib_how.html

#ifdef _MSC_VER
#pragma comment (lib,"zlib.lib")
#endif

#include "zlib_compression_routines.h"

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <util/common_hashtbl.hpp>
#include <util/common_argument_parser.hpp>

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

static void PrintHelp();
static int CompressBasedOnConfig(const char* a_configFileName, FILE *a_dest, int a_level);

/* compress or decompress from stdin to stdout */
int main(int a_argc, char * a_argv[])
{
	char** argv2 = a_argv + 1;
	FILE *fpOut=NULL;
	common::argument_parser aParser;
	int ret=1;
	int argc2 = a_argc-1;

	aParser.
		AddOption("--output-file,-of:Prvide output file name").
		AddOption("--config-file,-cf:Provide config file for input file tree").
		AddOption("--input-folder,-if:The name of the input folder")
		<<"--help,-h:To display this help";

	aParser.ParseCommandLine(argc2, argv2);

	if(aParser["-h"]){
		PrintHelp();
		std::cout<<aParser.HelpString()<<std::endl;
		ret = 0;
		goto returnPoint;
	}

	if (!aParser["-of"]) {
		fprintf(stderr, "Output file should be specified!\n");
		PrintHelp();
		std::cout << aParser.HelpString() << std::endl;
		goto returnPoint;
	}

	if (!aParser["-if"] && !aParser["-cf"] && (argc2==0)) {
		fprintf(stderr, "Input folder should be specified!\n");
		PrintHelp();
		std::cout << aParser.HelpString() << std::endl;
		goto returnPoint;
	}

	fpOut = fopen(aParser["-of"], "wb");
	if (!fpOut) { 
		fprintf(stderr, "Unable to open output file %s\n", aParser["-of"]);
		goto returnPoint; 
	}

	if(aParser["-if"]){
		ret = ZlibCompressFolder(aParser["-if"], fpOut, Z_DEFAULT_COMPRESSION);
	}
	else if(argc2){ 
		ret = ZlibCompressFolder(argv2[0], fpOut, Z_DEFAULT_COMPRESSION);
	}
	else {
		ret = CompressBasedOnConfig(aParser["-cf"], fpOut, Z_DEFAULT_COMPRESSION);
	}

	//ret = 0;
returnPoint:
	if (fpOut) { fclose(fpOut); }

	printf("return=%d\n",ret);
	if ((ret != Z_OK)&&(ret!=0)){/*zerr(ret);*/}

#if defined(_WIN32) & !defined(_M_ARM)
	printf("Press any key to exit!"); fflush(stdout);
	_getch();
	printf("\n");
#endif

	return ret;
}


static void PrintHelp()
{
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
		pItem = pItemTemp;
	}

	return nReturn;

	//GetPrintProcessorDirectory()

}


static int PrepareListFromFile(SCompressList* a_list,uint16_t* a_pHeaderSize, uint16_t* a_numberOfItems, const char* a_configFileName)
{	
	static const size_t scunLINE_LEN_MIN1 = 4 * MAX_PATH - 1;
	static const char svcTermStr[] = {' ','\t'};
	static const char svcTermStr2[] = { ' ','\t','\n','\r' };
	
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
				//pItem=ZlibCreateListItemCompress(pcSubDirs, fileNameLen,1,NULL);
				pItem = ZlibCreateListItemCompress(pcTargetPath, fileNameLen, 1, NULL);
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

