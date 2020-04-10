#include "chrome/iecore/trident/stdafx.h"
#include "chrome/iecore/trident/preheader.h"
#include "chrome/iecore/third_party/detourshook.h"
#include "chrome/iecore/third_party/comhook.h"
#include "HtmlDocumentHookApi.h"


ComHookInfo _ComHookHTMLDocumentInfo2[] =
{
	{ 36, (DWORD)CHtmlDocumentHook::Hookedget_referrer,(DWORD*)&CHtmlDocumentHook::Realget_referrer },
};


Document_get_referrer CHtmlDocumentHook::Realget_referrer = NULL;
Window_get_external CHtmlDocumentHook::orig_window_get_external=NULL;
Window_get_navigator CHtmlDocumentHook::orig_window_get_navigator=NULL;
Navigator_get_userAgent CHtmlDocumentHook::orig_navigator_get_userAgent=NULL;


CHtmlDocumentHook::CHtmlDocumentHook(void)
{

}

CHtmlDocumentHook::~CHtmlDocumentHook(void)
{
}

HWND CHtmlDocumentHook::GetBrowserWnd(IHTMLDocument2* pThis)
{
	if( pThis==NULL)
		return NULL;
	CComQIPtr<IHTMLDocument2> pDoc = pThis;
	if(pDoc)
	{
		CComQIPtr<IServiceProvider>  pServiceProvider = pDoc; 
		CComQIPtr<IWebBrowser2> pWebBrowser2; 
		pServiceProvider-> QueryService(IID_IWebBrowserApp, IID_IWebBrowser2, (void**)&pWebBrowser2); 
		if(pWebBrowser2)
		{
			CComQIPtr <IOleWindow> pOleWin;
			pWebBrowser2->QueryInterface(IID_IOleWindow,(void **)&pOleWin);	
			if(pOleWin)
			{
				HWND hWnd = NULL;
				pOleWin->GetWindow(&hWnd);

				if(hWnd)
				{
					HWND hParent = GetParent(hWnd);
					while (hParent)
					{
						TCHAR szClassName[MAX_PATH + 1] = {0};
						::GetClassName(hParent, szClassName, MAX_PATH);
						if(wcscmp(szClassName, L"Container") == 0)
							return hParent;

						hParent = ::GetParent(hParent);
					}
				}
			}
		}
	}
	return NULL;
}

HRESULT WINAPI CHtmlDocumentHook::Hookedget_referrer(IHTMLDocument2* pThis, BSTR *p)
{
	HRESULT hr = S_FALSE;
	if( Realget_referrer)
	{
		hr  = Realget_referrer(pThis,p);
		if( SUCCEEDED(hr)&& p && SysStringLen(*p))
			return hr;
		else
		{
			HWND hWnd = GetBrowserWnd(pThis);
			if( hWnd)
			{
				DWORD dwResult=0;
				//SendMessageTimeout( hWnd, WM_BROWSER_GET_REFERRER, (WPARAM)p, 0, SMTO_ABORTIFHUNG, 6000, &dwResult );
				if(dwResult==1)
					return S_OK;
			}
		}
	}
	return hr;

}

LRESULT CHtmlDocumentHook::Hooked_window_get_external(IHTMLWindow2* pThis,IDispatch **p)
{
  HRESULT hr=S_OK;
  hr = orig_window_get_external(pThis,p);
  return hr;
}

HRESULT __stdcall CHtmlDocumentHook::Hooked_window_get_navigator( IHTMLWindow2* pThis,IOmNavigator **p)
{
  HRESULT ret = orig_window_get_navigator(pThis,p);
  static bool bHook=false;
  if(!bHook&&p)
  {
    IOmNavigator* pNavigator = *(p);
    LPDWORD pThunk = (PDWORD)( (DWORD)pNavigator);
    LPVOID *vfptr = *(LPVOID**) pThunk;
    orig_window_get_external = (Window_get_external) (vfptr[76]);
    //DetourAttach( &(PVOID&)orig_window_get_external, Hooked_window_get_external);
  }
  return ret;
}

BOOL AddMSIE(LPCTSTR lpszOriginalAgent, LPTSTR lpszBrowserAgent, DWORD dwSize)
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
        _tcsncpy(szBuffer, lpszOriginalAgent, lpEnd-lpszOriginalAgent);
        _sntprintf(lpszBrowserAgent, dwSize-1, _T("%s; %s%s"), szBuffer, L"MSIE 11.0",lpEnd);
        return TRUE;
      }
    }
  }
  return FALSE;
}

HRESULT __stdcall CHtmlDocumentHook::Hooked_navigator_get_userAgent( IOmNavigator* pThis,BSTR *p)
{
  CComBSTR ua;
  HRESULT ret = orig_navigator_get_userAgent(pThis,&ua);
  if(p)
  {
    TCHAR szUserAgent[MAX_URL_LEN]={0};
    wcscpy_s(szUserAgent,ua.m_str);
    TCHAR szBrowserAgent[MAX_URL_LEN]={0};
    if(AddMSIE(szUserAgent, szBrowserAgent, _ARRAYSIZE(szBrowserAgent)-1))
    {
      CComBSTR new_ua(szBrowserAgent);
      *p = new_ua;
    }
    else
      *p = ua;
  }
  return ret;
}

//Remarks:dz@2013/7/16 不区分IE版本，且必须detours hook,com hook无效
void CHtmlDocumentHook::StartAPIHook()
{
	IHTMLDocument2* pdoc2 = NULL;
	if (::CoCreateInstance( CLSID_HTMLDocument, NULL, CLSCTX_INPROC_SERVER, IID_IHTMLDocument2, (void**)&pdoc2 ) == S_OK&&pdoc2)
	{

		LPDWORD pThunk = (PDWORD)( (DWORD)pdoc2 + 0x10);
		LPVOID *vfptr = *(LPVOID**) pThunk;

		Realget_referrer = (Document_get_referrer) (vfptr[36]);
		//DetourAttach( &(PVOID&)Realget_referrer, Hookedget_referrer);
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
        orig_navigator_get_userAgent = (Navigator_get_userAgent) (vfptr[10]);
        //DetourAttach( &(PVOID&)orig_navigator_get_userAgent, Hooked_navigator_get_userAgent);
        pNavigator->Release();
      }
//       LPDWORD pThunk = (PDWORD)( (DWORD)pWin2);
//       LPVOID *vfptr = *(LPVOID**) pThunk;
//       orig_window_get_external = (Window_get_external) (vfptr[76]);
//       DetourAttach( &(PVOID&)orig_window_get_external, Hooked_window_get_external);
//       orig_window_get_navigator = (Window_get_navigator) (vfptr[25]);
//       DetourAttach( &(PVOID&)orig_window_get_navigator, Hooked_window_get_navigator);
      pWin2->Release();
    }
		pdoc2->Release();
	}
}

