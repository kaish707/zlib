
#include <stdio.h>
#include <string.h>
#include "zlib_decompress_routines.h"
#include "installer_common.h"
#include <malloc.h>
#ifdef _WIN32
//#include <Windows.h>
#else
#endif
#include "create_process_functionalities.h"

#define REMOTE_URI_MIN1		1023
#define TEMPORARY_DIR		"C:\\.dsi-tmp"


#define ROOT_KEY	"root:"
#define RUN_KEY		"run:"

#ifdef _MSC_VER
#pragma warning (disable:4996)
#pragma comment(lib,"Wininet.lib")
#pragma comment(lib,"zlib.lib")
#endif


int main(int a_argc, char* a_argv[])
{
	static const char svcTermStr[] = { ' ','\t' };
	static const char svcTermStr2[] = { ' ','\t','\n','\r' };
	char *pcOutput,*pcNextToRoot;
	FILE* fpInsFile=NULL;
	size_t unRead;
	size_t unRemaining;
	TProcInfo aProcInfo;
	int nReturn = -1;
	char vcTmpBuffer[REMOTE_URI_MIN1+1];
	char vcTargetPath[REMOTE_URI_MIN1+1];
	char cTemp;

	if(a_argc<2){
		fprintf(stderr,"Arguments are less, than necessary!\n");
		goto returnPoint;
	}

	if((a_argc>2) && (strcmp(a_argv[1],"install")==0)){
		_snprintf(vcTmpBuffer,REMOTE_URI_MIN1,"%s%s.dsi","https://desycloud.desy.de/index.php/s/XvNhzouZlV5s12W/download?path=%2FARM&files=",a_argv[2]);
		nReturn = ZlibDecompressFromWeb(vcTmpBuffer, TEMPORARY_DIR);
		if (nReturn) {
			fprintf(stderr,"Unable to get package from web\n");
			goto returnPoint;
		}

		fpInsFile=fopen(TEMPORARY_DIR "\\" INSTALLERS_STEP_FL_NAME, "rb");
		if(!fpInsFile){
			goto returnPoint;
		}

		unRead=fread(vcTmpBuffer,1,REMOTE_URI_MIN1,fpInsFile);
		vcTmpBuffer[unRead] = 0;
		fclose(fpInsFile); fpInsFile=NULL;
		DeleteFileA(TEMPORARY_DIR "\\" INSTALLERS_STEP_FL_NAME);
		
		pcOutput=strstr(vcTmpBuffer, ROOT_KEY);
		if(!pcOutput){
			goto returnPoint;
		}
		pcOutput += strlen(ROOT_KEY);
		unRead = strspn(pcOutput, svcTermStr);  // skip all tabs and spaces
		pcOutput += unRead;
		unRead = strcspn(pcOutput, svcTermStr2);
		cTemp = pcOutput[unRead];
		pcOutput[unRead] = 0;
		if((unRead+1)>REMOTE_URI_MIN1){goto returnPoint;}
		//pcOutputRoot = (char*)_alloca(unRead + 1);
		unRemaining = REMOTE_URI_MIN1 - unRead-1;
		memcpy(vcTargetPath, pcOutput, unRead);
		vcTargetPath[unRead] = '\\';
		pcNextToRoot = vcTargetPath + unRead+1;

		if(strcmp(TEMPORARY_DIR, pcOutput)){
			if (!MoveFileExA(TEMPORARY_DIR, pcOutput,MOVEFILE_WRITE_THROUGH)){
				fprintf(stderr,"MoveFileEx failed with error %d\n", GetLastError());
				goto returnPoint;
			}
			pcOutput[unRead] = cTemp;
		}  // if(strcmp(TEMPORARY_DIR, pcpOutput)){
		else{
			pcOutput[unRead] = cTemp;
		}

		// at this point installation is sucessfull
		// setup env
		// register services
		// schedule tasks
		nReturn = 0;
		pcOutput = strstr(vcTmpBuffer, RUN_KEY);
		if (!pcOutput) {goto returnPoint;}
		pcOutput += strlen(RUN_KEY);
		unRead = strspn(pcOutput, svcTermStr);  // skip all tabs and spaces
		pcOutput += unRead;
		unRead = strcspn(pcOutput, svcTermStr2);
		cTemp = pcOutput[unRead];
		pcOutput[unRead] = 0;
		snprintf(pcNextToRoot, unRemaining,"%s /Install", pcOutput);
		if(!CreateNewProcess4Std(vcTargetPath, 0, &aProcInfo)){
			WaitAndClearProcess(&aProcInfo, INFINITE, 1);
		}
	}


	nReturn = 0;
returnPoint:
	if (fpInsFile) {fclose(fpInsFile);}
	return nReturn;
}
