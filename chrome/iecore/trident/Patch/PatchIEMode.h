/*
来自于对IE的逆向,by dangzhuang@2013/11/16,please remember.
// 				iedvtool!CIEDevToolsMenu::SetHtmlDocumentMode(0x15f90,0)   //IE9 ok
// 				iedvtool!CIEDevToolsMenu::SetHtmlDocumentMode(0x13880,0)   //IE8 ok
// 				iedvtool!CIEDevToolsMenu::SetHtmlDocumentMode(0x11170,0)   //IE7  ok
// 				iedvtool!CIEDevToolsMenu::SetHtmlDocumentMode(0xC350,0)    //IE5 quriks ok
// 				iedvtool!CIEDevToolsMenu::SetHtmlDocumentMode(0x186a0,1)   //IE10 quriks not support
// 				iedvtool!CIEDevToolsMenu::SetHtmlDocumentMode(0x186a0,0)   //IE10 standards ok
// 				iedvtool!CIEDevToolsMenu::SetHtmlDocumentMode(0x1adb0,1)   //IE11 quris not support
// 				iedvtool!CIEDevToolsMenu::SetHtmlDocumentMode(0x1adb0,0)   //IE11 standards ok

由于SetHtmlDocumentMode修改文档模式，需要才重新刷新，因此就导致referrer丢失的：
1、RequestHeader中的referrer  
2、document.referrer

可以通过OnBeforeNavigate中取消本次请求，然后伪造referrer，构造成Navigate的方式，同时hook document.referrer也给填补上referrer
*/
#pragma  once
#include <windows.h>
#include <ObjBase.h>
#include <ExDisp.h>
namespace PatchIEMode
{
	enum _Document_Mode{
		Quriks_IE5 = 0xc350,
		Standard_IE7 = 0x11170,
		Standard_IE8 = 0x13880,
		Standard_IE9 = 0x15f90,
		Standard_IE10 = 0x186a0,
		Standard_IE11 = 0x1adb0
	};
	enum _UserAgent_Mode
	{
		UA_IE7=7,
		UA_IE8=8,
		UA_IE9=9,
		UA_IE10=10,
		UA_IE11=11,
	};
	void SetHtmlDocumentMode(IWebBrowser2* pWebBrowser,DWORD dwMode);
	void SetUserAgentMode(DWORD dwMode);
}