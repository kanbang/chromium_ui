#include "chrome/iecore/trident/stdafx.h"
#include "chrome/iecore/third_party/comhook.h"
#include "chrome/iecore/third_party/detours/src/detours.h"
#include "HtmlWindowHookApi.h"
#include "chrome/iecore/trident/webbrowserhost/IEBrowserWnd.h"
#include <Shlwapi.h>
//
Window_Open CHtmlWindowHook::orig_window_open = NULL;
Window_MoveTo CHtmlWindowHook::orig_window_moveTo = NULL;
Window_MoveBy CHtmlWindowHook::orig_window_moveBy = NULL;
Window_ResizeTo CHtmlWindowHook::orig_window_resizeTo = NULL;
Window_ResizeBy CHtmlWindowHook::orig_window_resizeBy = NULL;
// Window_get_external CHtmlWindowHook::orig_window_get_external=NULL;
// Window_get_navigator CHtmlWindowHook::orig_window_get_navigator=NULL;

ComHookInfo _ComHookHTMLWindow2Info[] =
{
  { 29, (DWORD)CHtmlWindowHook::Hooked_window_open,(DWORD*)&CHtmlWindowHook::orig_window_open },
  { 72, (DWORD)CHtmlWindowHook::Hooked_window_moveTo,(DWORD*)&CHtmlWindowHook::orig_window_moveTo },
  { 73, (DWORD)CHtmlWindowHook::Hooked_window_moveBy,(DWORD*)&CHtmlWindowHook::orig_window_moveBy },
  { 74, (DWORD)CHtmlWindowHook::Hooked_window_resizeTo,(DWORD*)&CHtmlWindowHook::orig_window_resizeTo },
  { 75, (DWORD)CHtmlWindowHook::Hooked_window_resizeBy,(DWORD*)&CHtmlWindowHook::orig_window_resizeBy },
//   { 76, (DWORD)CHtmlWindowHook::Hooked_window_get_external,(DWORD*)&CHtmlWindowHook::orig_window_get_external },
//   { 25, (DWORD)CHtmlWindowHook::Hooked_window_get_navigator,(DWORD*)&CHtmlWindowHook::orig_window_get_navigator },
};

//////////////////////////////////////////////////////////////////////////
//
CHtmlWindowHook::CHtmlWindowHook(void)
{
}

CHtmlWindowHook::~CHtmlWindowHook(void)
{
}

LRESULT CHtmlWindowHook::Hooked_window_open( IHTMLWindow2* pThis, BSTR url, BSTR name, BSTR features, VARIANT_BOOL replace, IHTMLWindow2 **pomWindowResult )
{
  HRESULT hr = S_OK;
  CIEBrowserWnd* pBrowser = CIEBrowserWnd::Current();
  if(pBrowser)
  {
    pBrowser->SetPopupWindow(true);
  }
  hr = orig_window_open(pThis, url, name, features, replace, pomWindowResult);
  return hr;
}

LRESULT CHtmlWindowHook::Hooked_window_moveBy( IHTMLWindow2* pThis, long x, long y )
{
  HRESULT hr = S_OK;
  hr = orig_window_moveBy(pThis, x, y);
  return hr;
}

LRESULT CHtmlWindowHook::Hooked_window_moveTo( IHTMLWindow2* pThis, long x, long y )
{
  HRESULT hr = S_OK;
  hr = orig_window_moveTo(pThis, x, y);
  return hr;
}

LRESULT CHtmlWindowHook::Hooked_window_resizeBy( IHTMLWindow2* pThis, long x, long y )
{
  HRESULT hr = S_OK;
	hr = orig_window_resizeBy(pThis, x, y);

  return hr;
}

LRESULT CHtmlWindowHook::Hooked_window_resizeTo( IHTMLWindow2* pThis, long x, long y )
{
  HRESULT hr = S_OK;
  hr = orig_window_resizeTo(pThis, x, y);
  return hr;
}

/*在IE11 IE10标准文档模式下，不再查询函数虚表，而改为直接调用
因此不再采用com hook，而使用detours hook
*/
void CHtmlWindowHook::StartAPIHook()
{
  IHTMLDocument2* pDoc2 = NULL;
  if (::CoCreateInstance( CLSID_HTMLDocument, NULL, CLSCTX_INPROC_SERVER, IID_IHTMLDocument2, (void**)&pDoc2 ) == S_OK)
  {
    if( pDoc2 )
    {
      IHTMLWindow2* pWin2 = NULL;
      pDoc2->get_parentWindow( &pWin2 );
      if(pWin2)
      {

        LPDWORD pThunk = (PDWORD)( (DWORD)pWin2);
        LPVOID *vfptr = *(LPVOID**) pThunk;
        for( int i = 0; i < sizeof( _ComHookHTMLWindow2Info)/sizeof(ComHookInfo); i ++ )
        {
          int offset = _ComHookHTMLWindow2Info[i].nOffset;
          *_ComHookHTMLWindow2Info[i].OldFunc = (DWORD)vfptr[offset];
          DetourAttach(&(PVOID&)(*_ComHookHTMLWindow2Info[i].OldFunc),(PVOID)_ComHookHTMLWindow2Info[i].NewFunc);
        }
      }
      pWin2->Release();
    }
    pDoc2->Release();
  }
}


CHtmlWindowHook::WINDOWOPEN_POLICY CHtmlWindowHook::GetWindowOpenPilicy()
{
  static WINDOWOPEN_POLICY policy = WINDOWOPEN_UNKNOWN;
  if(policy == WINDOWOPEN_UNKNOWN)
  {
    DWORD dwType = REG_DWORD;
    DWORD dwData = 0;
    DWORD cbData = sizeof(dwData);
    if(ERROR_SUCCESS == SHGetValue(HKEY_CURRENT_USER, L"Software\\Microsoft\\Internet Explorer\\TabbedBrowsing", L"PopupsUseNewWindow", &dwType, (void*)&dwData, &cbData))
    {
      /* 0.1.2分别对应ie选换卡中关于window.open的设置
      // 0:由 IE 决定如何打开弹出窗口(默认的)
      // 1:始终在新窗口中打开弹出窗口
      // 2:始终在新选项卡中打开弹出窗口
      */
      if(dwData == 1)
        policy = WINDOWOPEN_NEWWINDOW;
      else if(dwData == 2)
        policy = WINDOWOPEN_NEWTAB;
      else
        policy = WINDOWOPEN_SUGGEST;
    }
    else
      policy = WINDOWOPEN_SUGGEST;
  }
  
  return policy;
}