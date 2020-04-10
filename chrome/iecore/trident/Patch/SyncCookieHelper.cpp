#include "chrome/iecore/trident/stdafx.h"
#include "SyncCookieHelper.h"
#include "chrome/iecore/third_party/detourshook.h"
#include <wininet.h>
#pragma comment(lib,"wininet.lib")
//#include "SystemInfo.h"

pfnInternetSetCookieA CSyncCookieHelper::RealInternetSetCookieA=NULL;
pfnInternetSetCookieW CSyncCookieHelper::RealInternetSetCookieW=NULL;
pfnInternetSetCookieExA CSyncCookieHelper::RealInternetSetCookieExA=NULL;
pfnInternetSetCookieExW CSyncCookieHelper::RealInternetSetCookieExW=NULL;
pfnInternetGetCookieExA CSyncCookieHelper::RealInternetGetCookieExA=NULL;
pfnInternetGetCookieExW CSyncCookieHelper::RealInternetGetCookieExW=NULL;
//以下是cookie伪装 用来解决HTTP Request Header丢cookie
pfnHttpOpenRequestA CSyncCookieHelper::RealHttpOpenRequestA=NULL;
pfnHttpOpenRequestW CSyncCookieHelper::RealHttpOpenRequestW=NULL;
pfnInternetSetStatusCallbackA CSyncCookieHelper::RealInternetSetStatusCallbackA=NULL;

INTERNET_STATUS_CALLBACK _lpfnInternetCallback=NULL;
extern BOOL _bIE11;

CSyncCookieHelper::CSyncCookieHelper()
{

}

CSyncCookieHelper::~CSyncCookieHelper()
{

}

void CSyncCookieHelper::StartAPIHook()
{
	HMODULE wininet = GetModuleHandle(_T("wininet.dll"));
	if (!wininet)
		wininet = LoadLibrary(_T("wininet.dll"));
	if( wininet ) {
		RealHttpOpenRequestA = (pfnHttpOpenRequestA)GetProcAddress(wininet, "HttpOpenRequestA");
// 		if( RealHttpOpenRequestA )
// 			DetourAttach(&(PVOID&)RealHttpOpenRequestA,HookedHttpOpenRequestA);
		RealHttpOpenRequestW = (pfnHttpOpenRequestW)GetProcAddress(wininet, "HttpOpenRequestW");
// 		if( RealHttpOpenRequestW )
// 			DetourAttach(&(PVOID&)RealHttpOpenRequestW,HookedHttpOpenRequestW);
		RealInternetSetCookieA = (pfnInternetSetCookieA)GetProcAddress(wininet, "InternetSetCookieA");
// 		if( RealInternetSetCookieA )
// 			DetourAttach(&(PVOID&)RealInternetSetCookieA,HookedInternetSetCookieA);
		RealInternetSetCookieW = (pfnInternetSetCookieW)GetProcAddress(wininet, "InternetSetCookieW");
// 		if( RealInternetSetCookieW )
// 			DetourAttach(&(PVOID&)RealInternetSetCookieW,HookedInternetSetCookieW);
		RealInternetSetCookieExA = (pfnInternetSetCookieExA)GetProcAddress(wininet, "InternetSetCookieExA");
// 		if( RealInternetSetCookieExA )
// 			DetourAttach(&(PVOID&)RealInternetSetCookieExA,HookedInternetSetCookieExA);
		RealInternetSetCookieExW = (pfnInternetSetCookieExW)GetProcAddress(wininet, "InternetSetCookieExA");
// 		if( RealInternetSetCookieExW )
// 			DetourAttach(&(PVOID&)RealInternetSetCookieExW,HookedInternetSetCookieExW);
		if( _bIE11)
		{
			RealInternetGetCookieExA = (pfnInternetGetCookieExA)GetProcAddress(wininet, "InternetGetCookieExA");
// 			if( RealInternetGetCookieExA )
// 				DetourAttach(&(PVOID&)RealInternetGetCookieExA,HookedInternetGetCookieExA);
		}
		else
		{
			RealInternetGetCookieExW = (pfnInternetGetCookieExW)GetProcAddress(wininet, "InternetGetCookieExW");
// 			if( RealInternetGetCookieExW )
// 				DetourAttach(&(PVOID&)RealInternetGetCookieExW,HookedInternetGetCookieExW);
		}

		RealInternetSetStatusCallbackA = (pfnInternetSetStatusCallbackA)GetProcAddress(wininet, "InternetSetStatusCallbackA");
// 		if( RealInternetSetStatusCallbackA )
// 			DetourAttach(&(PVOID&)RealInternetSetStatusCallbackA,HookedInternetSetStatusCallbackA);
	}
}

HINTERNET  CSyncCookieHelper::ProxyHttpOpenRequestA(HINTERNET hConnect,LPCSTR lpszVerb,LPCSTR lpszObjectName,LPCSTR lpszVersion,LPCSTR lpszReferrer,LPCSTR FAR * lplpszAcceptTypes,DWORD dwFlags,DWORD_PTR dwContext)
{
	if (0 == dwContext)
		dwContext = 0x19821982;
	HINTERNET hRequest = RealHttpOpenRequestA(hConnect, lpszVerb, lpszObjectName, lpszVersion, lpszReferrer, lplpszAcceptTypes, dwFlags, dwContext);
	if (hRequest) 
	{
		if ((dwFlags & INTERNET_FLAG_NO_COOKIES) != INTERNET_FLAG_NO_COOKIES) 
		{
			CheatIECoookie(hConnect);
		}
	}
	return hRequest;
}
HINTERNET  CSyncCookieHelper::ProxyHttpOpenRequestW(HINTERNET hConnect,LPCWSTR lpszVerb,LPCWSTR lpszObjectName,LPCWSTR lpszVersion,LPCWSTR lpszReferrer,LPCWSTR FAR * lplpszAcceptTypes,DWORD dwFlags,DWORD_PTR dwContext)
{
	if (0 == dwContext)
		dwContext = 0x19821982;
	HINTERNET hRequest = RealHttpOpenRequestW(hConnect, lpszVerb, lpszObjectName, lpszVersion, lpszReferrer, lplpszAcceptTypes, dwFlags, dwContext);
	if (hRequest) 
	{
		if ((dwFlags & INTERNET_FLAG_NO_COOKIES) != INTERNET_FLAG_NO_COOKIES) 
		{
			CheatIECoookie(hConnect);
		}
	}
	return hRequest;
}
BOOL  CSyncCookieHelper::ProxyInternetSetCookieA(LPCSTR lpszUrl,LPCSTR lpszCookieName,LPCSTR lpszCookieData) 
{
	return RealInternetSetCookieA(lpszUrl,lpszCookieName,lpszCookieData);
}
BOOL  CSyncCookieHelper::ProxyInternetSetCookieW(LPCWSTR lpszUrl,LPCWSTR lpszCookieName,LPCWSTR lpszCookieData)
{
	return RealInternetSetCookieW(lpszUrl,lpszCookieName,lpszCookieData);
}
DWORD  CSyncCookieHelper::ProxyInternetSetCookieExA(LPCSTR lpszUrl,LPCSTR lpszCookieName,LPCSTR lpszCookieData,DWORD dwFlags,DWORD_PTR dwReserved)
{
	return RealInternetSetCookieExA( lpszUrl, lpszCookieName, lpszCookieData, dwFlags, dwReserved);
}
DWORD  CSyncCookieHelper::ProxyInternetSetCookieExW(LPCWSTR lpszUrl,LPCWSTR lpszCookieName,LPCWSTR lpszCookieData,DWORD dwFlags,DWORD_PTR dwReserved) 
{
	return RealInternetSetCookieExW( lpszUrl, lpszCookieName, lpszCookieData, dwFlags, dwReserved);
}
BOOL  CSyncCookieHelper::ProxyInternetGetCookieExA(LPCSTR lpszUrl,LPCSTR lpszCookieName,LPSTR lpszCookieData,LPDWORD pcchCookieData,DWORD dwFlags,LPVOID dwReserved) 
{
	return RealInternetGetCookieExA( lpszUrl, lpszCookieName, lpszCookieData, pcchCookieData, dwFlags, dwReserved) ;
}
//add to support IE11
BOOL  CSyncCookieHelper::ProxyInternetGetCookieExW(LPCWSTR lpszUrl, LPCWSTR lpszCookieName, LPWSTR lpszCookieData, LPDWORD pcchCookieData, DWORD dwFlags, LPVOID dwReserved) 
{
	return RealInternetGetCookieExW( lpszUrl, lpszCookieName, lpszCookieData, pcchCookieData, dwFlags, dwReserved) ;
}

static BOOL AddressInModule(LPCTSTR lpszModuleName, LPVOID lpAddress)
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

INTERNET_STATUS_CALLBACK  CSyncCookieHelper::ProxyInternetSetStatusCallbackA(HINTERNET hInternet,INTERNET_STATUS_CALLBACK lpfnInternetCallback)
{
	LPVOID lpRetAddr=*(&hInternet-1);
	if(AddressInModule(_T("urlmon.dll"),lpRetAddr))
	{
		_lpfnInternetCallback=lpfnInternetCallback;
		lpfnInternetCallback = InternetCallbackFunc;
	}
	return RealInternetSetStatusCallbackA(hInternet, lpfnInternetCallback);
}

void CALLBACK CSyncCookieHelper::InternetCallbackFunc(HINTERNET hInternet,DWORD_PTR dwContext,DWORD dwInternetStatus,LPVOID lpvStatusInformation,DWORD dwStatusInformationLength)
{
	switch(dwInternetStatus)
	{
	case INTERNET_STATUS_RESOLVING_NAME:
		onResolvingName(hInternet, lpvStatusInformation);
		break;

	case INTERNET_STATUS_COOKIE_RECEIVED:
		onCookieReceived(hInternet);
		break;

	case INTERNET_STATUS_COOKIE_SENT:
		onCookieSent(hInternet);
		break;

	case INTERNET_STATUS_REDIRECT: 
		CheatIECoookieForRedirect(hInternet);
		break;
	}
	//fix bug:503 
	//如果是自己设置的dwcontext，就不要调用了callback了
	if (0x19821982 != dwContext)
		_lpfnInternetCallback(hInternet, dwContext, dwInternetStatus, lpvStatusInformation, dwStatusInformationLength);
}

void CSyncCookieHelper::onCookieReceived(HINTERNET hInternet)
{
	CHAR szUrl[INTERNET_MAX_URL_LENGTH]={0};
	DWORD dwSize=sizeof(szUrl)-1;

	if(InternetQueryOptionA(hInternet,INTERNET_OPTION_URL,szUrl,&dwSize) && '\0'!=szUrl[0])
	{
		DWORD dwIndex=0;
		while(dwIndex!=ERROR_HTTP_HEADER_NOT_FOUND)
		{
			dwSize=0;
			HttpQueryInfoA(hInternet,HTTP_QUERY_SET_COOKIE,NULL,&dwSize,&dwIndex);

			if(0!=dwSize)
			{
				LPSTR lpBuffer=new CHAR[dwSize+1];
				if(lpBuffer)
				{
					memset(lpBuffer,0,dwSize+1);
					if(HttpQueryInfoA(hInternet,HTTP_QUERY_SET_COOKIE,lpBuffer,&dwSize,&dwIndex))
						SetChromeCookie(szUrl,lpBuffer,true);
					else
						dwIndex=ERROR_HTTP_HEADER_NOT_FOUND;
					delete[] lpBuffer;
				}
			}
			else
				dwIndex=ERROR_HTTP_HEADER_NOT_FOUND;
		}
	}
}

void CSyncCookieHelper::onCookieSent(HINTERNET hInternet)
{
	CHAR szUrl[INTERNET_MAX_URL_LENGTH]={0};
	DWORD dwSize=sizeof(szUrl)-1;

	if(InternetQueryOptionA(hInternet,INTERNET_OPTION_URL,szUrl,&dwSize) && '\0'!=szUrl[0])
	{
		LPCSTR pszCookieHeader="Cookie: ";
		const int nCookieHeaderLen=strlen(pszCookieHeader);

		CHAR szCookie[INTERNET_MAX_URL_LENGTH]={0};
		dwSize=sizeof(szCookie)-nCookieHeaderLen-3;
		LPSTR pszCookie=szCookie;
		BOOL bNeedFree=FALSE;

		if(!GetChromeCookie(szUrl,pszCookie+nCookieHeaderLen,&dwSize,true) && ERROR_INSUFFICIENT_BUFFER==GetLastError())
		{
			bNeedFree=TRUE;
			pszCookie=new CHAR[dwSize+nCookieHeaderLen+3];
			if(pszCookie)
			{
				memset(pszCookie,0,dwSize+nCookieHeaderLen+3);
				GetChromeCookie(szUrl,pszCookie+nCookieHeaderLen,&dwSize,true);
			}
		}

		if(pszCookie)
		{
			strncpy(pszCookie,pszCookieHeader,nCookieHeaderLen);
			strcat(pszCookie,"\r\n");
			HttpAddRequestHeadersA(hInternet,pszCookie,-1,HTTP_ADDREQ_FLAG_ADD|HTTP_ADDREQ_FLAG_REPLACE);

			if(bNeedFree)
				delete[] pszCookie;
		}
	}
}


void CSyncCookieHelper::onResolvingName(HINTERNET hInternet, LPVOID lpvStatusInformation)
{
	if(lpvStatusInformation)
	{
		CHAR szUrl[MAX_URL_LEN]={0};
		_snprintf_s(szUrl,_ARRAYSIZE(szUrl)-1,_TRUNCATE,"http://%s",(LPSTR)lpvStatusInformation);

		DWORD dwSize=0;
		if( RealInternetGetCookieExA )
		{
			RealInternetGetCookieExA(szUrl,NULL,NULL,&dwSize,0,NULL);
			if(0==dwSize) {
				CHAR szCookie[MAX_PATH]={0};
				_snprintf_s(szCookie,_ARRAYSIZE(szCookie)-1,_TRUNCATE,"%s=%s; path=/","theworld_client_none","none");
				RealInternetSetCookieExA(szUrl,NULL,szCookie,0,0);  //InternetSetCookieExA是最终实现函数,相当于种个假的IE cookie
				onCookieSent(hInternet);
			}
		}
		else if( RealInternetGetCookieExW)
		{
			USES_CONVERSION;
			RealInternetGetCookieExW(A2W(szUrl),NULL,NULL,&dwSize,0,NULL);
			if(0==dwSize) {
				WCHAR szCookie[MAX_PATH]={0};
				_snwprintf_s(szCookie,_ARRAYSIZE(szCookie)-1,L"%s=%s; path=/",L"theworld_client_none",L"none");
				RealInternetSetCookieExW(A2W(szUrl),NULL,szCookie,0,0); ////InternetSetCookieExA是最终实现函数
				onCookieSent(hInternet);
			}
		}
	}
}


void CSyncCookieHelper::CheatIECoookie(HINTERNET hRequest) 
{
	CHAR szUrl[INTERNET_MAX_URL_LENGTH]={0};
	DWORD dwSize=sizeof(szUrl)-1;

	if( InternetQueryOptionA(hRequest, INTERNET_OPTION_URL, szUrl, &dwSize) && '\0'!=szUrl[0] ) {
		dwSize=0;
// 		GURL gurl(szUrl);
// 		_snprintf(szUrl, _ARRAYSIZE(szUrl)-1,"http://%s", gurl.host().c_str());
		if( RealInternetGetCookieExA )
		{
			RealInternetGetCookieExA(szUrl,NULL,NULL,&dwSize,0,NULL);
			if(0==dwSize) {
				CHAR szCookie[MAX_PATH]={0};
				_snprintf(szCookie,_ARRAYSIZE(szCookie)-1,"%s=%s; path=/","theworld_client_none","none");
				RealInternetSetCookieExA(szUrl,NULL,szCookie,0,0);
			}
		}
		else if( RealInternetGetCookieExW)
		{
			USES_CONVERSION;
			RealInternetGetCookieExW(A2W(szUrl),NULL,NULL,&dwSize,0,NULL);
			if(0==dwSize) {
				WCHAR szCookie[MAX_PATH]={0};
				_snwprintf(szCookie,_ARRAYSIZE(szCookie)-1,L"%s=%s; path=/",L"theworld_client_none",L"none");
				RealInternetSetCookieExW(A2W(szUrl),NULL,szCookie,0,0);
			}
		}
	}
}

void CSyncCookieHelper::CheatIECoookieForRedirect(HINTERNET hRequest) 
{
	CHAR szUrl[INTERNET_MAX_URL_LENGTH]={0};
	DWORD dwSize=sizeof(szUrl)-1;

	if( InternetQueryOptionA(hRequest, HTTP_QUERY_LOCATION, szUrl, &dwSize) && '\0'!=szUrl[0] ) {
		dwSize=0;
// 		GURL gurl(szUrl);
// 		_snprintf(szUrl, _ARRAYSIZE(szUrl)-1,"http://%s", gurl.host().c_str());
		if( RealInternetGetCookieExA )
		{
			RealInternetGetCookieExA(szUrl,NULL,NULL,&dwSize,0,NULL);
			if(0==dwSize) {
				CHAR szCookie[MAX_PATH]={0};
				_snprintf(szCookie,_ARRAYSIZE(szCookie)-1,"%s=%s; path=/","theworld_client_none","none");
				RealInternetSetCookieExA(szUrl,NULL,szCookie,0,0);
			}
		}
		else if( RealInternetGetCookieExW)
		{
			USES_CONVERSION;
			RealInternetGetCookieExW(A2W(szUrl),NULL,NULL,&dwSize,0,NULL);
			if(0==dwSize) {
				WCHAR szCookie[MAX_PATH]={0};
				_snwprintf(szCookie,_ARRAYSIZE(szCookie)-1,L"%s=%s; path=/",L"theworld_client_none",L"none");
				RealInternetSetCookieExW(A2W(szUrl),NULL,szCookie,0,0);
			}
		}
	}
}

void CSyncCookieHelper::SetChromeCookie(const char* pszUrl,const char* pszCookie,bool request_cookie)
{

}
bool CSyncCookieHelper::GetChromeCookie(const char* pszUrl,char* pszCookieData,unsigned long* pcchCookieData,bool request_cookie)
{
	return false;
}