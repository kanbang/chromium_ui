// DocHostUIHandler.cpp: implementation of the CWebBrowserEx class.
//
//////////////////////////////////////////////////////////////////////

#include "chrome/iecore/trident/stdafx.h"
#include "ExternalObj.h"
#include "CDocHostUI.h"

CDocHostUI::CDocHostUI()
{
	m_pIEBrowserWnd=NULL;
}

CDocHostUI::~CDocHostUI()
{
	m_pIEBrowserWnd=NULL;
}

STDMETHODIMP_(ULONG)CDocHostUI::AddRef(THIS)
{
	return 1;
}

STDMETHODIMP_(ULONG)CDocHostUI::Release(THIS)
{
	return 1;
}

STDMETHODIMP CDocHostUI::QueryInterface(REFIID riid, LPVOID FAR* ppvObj)
{
	*ppvObj = NULL;

	if( IsEqualIID(riid, IID_IUnknown) )
		*ppvObj = this;
	else if( IsEqualIID(riid, IID_IDocHostUIHandler) )
		*ppvObj = (IDocHostUIHandler *)this;
	else if( IsEqualIID(riid, IID_IDocHostUIHandler2) )
		*ppvObj = (IDocHostUIHandler2 *)this;
	else if( IsEqualIID(riid, IID_IOleCommandTarget) )
		*ppvObj = (IOleCommandTarget *)this;
	else if( IsEqualIID(riid, IID_IDocHostShowUI) )
		*ppvObj = (IDocHostShowUI *)this;
	else
	{
		*ppvObj = 0;
		return E_NOINTERFACE;
	}
	return S_OK;	
}

STDMETHODIMP CDocHostUI::GetExternal( IDispatch **ppDispatch )
{
	*ppDispatch = &m_oExternalObj;
	return(S_OK);
}

STDMETHODIMP CDocHostUI::TranslateUrl( DWORD dwTranslate, OLECHAR *pchURLIn, OLECHAR **ppchURLOut)
{
	*ppchURLOut = 0;
	return(S_FALSE);
}

STDMETHODIMP CDocHostUI::FilterDataObject( IDataObject *pDO, IDataObject **ppDORet )
{
	*ppDORet = 0;
	return(S_FALSE);
}


STDMETHODIMP CDocHostUI::ShowContextMenu( DWORD dwID, POINT *ppt, IUnknown *pcmdTarget, IDispatch *pdispReserved )
{
	return E_NOTIMPL;
}


STDMETHODIMP CDocHostUI::GetDropTarget( IDropTarget *pDropTarget, IDropTarget **ppDropTarget )
{
	return E_NOTIMPL;
}

STDMETHODIMP CDocHostUI::Exec( const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt, VARIANT *pvaIn, VARIANT *pvaOut)
{
	//这里的IOleCommandTarget代表的是shell外壳的。
	if(pguidCmdGroup && IsEqualGUID(*pguidCmdGroup, CGID_DocHostCommandHandler))
	{
		if(nCmdID == OLECMDID_SHOWSCRIPTERROR)
		{
			(*pvaOut).vt = VT_BOOL; 				
			(*pvaOut).boolVal = VARIANT_TRUE;
			return S_OK;
		}
		else if( nCmdID == OLECMDID_SHOWMESSAGE)
		{
			int k=0;
		}
	}

	return OLECMDERR_E_NOTSUPPORTED;
}

STDMETHODIMP CDocHostUI::GetHostInfo( DOCHOSTUIINFO *pInfo )
{
	pInfo->cbSize = sizeof(DOCHOSTUIINFO);
	pInfo->dwFlags |= DOCHOSTUIFLAG_NO3DBORDER;
	pInfo->dwDoubleClick = DOCHOSTUIDBLCLK_DEFAULT;

	return S_OK;
}
//show message
STDMETHODIMP CDocHostUI::ShowMessage( HWND hwnd,LPOLESTR lpstrText,LPOLESTR lpstrCaption,DWORD dwType,LPOLESTR lpstrHelpFile,DWORD dwHelpContext,LRESULT *plResult)
{
	return E_NOTIMPL;
}