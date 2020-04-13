
#include "chrome/iecore/trident/preheader.h"
#include <mshtml.h>

inline HWND GetWindow( IWebBrowser2* spBrowser )
{
	HWND hwndWB = NULL;
	spBrowser->get_HWND( (SHANDLE_PTR*)&hwndWB );

	if( NULL==hwndWB )
	{
		IOleWindowPtr pOleWnd = spBrowser;
		if( pOleWnd )
			pOleWnd->GetWindow( &hwndWB );
	}

	return hwndWB;
}

class CWebBrowserAuto : public IWebBrowser2
{
public:
    // IUnknown (we multiply inherit from IUnknown, disambiguate here)
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID * ppvObj)
	{
		if( IID_IUnknown==riid || IID_IDispatch==riid || IID_IWebBrowser2==riid || IID_IWebBrowser==riid || IID_IWebBrowserApp==riid )
		{
			*ppvObj = this;
			 (*m_pBrowser2).AddRef();
			 return S_OK;
		}
		else
		{
			return m_pBrowser2->QueryInterface(riid, ppvObj);
		}
	}
    STDMETHOD_(ULONG, AddRef)() { return (*m_pBrowser2).AddRef(); }
    STDMETHOD_(ULONG, Release)() { return (*m_pBrowser2).Release(); }

    // IDispatch (we multiply inherit from IDispatch, disambiguate here)
    STDMETHOD(GetTypeInfoCount)(UINT *pctinfo) { return m_pBrowser2->GetTypeInfoCount(pctinfo); }
    STDMETHOD(GetTypeInfo)(UINT itinfo, LCID lcid, ITypeInfo **pptinfo)
        { return m_pBrowser2->GetTypeInfo(itinfo, lcid, pptinfo); }
    STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR *rgszNames, UINT cNames, LCID lcid, DISPID *rgdispid)
        { return m_pBrowser2->GetIDsOfNames(riid,rgszNames,cNames,lcid,rgdispid); }
    STDMETHOD(Invoke)(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pdispparams, VARIANT *pvarResult, EXCEPINFO *pexcepinfo, UINT *puArgErr)
        { return m_pBrowser2->Invoke(dispidMember, riid, lcid, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr); }

    // IWebBrowser
    STDMETHOD(GoBack)(){ return m_pBrowser2->GoBack(); }
    STDMETHOD(GoForward)(){ return m_pBrowser2->GoForward(); }
    STDMETHOD(GoHome)(){ return m_pBrowser2->GoHome(); }
    STDMETHOD(GoSearch)(){ return m_pBrowser2->GoSearch(); }
    STDMETHOD(Navigate)(BSTR URL, VARIANT *Flags, VARIANT *TargetFrameName, VARIANT *PostData, VARIANT *Headers)
	{ return m_pBrowser2->Navigate( URL, Flags, TargetFrameName, PostData, Headers); }
    STDMETHOD(Refresh)(){ return m_pBrowser2->Refresh(); }
    STDMETHOD(Refresh2)(VARIANT *Level){ return m_pBrowser2->Refresh2(Level); }
    STDMETHOD(Stop)(){ return m_pBrowser2->Stop(); }
    STDMETHOD(get_Application)(IDispatch **ppDisp){ return m_pBrowser2->get_Application(ppDisp); }
    STDMETHOD(get_Parent)(IDispatch **ppDisp){ return m_pBrowser2->get_Parent(ppDisp); }
    STDMETHOD(get_Container)(IDispatch **ppDisp){ return m_pBrowser2->get_Container(ppDisp); }
    STDMETHOD(get_Document)(IDispatch **ppDisp){ return m_pBrowser2->get_Document(ppDisp); }
    STDMETHOD(get_TopLevelContainer)(VARIANT_BOOL *pBool){ return m_pBrowser2->get_TopLevelContainer(pBool); }
    STDMETHOD(get_Type)(BSTR * pbstrType){ return m_pBrowser2->get_Type(pbstrType); }
    STDMETHOD(get_Left)(long * pl){ return m_pBrowser2->get_Left(pl); }
    STDMETHOD(put_Left)(long Left){ return m_pBrowser2->put_Left(Left); }
    STDMETHOD(get_Top)(long * pl){ return m_pBrowser2->get_Top(pl); }
    STDMETHOD(put_Top)(long Top){ return m_pBrowser2->put_Top(Top); }
    STDMETHOD(get_Width)(long * pl){ return m_pBrowser2->get_Width(pl); }
    STDMETHOD(put_Width)(long Width){ return m_pBrowser2->put_Width(Width); }
    STDMETHOD(get_Height)(long * pl){ return m_pBrowser2->get_Height(pl); }
    STDMETHOD(put_Height)(long Height){ return m_pBrowser2->put_Height(Height); }
    STDMETHOD(get_LocationName)(BSTR * pbstrLocationName){ return m_pBrowser2->get_LocationName(pbstrLocationName); }
    STDMETHOD(get_LocationURL)(BSTR * pbstrLocationURL){ return m_pBrowser2->get_LocationURL(pbstrLocationURL); }
    STDMETHOD(get_Busy)(VARIANT_BOOL * pBool){ return m_pBrowser2->get_Busy(pBool); }

    /* IWebBrowserApp methods */
    STDMETHOD(Quit)(THIS){ return m_pBrowser2->Quit(); }
    STDMETHOD(ClientToWindow)(THIS_ int FAR* pcx, int FAR* pcy){ return m_pBrowser2->ClientToWindow(pcx,pcy); }
    STDMETHOD(PutProperty)(THIS_ BSTR szProperty, VARIANT vtValue){ return m_pBrowser2->PutProperty(szProperty,vtValue); }
    STDMETHOD(GetProperty)(THIS_ BSTR szProperty, VARIANT FAR* pvtValue){ return m_pBrowser2->GetProperty(szProperty,pvtValue); }
    STDMETHOD(get_Name)(THIS_ BSTR FAR* pbstrName){ return m_pBrowser2->get_Name(pbstrName); }
    STDMETHOD(get_HWND)(THIS_ long FAR* pHWND)
	{
		if( NULL==pHWND || NULL==(LPUNKNOWN)m_pBrowser2 )
			return E_INVALIDARG;

		*pHWND = (long)::GetWindow( m_pBrowser2 );

		return *pHWND ? S_OK : E_FAIL;
	}
    STDMETHOD(get_FullName)(THIS_ BSTR FAR* pbstrFullName){ return m_pBrowser2->get_FullName(pbstrFullName); }
    STDMETHOD(get_Path)(THIS_ BSTR FAR* pbstrPath){ return m_pBrowser2->get_Path(pbstrPath); }
    STDMETHOD(get_FullScreen)(THIS_ VARIANT_BOOL FAR* pBool){ return m_pBrowser2->get_FullScreen(pBool); }
    STDMETHOD(put_FullScreen)(THIS_ VARIANT_BOOL Value){ return m_pBrowser2->put_FullScreen(Value); }
    STDMETHOD(get_Visible)(THIS_ VARIANT_BOOL FAR* pBool){ return m_pBrowser2->get_Visible(pBool); }
    STDMETHOD(put_Visible)(THIS_ VARIANT_BOOL Value){ return m_pBrowser2->put_Visible(Value); }
    STDMETHOD(get_StatusBar)(THIS_ VARIANT_BOOL FAR* pBool){ return m_pBrowser2->get_StatusBar(pBool); }
    STDMETHOD(put_StatusBar)(THIS_ VARIANT_BOOL Value){ return m_pBrowser2->put_StatusBar(Value); }
    STDMETHOD(get_StatusText)(THIS_ BSTR FAR* pbstr){ return m_pBrowser2->get_StatusText(pbstr); }
    STDMETHOD(put_StatusText)(THIS_ BSTR bstr){ return m_pBrowser2->put_StatusText(bstr); }
    STDMETHOD(get_ToolBar)(THIS_ int FAR* pBool){ return m_pBrowser2->get_ToolBar(pBool); }
    STDMETHOD(put_ToolBar)(THIS_ int Value){ return m_pBrowser2->put_ToolBar(Value); }
    STDMETHOD(get_MenuBar)(THIS_ VARIANT_BOOL FAR* pValue){ return m_pBrowser2->get_MenuBar(pValue); }
    STDMETHOD(put_MenuBar)(THIS_ VARIANT_BOOL Value){ return m_pBrowser2->put_MenuBar(Value); }

    // IWebBRowser2 methods
    STDMETHOD(Navigate2)(THIS_ VARIANT FAR* URL, VARIANT FAR* Flags, VARIANT FAR* TargetFrameName, VARIANT FAR* PostData, VARIANT FAR* Headers)
	{ return m_pBrowser2->Navigate2(URL, Flags, TargetFrameName, PostData, Headers); }
    STDMETHOD(ShowBrowserBar)(THIS_ VARIANT FAR* pvaClsid, VARIANT FAR* pvaShow, VARIANT FAR* pvaSize)
	{ return m_pBrowser2->ShowBrowserBar(pvaClsid, pvaShow, pvaSize); }
    STDMETHOD(QueryStatusWB)(THIS_ OLECMDID cmdID, OLECMDF FAR* pcmdf){ return m_pBrowser2->QueryStatusWB(cmdID, pcmdf); }
    STDMETHOD(ExecWB)(THIS_ OLECMDID cmdID, OLECMDEXECOPT cmdexecopt, VARIANT FAR* pvaIn, VARIANT FAR* pvaOut)
	{ return m_pBrowser2->ExecWB(cmdID, cmdexecopt, pvaIn, pvaOut); }
    STDMETHOD(get_ReadyState)(THIS_ READYSTATE FAR* plReadyState){ return m_pBrowser2->get_ReadyState(plReadyState); }
    STDMETHOD(get_Offline)(THIS_ VARIANT_BOOL FAR* pbOffline){ return m_pBrowser2->get_Offline(pbOffline); }
    STDMETHOD(put_Offline)(THIS_ VARIANT_BOOL bOffline){ return m_pBrowser2->put_Offline(bOffline); }
    STDMETHOD(get_Silent)(THIS_ VARIANT_BOOL FAR* pbSilent){ return m_pBrowser2->get_Silent(pbSilent); }
    STDMETHOD(put_Silent)(THIS_ VARIANT_BOOL bSilent){ return m_pBrowser2->put_Silent(bSilent); }
    STDMETHOD(get_RegisterAsBrowser)(THIS_ VARIANT_BOOL FAR* pbRegister){ return m_pBrowser2->get_RegisterAsBrowser(pbRegister); }
    STDMETHOD(put_RegisterAsBrowser)(THIS_ VARIANT_BOOL bRegister){ return m_pBrowser2->put_RegisterAsBrowser(bRegister); }
    STDMETHOD(get_RegisterAsDropTarget)(THIS_ VARIANT_BOOL FAR* pbRegister){ return m_pBrowser2->get_RegisterAsDropTarget(pbRegister); }
    STDMETHOD(put_RegisterAsDropTarget)(THIS_ VARIANT_BOOL bRegister){ return m_pBrowser2->put_RegisterAsDropTarget(bRegister); }
    STDMETHOD(get_TheaterMode)(THIS_ VARIANT_BOOL FAR* pValue){ return m_pBrowser2->get_TheaterMode(pValue); }
    STDMETHOD(put_TheaterMode)(THIS_ VARIANT_BOOL Value){ return m_pBrowser2->put_TheaterMode(Value); }
    STDMETHOD(get_AddressBar)(THIS_ VARIANT_BOOL FAR* Value){ return m_pBrowser2->get_AddressBar(Value); }
    STDMETHOD(put_AddressBar)(THIS_ VARIANT_BOOL Value){ return m_pBrowser2->put_AddressBar(Value); }
    STDMETHOD(get_Resizable)(THIS_ VARIANT_BOOL FAR* Value) { return m_pBrowser2->get_Resizable(Value); }
    STDMETHOD(put_Resizable)(THIS_ VARIANT_BOOL Value) { return m_pBrowser2->put_Resizable(Value); }

    CWebBrowserAuto( LPUNKNOWN punkOuter ) : m_pBrowser2( punkOuter ){};
    ~CWebBrowserAuto(){ m_pBrowser2.Release(); }

protected:
	CComQIPtr<IWebBrowser2> m_pBrowser2;
};


interface vIEDevToolStub : public IUnknown
{
public:
    STDMETHOD (QueryInterface)(THIS_ REFIID riid, void **ppv)
	{
		return E_NOINTERFACE;
	}
    STDMETHOD_(ULONG, AddRef)()
	{
		return 0;
	}
    STDMETHOD_(ULONG, Release)()
	{
		return 0;
	}
	STDMETHOD (SetDevToolSite )( LPUNKNOWN pDispWeb ){ return S_OK; }
	STDMETHOD (ShowDevTools)( long lshow ){ return S_OK; }
	STDMETHOD (JITShowDevTools)(long lshow){ return S_OK; }
	STDMETHOD (CloseDevTools)(){ return S_OK; }
	STDMETHOD (IsVisible)(){ return S_OK; }
	STDMETHOD (GetPinState)(){ return S_OK; }
	STDMETHOD (SetPinState)(LONG state){ return S_OK; }
	STDMETHOD (_IsMinimized)(){ return S_OK; }
	STDMETHOD (DevToolsReady)(){ return S_OK; }
};

class vShellBrowser2 : public IUnknown
{
public:
	vShellBrowser2( LPUNKNOWN pUnkWeb ) : m_spWebAuto(pUnkWeb),m_uRef(0){}
    STDMETHOD (QueryInterface)(THIS_ REFIID riid, void **ppv)
	{
		return E_NOINTERFACE;
	}
    STDMETHOD_(ULONG, AddRef)()
	{
		return m_uRef++;
	}
    STDMETHOD_(ULONG, Release)()
	{
		if( 0==--m_uRef )
		{
			delete this;
			return 0;
		}

		return m_uRef;
	}

	STDMETHOD ( GetBrowser )( LPUNKNOWN* ppDispWeb )
	{
		if( NULL==ppDispWeb )
			return E_INVALIDARG;

		m_spWebAuto.AddRef();
		*ppDispWeb = (LPUNKNOWN)&m_spWebAuto;
		return S_OK;
	}
	STDMETHOD (PinUnPin)( long lshow,long unknown=0){ return S_OK; }  //iedvtool!CIEDevToolsExplorerBar::Pin崩溃 堆栈不平衡
	STDMETHOD (OnCloseDevTools)(){ return S_OK; }
	STDMETHOD (OnLoseFocus)(){ return S_OK; }
	STDMETHOD (JITShowDevTools)(){ return S_OK; }
	STDMETHOD (DevToolsOpened)(){ return S_OK; }
	STDMETHOD (CloseTools)(){ return S_OK; }
	STDMETHOD (OnUIActivate)(){ return S_OK; }

	static vIEDevToolStub* CreateDevTool( IWebBrowser2* pspWeb )
	{
		::SHDeleteValue (HKEY_CURRENT_USER,_T("Software\\Microsoft\\Internet Explorer\\IEDevTools"),_T("WindowPos"));
		TCHAR szDirIE[MAX_PATH+1] = {0};
		::SHRegGetPath( HKEY_CLASSES_ROOT, _T("CLSID\\{1a6fe369-f28c-4ad9-a3e6-2bcb50807cf1}\\InProcServer32"),_T(""),szDirIE, MAX_PATH );
		::PathRemoveFileSpec( szDirIE );
		::SetCurrentDirectory( szDirIE );

		//IE7-IE10
		const CLSID CLSID_IEDevTool = { 0x1a6fe369,0xf28c,0x4ad9,0xa3,0xe6,0x2b,0xcb,0x50,0x80,0x7c,0xf1 };
		const CLSID IID_IEDevTool = { 0x059055df,0x6bb0,0x402a,0xba,0x48,0x58,0xbf,0xa3,0x43,0x71,0x9c };
		vIEDevToolStub* pDevTool = NULL;
		//IE11
		const CLSID CLSID_IEDevTool11 = { 0x28bccb9a,0xe66b,0x463c,0x82,0xa4,0x09,0xf3,0x20,0xde,0x94,0xd7 };
		const CLSID IID_IEDevTool11 = { 0x181e3828,0xfe6e,0x4602,0xa3,0x27,0x78,0x6a,0x76,0xfd,0xfb,0x3a };
		HRESULT hr = ::CoCreateInstance( CLSID_IEDevTool,NULL,CLSCTX_INPROC_SERVER,IID_IEDevTool,(void**)&pDevTool );
		if( FAILED(hr))
		{
			//支持IE11调起旧的F12工具
			CoCreateInstance( CLSID_IEDevTool,NULL,CLSCTX_INPROC_SERVER,IID_IEDevTool11,(void**)&pDevTool );
		}
		if( NULL == pDevTool )
		{
			return NULL;
		}
		pDevTool->SetDevToolSite( new vShellBrowser2(pspWeb) );  //F12Tools!BHOSite::SetDevToolSite
		pDevTool->DevToolsReady();//F12Tools!BHOSite::DevToolsReady
		pDevTool->SetPinState(0);
		pDevTool->ShowDevTools( 1 );//F12Tools!BHOSite::ShowDevTools
		//pDevTool->JITShowDevTools(1); 立即调试
		return pDevTool;
	}

protected:
	CWebBrowserAuto m_spWebAuto;
	UINT m_uRef;
};

