#include "chrome/iecore/trident/stdafx.h"
#include "HTMLOperation.h"
#include <ShlGuid.h>
/*
IHTMLDocument2接口，可以获取IHTMLWindow2,IDispatch(pDoc->get_get_Script)、IWebBrowser2(get_Document获取Document)
IHTMLDocument可以获取charset referrer navigator sourcecode  url cookie
*/
HWND HtmlOperation::GetBrowserWnd(IHTMLWindow2* pThis)
{
	CComQIPtr<IHTMLDocument2> pDoc;
	pThis->get_document(&pDoc);
	if(pDoc)
	{
		CComQIPtr<IServiceProvider>  pServiceProvider = pDoc; 
		CComQIPtr<IWebBrowser2> pWebBrowser2; 
		pServiceProvider->QueryService(IID_IWebBrowserApp, IID_IWebBrowser2, (void**)&pWebBrowser2); 
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

HWND HtmlOperation::GetBrowserWnd(IHTMLDocument2* pThis)
{
	if( pThis==NULL)
		return NULL;
	CComQIPtr<IHTMLDocument2> pDoc = pThis;
	if(pDoc)
	{
		CComQIPtr<IServiceProvider>  pServiceProvider = pDoc; 
		CComQIPtr<IWebBrowser2> pWebBrowser2; 
		pServiceProvider->QueryService(IID_IWebBrowserApp, IID_IWebBrowser2, (void**)&pWebBrowser2); 
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

//向浏览器窗口（类名"Internet Explorer_Server"）发送WM_HTML_GETOBJECT消息
BOOL HtmlOperation::GetIWebBrowser2Interface(HWND hIEServerWnd,IWebBrowser2** pWebBrowser2)  //外部请释放pWebBrowser2->Release()
{
	ATLASSERT(pWebBrowser2);
	*pWebBrowser2=NULL;
	BOOL bRet = FALSE;
	CoInitialize(NULL);
	HRESULT hr;
	HINSTANCE hInst = ::LoadLibrary( _T("OLEACC.DLL") );
	if ( hInst )
	{
		LRESULT lRes; 
		UINT nMsg = ::RegisterWindowMessage( _T("WM_HTML_GETOBJECT") );
		::SendMessageTimeout( hIEServerWnd, nMsg, 0L, 0L, SMTO_ABORTIFHUNG, 1000, (DWORD*)&lRes );
		//获取函数pfObjectFromLresult
		LPFNOBJECTFROMLRESULT pfObjectFromLresult = (LPFNOBJECTFROMLRESULT)::GetProcAddress( hInst, "ObjectFromLresult");
		if ( pfObjectFromLresult  )
		{
			CComPtr<IServiceProvider> spServiceProv;
			hr = (*pfObjectFromLresult)( lRes, IID_IServiceProvider, 0, (void**)&spServiceProv );
			if ( SUCCEEDED(hr) )
			{
				hr = spServiceProv->QueryService(SID_SWebBrowserApp,IID_IWebBrowser2,(void**)&pWebBrowser2);
				bRet = TRUE;
			}
		} 
		::FreeLibrary( hInst );
	} 
	CoUninitialize();
	return bRet;
}
BOOL HtmlOperation::GetIHTMLDocument2Interface(HWND hIEServerWnd,IHTMLDocument2** ppHtmlDoc)  //外部请释放ppHtmlDoc->Release()
{
	ATLASSERT(ppHtmlDoc);
	*ppHtmlDoc=NULL;
	BOOL bRet = FALSE;
	CoInitialize(NULL);
	HRESULT hr;
	HINSTANCE hInst = ::LoadLibrary( _T("OLEACC.DLL") );
	if ( hInst )
	{
		LRESULT lRes; 
		UINT nMsg = ::RegisterWindowMessage( _T("WM_HTML_GETOBJECT") );
		::SendMessageTimeout( hIEServerWnd, nMsg, 0L, 0L, SMTO_ABORTIFHUNG, 1000, (DWORD*)&lRes );
		//获取函数pfObjectFromLresult
		LPFNOBJECTFROMLRESULT pfObjectFromLresult = (LPFNOBJECTFROMLRESULT)::GetProcAddress( hInst, "ObjectFromLresult");
		if ( pfObjectFromLresult  )
		{
			hr = (*pfObjectFromLresult)( lRes, IID_IHTMLDocument, 0, (void**)&ppHtmlDoc );
			if ( SUCCEEDED(hr) )
			{
				bRet = TRUE;
			}
		} 
		::FreeLibrary( hInst );
	} 
	CoUninitialize();
	return bRet;
}
//获取编码
BOOL HtmlOperation::GetDocumentCharset( IHTMLDocument2* pHtmlDoc, CComBSTR &bstrVal )//使用智能指针，自动释放内存
{
	ATLASSERT(pHtmlDoc);
	HRESULT hr = pHtmlDoc->get_charset( &bstrVal );  //BSTR由调用者释放内存，SysFreeString(charset)
	if( SUCCEEDED(hr))
		return TRUE;
	return FALSE;
}
//获取网页的source code
HRESULT HtmlOperation::GetHtmlSourceText(IHTMLDocument2* pHtmlDoc, std::wstring& strText)
{ 
	ATLASSERT(pHtmlDoc);
	HRESULT hr = S_OK;
	HGLOBAL hMemory = GlobalAlloc(GMEM_FIXED, 0);//使用FIXED内存
	if (hMemory == NULL)
		return S_FALSE;
	CComPtr<IPersistStreamInit> spPersistStream;
	CComPtr<IStream> spStream ;
	hr = pHtmlDoc->QueryInterface(&spPersistStream);
	if (SUCCEEDED(hr))
	{
		hr = CreateStreamOnHGlobal(hMemory, TRUE, &spStream);
		if (SUCCEEDED(hr))
		{
			hr = spPersistStream->Save(spStream, FALSE);
			if (SUCCEEDED(hr))
			{
				LPVOID pMem = GlobalLock(hMemory);
				if (pMem != NULL)
				{
					CComBSTR strCharset ;
					GetDocumentCharset(pHtmlDoc,strCharset);
					if(wcscmp(strCharset,L"utf-8") == 0)
					{
						// utf-8 charset
						int len = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)pMem, -1, NULL, 0);
						WCHAR* wszUtf8 = new WCHAR[len + 1];
						memset(wszUtf8, 0, (len + 1) * sizeof(WCHAR));
						MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)pMem, -1, wszUtf8, len);
						strText = wszUtf8;
						delete[] wszUtf8;
					}
					else if (wcscmp(strCharset,L"utf-16") == 0)
					{
						// utf-16 charset
						strText = (LPCWSTR)pMem;
					}
					else
					{
						// other charset
					}
					GlobalUnlock(hMemory);

				}	
			}
		}
	}
	GlobalFree(hMemory);
	return hr;
}
BOOL HtmlOperation::GetIHTMLDocument2Interface(IWebBrowser2* pThis,IHTMLDocument2** ppHtmlDoc)
{
	CComPtr<IDispatch> pDp; 
	pThis->get_Document(&pDp);
	HRESULT hr = pDp->QueryInterface(IID_IHTMLDocument2,(void**)&ppHtmlDoc);
	if( SUCCEEDED(hr))
	{
		return TRUE;
	}
	return FALSE;
}
BOOL HtmlOperation::GetIWebBrowser2Interface(IHTMLDocument2* pThis,IWebBrowser2** ppWebBrowser2)
{
	CComQIPtr<IServiceProvider>  pServiceProvider = pThis; 
	CComQIPtr<IWebBrowser2> pWebBrowser2; 
	HRESULT hr = pServiceProvider->QueryService(IID_IWebBrowserApp, IID_IWebBrowser2, (void**)&pWebBrowser2); 
	if( SUCCEEDED(hr))
		return TRUE;
	return FALSE;
}