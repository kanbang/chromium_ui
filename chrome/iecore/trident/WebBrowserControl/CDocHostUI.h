// DocHostUIHandler.h: interface for the CWebBrowserEx class.

#pragma once

#include <mshtmhst.h>
class CIEBrowserWnd;
class CExternalObj;
class CDocHostUI :	 public IOleCommandTarget,
					 public IDocHostUIHandler2,
					 public IDocHostShowUI
{
public:
	CDocHostUI();
	virtual ~CDocHostUI();

	//	IUnKnown
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);
	//	IOleCommandTarget
	STDMETHOD(QueryStatus)( const GUID *pguidCmdGroup, ULONG cCmds, OLECMD prgCmds[], OLECMDTEXT *pCmdText){return OLECMDERR_E_NOTSUPPORTED;};
	STDMETHOD(Exec)( const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt, VARIANT *pvaIn, VARIANT *pvaOut);

	//	IDocHostUIHandler
	STDMETHOD(ShowContextMenu)( DWORD dwID, POINT *ppt, IUnknown *pcmdtReserved, IDispatch *pdispReserved );
	STDMETHOD(ShowUI)( DWORD dwID, IOleInPlaceActiveObject *pActiveObject, IOleCommandTarget *pCommandTarget, IOleInPlaceFrame *pFrame, IOleInPlaceUIWindow *pDoc ){return S_OK;};
	STDMETHOD(HideUI)( void ){return S_OK;};
	STDMETHOD(UpdateUI)( void ){return S_OK;};
	STDMETHOD(EnableModeless)( BOOL fEnable ){return S_OK;};
	STDMETHOD(OnDocWindowActivate)( BOOL fActivate ){return S_OK;};
	STDMETHOD(OnFrameWindowActivate)( BOOL fActivate ){return S_OK;};
	STDMETHOD(ResizeBorder)( LPCRECT prcBorder, IOleInPlaceUIWindow *pUIWindow, BOOL fRameWindow ){return S_OK;};
	STDMETHOD(TranslateAccelerator)( LPMSG lpMsg, const GUID *pguidCmdGroup, DWORD nCmdID ){return S_FALSE;};
	STDMETHOD(GetOptionKeyPath)( LPOLESTR *pchKey, DWORD dw ){return S_FALSE;};
	STDMETHOD(GetDropTarget)( IDropTarget *pDropTarget, IDropTarget **ppDropTarget );
	STDMETHOD(GetHostInfo)( DOCHOSTUIINFO *pInfo );
	STDMETHOD(GetExternal)( IDispatch **ppDispatch );
	STDMETHOD(TranslateUrl)( DWORD dwTranslate, OLECHAR *pchURLIn, OLECHAR **ppchURLOut);
	STDMETHOD(FilterDataObject)( IDataObject *pDO, IDataObject **ppDORet );
	//	IDocHostUIHandle2
	STDMETHOD(GetOverrideKeyPath)( LPOLESTR *pchKey,DWORD dw){ return E_NOTIMPL;};
	//	IDocHostShowUI
	STDMETHOD(ShowMessage)( HWND hwnd,LPOLESTR lpstrText,LPOLESTR lpstrCaption,DWORD dwType,LPOLESTR lpstrHelpFile,DWORD dwHelpContext,LRESULT *plResult);
	STDMETHOD(ShowHelp)( HWND hwnd,LPOLESTR pszHelpFile,UINT uCommand,DWORD dwData,POINT ptMouse,IDispatch *pDispatchObjectHit){return E_NOTIMPL;};
public:
	void SetIEBrowserWnd( CIEBrowserWnd* pIEBrowserWnd ){ m_pIEBrowserWnd = pIEBrowserWnd;};

private:
	CIEBrowserWnd*	m_pIEBrowserWnd;
	CExternalObj m_oExternalObj;
};
