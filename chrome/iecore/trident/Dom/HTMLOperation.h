#pragma once
#include <atlbase.h>
#include <ObjBase.h>
#include <ExDisp.h>
#include <Windows.h>
#include <MsHTML.h>
#include <string>

namespace HtmlOperation
{

	HWND GetBrowserWnd(IHTMLWindow2* pThis);//ÈÝÆ÷HWND
	HWND GetBrowserWnd(IHTMLDocument2* pThis);
	BOOL GetIWebBrowser2Interface(HWND hIEServerWnd,IWebBrowser2** pWebBrowser2);
	BOOL GetIWebBrowser2Interface(IHTMLDocument2* pThis,IWebBrowser2** ppWebBrowser2);
	BOOL GetIHTMLDocument2Interface(HWND hIEServerWnd,IHTMLDocument2** ppHtmlDoc);
	BOOL GetIHTMLDocument2Interface(IWebBrowser2* pThis,IHTMLDocument2** ppHtmlDoc);
	BOOL GetDocumentCharset( IHTMLDocument2* pHtmlDoc, CComBSTR &bstrVal );
	HRESULT GetHtmlSourceText(IHTMLDocument2* pHtmlDoc, std::wstring& strText);
}