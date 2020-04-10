#include "chrome/iecore/trident/stdafx.h"
#include "WebBrowser.h"
#include "chrome/iecore/trident/webbrowserhost/IEBrowserWnd.h"


STDMETHODIMP CWebBrowser::QueryInterface(REFIID riid, LPVOID FAR* ppvObj)
{
	*ppvObj = NULL;

	if( IsEqualIID(riid, IID_IUnknown) )
		*ppvObj = this;
	else if( IsEqualIID(riid, IID_IServiceProvider) )//OleClientSite
		*ppvObj = (IServiceProvider *)this;
	else if( IsEqualIID(riid, IID_IOleClientSite) )
		*ppvObj = (IOleClientSite *)this;
	else if( IsEqualIID(riid, IID_IOleInPlaceSite) )//OleCreate创建的时候，OleClientSite塞给了IE的CShellEmbedding
		*ppvObj = (IOleInPlaceSite *)this;
	else if( IsEqualIID(riid, IID_IOleInPlaceFrame) )
		*ppvObj = (IOleInPlaceFrame *)this;
	else if( IsEqualIID(riid, IID_IStorage) )
		*ppvObj = (IStorage *)this;
	else if( IsEqualIID(riid, IID_IDispatch) )//OleClientSite
		*ppvObj = (IDispatch *)this;
	else if( IsEqualIID(riid, IID_IOleCommandTarget) )//OleClientSite
		*ppvObj = (IOleCommandTarget *)&m_oDocHostUI;
	else if( IsEqualIID(riid, IID_IDocHostUIHandler) )//CDocObjectHost
		*ppvObj = (IDocHostUIHandler *)&m_oDocHostUI;
	else if( IsEqualIID(riid, IID_IDocHostUIHandler2) )
		*ppvObj = (IDocHostUIHandler2 *)&m_oDocHostUI;
	else if( IsEqualIID(riid, IID_IDocHostShowUI) )
		*ppvObj = (IDocHostShowUI *)&m_oDocHostUI;
	else
	{
		*ppvObj = 0;
		return E_NOINTERFACE;
	}

	return S_OK;
	
}

STDMETHODIMP_(ULONG) CWebBrowser::AddRef(THIS)
{
	return 1;
}

STDMETHODIMP_(ULONG) CWebBrowser::Release(THIS)
{
	return 1;
}

STDMETHODIMP CWebBrowser::QueryService( REFGUID guidService, REFIID riid, void __RPC_FAR *__RPC_FAR *ppvObj )
{
	HRESULT hr = E_NOINTERFACE;
	if( IsEqualIID(riid, IID_IInternetSecurityManager) )
	{
		*ppvObj = (IInternetSecurityManager *)&m_oSecurityManager;
		hr = S_OK;
	}
	else if( IsEqualIID(riid, IID_IHttpSecurity) )
	{
		*ppvObj = (IHttpSecurity *)&m_oSecurityManager;
		hr = S_OK;
	}
	else if( IsEqualIID(riid, IID_INewWindowManager) )
	{
		*ppvObj = (IHttpSecurity *)&m_oNewWindowManager;
		hr = S_OK;
	}	
	else if( IsEqualIID(riid, IID_IDownloadManager) )
	{
		*ppvObj = (IHttpSecurity *)&m_oDownloadManager;
		hr = S_OK;
	}	
	return hr;
}

STDMETHODIMP CWebBrowser::GetWindow( HWND *phwnd )
{
	*phwnd = m_pIEBrowserWnd->GetWindow();
	return S_OK;
}

STDMETHODIMP CWebBrowser::GetWindowContext( IOleInPlaceFrame **ppFrame,
	IOleInPlaceUIWindow **ppDoc, LPRECT lprcPosRect, 
	LPRECT lprcClipRect, LPOLEINPLACEFRAMEINFO lpFrameInfo )
{
	*ppFrame = this;
	*ppDoc = this;
	lpFrameInfo->fMDIApp = FALSE;
	lpFrameInfo->haccel = NULL;
	lpFrameInfo->cAccelEntries = 0;
	lpFrameInfo->hwndFrame = m_pIEBrowserWnd->GetWindow();
	return S_OK;
}


STDMETHODIMP CWebBrowser::GetContainer(IOleContainer **ppContainer )
{
	*ppContainer = 0;
	return E_NOTIMPL;
}

STDMETHODIMP CWebBrowser::Invoke( DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, 
	VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr )
{
	if (!pDispParams)
		return E_INVALIDARG;

// 	switch( dispIdMember )
// 	{
// // 	case DISPID_AMBIENT_DLCONTROL:
// // 		pVarResult->vt = VT_I4;
// // 		pVarResult->lVal = 0;
// // 
// // 		if ( !_settings.GetOptionValue(ID_NOIMAGE)  )
// // 			pVarResult->lVal |=  DLCTL_DLIMAGES;
// // 		if ( !_settings.GetOptionValue(ID_NOVIDEO) )
// // 			pVarResult->lVal |=  DLCTL_VIDEOS;
// // 		if ( _settings.GetOptionValue(ID_NOAXINST) )
// // 			pVarResult->lVal |=  DLCTL_NO_DLACTIVEXCTLS;
// // 		if ( _settings.GetOptionValue(ID_NOSCRIPT) && !m_pWebBrowser->m_pIEItem->IsForceEnableScript() )
// // 			pVarResult->lVal |=  DLCTL_NO_SCRIPTS;
// // 		//		if( _Settings.bPIS_BGSOUND )
// // 		pVarResult->lVal |=  DLCTL_BGSOUNDS;
// // 		if( _settings.GetOptionValue(ID_NOJAVA) )
// // 			pVarResult->lVal |=  DLCTL_NO_JAVA;
// // 		break;
// // 
// 	default:
// 		return DISP_E_MEMBERNOTFOUND;
// 	}

	return DISP_E_MEMBERNOTFOUND;  //返回值错误影响F12调试工具
}
HRESULT CWebBrowser::Connect()
{
	return m_oBrowserEvent.Connect();
}

HRESULT CWebBrowser::DisConnect()
{
	return m_oBrowserEvent.DisConnect();
}
