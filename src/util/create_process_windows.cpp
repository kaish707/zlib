// create_process_windows.cpp
// 2018 March 16

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include "create_process_functionalities.h"

#ifdef __cplusplus
extern "C" {
#endif


int CreateNewProcess4(const char* a_cpcExecute, int a_nSuspended,HANDLE a_hInp, HANDLE a_hOut, HANDLE a_hErr, TProcInfo* a_procInfo)
{
	DWORD dwErrCode, dwCreationFlags= a_nSuspended ? 
		NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW | CREATE_SUSPENDED :
		NORMAL_PRIORITY_CLASS ;
	STARTUPINFOA aStartInfo;
	PROCESS_INFORMATION aProcInf;

	//STARTUPINFOA aStartInfo;
	ZeroMemory(&aStartInfo, sizeof(aStartInfo));
	//aStartInfo.dwFlags = STARTF_USESTDHANDLES;
	aStartInfo.hStdInput = a_hInp;
	aStartInfo.hStdOutput = a_hOut;
	aStartInfo.hStdError = a_hErr;
	aStartInfo.wShowWindow = SW_HIDE;
	//aStartInfo.wShowWindow = SW_SHOW;
	//aStartInfo.dwFlags = STARTF_USESTDHANDLES;

	ZeroMemory(&aProcInf, sizeof(PROCESS_INFORMATION));

	dwErrCode = CreateProcessA(
		NULL,
		//const_cast<char*>(a_cpcExecute),
		(char*)a_cpcExecute,
		NULL,
		NULL,
		TRUE,
		dwCreationFlags,
		NULL,
		NULL,
		&aStartInfo,
		&aProcInf
		);

	if(dwErrCode){
		a_procInfo->hProc = aProcInf.hProcess;
		a_procInfo->hThread = aProcInf.hThread;
	}

	return dwErrCode ? 0 : GetLastError();
}


int WaitAndClearProcess(TProcInfo* a_procInfo, int a_nTimeoutMs, int a_nCloseHandleInAnyCase)
{
	DWORD dwWaitReturn=WaitForSingleObject(a_procInfo->hProc, a_nTimeoutMs);

	if((dwWaitReturn==WAIT_OBJECT_0)||a_nCloseHandleInAnyCase){
		CloseHandle(a_procInfo->hProc);
		CloseHandle(a_procInfo->hThread);
	}

	return (dwWaitReturn == WAIT_OBJECT_0)? 0:dwWaitReturn;
}


#ifdef __cplusplus
}
#endif
