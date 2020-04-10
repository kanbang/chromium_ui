

//高速渲染（即兼容性视图功能），由于IE9&IE10，CoInternetSetBrowserEmulationMode定义不同，做不同的处理
//优先使用IE的兼容视图名单，然后是我们自己的名单
//IE9下这个结构是9*DWORD空间，IE10下这个结构是0x4D*DWORD个空间，这个函数就是把这个结构给复制到另一块内存区域。
//SE定义这个结构，0x50*DWORD个空间
#pragma once
#include "chrome/iecore/trident/preheader.h"
#include <WinInet.h>
#pragma pack(2)
typedef struct _tagHeadInfo
{
	DWORD dwVerLow;
	DWORD dwVerHigh;
	DWORD dwAttr;
	DWORD dwFeature;
	DWORD dwReserve;
	short unknown;
}HeadInfo; //sizeof = 26 bytes
#pragma pack()

typedef struct _tagEmulationMode
{
	DWORD dwCurMode;
	DWORD dwBrowserMode;   //设置其为IE7模式，IE9&IE10给的参数是0x201,含义不清楚
	DWORD dwReserve[0x4E];
}EmulationMode, *PEmulationMode;

//IE8下函数为：urlmon!CoInternetSetBrowserEmulationMode
typedef HRESULT (__stdcall* pfnCoInternetSetBrowserEmulationMode)(	DWORD dwCurMode	);
//IE9&IE10下函数为：urlmon!CoInternetSetBrowserEmulationState
typedef HRESULT (__stdcall* pfnCoInternetSetBrowserEmulationState)(	PEmulationMode	);

typedef BOOL (WINAPI *pfnGetUrlCacheEntryInfoA)(LPCSTR lpszUrlName, LPINTERNET_CACHE_ENTRY_INFOA lpCacheEntryInfo, LPDWORD lpcbCacheEntryInfo);
typedef BOOL (WINAPI *pfnGetUrlCacheEntryInfoW)(LPCWSTR lpszUrlName, LPINTERNET_CACHE_ENTRY_INFOW lpCacheEntryInfo, LPDWORD lpcbCacheEntryInfo);

//funtion 
class CIECompatibilityView
{
public:
	CIECompatibilityView();
	~CIECompatibilityView();

	static pfnGetUrlCacheEntryInfoA RealGetUrlCacheEntryInfoA;
	static pfnGetUrlCacheEntryInfoW RealGetUrlCacheEntryInfoW ;
	static BOOL WINAPI HookedGetUrlCacheEntryInfoA(LPCSTR lpszUrlName, LPINTERNET_CACHE_ENTRY_INFOA lpCacheEntryInfo, LPDWORD lpcbCacheEntryInfo);
	static BOOL WINAPI HookedGetUrlCacheEntryInfoW(LPCWSTR lpszUrlName, LPINTERNET_CACHE_ENTRY_INFOW lpCacheEntryInfo, LPDWORD lpcbCacheEntryInfo);
	static CIECompatibilityView& GetInstance()
	{
		static CIECompatibilityView instance;
		return instance;
	}
	static void StartAPIHook();
private:
	BOOL ProxyGetUrlCacheEntryInfoA(LPCSTR lpszUrlName, LPINTERNET_CACHE_ENTRY_INFOA lpCacheEntryInfo, LPDWORD lpcbCacheEntryInfo); 
	BOOL ProxyGetUrlCacheEntryInfoW(LPCWSTR lpszUrlName, LPINTERNET_CACHE_ENTRY_INFOW lpCacheEntryInfo, LPDWORD lpcbCacheEntryInfo);

	BOOL GetCompatViewListVersion(DWORD* pVerHigh, DWORD* pVerLow);
};

