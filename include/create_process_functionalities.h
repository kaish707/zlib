
// create_process_functionalities.h
// 2018 March 16

#ifndef __create_process_windows_h__
#define __create_process_windows_h__

#ifdef _WIN32
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
typedef struct SProcInfo {
	HANDLE hThread;
	HANDLE hProc;
}TProcInfo;
#else
#endif

#ifdef __cplusplus
extern "C" {
#endif

int WaitAndClearProcess(TProcInfo* a_procInfo, int a_nTimeoutMs, int a_nCloseHandleInAnyCase);

#ifdef _WIN32
int CreateNewProcess4(const char* a_cpcExecute, int a_nSuspended,HANDLE a_hInp, HANDLE a_hOut, HANDLE a_hErr, TProcInfo* a_procInfo);
#define CreateNewProcess4Std(_cpcExecute,_suspended,_procInfo) \
	CreateNewProcess4(	(_cpcExecute),(_suspended),(HANDLE)((size_t)STD_INPUT_HANDLE), \
						(HANDLE)((size_t)STD_OUTPUT_HANDLE),(HANDLE)((size_t)STD_ERROR_HANDLE),(_procInfo))
#endif


#ifdef __cplusplus
}
#endif


#endif  // #ifndef __create_process_windows_h__
