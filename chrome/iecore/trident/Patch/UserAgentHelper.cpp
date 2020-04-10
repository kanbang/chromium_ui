#include "chrome/iecore/trident/stdafx.h"
#include "UserAgentHelper.h"
#include "chrome/iecore/third_party/detours/src/detours.h"

#pragma comment(lib,"urlmon.lib")

pfnObtainUserAgentString CUserAgentHelper::RealObtainUserAgentString=NULL;
pfnHttpAddRequestHeadersA CUserAgentHelper::RealHttpAddRequestHeadersA=NULL;
pfnNavigator_get_userAgent CUserAgentHelper::RealNavigator_get_userAgent=NULL;

#define  BROWSER_AGENT_STRING_A  "798youxi;MSIE 11.0;compatibale"
#define  BROWSER_AGENT_STRING_W  L"798youxi;MSIE 11.0;compatibale"

namespace {
  int min(int a, int b) {
    return a < b ? a : b;
  }
}
CUserAgentHelper::CUserAgentHelper()
{

}
CUserAgentHelper::~CUserAgentHelper()
{

}

void CUserAgentHelper::StartAPIHook()
{
  HMODULE urlmon = GetModuleHandle(_T("urlmon.dll"));
  if (!urlmon)
    urlmon = LoadLibrary(_T("urlmon.dll"));
  if( urlmon ) {
    RealObtainUserAgentString = (pfnObtainUserAgentString)GetProcAddress(urlmon, "ObtainUserAgentString");
    if( RealObtainUserAgentString )
      DetourAttach(&(PVOID&)RealObtainUserAgentString,HookedObtainUserAgentString);
  }
  HMODULE wininet = GetModuleHandle(_T("wininet.dll"));
  if (!wininet)
    wininet = LoadLibrary(_T("wininet.dll"));
  if( wininet ) {
    RealHttpAddRequestHeadersA = (pfnHttpAddRequestHeadersA)GetProcAddress(wininet, "HttpAddRequestHeadersA");
    if( RealHttpAddRequestHeadersA )
      DetourAttach(&(PVOID&)RealHttpAddRequestHeadersA,HookedHttpAddRequestHeadersA);
  }
}

HRESULT WINAPI CUserAgentHelper::HookedObtainUserAgentString(DWORD dwOption, LPSTR pcszUAOut, DWORD *cbSize)
{
  return CUserAgentHelper::GetInstance().ProxyObtainUserAgentString(dwOption,pcszUAOut,cbSize);
}
BOOL WINAPI CUserAgentHelper::HookedHttpAddRequestHeadersA(HINTERNET hConnect, LPCSTR lpszHeaders, DWORD dwHeadersLength, DWORD dwModifiers)
{
  return CUserAgentHelper::GetInstance().ProxyHttpAddRequestHeadersA(hConnect, lpszHeaders, dwHeadersLength, dwModifiers);

}
HRESULT CUserAgentHelper::ProxyObtainUserAgentString(DWORD dwOption, LPSTR pcszUAOut, DWORD *cbSize)
{
  if(pcszUAOut && cbSize )
  {
    CHAR szAgent[MAX_URL_LEN]={0};
    DWORD dwSize=sizeof(szAgent)-1;
    RealObtainUserAgentString(dwOption, szAgent, &dwSize);
    if('\0'!=szAgent[0] && NULL == strstr(szAgent,BROWSER_AGENT_STRING_A))
    {
      USES_CONVERSION;
      TCHAR szBrowserAgent[MAX_URL_LEN]={0};
      if(MakeBrowserAgent(A2T(szAgent), szBrowserAgent, _ARRAYSIZE(szBrowserAgent)-1))
      {
        _snprintf_s(pcszUAOut, *cbSize-1,*cbSize-1, "%s", T2A(szBrowserAgent));
        *cbSize=strlen(pcszUAOut)+1;
        return NOERROR;
      }
    }
  }
  return RealObtainUserAgentString(dwOption,pcszUAOut,cbSize);
}
BOOL CUserAgentHelper::ProxyHttpAddRequestHeadersA(HINTERNET hConnect, LPCSTR lpszHeaders, DWORD dwHeadersLength, DWORD dwModifiers)
{
  LPCSTR lpszAgent = "User-Agent:";
  LPCSTR lpszOldAgent = NULL;

  if((NULL!=(lpszOldAgent=strstr(lpszHeaders, lpszAgent))))
  {
    LPCSTR lpszAgentHeader = "Mozilla";
    lpszOldAgent += strlen(lpszAgent);
    if(' '==lpszOldAgent[0])
      lpszOldAgent ++;
    if( 0==strncmp(lpszOldAgent, lpszAgentHeader, min(strlen(lpszOldAgent), strlen(lpszAgentHeader)))
      && NULL==strstr(lpszOldAgent,BROWSER_AGENT_STRING_A) )
    {
      LPCSTR lpszAgentEnd = NULL;
      LPCSTR lpszAgentStart = strstr(lpszHeaders, lpszAgent);
      if(lpszAgentStart)
        lpszAgentEnd = strstr(lpszAgentStart, ")");

      if(lpszAgentStart && lpszAgentEnd)
      {
        DWORD dwLen = ((-1==dwHeadersLength)?strlen(lpszHeaders):dwHeadersLength)+MAX_PATH;
        LPSTR lpszNewHeaders = new CHAR[dwLen];

        if(lpszNewHeaders)
        {
          memset(lpszNewHeaders, 0, dwLen);

          USES_CONVERSION;
          strncpy_s(lpszNewHeaders, dwLen, lpszHeaders, lpszAgentEnd-lpszHeaders);

          int nNewLen = strlen(lpszNewHeaders);
          _snprintf_s(lpszNewHeaders+nNewLen, dwLen-nNewLen-1, _TRUNCATE, "; %s%s", BROWSER_AGENT_STRING_A, lpszAgentEnd);

          BOOL bRet = FALSE;
          bRet = RealHttpAddRequestHeadersA(hConnect, lpszNewHeaders, strlen(lpszNewHeaders), dwModifiers);
          delete[] lpszNewHeaders;			
          return bRet;
        }
      }
    }
  }
  else
  {
    BOOL ret = RealHttpAddRequestHeadersA(hConnect, lpszHeaders, dwHeadersLength, dwModifiers);

    CHAR szOriginalAgent[ INTERNET_MAX_URL_LENGTH ] = "";
    DWORD dwAgentSize = sizeof( szOriginalAgent ) - 1;
    if(NOERROR==ObtainUserAgentString(0, szOriginalAgent, &dwAgentSize))
    {
      USES_CONVERSION;
      TCHAR	szDefaultAgent[INTERNET_MAX_URL_LENGTH]={0};
      TCHAR	szFullAgent[INTERNET_MAX_URL_LENGTH]={0};
      BOOL bRet = GetBrowserAgentFromOriginal(A2T(szOriginalAgent), szDefaultAgent, _countof(szDefaultAgent));
      if( bRet )
      {
        _sntprintf_s(szFullAgent, _TRUNCATE, _T("User-Agent: %s\r\n"), szDefaultAgent);
        RealHttpAddRequestHeadersA(hConnect, T2A(szFullAgent), -1, HTTP_ADDREQ_FLAG_ADD|HTTP_ADDREQ_FLAG_REPLACE);
      }
    }
    return ret;
  }

  return RealHttpAddRequestHeadersA(hConnect, lpszHeaders, dwHeadersLength, dwModifiers);
}

BOOL CUserAgentHelper::MakeBrowserAgent(LPCTSTR lpszOriginalAgent, LPTSTR lpszBrowserAgent, DWORD dwSize)
{
  if(lpszOriginalAgent)
  {
    if(NULL!=StrStrI(lpszOriginalAgent, BROWSER_AGENT_STRING_W))
    {
      _sntprintf_s(lpszBrowserAgent, dwSize-1,dwSize-1,_T("%s"), lpszOriginalAgent);
      return TRUE;
    }
    else
    {
      USES_CONVERSION;
      LPCTSTR lpEnd = _tcsrchr(lpszOriginalAgent, _T(')') );
      if(lpEnd && 1==_tcslen(lpEnd))
      {
        TCHAR szBuffer[INTERNET_MAX_URL_LENGTH]=_T("");
        _tcsncpy_s(szBuffer, lpszOriginalAgent, min(_ARRAYSIZE(szBuffer)-1, lpEnd-lpszOriginalAgent));
        _sntprintf_s(lpszBrowserAgent, dwSize-1,_TRUNCATE, _T("%s; %s)"), szBuffer, BROWSER_AGENT_STRING_W);
        return TRUE;
      }
    }
  }
  return FALSE;
}

//Remarks:dz@2013/7/16 不区分IE版本，且必须detours hook,com hook无效  副作用比较大
void CUserAgentHelper::StartNavigatorHook()
{
  IHTMLDocument2* pdoc2 = NULL;
  if (::CoCreateInstance( CLSID_HTMLDocument, NULL, CLSCTX_INPROC_SERVER, IID_IHTMLDocument2, (void**)&pdoc2 ) == S_OK&&pdoc2)
  {
    IHTMLWindow2* pWin2 = NULL;
    pdoc2->get_parentWindow( &pWin2 );
    if(pWin2)
    {
      IOmNavigator* pNavigator=NULL;
      pWin2->get_navigator( &pNavigator );
      if( pNavigator )
      {
        LPDWORD pThunk = (PDWORD)( (DWORD)pNavigator+0x10);
        LPVOID *vfptr = *(LPVOID**) pThunk;
        RealNavigator_get_userAgent = (pfnNavigator_get_userAgent) (vfptr[10]);
        DetourAttach( &(PVOID&)RealNavigator_get_userAgent, HookedNavigator_get_userAgent);
        pNavigator->Release();
      }
      pWin2->Release();
    }
    pdoc2->Release();
  }
}
HRESULT WINAPI CUserAgentHelper::HookedNavigator_get_userAgent( IOmNavigator* pThis,BSTR *p)
{
  return CUserAgentHelper::GetInstance().ProxyNavigator_get_userAgent(pThis,p);
}


HRESULT  CUserAgentHelper::ProxyNavigator_get_userAgent( IOmNavigator* pThis,BSTR *p)
{
  CComBSTR ua;
  HRESULT ret = RealNavigator_get_userAgent(pThis,&ua);
  if(p)
  {
    TCHAR szUserAgent[MAX_URL_LEN]={0};
    wcscpy_s(szUserAgent,ua.m_str);
    TCHAR szBrowserAgent[MAX_URL_LEN]={0};
    if(AddMSIE11(szUserAgent, szBrowserAgent, _ARRAYSIZE(szBrowserAgent)-1))
    {
      CComBSTR new_ua(szBrowserAgent);
      *p = new_ua;
    }
    else
      *p = ua;
  }
  return ret;
}

BOOL CUserAgentHelper::AddMSIE11(LPCTSTR lpszOriginalAgent, LPTSTR lpszBrowserAgent, DWORD dwSize)
{
  if(lpszOriginalAgent)
  {
    if(NULL!=StrStrI(lpszOriginalAgent, L"MSIE"))
    {
      _sntprintf_s(lpszBrowserAgent, dwSize-1, _TRUNCATE,_T("%s"), lpszOriginalAgent);
      return FALSE;
    }
    else
    {
      LPCTSTR lpEnd = _tcsrchr(lpszOriginalAgent, _T(')') );
      if(lpEnd )//&& 1==_tcslen(lpEnd))
      {
        TCHAR szBuffer[MAX_URL_LEN]=_T("");
        _tcsncpy(szBuffer, lpszOriginalAgent, min(_ARRAYSIZE(szBuffer)-1, lpEnd-lpszOriginalAgent));
        _sntprintf(lpszBrowserAgent, dwSize-1, _T("%s; %s%s"), szBuffer, L"MSIE 11.0;compatible",lpEnd);
        return TRUE;
      }
    }
  }
  return FALSE;
}

BOOL CUserAgentHelper::GetBrowserAgentFromOriginal(LPCTSTR lpszOriginalAgent, LPTSTR lpszBrowserAgent, DWORD dwSize)
{
  if(lpszOriginalAgent)
  {
    if(NULL!=StrStrI(lpszOriginalAgent, BROWSER_AGENT_STRING_W))
    {
      _tcscpy_s(lpszBrowserAgent, dwSize, lpszOriginalAgent);
      return TRUE;
    }
    else
    {
      USES_CONVERSION;
      LPCTSTR lpEnd = _tcsrchr(lpszOriginalAgent, _T(')') );
      if(lpEnd && 1==_tcslen(lpEnd))
      {
        TCHAR szBuffer[INTERNET_MAX_URL_LENGTH]=_T("");
        _tcsncpy_s(szBuffer, lpszOriginalAgent, min(_countof(szBuffer)-1, lpEnd-lpszOriginalAgent));
        _sntprintf_s(lpszBrowserAgent, dwSize, _TRUNCATE, _T("%s; %s)"), szBuffer, BROWSER_AGENT_STRING_W);
        return TRUE;
      }
    }
  }
  return FALSE;
}