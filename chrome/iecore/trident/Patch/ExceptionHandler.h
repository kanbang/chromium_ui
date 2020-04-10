
#pragma once
#include <DbgHelp.h>
#include <string>
/*todo:参考 http://blog.csdn.net/pkrobbie/article/details/6636310
1、dump的大小及保存的数据 参考http://www.debuginfo.com/tools/minidumpwizard.html
2、dump包含用户自定义数据
*/
typedef enum _WG_DUMPTYPE
{
	SMALL_DUMP = MiniDumpNormal,
	BIG_DUMP = MiniDumpNormal | MiniDumpWithUnloadedModules |MiniDumpWithProcessThreadData|MiniDumpWithIndirectlyReferencedMemory,
	FULL_DUMP = MiniDumpWithFullMemory,

}WG_DUMPTYPE;

class CExceptionHandler
{
public:
	CExceptionHandler();
	virtual ~CExceptionHandler();

public:
	void SetExceptionHandler();
	//try_catch 3次即为崩溃
	static LONG CatchException(PEXCEPTION_POINTERS ExceptionInfo);
	//崩溃
	static LONG WINAPI ExceptionFilter(PEXCEPTION_POINTERS ExceptionInfo);

private:
	static void InitDumpFile( std::wstring &szFile);
	static void GetTime(int nDays, PTSTR szTime, int nLen);
	static void CreateDumpFile(LPCTSTR szDumpFile,PEXCEPTION_POINTERS pExceptionInfo,
		PMINIDUMP_USER_STREAM_INFORMATION pUserStreamParam=NULL,WG_DUMPTYPE type =BIG_DUMP );
	static PMINIDUMP_USER_STREAM_INFORMATION InitUserStreamData();
};
