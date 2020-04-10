#pragma once
#include <MsHTML.h>
/*
document.referrer
window.external
window.navigator
*/
struct HtmlComHookInfo
{
	int         nOffset;
	DWORD       NewFunc;
	DWORD*      OldFunc;
};

typedef HRESULT (__stdcall *Document_get_referrer)(IHTMLDocument2* pThis, BSTR *p);
typedef HRESULT (__stdcall *Window_get_external)( IHTMLWindow2* pThis,IDispatch **p);
typedef HRESULT (__stdcall *Window_get_navigator)( IHTMLWindow2* pThis,IOmNavigator **p);
typedef HRESULT (__stdcall *Navigator_get_userAgent)( IOmNavigator* pThis,BSTR *p);

class CHtmlDocumentHook
{
public:
  CHtmlDocumentHook(void);
  virtual ~CHtmlDocumentHook(void);

  static Document_get_referrer Realget_referrer;
  static Window_get_external   orig_window_get_external;
  static Window_get_navigator  orig_window_get_navigator;
  static Navigator_get_userAgent orig_navigator_get_userAgent;

  static HRESULT WINAPI Hookedget_referrer(IHTMLDocument2* pThis, BSTR *p);
  static HRESULT __stdcall Hooked_window_get_external( IHTMLWindow2* pThis,IDispatch **p);
  static HRESULT __stdcall Hooked_window_get_navigator( IHTMLWindow2* pThis,IOmNavigator **p);
  static HRESULT __stdcall Hooked_navigator_get_userAgent( IOmNavigator* pThis,BSTR *p);

  static HWND GetBrowserWnd(IHTMLDocument2* pThis);

  static void StartAPIHook();
};
