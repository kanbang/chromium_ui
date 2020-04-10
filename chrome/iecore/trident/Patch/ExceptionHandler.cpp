#include "chrome/iecore/trident/stdafx.h"
#include "chrome/iecore/trident/preheader.h"
#include "ExceptionHandler.h"
#include "HookApi.h"
#include <time.h>

CExceptionHandler::CExceptionHandler()
{

}

CExceptionHandler::~CExceptionHandler()
{

}


void CExceptionHandler::SetExceptionHandler()
{
	//RealSetUnhandledExceptionFilter(ExceptionFilter);
}

LONG CExceptionHandler::ExceptionFilter(PEXCEPTION_POINTERS ExceptionInfo)
{
	::MessageBox(0,L"ExceptionFilter",L"±¿¿£",0);
	std::wstring szDumpFile;
	InitDumpFile(szDumpFile);
	CreateDumpFile(szDumpFile.c_str(),ExceptionInfo);
	return EXCEPTION_EXECUTE_HANDLER;
}

LONG CExceptionHandler::CatchException(PEXCEPTION_POINTERS ExceptionInfo)
{
	::MessageBox(0,L"CatchException",L"“Ï≥£",0);
	std::wstring szDumpFile;
	InitDumpFile(szDumpFile);
	CreateDumpFile(szDumpFile.c_str(),ExceptionInfo);
	return EXCEPTION_EXECUTE_HANDLER;
}

void CExceptionHandler::GetTime(int nDays, PTSTR szTime, int nLen)
{
	time_t ltime ;
	time(&ltime) ;

	ltime -= nDays * 24 * 60 * 60 ;
	tm tmCurTime = {0} ;
	localtime_s(&tmCurTime, &ltime) ;
	_sntprintf_s(szTime, nLen - 1, _TRUNCATE, _T("%d-%02d-%02d-%02d-%02d-%02d"), 1900 + tmCurTime.tm_year, tmCurTime.tm_mon+1, tmCurTime.tm_mday,
		tmCurTime.tm_hour, tmCurTime.tm_min, tmCurTime.tm_sec) ;
}

void CExceptionHandler::InitDumpFile( std::wstring &szFile)
{
	std::wstring wszDumpFile;
//	wszDumpFile = g_App.GetAppDataDir();
	wszDumpFile=wszDumpFile+L"\\WebGame";
	if( !PathFileExists(wszDumpFile.c_str()))
	{
		CreateDirectoryW(wszDumpFile.c_str(),NULL);
	}

	WCHAR szTime[100]={0};
	GetTime(0,szTime,_ARRAYSIZE(szTime)-1);
	wszDumpFile=wszDumpFile+L"\\"+szTime+L".dmp";
	szFile = wszDumpFile;
}

void CExceptionHandler::CreateDumpFile(LPCTSTR szDumpFile,PEXCEPTION_POINTERS pExceptionInfo,PMINIDUMP_USER_STREAM_INFORMATION pUserInfo,WG_DUMPTYPE type)
{
	typedef BOOL (__stdcall *LPMiniDumpWriteDump)(HANDLE hProcess, DWORD ProcessId, HANDLE hFile, MINIDUMP_TYPE DumpType, PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam, PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam, PMINIDUMP_CALLBACK_INFORMATION CallbackParam);

	TCHAR szDll[MAX_PATH + 1] = {0} ;
	GetSystemDirectory(szDll, MAX_PATH) ;
	PathAppend(szDll, _T("dbghelp.dll")) ;
	HMODULE hDbgHelp = LoadLibrary(szDll);
	if(hDbgHelp)
	{
		LPMiniDumpWriteDump lpMiniDumpWriteDump = (LPMiniDumpWriteDump)GetProcAddress(hDbgHelp, "MiniDumpWriteDump");
		if(lpMiniDumpWriteDump && szDumpFile && szDumpFile[0])
		{
			HANDLE hFile = CreateFile(szDumpFile, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			if(hFile!=INVALID_HANDLE_VALUE)
			{
				MINIDUMP_EXCEPTION_INFORMATION exInfo;
				memset(&exInfo, 0, sizeof(exInfo));

				exInfo.ThreadId = ::GetCurrentThreadId();
				exInfo.ExceptionPointers = pExceptionInfo;
				exInfo.ClientPointers = FALSE;

				lpMiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, (MINIDUMP_TYPE)type, &exInfo, pUserInfo, NULL);
				CloseHandle(hFile);
			}
		}
		FreeLibrary(hDbgHelp);
	}
}
