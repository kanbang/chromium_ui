// WebBrowser.h: interface for the CWebBrowser class.

#pragma once

#include "ExternalObj.h"
#include "BrowserEvent.h"
#include "SecurityManager.h"
#include "CDocHostUI.h"
#include "NewWindowManager.h"
#include "DownloadManager.h"
#include <string>
#include <MsHtmHst.h>

class CIEBrowserWnd;
/*
WebBrowser相关资料
http://msdn.microsoft.com/en-us/library/aa752041(v=vs.85).aspx  WebBrowser Control
http://msdn.microsoft.com/en-us/library/aa753618(v=vs.85).aspx  Implementing a Custom Download Manager
http://msdn.microsoft.com/en-us/library/dd565719(v=vs.85).aspx  mshtml interface
http://msdn.microsoft.com/en-us/library/aa768284(v=vs.85).aspx  DWebBrowserEvents2
WININET资料
http://msdn.microsoft.com/zh-cn/subscriptions/downloads/aa383630.aspx
iefans会有一些IE的资料
http://www.iefans.net/
ie9新版防假死资料
http://blogs.msdn.com/b/ie/archive/2011/04/19/hang-resistance-in-ie9.aspx
通过QueryInterface获取接口(CWebBrowser)
	IOleCommandTarget
	IDocHostUIHandler
	IDocHostUIHandler2
	IDocHostShowUI
通过QueryService获取接口(CWebBrowserEx)
	IHttpSecurity
	IInternetSecurityManager
	INewWindowManager
	IDownloadManager
	IAuthenticate
*/

class CWebBrowser:  public IOleClientSite,
					public IServiceProvider,
					public IOleInPlaceSite,
					public IOleInPlaceFrame,
	    		    public IStorage,
					public IDispatch
 {

public:
	CWebBrowser()
	{
		m_pIEBrowserWnd=NULL;
	}
	virtual ~CWebBrowser()
	{
	}

	//	IUnKnown
	STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	//	IOleClientSite
	STDMETHOD(SaveObject)(THIS){return E_NOTIMPL;};
	STDMETHOD(GetMoniker)(DWORD dwAssign, DWORD dwWhichMoniker, IMoniker **ppmk ){return E_NOTIMPL;};
	STDMETHOD(GetContainer)(IOleContainer **ppContainer );
	STDMETHOD(ShowObject)(THIS){return S_OK;};
	STDMETHOD(OnShowWindow)(BOOL fShow){return E_NOTIMPL;};
	STDMETHOD(RequestNewObjectLayout)(THIS){return E_NOTIMPL;};

	//	IOleInPlaceSite
	STDMETHOD(CanInPlaceActivate)( void){return S_OK;};
	STDMETHOD(OnInPlaceActivate)( void){return S_OK;};
	STDMETHOD(OnUIActivate)( void){return S_OK;};
	STDMETHOD(GetWindowContext)( IOleInPlaceFrame **ppFrame, IOleInPlaceUIWindow **ppDoc, LPRECT lprcPosRect, LPRECT lprcClipRect, LPOLEINPLACEFRAMEINFO lpFrameInfo );
	STDMETHOD(Scroll)( SIZE scrollExtant ){return E_NOTIMPL;};
	STDMETHOD(OnUIDeactivate)( BOOL fUndoable ){return S_OK;};
	STDMETHOD(OnInPlaceDeactivate)( void){return S_OK;};
	STDMETHOD(DiscardUndoState)( void){return E_NOTIMPL;};
	STDMETHOD(DeactivateAndUndo)( void){return E_NOTIMPL;};
	STDMETHOD(OnPosRectChange)( LPCRECT lprcPosRect){return E_NOTIMPL;};;

	//	IDisptach
	STDMETHOD(GetTypeInfoCount)( UINT *pctinfo ){return E_NOINTERFACE;};
	STDMETHOD(GetTypeInfo)( UINT iTInfo, LCID lcid, ITypeInfo **ppTInfo ){return E_NOINTERFACE;};
	STDMETHOD(GetIDsOfNames)( REFIID riid, LPOLESTR *rgszNames, UINT cNames, LCID lcid, DISPID *rgDispId ){return E_NOINTERFACE;};
	STDMETHOD(Invoke)( DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr );
	//	IServiceProvider
	STDMETHOD(QueryService)( REFGUID guidService, REFIID riid, void __RPC_FAR *__RPC_FAR *ppvObject );

	//	IOleWindow
    STDMETHOD(GetWindow)( HWND *phwnd );
    STDMETHOD(ContextSensitiveHelp)( BOOL fEnterMode ){return E_NOTIMPL;};

	//	IOleInPlaceFrame
    STDMETHOD(InsertMenus)( HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths ){return E_NOTIMPL;};
    STDMETHOD(SetMenu)( HMENU hmenuShared, HOLEMENU holemenu, HWND hwndActiveObject ){return S_OK;};
    STDMETHOD(RemoveMenus)( HMENU hmenuShared ){return E_NOTIMPL;};
    STDMETHOD(SetStatusText)( LPCOLESTR pszStatusText ){return S_OK;};
    STDMETHOD(EnableModeless)( BOOL fEnable ){return S_OK;};
    STDMETHOD(TranslateAccelerator)( LPMSG lpmsg, WORD wID ){return E_NOTIMPL;};

	//	IOleInPlaceUIWindow
	STDMETHOD(GetBorder)( LPRECT lprectBorder ){return E_NOTIMPL;};
    STDMETHOD(RequestBorderSpace)( LPCBORDERWIDTHS pborderwidths ){return E_NOTIMPL;};
    STDMETHOD(SetBorderSpace)( LPCBORDERWIDTHS pborderwidths ){return E_NOTIMPL;};
    STDMETHOD(SetActiveObject)( IOleInPlaceActiveObject *pActiveObject, LPCOLESTR pszObjName ){return E_NOTIMPL;};;

	//	IStorage
	STDMETHOD(CreateStream)( const OLECHAR *pwcsName, DWORD grfMode, DWORD reserved1, DWORD reserved2, IStream **ppstm ){return E_NOTIMPL;};
    STDMETHOD(OpenStream)( const OLECHAR *pwcsName, void *reserved1, DWORD grfMode, DWORD reserved2, IStream **ppstm ){return E_NOTIMPL;};
    STDMETHOD(CreateStorage)( const OLECHAR *pwcsName, DWORD grfMode, DWORD reserved1, DWORD reserved2, IStorage **ppstg ){return E_NOTIMPL;};
    STDMETHOD(OpenStorage)( const OLECHAR *pwcsName, IStorage *pstgPriority, DWORD grfMode, SNB snbExclude, DWORD reserved, IStorage **ppstg ){return E_NOTIMPL;};
    STDMETHOD(CopyTo)( DWORD ciidExclude, const IID *rgiidExclude, SNB snbExclude, IStorage *pstgDest ){return E_NOTIMPL;};
    STDMETHOD(MoveElementTo)( const OLECHAR *pwcsName, IStorage *pstgDest, const OLECHAR *pwcsNewName, DWORD grfFlags ){return E_NOTIMPL;};
    STDMETHOD(Commit)( DWORD grfCommitFlags ){return E_NOTIMPL;};
    STDMETHOD(Revert)( void ){return E_NOTIMPL;};
    STDMETHOD(EnumElements)( DWORD reserved1, void *reserved2, DWORD reserved3, IEnumSTATSTG **ppenum ){return E_NOTIMPL;};
    STDMETHOD(DestroyElement)( const OLECHAR *pwcsName ){return E_NOTIMPL;};
    STDMETHOD(RenameElement)( const OLECHAR *pwcsOldName, const OLECHAR *pwcsNewName ){return E_NOTIMPL;};
    STDMETHOD(SetElementTimes)( const OLECHAR *pwcsName, const FILETIME *pctime, const FILETIME *patime, const FILETIME *pmtime){return E_NOTIMPL;};
    STDMETHOD(SetClass)( REFCLSID clsid ){return S_OK;};
    STDMETHOD(SetStateBits)( DWORD grfStateBits, DWORD grfMask ){return E_NOTIMPL;};
    STDMETHOD(Stat)( STATSTG *pstatstg, DWORD grfStatFlag ){return E_NOTIMPL;};

public:
	HRESULT Connect();
	HRESULT DisConnect();
	void SetIEBrowserWnd( CIEBrowserWnd* pIEBrowserWnd )
	{ 
		m_pIEBrowserWnd = pIEBrowserWnd;
		m_oDocHostUI.SetIEBrowserWnd(m_pIEBrowserWnd);
		m_oBrowserEvent.SetIEBrowserWnd(m_pIEBrowserWnd);
		m_oSecurityManager.SetIEBrowserWnd(m_pIEBrowserWnd);
		m_oDownloadManager.SetIEBrowserWnd(m_pIEBrowserWnd);
		m_oNewWindowManager.SetIEBrowserWnd(m_pIEBrowserWnd);
	};

protected:
	DWORD m_dwTlsIndex;
	CDocHostUI      m_oDocHostUI;					//UI相关：右键菜单，及一些菜单功能
	CBrowserEvent   m_oBrowserEvent;				//BrowserEvent接口
	CSecurityManager m_oSecurityManager;			//安全：HTTPS证书、及IE的安全选项
	CDownloadManager  m_oDownloadManager;           //下载
	CNewWindowManager  m_oNewWindowManager;		    //弹出窗口管理

	CIEBrowserWnd* m_pIEBrowserWnd;
};