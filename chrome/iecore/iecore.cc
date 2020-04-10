#define DLL_EXPORTS
#ifdef DLL_EXPORTS
#define DLL_API __declspec(dllexport)
#else
#define DLL_API __declspec(dllimport)
#endif

/* //kk
#ifndef _DEBUG
#pragma comment(lib,"D:\\sdk\\Detours Express 3.0\\lib.X86\\detours.lib")  
#else if
//#pragma comment(lib,"D:\\dev\\c-frame\\phoenix\\third_party\\lib\\detours_d.lib")  
#pragma comment(lib,"D:\\sdk\\Detours Express 3.0\\lib.X86\\detours.lib")  
#endif
*/

#include <windowsx.h>
#include <windows.h>

#include "base/command_line.h"
#include "base/message_loop/message_loop.h"
#include "base/strings/string_number_conversions.h"
#include "base/process/process_handle.h"

#include "ipc/ipc_switches.h"

#include "chrome/iecore/child/child_process.h"
#include "chrome/iecore/child/child_thread.h"

HINSTANCE   _hInstance = NULL;
HINSTANCE   _hInstanceRes = NULL;

base::ProcessId _browser_pid;

extern "C" {
  DLL_API int WINAPI IECore_wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
  {
#ifdef _DEBUG
    //::MessageBoxA(NULL, "IECore_wWinMain", "IECore_wWinMain", 0);
#endif
    
    CommandLine::Init(0, NULL);
    const CommandLine& parsed_command_line = *CommandLine::ForCurrentProcess();

    base::MessageLoop main_message_loop(base::MessageLoop::TYPE_UI);
    std::string channel_name = parsed_command_line.GetSwitchValueASCII(switches::kProcessChannelID);
    int pid = 0;
    base::StringToInt(channel_name,&pid);
    _browser_pid = static_cast<base::ProcessId>(pid);

    content::ChildProcess worker_process;
    worker_process.set_main_thread(new content::ChildThread());

    base::MessageLoop::current()->Run();

    ::TerminateProcess(GetCurrentProcess(),0);
    return 0;
    return 0;
  }
}