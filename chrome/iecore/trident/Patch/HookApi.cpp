#include "chrome/iecore/trident/stdafx.h"
#include "HookApi.h"
#include "chrome/iecore/third_party/detours/src/detours.h"
#include "chrome/iecore/third_party/comhook.h"
#include "HtmlWindowHookApi.h"
#include "HtmlDocumentHookApi.h"
#include "CompatibilityView.h"
//#include "IE8/IE8stub.h"
#include "SystemInfo.h"
#include "UserAgentHelper.h"
#include "ExceptionHandler.h"

// extern BOOL _bIE9;
// extern BOOL _bIE10;
// extern BOOL _bIE11;
/* 
Author Time Event

dz 2013/4/21 添加异常捕获dump文件
dz 2013/5/28 添加自带Unity3D插件

*/

HANDLE g_hCreateThreadHeap=NULL;

LPTOP_LEVEL_EXCEPTION_FILTER (WINAPI *RealSetUnhandledExceptionFilter)(LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter) = &SetUnhandledExceptionFilter;
static LONG (WINAPI *RealUnhandledExceptionFilter)(PEXCEPTION_POINTERS ExceptionInfo) = UnhandledExceptionFilter;
static HANDLE (WINAPI *RealCreateThread)(LPSECURITY_ATTRIBUTES lpThreadAttributes, SIZE_T dwStackSize, LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId)=&CreateThread;
static DWORD (WINAPI *RealGetModuleFileNameW)(HMODULE hModule, LPWSTR lpFilename, DWORD nSize) = &GetModuleFileNameW;
static DWORD (WINAPI *RealGetModuleBaseNameW)(HANDLE hProcess, HMODULE hModule, LPWSTR lpBaseName, DWORD nSize) = &GetModuleBaseNameW;
typedef HRESULT (WINAPI* PROC_IEUTIL_GETVALUE)(struct _GUID const *, int nDeserve, int  dataTypeFlag, PBYTE retBuf, int cbBuf, int nDeserve1, int nDeserve2 );
PROC_IEUTIL_GETVALUE RealIEUtilGetVal = NULL;
//unity 3d免安装 hook ole32.dll CoGetClassObject  CLSIDFromProgID OleRegGetMiscStatus LoadRegTypeLib
typedef HRESULT (WINAPI* PROC_CLSIDFromProgID)( LPCOLESTR lpszProgID, LPCLSID pclsid); 
PROC_CLSIDFromProgID RealCLSIDFromProgID=NULL;
typedef HRESULT (WINAPI* PROC_OleRegGetMiscStatus)(REFCLSID clsid,DWORD dwAspect,DWORD *pdwStatus);
PROC_OleRegGetMiscStatus RealOleRegGetMiscStatus=NULL;
const static CLSID CLSID_UnityWebPlayerControl = {0x444785f1, 0xde89, 0x4295, {0x86, 0x3a, 0xd4, 0x6c, 0x3a, 0x78, 0x13, 0x94}};
const static CLSID CLSID_FancyGame =  {0xb2e8d85e, 0xc0c5, 0x48df, {0x8d, 0xbc, 0x13, 0x59, 0xb3, 0x39, 0xae, 0x32}};
typedef HRESULT (WINAPI *PROC_CoGetClassObject)(REFCLSID rclsid, DWORD dwClsContext, LPVOID pvReserved, REFIID riid, LPVOID FAR* ppv);
PROC_CoGetClassObject RealCoGetClassObject = NULL;
typedef HRESULT (WINAPI* PROC_DllGetClassObject)(const CLSID& clsid, const IID& iid, void** ppv);
//todo:some activex may need this ,example:Flash
typedef HRESULT (WINAPI* PROC_LoadRegTypeLib)(REFGUID rguid, unsigned short wVerMajor, unsigned short  wVerMinor, LCID  lcid, ITypeLib FAR* FAR  *pptlib);
PROC_LoadRegTypeLib RealLoadRegTypeLib=NULL;
//fix windows7+ie8 右键无法刷新:因为ie开启了保护模式,我们伪装为IE
typedef HRESULT (WINAPI *PROC_CoInternetCreateZoneManager)(IServiceProvider* pSP, IInternetZoneManager **ppSM, DWORD dwReserved);
PROC_CoInternetCreateZoneManager RealCoInternetCreateZoneManager=NULL;
typedef HRESULT (WINAPI *PROC_CoInternetCreateSecurityManager)(IServiceProvider* pSP, IInternetSecurityManager **ppSM, DWORD dwReserved);
PROC_CoInternetCreateSecurityManager RealCoInternetCreateSecurityManager=NULL;

//hook 注册表函数 kernel32!RegOpenKeyExW
typedef LONG (WINAPI* PROC_RegOpenKeyExW) (HKEY hKey,LPCWSTR lpSubKey,DWORD ulOptions,REGSAM samDesired,PHKEY phkResult);
PROC_RegOpenKeyExW RealRegOpenKeyExW=NULL;
//
typedef HRESULT (__stdcall* PROC_CoInternetSetBrowserUserAgentMode)( DWORD );//447
PROC_CoInternetSetBrowserUserAgentMode RealCoInternetSetBrowserUserAgentMode=NULL;
typedef HRESULT (__stdcall* PROC_CoInternetSetBrowserEmulationMode)( PEmulationMode );//450
PROC_CoInternetSetBrowserEmulationMode RealCoInternetSetBrowserEmulationMode=NULL;
typedef HRESULT (__stdcall* PROC_MapBrowserEmulationStateToUserAgent)(DWORD,PEmulationMode );//445
PROC_MapBrowserEmulationStateToUserAgent RealMapBrowserEmulationStateToUserAgent=NULL;

HRESULT (WINAPI *OldGetZoneActionPolicy)(LPVOID lpThis, DWORD dwZone, DWORD dwAction, BYTE *pPolicy, DWORD cbPolicy, URLZONEREG urlZoneReg) = 0;
HRESULT WINAPI NewGetZoneActionPolicy(LPVOID lpThis, DWORD dwZone, DWORD dwAction, BYTE *pPolicy, DWORD cbPolicy, URLZONEREG urlZoneReg)
{
	if (URLACTION_LOWRIGHTS == dwAction)
	{
		if(cbPolicy >= sizeof(DWORD))
			*(DWORD*)pPolicy = URLPOLICY_DISALLOW;
		return S_OK;
	}
	return OldGetZoneActionPolicy(lpThis, dwZone, dwAction, pPolicy, cbPolicy, urlZoneReg);
}

HRESULT WINAPI HookedCoInternetCreateZoneManager(IServiceProvider* pSP, IInternetZoneManager** ppSM, DWORD dwReserved)
{
	HRESULT hr = RealCoInternetCreateZoneManager(pSP, ppSM, dwReserved);
	if (ppSM && *ppSM)
		HookComInterface((DWORD**)*ppSM, 7, (DWORD)NewGetZoneActionPolicy, (DWORD*)&OldGetZoneActionPolicy);
	return hr;
}

HRESULT (WINAPI *OldProcessUrlActionEx2)(LPVOID lpThis, IUri *pUri, DWORD dwAction, BYTE *pPolicy, DWORD cbPolicy, BYTE *pContext, DWORD cbContext, DWORD dwFlags, DWORD dwReserved, DWORD *pdwOutFlags) = 0;
HRESULT WINAPI NewProcessUrlActionEx2(LPVOID lpThis, IUri *pUri, DWORD dwAction, BYTE *pPolicy, DWORD cbPolicy, BYTE *pContext, DWORD cbContext, DWORD dwFlags, DWORD dwReserved, DWORD *pdwOutFlags)
{
	if (URLACTION_LOWRIGHTS == dwAction)
	{
		if(cbPolicy >= sizeof(DWORD))
			*(DWORD*)pPolicy = URLPOLICY_DISALLOW;
		return S_OK;
	}
	return OldProcessUrlActionEx2(lpThis, pUri, dwAction, pPolicy, cbPolicy, pContext, cbContext, dwFlags, dwReserved, pdwOutFlags);
}

HRESULT WINAPI HookedCoInternetCreateSecurityManager(IServiceProvider* pSP, IInternetSecurityManager** ppSM, DWORD dwReserved)
{
	HRESULT hr = RealCoInternetCreateSecurityManager(pSP, ppSM, dwReserved);
	if (ppSM && *ppSM)
	{
		IInternetSecurityManagerEx2* ismex2 = NULL;
		(*ppSM)->QueryInterface(IID_IInternetSecurityManagerEx2, (void**)&ismex2);
		if (ismex2)
		{
			HookComInterface((DWORD**)ismex2, 13, (DWORD)NewProcessUrlActionEx2, (DWORD*)&OldProcessUrlActionEx2);
			ismex2->Release();
		}
	}
	return hr;
}

DWORD WINAPI WgThreadStartProc(LPVOID p)
{
	DWORD dwRet=0;
	LPWGCREATETHREAD lpCreateThread=(LPWGCREATETHREAD)p;

	if(lpCreateThread)
	{
		LPVOID lpParameter=lpCreateThread->lpParameter;
		LPTHREAD_START_ROUTINE lpStartAddress=lpCreateThread->lpStartAddress;
		HeapFree(g_hCreateThreadHeap, 0, p);

		__try
		{
			//todo:In Debug Flash may have exception
			dwRet=lpStartAddress(lpParameter);
		}__except(EXCEPTION_EXECUTE_HANDLER,CExceptionHandler::CatchException(GetExceptionInformation()))
		{
			dwRet=0;
		}
	}
	return dwRet;
}


HANDLE WINAPI HookedCreateThread(LPSECURITY_ATTRIBUTES lpThreadAttributes, SIZE_T dwStackSize, LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId)
{
	if(g_hCreateThreadHeap)
	{
		LPWGCREATETHREAD lpCreateThread=(LPWGCREATETHREAD)HeapAlloc(g_hCreateThreadHeap, HEAP_ZERO_MEMORY, sizeof(WGCREATETHREAD));
		if(lpCreateThread)
		{
			lpCreateThread->lpParameter=lpParameter;
			lpCreateThread->lpStartAddress=lpStartAddress;

			lpParameter=lpCreateThread;
			lpStartAddress=WgThreadStartProc;
		}
	}
	return RealCreateThread(lpThreadAttributes, dwStackSize, lpStartAddress, lpParameter, dwCreationFlags, lpThreadId);
}

LPTOP_LEVEL_EXCEPTION_FILTER WINAPI HookedSetUnhandledExceptionFilter(LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter) 
{
		return NULL;
}


LONG WINAPI HookedUnhandledExceptionFilter(PEXCEPTION_POINTERS ExceptionInfo) 
{
	return CExceptionHandler::ExceptionFilter(ExceptionInfo);
}


//dz:关闭优化，保证取返回地址正确

enum
{
	GMF_MODULEFILENAME_IEDVTOOL_DLL = 0,
	GMF_MODULEFILENAME_MSHTML_DLL = 1,
	GMF_MODULEFILENAME_URLMON_DLL = 2,
	GMF_MODULEFILENAME_IEFRAME_DLL = 3,
	GMF_MODULEFILENAME_WININET_DLL = 4,
	GMF_MODULEFILENAME_IERTUTIL_DLL = 5,
	GMF_MODULEFILENAME_UNITY3D_DLL = 6,
	GMF_MODULEFILENAME_NPUNITY3D_DLL = 7,

	GMF_MODULEFILENAME_CNT = 20,
};
LPVOID g_lpModuleAddr[GMF_MODULEFILENAME_CNT][2] = {0};

BOOL GetModuleFileNameByAddress(LPTSTR lpszModule,DWORD dwSize,LPVOID lpAddress)
{
	MEMORY_BASIC_INFORMATION mbi={0};
	if(!VirtualQuery(lpAddress,&mbi,sizeof(mbi)) || 0==mbi.AllocationBase)
		return FALSE;

	return GetModuleFileName((HMODULE)mbi.AllocationBase,lpszModule,dwSize-1);
}

BOOL AddressInFlashModule(LPVOID lpAddress, HMODULE * pModule=NULL)
{
	TCHAR szModuleName[MAX_PATH]={0};
	if(GetModuleFileNameByAddress(szModuleName, _ARRAYSIZE(szModuleName)-1, lpAddress))
	{
		LPTSTR pszSep=(LPTSTR)_tcsrchr(szModuleName, _T('\\'));
		if(pszSep && 0==_tcsnicmp(++pszSep,_T("flash"),5))
		{
			UINT uLen=_tcslen(pszSep);
			if(_T('x')==pszSep[uLen-1] && _T('c')==pszSep[uLen-2] && _T('o')==pszSep[uLen-3] && _T('.')==pszSep[uLen-4])
			{
				if(pModule)
					*pModule = GetModuleHandle(szModuleName);
				return TRUE;
			}
		}
	}
	return FALSE;
}

BOOL AddressInModule(LPCTSTR lpszModuleName, LPVOID lpAddress)
{
	HANDLE hModule = GetModuleHandle(lpszModuleName);
	if(hModule)
	{
		MODULEINFO mi={0};
		if(GetModuleInformation(GetCurrentProcess(), (HMODULE)hModule, &mi, sizeof(mi)))
		{
			if(lpAddress>mi.lpBaseOfDll && lpAddress<((LPBYTE)mi.lpBaseOfDll + mi.SizeOfImage))
				return TRUE;
		}
	}
	return FALSE;
}

BOOL  AddressInModuleEx(LPCTSTR lpszModuleName, LPVOID lpAddress, LPVOID & lpModuleStart, LPVOID & lpModuleEnd){
	BOOL bRet = FALSE;
	if(!lpModuleStart || !lpModuleEnd)
	{	
		HANDLE hModule = GetModuleHandle(lpszModuleName);
		if(hModule)
		{
			MODULEINFO mi={0};
			if(GetModuleInformation(GetCurrentProcess(), (HMODULE)hModule, &mi, sizeof(mi)))
			{
				if(lpAddress>mi.lpBaseOfDll && lpAddress<((LPBYTE)mi.lpBaseOfDll + mi.SizeOfImage)){
					lpModuleStart = (LPBYTE)mi.lpBaseOfDll;
					lpModuleEnd = (LPBYTE)mi.lpBaseOfDll + mi.SizeOfImage;
				}
			}
		}	
	}

	if(lpModuleStart && lpModuleEnd){
		if(lpAddress>lpModuleStart && lpAddress<lpModuleEnd)
			bRet = true;
	}
	return bRet;	
}

BOOL IsNeedFixedFlashBug()
{
	/*Flash32_11_9_900_117.ocx通过GetModuleFileNameW必须是真实的IE路径。flash的bug必须是IE的真正路径，只伪装修改名字为iexplore.exe无效
	至于具体的原因，暂不清楚
	bool __stdcall Flash_IE_Patch(HMODULE hModule, int a2)
	{
		DWORD v2; // eax@1
		bool v3; // bl@1
		WCHAR Filename; // [sp+8h] [bp-20Ch]@1
		unsigned int v6; // [sp+210h] [bp-4h]@1
		int v7; // [sp+214h] [bp+0h]@1

		v6 = (unsigned int)&v7 ^ __security_cookie;
		v2 = GetModuleFileNameW(hModule, &Filename, 0x104u);
		v3 = v2 != 0;
		if ( v2 )
			sub_1005ECB6((void *)a2, (int)&Filename);
		return v3;
	}
	*/
	//win8.1不能伪装，否则土豆、优酷视频无法播放.这个现在已经可以播放了，是不是flash已经修好了？
	//win8+ie10  360云盘无法上传 Qzone空间背景音乐无法切换（flash与脚本交互错误，有大量的脚本错误）
	//win7+ie11  360云盘无法上传 Qzone空间背景音乐无法切换（flash与脚本交互错误，有大量的脚本错误）
	// 1: 非win8下黑屏问题，需要返回IE；
	// 2: 游戏网站不能登录。 例如：http://game.zg.qq.com/index.html
	// 3: win8下，不能打开flash视频。
  // 4: win8.1下，flash上传方式上传图片失败。 http://www.admin.ccoo.cn/yunying/news/v3/news_add.asp  测试用户名： yueyangtest12345678
	BOOL bNeed = FALSE;
	//if( !SystemInfo::IsWindows81())
	return TRUE;
}

#pragma optimize("",off)
DWORD WINAPI HookedGetModuleFileNameW(HMODULE hModule, LPWSTR lpFilename, DWORD nSize)
{
	LPVOID lpRetAddr = *(&hModule - 1);
	if(NULL == hModule){
		bool bInModule = false;

		//if( IE8CORE::IsCanUseIe8Core() && 0)
		{
			if(!bInModule && AddressInModuleEx(_T("8edvtool.dll"), lpRetAddr, g_lpModuleAddr[GMF_MODULEFILENAME_IEDVTOOL_DLL][0], g_lpModuleAddr[GMF_MODULEFILENAME_IEDVTOOL_DLL][1]))
				bInModule = true;
			if(!bInModule && AddressInModuleEx(_T("8shtml.dll"), lpRetAddr, g_lpModuleAddr[GMF_MODULEFILENAME_MSHTML_DLL][0], g_lpModuleAddr[GMF_MODULEFILENAME_MSHTML_DLL][1]))
				bInModule = true;
			//urlmon保证了HKEY_CURRENT_USER\Software\Microsoft\Internet Explorer\Main\FeatureControl 与IE保持一致，与applications hosting the WebBrowser Control不同
			//urlmon还影响到IE的渲染
			//参考http://msdn.microsoft.com/zh-cn/library/ee330720(v=vs.85).aspx  
			if(!bInModule && AddressInModuleEx(_T("8rlmon.dll"), lpRetAddr, g_lpModuleAddr[GMF_MODULEFILENAME_URLMON_DLL][0], g_lpModuleAddr[GMF_MODULEFILENAME_URLMON_DLL][1]))
				bInModule = true;
			if(!bInModule && AddressInModuleEx(_T("8ininet.dll"), lpRetAddr, g_lpModuleAddr[GMF_MODULEFILENAME_WININET_DLL][0], g_lpModuleAddr[GMF_MODULEFILENAME_WININET_DLL][1]))
				bInModule = true;
			//iertutil  bug:win8+ie10+outlook，导致mailto:失败 mshtml.dll!CMailtoProtocol::RunMailClient() 
			//为了解决SSL证书错误与IE保持一致
			//https://mss.newlandcomputer.com/ 512公钥
			//https://img.atpanel.com/1.gif?cache=4194818&pre=&scr=1920x1080&ul_chk=15092…tid=68beafcf6cf04ab9a91e9829cd56378e&channel=112&atpanel_end&upi_bi=0TAh8V
			if(!bInModule && AddressInModuleEx(_T("8ertutil.dll"), lpRetAddr, g_lpModuleAddr[GMF_MODULEFILENAME_IERTUTIL_DLL][0], g_lpModuleAddr[GMF_MODULEFILENAME_IERTUTIL_DLL][1]))
				bInModule = true;
			if(!bInModule && AddressInModuleEx(_T("8eframe.dll"), lpRetAddr, g_lpModuleAddr[GMF_MODULEFILENAME_IEFRAME_DLL][0], g_lpModuleAddr[GMF_MODULEFILENAME_IEFRAME_DLL][1]))
				bInModule = true;
			if(bInModule){
				//IE F12伪装
				::SHRegGetPath( HKEY_CURRENT_USER, _T("webgame\\IEStub\\8CLSID\\{1a6fe369-f28c-4ad9-a3e6-2bcb50807cf1}\\InProcServer32"),_T(""),lpFilename, nSize );
				::PathRemoveFileSpec( lpFilename );
				::PathAppend( lpFilename, _T("iexplore.exe") );
				return wcslen(lpFilename);
				// 			RealGetModuleFileNameW(NULL, lpFilename, nSize);
				// 			LPWSTR sep = wcsrchr(lpFilename, L'\\');
				// 			if (sep) 
				// 			{
				// 				*sep = L'\0';
				// 				PathAppend(lpFilename,L"iexplore.exe");//todo: swprintf_s(lpFilename, nSize, L"%s\\360se.exe", lpFilename); when nSize=1024,crash
				// 				return wcslen(lpFilename);
				// 			}
			}
		}
		//else
		{
			//dz@2013/5/13 修复IE8下，F12调试工具无法调试脚本及F12在IE10崩溃问题
			if(!bInModule && AddressInModuleEx(_T("iedvtool.dll"), lpRetAddr, g_lpModuleAddr[GMF_MODULEFILENAME_IEDVTOOL_DLL][0], g_lpModuleAddr[GMF_MODULEFILENAME_IEDVTOOL_DLL][1]))
				bInModule = true;
			if(!bInModule && AddressInModuleEx(_T("mshtml.dll"), lpRetAddr, g_lpModuleAddr[GMF_MODULEFILENAME_MSHTML_DLL][0], g_lpModuleAddr[GMF_MODULEFILENAME_MSHTML_DLL][1]))
				bInModule = true;
			//urlmon保证了HKEY_CURRENT_USER\Software\Microsoft\Internet Explorer\Main\FeatureControl 与IE保持一致，与applications hosting the WebBrowser Control不同
			//urlmon还影响到IE的渲染
			//参考http://msdn.microsoft.com/zh-cn/library/ee330720(v=vs.85).aspx  
			if(!bInModule && AddressInModuleEx(_T("urlmon.dll"), lpRetAddr, g_lpModuleAddr[GMF_MODULEFILENAME_URLMON_DLL][0], g_lpModuleAddr[GMF_MODULEFILENAME_URLMON_DLL][1]))
				bInModule = true;
			if(!bInModule && AddressInModuleEx(_T("wininet.dll"), lpRetAddr, g_lpModuleAddr[GMF_MODULEFILENAME_WININET_DLL][0], g_lpModuleAddr[GMF_MODULEFILENAME_WININET_DLL][1]))
				bInModule = true;
			//iertutil  bug:win8+ie10+outlook，导致mailto:失败 mshtml.dll!CMailtoProtocol::RunMailClient() 
			//为了解决SSL证书错误与IE保持一致
			//https://mss.newlandcomputer.com/ 512公钥
			//https://img.atpanel.com/1.gif?cache=4194818&pre=&scr=1920x1080&ul_chk=15092…tid=68beafcf6cf04ab9a91e9829cd56378e&channel=112&atpanel_end&upi_bi=0TAh8V
			if(!bInModule && AddressInModuleEx(_T("iertutil.dll"), lpRetAddr, g_lpModuleAddr[GMF_MODULEFILENAME_IERTUTIL_DLL][0], g_lpModuleAddr[GMF_MODULEFILENAME_IERTUTIL_DLL][1]))
				bInModule = true;
			if(!bInModule && AddressInModuleEx(_T("ieframe.dll"), lpRetAddr, g_lpModuleAddr[GMF_MODULEFILENAME_IEFRAME_DLL][0], g_lpModuleAddr[GMF_MODULEFILENAME_IEFRAME_DLL][1]))
				bInModule = true;
			if( IsNeedFixedFlashBug()&& !bInModule && AddressInFlashModule(lpRetAddr) )
				bInModule = true;
			if(bInModule){
				//IE F12伪装
				::SHRegGetPath( HKEY_CLASSES_ROOT, _T("CLSID\\{1a6fe369-f28c-4ad9-a3e6-2bcb50807cf1}\\InProcServer32"),_T(""),lpFilename, nSize );
				::PathRemoveFileSpec( lpFilename );
				::PathAppend( lpFilename, _T("iexplore.exe") );
				return wcslen(lpFilename);
				// 			RealGetModuleFileNameW(NULL, lpFilename, nSize);
				// 			LPWSTR sep = wcsrchr(lpFilename, L'\\');
				// 			if (sep) 
				// 			{
				// 				*sep = L'\0';
				// 				PathAppend(lpFilename,L"iexplore.exe");//todo: swprintf_s(lpFilename, nSize, L"%s\\360se.exe", lpFilename); when nSize=1024,crash
				// 				return wcslen(lpFilename);
				// 			}
			}

		}
		//unity3d 4.0.1.62179 do with 360se.exe we need this code,or RedDirect
// 		if( AddressInModuleEx(_T("UnityWebPluginAX.ocx"), lpRetAddr, g_lpModuleAddr[GMF_MODULEFILENAME_UNITY3D_DLL][0], g_lpModuleAddr[GMF_MODULEFILENAME_UNITY3D_DLL][1])||
// 			AddressInModuleEx(_T("npUnity3D32.dll"), lpRetAddr, g_lpModuleAddr[GMF_MODULEFILENAME_NPUNITY3D_DLL][0], g_lpModuleAddr[GMF_MODULEFILENAME_NPUNITY3D_DLL][1]))
// 		{
// 			RealGetModuleFileNameW(NULL, lpFilename, nSize);
// 			LPWSTR sep = wcsrchr(lpFilename, L'\\');
// 			if (sep) {
// 				*sep = L'\0';
// 				PathAppend(lpFilename,L"360se.exe");//todo: swprintf_s(lpFilename, nSize, L"%s\\360se.exe", lpFilename); when nSize=1024,crash
// 				return wcslen(lpFilename);
// 			}
// 		}
	}
	return RealGetModuleFileNameW(hModule, lpFilename, nSize);
}

DWORD WINAPI HookedGetModuleBaseNameW(HANDLE hProcess, HMODULE hModule, LPWSTR lpBaseName, DWORD nSize)
{
	//dz@2013/5/13 修复IE8下，F12调试工具无法attach 调试脚本 pdm.dll
	if(NULL == hModule){
		bool bInModule = false;
		LPVOID lpRetAddr = *(&hProcess - 1);
		//if( IE8CORE::IsCanUseIe8Core() && 0)
		{
			if(AddressInModuleEx(_T("8shtml.dll"), lpRetAddr, g_lpModuleAddr[GMF_MODULEFILENAME_MSHTML_DLL][0], g_lpModuleAddr[GMF_MODULEFILENAME_MSHTML_DLL][1]))
				bInModule = true;
		}
		//else
		{
			if(AddressInModuleEx(_T("mshtml.dll"), lpRetAddr, g_lpModuleAddr[GMF_MODULEFILENAME_MSHTML_DLL][0], g_lpModuleAddr[GMF_MODULEFILENAME_MSHTML_DLL][1]))
				bInModule = true;
		}
		if(bInModule){
			_tcsncpy_s(lpBaseName, nSize, _T("iexplore.exe"), _TRUNCATE);
			return wcslen(lpBaseName);
		}
	}
	return RealGetModuleBaseNameW(hProcess, hModule, lpBaseName, nSize);
}
#pragma optimize("",on)
//关闭：SmartScreen筛选和activex筛选引起崩溃
const GUID IEVALUE_PhishingFilter_EnabledState = {0x62a03e46, 0x344d, 0x4a3a, 0x82, 0x63, 0x44, 0xeb, 0x73, 0x02, 0x25, 0x8f};
const GUID IEVALUE_ActiveXFiltering_IsEnabled = {0x6c93e63f, 0x6f11, 0x458b, 0x8a, 0xee, 0x92, 0xcc, 0x3d, 0x26, 0x36, 0x3a};
//开启：IE高级中的“加速图形”解决一些显示问题  HKEY_CURRENT_USER\Software\Microsoft\Internet Explorer\Main UseSWRender
const GUID IEVALUE_GUID_Browser_UseSWRender = {0xdda2d0e0,0x568f,0x4f9b,0xbd,0x38,0xe9,0x35,0xa3,0x2d,0xcf,0xfa};
//勾选“在兼容性视图中显示Internet站点”，兼容性视图功能必须勾选
const GUID Featue_BrowserEmulation_MSCompatibilityMode = {0x9BB0FC82, 0x8A74, 0x4BB9, 0x8A, 0xBE, 0x57, 0x27, 0x2E, 0x7C, 0xFB, 0x20};
const GUID IEVALUE_GUID_BrowserEmulation_UserFilter = {0x885a43d4,0x7867,0x4d01,0xad,0x70,0xa2,0x62,0xb1,0x8e,0x7f,0x94};

const GUID IEKEY_GUID_Ried_InternetExplorer_LowRights = {0x42e1483f,0x29a0,0x45d9,0x8a,0x9d,0xe0,0x55,0x16,0x05,0x96,0x69};
const GUID IEVALUE_GUID_Ried_InternetSettings_Zones_32500 ={0x950854ab,0x3450,0x4e88,0xa8,0xf3,0x17,0xb9,0x92,0x49,0xf8,0x28};

HRESULT WINAPI HookedIEUtilGetValue(struct _GUID const  *guid, int nDeserve, int  dataTypeFlag, PBYTE retBuf, int cbBuf, int nDeserve1, int nDeserve2 )
{
	if(guid && retBuf)
	{
		//ie保护模式，关闭不掉的。不走这里
		if(IsEqualGUID(IEVALUE_GUID_Ried_InternetSettings_Zones_32500, *guid))
		{
			DWORD dwValue = 3;
			memcpy(retBuf, &dwValue,sizeof(DWORD));
			return S_OK;
		}
		if(IsEqualGUID(IEVALUE_PhishingFilter_EnabledState, *guid))
		{
			memset(retBuf, 0, cbBuf);
			return S_OK;
		}
		//为了解决崩溃，关闭功能，但是会引起了BUG：win8 x64 ie10下另存为下载不能结束的问题
		if(IsEqualGUID(IEVALUE_ActiveXFiltering_IsEnabled, *guid))
		{
			memset(retBuf, 0, cbBuf);
			return S_OK;
		}
		// Enable webgpu under ie9/ie10
		if(IsEqualGUID(IEVALUE_GUID_Browser_UseSWRender,*guid))
		{
// 			if(_bIE9 || _bIE10 || _bIE11){
// 				DWORD dwValue = 1;
// 				memcpy(retBuf,&dwValue,sizeof(DWORD));
// 				return S_OK;
// 			}
		}
		else if( IsEqualGUID(Featue_BrowserEmulation_MSCompatibilityMode, *guid) )
		{
			*(DWORD*)retBuf = 1;
			return S_OK;
		}
		else if( IsEqualGUID(IEVALUE_GUID_BrowserEmulation_UserFilter,*guid))
		{
			HRESULT hr =  RealIEUtilGetVal(guid, nDeserve, dataTypeFlag, retBuf, cbBuf, nDeserve1, nDeserve2);

			return hr;
		}
	}
	return  RealIEUtilGetVal(guid, nDeserve, dataTypeFlag, retBuf, cbBuf, nDeserve1, nDeserve2);
}


HRESULT WINAPI HookedOleRegGetMiscStatus(REFCLSID clsid,DWORD dwAspect,DWORD *pdwStatus)
{
	if( (IsEqualCLSID(clsid ,CLSID_UnityWebPlayerControl ))&& pdwStatus)
	{
		DWORD status = 0x20191;
		*pdwStatus = status;
		return S_OK;
	}
	return RealOleRegGetMiscStatus(clsid,dwAspect,pdwStatus);
}

HRESULT WINAPI HookedCLSIDFromProgID( LPCOLESTR lpszProgID, LPCLSID pclsid)
{
	if( lpszProgID&&pclsid)
	{
		if( !wcscmp( lpszProgID ,L"UnityWebPlayer.UnityWebPlayer.1") )
		{
			*pclsid = CLSID_UnityWebPlayerControl;
			return S_OK;
		}
		else if( !wcscmp( lpszProgID ,L"FANCY3DOCX.Fancy3DOCXCtrl.1")) 
		{
			*pclsid = CLSID_FancyGame;
			return S_OK;
		}
	}
	return RealCLSIDFromProgID( lpszProgID, pclsid); 
}


ITypeLib* CreateTypeLib(REFGUID rguid)
{
	return NULL;
}


HRESULT WINAPI HookedLoadRegTypeLib(REFGUID rguid, unsigned short  wVerMajor, unsigned short  wVerMinor, LCID  lcid, ITypeLib FAR* FAR  *pptlib)
{
	*pptlib = CreateTypeLib(rguid);
	if( *pptlib )
		return S_OK;
	return RealLoadRegTypeLib( rguid, wVerMajor, wVerMinor, lcid, pptlib );
}

LONG WINAPI HookedRegOpenKeyExW (HKEY hKey,LPCWSTR lpSubKey,DWORD ulOptions,REGSAM samDesired,PHKEY phkResult)
{
	//like this function not do system funtion，like:LoadLibrary GetModuleHandle SHGetSpecialFolderPath because this function have LOCK ,may lead to DeadLock.
	if( hKey == HKEY_CURRENT_USER&& lpSubKey)
	{
		if( (0== wcscmp(lpSubKey,L"SOFTWARE\\Unity\\WebPlayer")) || 
			(0==wcscmp(lpSubKey,L"Software\\MozillaPlugins\\@unity3d.com/UnityPlayer,version=1.0"))
			)
		{
			lpSubKey = _T("Software\\3DWebGame\\unity3d");
		}
	}
	return RealRegOpenKeyExW(hKey,lpSubKey,ulOptions,samDesired,phkResult);
}

void BuildRegDirect()
{
	//unity3d_4.1.2.1635 依赖注册表 //todo:this must set in InstallPacket
	DWORD dwValue =0;
	DWORD dwType = REG_DWORD;
	DWORD dwSize = sizeof(DWORD);
	SHGetValue(HKEY_CURRENT_USER,_T("Software\\3DWebGame"),_T("InitStatus"),&dwType,&dwValue,&dwSize);
	if( dwValue == 1)
		return;
	dwValue =1;
	SHSetValue(HKEY_CURRENT_USER,_T("Software\\3DWebGame"),_T("InitStatus"),REG_DWORD,&dwValue,dwSize);
	TCHAR szDirectory[MAX_PATH]={0};
	TCHAR szPath[MAX_PATH]={0};
	//wstring szExePath;
	//_App.GetExePath(szExePath);
	//_sntprintf_s(szDirectory,_ARRAYSIZE(szDirectory)-1,_T("%s\\3dplugin\\unity3d"),szExePath.c_str());
	//_sntprintf_s(szPath,_ARRAYSIZE(szPath)-1,_T("%s\\3dplugin\\unity3d\\loader\\npUnity3D32.dll"),szExePath.c_str());
	SHSetValue(HKEY_CURRENT_USER,_T("Software\\3DWebGame\\unity3d"),_T("Directory"),REG_SZ,szDirectory,(_tcslen(szDirectory)+1)*sizeof(TCHAR));  //todo:here cbData must not have enough zero like this wrong:sizeof(szDirectory)
	SHSetValue(HKEY_CURRENT_USER,_T("Software\\3DWebGame\\unity3d"),_T("Path"),REG_SZ,szPath,(_tcslen(szPath)+1)*sizeof(TCHAR));
}

HRESULT WINAPI HookedCoGetClassObject(REFCLSID rclsid, DWORD dwClsContext, LPVOID pvReserved, REFIID riid, LPVOID FAR* ppv)
{
	HRESULT hRet = S_FALSE;
	if( IsEqualCLSID( CLSID_UnityWebPlayerControl, rclsid ) )
	{
// 		wstring szPath;
// 		if( szPath.empty())
// 		{
// 			_App.GetExePath(szPath);
// 			szPath=szPath+L"\\modules\\ActiveX\\3dplugin\\unity3d\\loader\\UnityWebPluginAX.ocx";
// 		}
// 
// 		HMODULE hModule = GetModuleHandle(szPath.c_str());
// 		if( !hModule )
// 			hModule = ::LoadLibraryW(szPath.c_str());
// 		if( hModule )
// 		{
// 			PROC_DllGetClassObject DllGetClassObject = (PROC_DllGetClassObject)GetProcAddress(hModule,"DllGetClassObject");
// 			if( DllGetClassObject)
// 				hRet =DllGetClassObject(rclsid, riid, ppv);
// 		}
// 		return hRet;
	}
	else if(  IsEqualCLSID( CLSID_FancyGame, rclsid ))
	{
// 		wstring szFancyPath;
// 		if( szFancyPath.empty())
// 		{
// 			_App.GetExePath(szFancyPath);
// 			szFancyPath=szFancyPath+L"\\modules\\ActiveX\\3dplugin\\fancy3d\\fancygame.ocx";
// 		}

// 		HMODULE hModule = GetModuleHandle(szFancyPath.c_str());
// 		if( !hModule )
// 			hModule = ::LoadLibraryW(szFancyPath.c_str());
// 		if( hModule )
// 		{
// 			PROC_DllGetClassObject DllGetClassObject = (PROC_DllGetClassObject)GetProcAddress(hModule,"DllGetClassObject");
// 			if( DllGetClassObject)
// 				hRet =DllGetClassObject(rclsid, riid, ppv);
// 		}
		return hRet;
	}
	return RealCoGetClassObject(rclsid, dwClsContext, pvReserved, riid, ppv);
}
void APIHookBegin()
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
}
void APIHookEnd()
{
	DetourTransactionCommit();
}
void StartProcessAPIHook()
{
  return;
	if(NULL==g_hCreateThreadHeap)
		g_hCreateThreadHeap=HeapCreate(0, 0, 0);

		APIHookBegin();
		
		//DetourAttach(&(PVOID&)RealCreateThread,HookedCreateThread);

		//DetourAttach(&(PVOID&)RealSetUnhandledExceptionFilter,HookedSetUnhandledExceptionFilter);
		//DetourAttach(&(PVOID&)RealUnhandledExceptionFilter,HookedUnhandledExceptionFilter);

		HMODULE ole32 = GetModuleHandle(_T("ole32.dll"));
		if (!ole32)
			ole32 = LoadLibrary(_T("ole32.dll"));

		if( ole32 ) {
			RealCoGetClassObject= ( PROC_CoGetClassObject)GetProcAddress(ole32, "CoGetClassObject");
			//if( RealCoGetClassObject )
				//DetourAttach(&(PVOID&)RealCoGetClassObject, HookedCoGetClassObject);
			RealCLSIDFromProgID= ( PROC_CLSIDFromProgID)GetProcAddress(ole32, "CLSIDFromProgID");
			//if( RealCLSIDFromProgID )
				//DetourAttach(&(PVOID&)RealCLSIDFromProgID, HookedCLSIDFromProgID);

			RealOleRegGetMiscStatus= ( PROC_OleRegGetMiscStatus)GetProcAddress(ole32, "OleRegGetMiscStatus");
			//if( RealOleRegGetMiscStatus )
				//DetourAttach(&(PVOID&)RealOleRegGetMiscStatus, HookedOleRegGetMiscStatus);
		}
// 		//todo:unity3d don't need this hook.other activex may need example:Flash QVOD
// 		HMODULE oleaut32 = GetModuleHandle(_T("OLEAUT32.dll"));
// 		if (!oleaut32)
// 			oleaut32 = LoadLibrary(_T("OLEAUT32.dll"));
// 		if( oleaut32 ) {
// 			RealLoadRegTypeLib = (PROC_LoadRegTypeLib)GetProcAddress(oleaut32, "LoadRegTypeLib");
// 			if( RealLoadRegTypeLib )
// 				DetourAttach(&(PVOID&)RealLoadRegTypeLib, HookedLoadRegTypeLib);
// 		}
		//
		BuildRegDirect();
		HMODULE kernel32 = GetModuleHandle(_T("kernel32.dll"));
		if (!kernel32)
			kernel32 = LoadLibrary(_T("kernel32.dll"));
		if( kernel32 ) {
			RealRegOpenKeyExW = (PROC_RegOpenKeyExW)GetProcAddress(kernel32, "RegOpenKeyExW");
			//if( RealRegOpenKeyExW )
				//DetourAttach(&(PVOID&)RealRegOpenKeyExW, HookedRegOpenKeyExW);
		}
		//
		HMODULE iertutil = GetModuleHandle(_T("iertutil.dll"));
		if (!iertutil)
			iertutil = LoadLibrary(_T("iertutil.dll"));
		if( iertutil ) {
			RealIEUtilGetVal = (PROC_IEUTIL_GETVALUE)GetProcAddress(iertutil, (LPCSTR)650);
			//if(RealIEUtilGetVal)
				//DetourAttach(&(PVOID&)RealIEUtilGetVal, HookedIEUtilGetValue);
		}
		APIHookEnd();
}
void StartBrowserProcessAPIHook()
{
  return;
	//由于当前是单进程结构 因此这个函数只做一次
	static BOOL bHook=FALSE;
	if( bHook)
		return;
	bHook = TRUE;   //优先置变量，及时的解除多线程风险
	APIHookBegin();
	{
		HMODULE urlmon = GetModuleHandle(_T("urlmon.dll"));
		if (!urlmon)
			urlmon = LoadLibrary(_T("urlmon.dll"));
		if( urlmon ) {
			RealCoInternetCreateZoneManager = (HRESULT (__stdcall *)(IServiceProvider *,IInternetZoneManager **,DWORD))GetProcAddress(urlmon, "CoInternetCreateZoneManager");
			//if (RealCoInternetCreateZoneManager)
				//DetourAttach(&(PVOID&)RealCoInternetCreateZoneManager, HookedCoInternetCreateZoneManager);

			RealCoInternetCreateSecurityManager = (HRESULT (__stdcall *)(IServiceProvider *,IInternetSecurityManager **,DWORD))GetProcAddress(urlmon, "CoInternetCreateSecurityManager");
			//if (RealCoInternetCreateSecurityManager)
				//DetourAttach(&(PVOID&)RealCoInternetCreateSecurityManager, HookedCoInternetCreateSecurityManager);
		}
		//CHtmlDocumentHook::StartAPIHook();
		CIECompatibilityView::StartAPIHook();
    //CUserAgentHelper::StartAPIHook();
    //CUserAgentHelper::StartNavigatorHook();
    //CHtmlWindowHook::StartAPIHook();
	}
	APIHookEnd();
}

void CheckHookedGetModuleFileName()
{
  DWORD dwValue=0;
  DWORD dwType = REG_DWORD;
  DWORD dwSize = sizeof(DWORD);
  SHGetValueW(HKEY_CURRENT_USER,L"Software\\360\\360se6",L"CheckHook",&dwType,&dwValue,&dwSize);
//   if(dwValue!=1)
//     return;
  //we only check 5 iecore module
  const WCHAR szModuleName[][32]=
  {
    L"urlmon.dll",
    L"wininet.dll",
    L"iertutil.dll",
    L"ieframe.dll",
    L"mshtml.dll",
  };
  std::wstring text = L"IE内核模块伪装失败";
  bool bFailed=false;
  for (int i=0;i<_ARRAYSIZE(szModuleName);++i)
  {
    HMODULE hModule = NULL;
    hModule = ::GetModuleHandleW(szModuleName[i]);
    if(hModule)
    {
      text+=szModuleName[i];
      bFailed=true;
    }
  }
  if(bFailed)
    ::MessageBoxW(NULL,text.c_str(),L"IE内核伪装失效",MB_OK);
}

void DisguisedAsIE()
{
  CheckHookedGetModuleFileName();
  SystemInfo::GetWindowInfo();
  SystemInfo::GetIEVersion();
	/*伪装为IE:
	wininet.dll
	urlmon.dll
	iertutil.dll
	ieframe.dll
	mshtml.dll
	*/
	APIHookBegin();
	{
		DetourAttach(&(PVOID&)RealGetModuleFileNameW,HookedGetModuleFileNameW);
    //只针对IE8来做
    DetourAttach(&(PVOID&)RealGetModuleBaseNameW, HookedGetModuleBaseNameW);
	}
	APIHookEnd();
}
