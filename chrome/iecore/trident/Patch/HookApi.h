#pragma once

typedef struct _WGCREATETHREAD
{
	LPVOID lpParameter;
	LPTHREAD_START_ROUTINE lpStartAddress;

}WGCREATETHREAD,*LPWGCREATETHREAD;

void APIHookBegin();
void APIHookEnd();
void StartProcessAPIHook();
void StartBrowserProcessAPIHook();
void DisguisedAsIE();
extern LPTOP_LEVEL_EXCEPTION_FILTER (WINAPI *RealSetUnhandledExceptionFilter)(LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter);

