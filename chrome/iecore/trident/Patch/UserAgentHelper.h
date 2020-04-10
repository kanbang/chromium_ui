#pragma  once
#include "chrome/iecore/trident/preheader.h"
#include <WinInet.h>
#include <MsHTML.h>
/*
  1、自定义UA
  2、fixed IE11丢失MSIE 11.0;compatibale;
    一、服务端Header头中的UA
      部分网站在IE11下 提示不支持,可能是检测的Request Header头中的UA  http://211.158.18.211/seeyon/index.jsp 
      bugzilla在ie11下失效：https://bugzilla.mozilla.org/show_bug.cgi?id=902515
    二、js通过navigator.userAgent来获取客户端UA
      有的网站是利用js来检测支持浏览器的。
    bug:导致优酷无法检测安装了flash
    bug:http://note.youdao.com/ 点击登录没有弹出
*/

typedef HRESULT (WINAPI *pfnObtainUserAgentString)(DWORD dwOption, LPSTR pcszUAOut, DWORD *cbSize);
typedef BOOL (WINAPI *pfnHttpAddRequestHeadersA)(HINTERNET hConnect, LPCSTR lpszHeaders, DWORD dwHeadersLength, DWORD dwModifiers);
typedef HRESULT (WINAPI *pfnNavigator_get_userAgent)( IOmNavigator* pThis,BSTR *p);

class CUserAgentHelper
{
public:
	CUserAgentHelper();
	~CUserAgentHelper();


	static pfnObtainUserAgentString RealObtainUserAgentString;
	static pfnHttpAddRequestHeadersA RealHttpAddRequestHeadersA;
  static pfnNavigator_get_userAgent RealNavigator_get_userAgent;
	static HRESULT WINAPI HookedObtainUserAgentString(DWORD dwOption, LPSTR pcszUAOut, DWORD *cbSize);
	static BOOL WINAPI HookedHttpAddRequestHeadersA(HINTERNET hConnect, LPCSTR lpszHeaders, DWORD dwHeadersLength, DWORD dwModifiers);
  static HRESULT WINAPI HookedNavigator_get_userAgent( IOmNavigator* pThis,BSTR *p);

	static CUserAgentHelper& GetInstance()
	{
		static CUserAgentHelper instance;
		return instance;
	}
	static void StartAPIHook();
  static void StartNavigatorHook();
private:
	HRESULT ProxyObtainUserAgentString(DWORD dwOption, LPSTR pcszUAOut, DWORD *cbSize);
	BOOL  ProxyHttpAddRequestHeadersA(HINTERNET hConnect, LPCSTR lpszHeaders, DWORD dwHeadersLength, DWORD dwModifiers);
  HRESULT ProxyNavigator_get_userAgent( IOmNavigator* pThis,BSTR *p);
  //
  BOOL MakeBrowserAgent(LPCTSTR lpszOriginalAgent, LPTSTR lpszBrowserAgent, DWORD dwSize);
  BOOL AddMSIE11(LPCTSTR lpszOriginalAgent, LPTSTR lpszBrowserAgent, DWORD dwSize);
  BOOL GetBrowserAgentFromOriginal(LPCTSTR lpszOriginalAgent, LPTSTR lpszBrowserAgent, DWORD dwSize);
};