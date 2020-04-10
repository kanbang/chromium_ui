
#include "chrome/iecore/trident/stdafx.h"
#include "IEBrowserWnd.h"
#include "XWnd.h"
#include "Browserdef.h"
#include "chrome/iecore/trident/patch/HookApi.h"
#include "chrome/iecore/trident/patch/PatchIEMode.h"
#include "base/at_exit.h"
#include "base/command_line.h"
#include "base/bind.h"
#include "chrome/iecore/iecore_message.h"
#include "chrome/iecore/trident/Patch/CompatibilityViewRule.h"
//#include "chrome/iecore/child/browser_messages.h"
#include "chrome/iecore/child/render_view.h"
//base::AtExitManager exit_manager;

DWORD g_dwTlsBrowser = TlsAlloc();

CIEBrowserWnd::CIEBrowserWnd()
{
	m_pStorage=NULL;
	m_pClientSite=NULL;
	m_pOleObject=NULL;

	m_pOIPA=NULL;
	m_pWebBrowser=NULL;
	m_pDispatch=NULL;


	m_hWndParent=NULL;
	m_hWnd=NULL;
	m_hBrowserWnd = NULL;

	m_pDevTool = NULL;
	m_hNewWindowEvent =NULL;
	m_hNewWindowOK = NULL;

	m_bSpecialGesrure=false;
	//m_FuncRouteMessageToParent=NULL;
	m_TabId = -1;
  rotate_id_ = 0;
	//m_bFixedDocument = false;
  m_bCanBack=FALSE;
  m_bCanForward=FALSE;
  m_bPopupWindow = false;
  m_hMsgWnd = NULL;
  render_view_ = NULL;
  m_procIEBrowser = NULL;
  m_didFocus = true;
}

CIEBrowserWnd::~CIEBrowserWnd()
{
	SafeReleaseGesture();
}

void CIEBrowserWnd::SafeReleaseGesture()
{
}
// The information on how to set the thread name comes from
// a MSDN article: http://msdn2.microsoft.com/en-us/library/xcb2z8hs.aspx
const DWORD kVCThreadNameException = 0x406D1388;

typedef struct tagTHREADNAME_INFO {
	DWORD dwType;  // Must be 0x1000.
	LPCSTR szName;  // Pointer to name (in user addr space).
	DWORD dwThreadID;  // Thread ID (-1=caller thread).
	DWORD dwFlags;  // Reserved for future use, must be zero.
} THREADNAME_INFO;

void SetThreadName( DWORD dwThreadID, LPCSTR szThreadName)
{
	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = szThreadName;
	info.dwThreadID = dwThreadID;
	info.dwFlags = 0;
	__try
	{
		RaiseException( kVCThreadNameException, 0, sizeof(info)/sizeof(DWORD), (DWORD*)&info );
	}
	__except(EXCEPTION_CONTINUE_EXECUTION)
	{
	}
}

CIEBrowserWnd* CIEBrowserWnd::CreateBrowserThread( HWND hWndParent,HWND hMsgWnd,
  bool bNewWindow,int rotateid, RenderView* render_view)
{
	CIEBrowserWnd* pIEBrowser = new CIEBrowserWnd;
	if( pIEBrowser ){
		pIEBrowser->m_TabId = 0;
		pIEBrowser->m_hWndParent = hWndParent;
    pIEBrowser->m_bNewWindow = bNewWindow;
    pIEBrowser->rotate_id_ = rotateid;
    pIEBrowser->m_hMsgWnd = hMsgWnd;
    pIEBrowser->render_view_ = render_view;
		UINT nThreadID = 0;
		HANDLE hThread = (HANDLE)_beginthreadex( NULL, 0, BrowserThreadProc, (void*)pIEBrowser, 0, &nThreadID );
		if(hThread)
		{
			//SetThreadName(nThreadID,"WG_BrowserThredProc");
			CloseHandle(hThread);
		}
	}
	return pIEBrowser;
}

void CIEBrowserWnd::SetProcessFeatureControl()
{
	//不能直接调用Co函数，会过早的引入urlmon,导致hook GetMoudleFileName伪装IE失效
	//修复XP+IE8，文档模式与IE不一致 过早的引入了urlmon.dll
  //FeatureControl参考：http://msdn.microsoft.com/en-us/library/ms537169.aspx
	typedef HRESULT (__stdcall * COINTERNETSETFEATUREENABLED)(int,DWORD,BOOL);
	HMODULE urlmon= LoadLibrary(_T("urlmon.dll"));
	if( urlmon)
	{
		COINTERNETSETFEATUREENABLED SelfCoInternetSetFeatureEnabled = (COINTERNETSETFEATUREENABLED)GetProcAddress(urlmon,"CoInternetSetFeatureEnabled");
		if(SelfCoInternetSetFeatureEnabled)
		{
			SelfCoInternetSetFeatureEnabled(FEATURE_SECURITYBAND, SET_FEATURE_ON_PROCESS, TRUE);
			SelfCoInternetSetFeatureEnabled(FEATURE_HTTP_USERNAME_PASSWORD_DISABLE, SET_FEATURE_ON_PROCESS, TRUE);
			SelfCoInternetSetFeatureEnabled(FEATURE_RESTRICT_ACTIVEXINSTALL, SET_FEATURE_ON_PROCESS, TRUE);//URLACTION_AUTOMATIC_ACTIVEX_UI
			SelfCoInternetSetFeatureEnabled(FEATURE_ADDON_MANAGEMENT, SET_FEATURE_ON_PROCESS, FALSE);
			SelfCoInternetSetFeatureEnabled(FEATURE_WEBOC_POPUPMANAGEMENT, SET_FEATURE_ON_PROCESS, FALSE);
			SelfCoInternetSetFeatureEnabled(FEATURE_RESTRICT_FILEDOWNLOAD,SET_FEATURE_ON_PROCESS,FALSE); 
		}
	}
}

void CIEBrowserWnd::BrowserInitialize()
{
	StartBrowserProcessAPIHook();
	SetProcessFeatureControl();
}

unsigned int __stdcall CIEBrowserWnd::BrowserThreadProc(void * p)
{
	OleInitialize(NULL);
	BrowserInitialize();
	CIEBrowserWnd* pIEBrowser = (CIEBrowserWnd*)p;
	if( pIEBrowser )
	{
		if( pIEBrowser->DoCreate())
			pIEBrowser->BrowserMessageLoop();
		else
			pIEBrowser->DestoryTabPage();

		delete pIEBrowser;
	}
	OleUninitialize();
	return 1;
}

void CIEBrowserWnd::BrowserMessageLoop()
{
	CommandLine::Init(0, NULL);

	base::PlatformThread::SetName("render_BrowserThread");
	browser_message_loop_ = new base::MessageLoop(base::MessageLoop::TYPE_UI);

	//IPC_OnBrowserReady();
  Navigate(L"about:blank", NULL);
  browser_message_loop_->PostTask(FROM_HERE, base::Bind(&CIEBrowserWnd::OnBrowserReady, base::Unretained(this)));
	scoped_ptr<base::MessageLoopForUI::MessageFilter> message_filter(new RenderUIMessageFilter(this));
	base::MessageLoopForUI::current()->SetMessageFilter(message_filter.PassAs<base::MessageLoopForUI::MessageFilter>());  
	base::MessageLoop::current()->Run();

}
void CIEBrowserWnd::SetBrowserParentWnd(HWND hWnd)
{
	m_hWndParent = hWnd;
}

void CIEBrowserWnd::CreateContainerWnd()
{
  RECT rect={100,100,100,100};
  if(m_hWndParent)
  {
    GetClientRect(m_hWndParent,&rect);
  }
  CopyRect(&m_rcRect,&rect);
	m_hWnd = XWnd::Create( 0, _T("IEWin"), _T("Container"), WS_CHILD|WS_CLIPSIBLINGS|WS_CLIPCHILDREN, &m_rcRect, HWND_MESSAGE, 0 );
	//ShowWindow(m_hWnd,SW_SHOW);
  if(m_hWndParent)
  {
	  SetParent(m_hWnd,m_hWndParent);
  }
}

BOOL CIEBrowserWnd::DoCreate()
{
  SetIEBrowserToTls();

	CreateContainerWnd();

	if( m_hWnd)
	{
		m_oWebBrowser.SetIEBrowserWnd( this );
		m_pStorage = &m_oWebBrowser;
		m_pClientSite = &m_oWebBrowser;
		if( S_OK == OleCreate(CLSID_WebBrowser, IID_IOleObject, OLERENDER_DRAW, 0, m_pClientSite, m_pStorage, (void**)&m_pOleObject) )
		{
			if( m_pOleObject )
			{
				m_pOleObject->QueryInterface( IID_IOleInPlaceActiveObject, (void**)&m_pOIPA );
				m_pOleObject->QueryInterface( IID_IWebBrowser2, (void**)&m_pWebBrowser );
				m_pWebBrowser->QueryInterface( IID_IDispatch, (void **)&m_pDispatch );		
				if( m_pDispatch)
				{
					OnCreate();
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}


void CIEBrowserWnd::OnCreate()
{
	onInitBrowser();
	m_oWebBrowser.Connect();
}

void CIEBrowserWnd::onInitBrowser()
{
	m_pOleObject->SetHostNames( L"TheWorld Browser", 0 );
	m_pOleObject->SetClientSite(m_pClientSite);
	OleSetContainedObject( m_pOleObject, TRUE );

	m_pOleObject->DoVerb(OLEIVERB_SHOW, NULL, (IOleClientSite *)m_pClientSite, 0, m_hWnd,	NULL);
	SetBrowserWndRect();
	
	m_pWebBrowser->put_RegisterAsBrowser( VARIANT_TRUE );
}

void CIEBrowserWnd::OnResizeMsg(ViewMsg_Resize_Params params) {
  SetBrowserWndRect();
}

void CIEBrowserWnd::NavigateParams(ViewMsg_Navigate_Params params) {
  Navigate(base::UTF8ToWide(params.url).c_str());
}

void CIEBrowserWnd::OnActiveMsg(bool active) {

}

void CIEBrowserWnd::OnSetFocusMsg(bool enable) {
  FindBrowserWnd();

  if (!enable && IsWindow(m_hBrowserWnd)) {
    ::PostMessage(m_hBrowserWnd, WM_KILLFOCUS, NULL, NULL);
    return;
  }

  if ((::GetFocus() == m_hBrowserWnd))
    return;
  
  //bCondition为ture才setfocus,如果标记了来源,先检测来源是否还有焦点=
  if(enable){
    m_didFocus = false;
    if( IsWindow(m_hBrowserWnd) ){
        //::SetFocus( m_hWndBrowser );
      SetBrowserFocus();
    }
    m_didFocus = true;
  }
}

bool CIEBrowserWnd::HandledRedirectIPCMsg(UINT msg, WPARAM wparam, LPARAM lparam){
     if( msg != WM_IPCMSG_REDIRECT )
       return false;
     IPC::Message* pMsg  = (IPC::Message*)wparam;
     if( pMsg ){
       OnIPCMsg(*pMsg);
       delete pMsg;
     }else{
       NOTREACHED();
     }
     return true;
}

bool CIEBrowserWnd::OnIPCMsg(const IPC::Message& msg){
  bool handled = false;
  IPC_BEGIN_MESSAGE_MAP(CIEBrowserWnd, msg) 
    IPC_MESSAGE_HANDLER(ViewMsg_Resize, OnResizeMsg)
    IPC_MESSAGE_HANDLER(ViewMsg_Navigate, NavigateParams)
    IPC_MESSAGE_HANDLER(InputMsg_SetFocus, OnSetFocusMsg)
    IPC_MESSAGE_HANDLER(ViewMsg_SetActive, OnActiveMsg)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()
  return handled;
}

LRESULT CALLBACK CIEBrowserWnd::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{
  case CHILDTHREAD_TO_BROWSER_SET_PARENT:
    {
      m_hWndParent = (HWND)lParam;
      ::SetParent(m_hWnd, m_hWndParent);
      SetBrowserWndRect();
    }
    break;
	case WM_COMMAND:
		{
			if (1 == HIWORD(wParam))
				//快捷键
				OnAccelerator(LOWORD(wParam));
			return 1;
		}break;
  case WM_WINDOWPOSCHANGED:
    {
      SetBrowserWndRect();
    }
    break;
  case WM_IPCMSG_REDIRECT: {
      if(HandledRedirectIPCMsg(msg, wParam, lParam))
        return 0;
    }
    break;
// 	case WG_BROWSER_NAVIGATE:
// 		{
// 			onInitBrowser();
// 			Navigate((LPCTSTR)wParam,NULL);
// 			return 1;
// 		}
// 	case WM_BROWSER_SIZE:
// 		{
// 			SetBrowserWndRect();
// 			return 1;
// 		}
// 		break;
// 	case WG_BROWSER_INIT:
// 		{
// 			onInitBrowser();
// 			return 1;
// 		}break;
// 	case WG_BROWSER_GETMARSHALDATA:
// 		{
// 			onGetMarshalData();
// 			return S_OK;
// 		}break;
// 	case WG_BROWSER_SETFOCUS:
// 		{
// 			SetBrowserFocus();
// 			return 0;
// 		}break;
// 	case WG_BROWSER_DESTORY:
// 		{
// 			DestoryTabPage();
// 			return 0;
// 		}break;
	}
	return ::DefWindowProc( hWnd, msg, wParam, lParam );
}

LRESULT __stdcall CIEBrowserWnd::IEBrowserWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  CIEBrowserWnd * pThis = (CIEBrowserWnd*)GetProp(hWnd, WNDPROC_MINIBROWSERWNDPROC);
  if( !pThis )
    return 0;

  switch(message) {
  case WM_SETFOCUS:
    if(pThis->render_view_ && pThis->m_didFocus)
      pThis->render_view_->didFocus();
    break;
  }

  if (pThis->m_procIEBrowser) {
    LRESULT result = CallWindowProc(pThis->m_procIEBrowser, hWnd,
      message, wParam, lParam);
    return result;
  }
  return 0;
}

void CIEBrowserWnd::Navigate( LPCTSTR pszUrlPage, LPCTSTR lpszRefererUrl )
{
	if( NULL == pszUrlPage || *pszUrlPage==_T('\0'))
		return ;
	m_szUrl = pszUrlPage;
	IWebBrowser2 * pWebBrowser = m_pWebBrowser;
	if( pWebBrowser )
	{
		BOOL bNew = FALSE;

		LPWSTR pszUrl;
		pszUrl = (LPWSTR)pszUrlPage;

		VARIANT varUrl;
		VARIANT vtEmpty;
		VARIANT vtFlags;
		VARIANT vtHeaders;
		vtEmpty.vt = VT_EMPTY; 
		vtFlags.vt = VT_I4;
		vtFlags.lVal = navOpenInNewWindow;
		varUrl.vt = VT_BSTR;
		varUrl.bstrVal = T2BSTR(pszUrl);
		vtHeaders.vt = VT_BSTR;
		vtHeaders.bstrVal = NULL;
		if( lpszRefererUrl )
		{
			TCHAR szReferer[INTERNET_MAX_URL_LENGTH+MAX_PATH]=_T("");
			_stprintf_s(szReferer, _T("Referer: %s\r\n"), lpszRefererUrl);
			vtHeaders.bstrVal = T2BSTR(szReferer);
		}
		
		HRESULT hr = pWebBrowser->Navigate2(&varUrl, bNew?&vtFlags:&vtEmpty, &vtEmpty, &vtEmpty, vtHeaders.bstrVal?&vtHeaders:&vtEmpty );

		if(varUrl.bstrVal)
			SysFreeString( varUrl.bstrVal );
		if(vtHeaders.bstrVal)
			SysFreeString( vtHeaders.bstrVal );

		return ;
	}
	return ;
}
BOOL CIEBrowserWnd::Fire_onbeforeunload()
{
	HRESULT hr=S_FALSE;
	CComVariant varResult;
	if( m_pWebBrowser)
	{
		hr = m_pWebBrowser->ExecWB(OLECMDID_ONUNLOAD,OLECMDEXECOPT_DODEFAULT,NULL,&varResult);
		//ATLASSERT(SUCCEEDED(hr));
		if( SUCCEEDED(hr) &&varResult.bVal ==0x00)
			return FALSE;//留在此页面
	}
	return TRUE;
}
void CIEBrowserWnd::DestoryTabPage()
{
	DestroyWebBrowser();
}

void CIEBrowserWnd::OnBrowserReady() {
  Browser_Ready_Params params;
  params.hwnd = m_hWnd;
  params.roateid = rotate_id_;
  Send(new Browser_Ready(params));

  if(m_bNewWindow)
    base::MessageLoop::current()->PostTask(FROM_HERE,base::Bind(&CIEBrowserWnd::onGetMarshalData,base::Unretained(this)));
}

void CIEBrowserWnd::DestroyWebBrowser()
{
	//此函数轻易不要修改
	if(!Fire_onbeforeunload())
		return;

	if( !IsWindow(m_hWnd) )
		return;

	if( m_pDispatch ){
    m_pDispatch->Release();
    m_pDispatch = NULL;
  }

	m_oWebBrowser.DisConnect();

	if( m_pWebBrowser )
	{
		m_pWebBrowser->put_RegisterAsBrowser( VARIANT_FALSE );
		m_pWebBrowser->Stop();
		m_pWebBrowser->Release();
    m_pWebBrowser = NULL;
	}

	if( m_pOIPA )
  {
		m_pOIPA->Release();
    m_pOIPA=NULL;
  }

	if( m_pOleObject )
	{
		OleSetContainedObject( m_pOleObject, FALSE );
		m_pOleObject->SetClientSite(NULL);  //不加这句话在IE11下newwindow后关闭http://hao.360.cn，在OleUnIntialize必崩
		m_pOleObject->Close( OLECLOSE_NOSAVE );
		m_pOleObject->Release();
    m_pOleObject=NULL;
	}
	DestroyWindow(m_hWnd);
  //发送IPC销毁Host
  IPC_DestoryViewHost();

	PostQuitMessage(0);
}

void CIEBrowserWnd::UIActivate()
{
	if( m_pOleObject)
		m_pOleObject->DoVerb( OLEIVERB_UIACTIVATE, NULL, NULL, -1, NULL, NULL );
}

HWND CIEBrowserWnd::FindBrowserWnd()
{
  // 不能单纯根据有wnd决定不再查找，因为有可能本控件内的ax window已经不是IE，是Explorer View
  if( m_hBrowserWnd && ::IsWindow(m_hBrowserWnd) )
    return m_hBrowserWnd;

  HWND hNowBrowser;
  HWND hWnd = FindWindowEx( m_hWnd, NULL, _T("Shell Embedding"), NULL );
  if( hWnd )
  {
    hWnd = FindWindowEx( hWnd, NULL, _T("Shell DocObject View"), NULL );
    if( hWnd )
    {
      hNowBrowser = FindWindowEx( hWnd, NULL, _T("Internet Explorer_Server"), NULL );
      if( !hNowBrowser )
        hNowBrowser = FindWindowEx( hWnd, NULL, _T("SysListView32"), NULL );
      if( hNowBrowser && m_hBrowserWnd != hNowBrowser )
      {
        m_hBrowserWnd = hNowBrowser;

        m_procIEBrowser = (WNDPROC)GetWindowLong( m_hBrowserWnd, GWL_WNDPROC );
        ::SetProp( m_hBrowserWnd, WNDPROC_MINIBROWSERWNDPROC, this );
        SetWindowLong(m_hBrowserWnd, GWL_WNDPROC, (LONG)IEBrowserWndProc);
      }
    }
  }
  return m_hBrowserWnd;
}

void CIEBrowserWnd::SetBrowserFocus()
{
	//dz 2013/4/21  解决焦点问题
	m_hBrowserWnd = FindBrowserWnd();

	SetFocus(m_hBrowserWnd);

	//这句话 解决焦点在页面 无法F5刷新问题 ，也可以通过接管F5快捷键，调用pWebBrowser->Rfresh刷新（SE5处理方法）
	UIActivate();
}

void CIEBrowserWnd::OnAccelerator( DWORD identifier )
{
	/*switch(identifier)
	{
// 	case ID_ACCELERATOR_WEBDEBUGGER:
// 		ShowWebDebugger(TRUE);
// 		break;

	}*/
	return;
}

void CIEBrowserWnd::ShowWebDebugger(BOOL bShow )
{

	//IE8CORE::LoadIE8CoreF12Dlls();
	if( bShow )
	{
		if( NULL == m_pDevTool )
			m_pDevTool = vShellBrowser2::CreateDevTool(m_pWebBrowser);
		else
			m_pDevTool->ShowDevTools( 1 );
	}
	else
	{
		if( m_pDevTool )
		{
			m_pDevTool->CloseDevTools();
			m_pDevTool->Release();
			m_pDevTool = NULL;
		}		
	}	
}

//跨线程 请求另一个Browser的IDispatch接口
IDispatch* CIEBrowserWnd::OnNewWindow(LPCTSTR lpszUrl)
{
	IDispatch* pDispatch = NULL;
	//WGMSG::LWGMARSHALDATA  wgMarshaData={0};
  Browser_NewWindow_Params new_window;
	if( lpszUrl)
	{
    new_window.url = lpszUrl;
		m_szUrl=lpszUrl;
	}
  int test = 0;
	if( m_hNewWindowEvent == NULL)
		m_hNewWindowEvent = ::CreateEvent(NULL,TRUE, FALSE, NULL);
	if( m_hNewWindowEvent)
	{
		ResetEvent(m_hNewWindowEvent);
		new_window.hEvent = m_hNewWindowEvent;
		//发送给UI线程 索要IDispatch接口，同步等待10s
		//这里应该使用scope_ptr
    new_window.popwindow=m_bPopupWindow;
    new_window.browser_id = rotate_id_;
    
    //Send(new Browser_NewWindow(new_window));
    PostMessage(m_hMsgWnd, BROWSER_TO_CHILDTHREAD_NEWWINDOW, 0, (LPARAM)&new_window);
		//g_Frame->PostTaskToView(base::Bind(&ISoFrame::OnNewWindow,base::Unretained(g_Frame),(LPVOID)&wgMarshaData));
		WaitForSingleObject(m_hNewWindowEvent, /*INFINITE*/10000);//等待10s
	}
	GetInterfaceFromStream(IID_IDispatch, new_window.pStream, new_window.dwSize, (void**)&pDispatch);
	SetEvent(new_window.hNewEvent);
	return pDispatch;
}

HRESULT CIEBrowserWnd::GetInterfaceFromStream(REFIID riid, LPVOID lpStream, DWORD dwSize, LPVOID *ppObject)
{
	HRESULT hResult = S_FALSE;
	if(lpStream && 0!=dwSize)
	{
		LPSTREAM pStream = NULL;
		if(S_OK==CreateStreamOnHGlobal(NULL, TRUE, &pStream))
		{
			ULONG uLongWrite = 0;
			if(S_OK==pStream->Write(lpStream, dwSize, &uLongWrite))
			{
				LARGE_INTEGER lMove;
				lMove.LowPart = 0;
				lMove.HighPart = 0;
				pStream->Seek(lMove, STREAM_SEEK_SET, NULL);

				if(S_OK==CoUnmarshalInterface(pStream, riid, (void**)ppObject) && *ppObject)
					hResult = S_OK;
			}
			pStream->Release();
		}
	}
	return hResult;
}


void CIEBrowserWnd::onGetMarshalData()
{
  LPSTREAM pStream = NULL;
  Browser_NewWindow_Params params;
  if( m_hNewWindowOK == NULL)
    m_hNewWindowOK = ::CreateEvent(NULL,TRUE, FALSE, NULL);
  HGLOBAL hGlobal = NULL;
  if(S_OK==CreateStreamOnHGlobal(NULL, TRUE, &pStream))
  {
    if(S_OK==CoMarshalInterface(pStream, IID_IDispatch, m_pDispatch, MSHCTX_LOCAL, NULL, MSHLFLAGS_NORMAL))
    {
      if(S_OK==GetHGlobalFromStream(pStream, &hGlobal))
      {
        ResetEvent(m_hNewWindowOK);
        params.hNewEvent = m_hNewWindowOK;
        params.pStream = GlobalLock(hGlobal);
        params.dwSize = GlobalSize(hGlobal);
      }
    }
  }
  Send(new Browser_NewWindowReady(rotate_id_, params));
  /*if( !m_bPopupWindow)
    g_Frame->PostTaskToView(base::Bind(&ISoFrame::OnNewWindowReady,base::Unretained(g_Frame),m_TabId,(LPVOID)&twMarshalData));
  else
    g_Frame->PostTaskToView(base::Bind(&ISoFrame::OnPopupWindowReady,base::Unretained(g_Frame),m_TabId,(LPVOID)&twMarshalData));*/
  WaitForSingleObject(m_hNewWindowOK, /*INFINITE*/10000);//等待10s
  if(params.pStream){
    GlobalUnlock(hGlobal);
  }
  if(pStream)
    pStream->Release();
}

void CIEBrowserWnd::onBeforeNavigate(DISPPARAMS *pDispParams, VARIANT *pVarResult)
{
	LPCTSTR pUrl = OLE2T(pDispParams->rgvarg[5].pvarVal->bstrVal);
  if( pDispParams->rgvarg[6].vt == VT_DISPATCH)
  {
      //判断页面是否有iframe
      CComQIPtr<IWebBrowser2> pWeb =  pDispParams->rgvarg[6].pdispVal;
      if( pWeb == m_pWebBrowser)
      {
        int k=0;
      }
      else
      {
        int k=0;
      }
  }
	if( IsNineTab())
	{
		if( pDispParams->rgvarg[5].pvarVal && pUrl )
		{
			m_szNineTabUrl = pUrl;
			if( _tcsicmp(pUrl,m_szUrl.c_str()) != 0)
			{
// 				//发送当前标签m_hWndParent，URL给逻辑线程
// 				PostMessage(_hLogicWnd,WG_BROWSER_NINETAB_NAVIGATE,(WPARAM)m_hWndParent,(LPARAM)m_szNineTabUrl.c_str());
// 				*(pDispParams->rgvarg[0].pboolVal) = VARIANT_TRUE;
			}
		}
	}
}

bool CIEBrowserWnd::IsNineTab()
{
	bool bRet =false;
	if( StrStrI(m_szUrl.c_str(),L"NewTab.dll/index.html"))
	{
		return true;
	}
	return bRet;
}


void CIEBrowserWnd::OnDocumentComplete()
{
// 	if(!m_bFixedDocument && _bIE11)
// 	{
// 		m_bFixedDocument = true;
// 		//fixed IE11 Taobao卖家版 复制粘贴上传图片无法使用
// 		//fixed IE11 webqq 侧边栏不显示
// 		//PatchIEMode::SetHtmlDocumentMode(m_pWebBrowser,PatchIEMode::Standard_IE10);
	SetBrowserFocus();
}

bool CIEBrowserWnd::ProcessMessage(MSG msg)
{
	bool bHandled = false;
  if( msg.message >= WM_KEYFIRST && msg.message <= WM_KEYLAST ){
    if((msg.wParam=='F' && HIBYTE(GetKeyState(VK_CONTROL))) ||
      (msg.message == WM_SYSKEYDOWN && msg.wParam == VK_F4)) {
        PostMessage(GetParent(m_hWnd), msg.message, msg.wParam, msg.lParam);
        bHandled = true;
    }
    else {
      if( msg.wParam == VK_MENU 
        || msg.wParam == VK_CONTROL 
        || msg.wParam == VK_SHIFT)
        bHandled = false;
      else {
        PostMessage( GetParent(m_hWnd), msg.message, msg.wParam, msg.lParam );
        if(HIBYTE(GetKeyState(VK_CONTROL)))
          if(  VK_OEM_PLUS == msg.wParam || VK_OEM_MINUS == msg.wParam  ||
            VK_ADD ==msg.wParam || VK_SUBTRACT == msg.wParam   )
            bHandled =  true;
      }
    }
  }

  if( bHandled == false)
  {
    //页面对象处理
    if( m_pOIPA && S_OK == m_pOIPA->TranslateAccelerator( &msg ) )
    {
      bHandled = true; 
    }
  }
	return bHandled;
}
void CIEBrowserWnd::UpdateBrowserSize()
{
  RECT rect={0};
  GetClientRect(m_hWnd,&rect);
  IOleInPlaceObject* pObj = NULL;
  if( m_pOleObject )
  {
    m_pOleObject->QueryInterface( IID_IOleInPlaceObject, (void**)&pObj );
    if( pObj )
    {
      pObj->SetObjectRects( &rect, &rect );
      pObj->Release();
    }
  }
}

void CIEBrowserWnd::IPC_OnBrowserReady()
{
	//向自己的UI线程抛任务
  if(m_bNewWindow)
  {
    //向自己抛一个Task 然后wait
    base::MessageLoop::current()->PostTask(FROM_HERE,base::Bind(&CIEBrowserWnd::onGetMarshalData,base::Unretained(this)));
  }
  else if(!m_bCom) {
	  //g_Frame->PostTaskToView(base::Bind(&ISoFrame::OnBrowserReady,base::Unretained(g_Frame),m_TabId));
  }
  else
    OnComURLFinished();
}
void CIEBrowserWnd::IPC_DestoryViewHost()
{
  //向UI线程抛任务
  //g_Frame->PostTaskToView(base::Bind(&ISoFrame::DestoryViewHost,base::Unretained(g_Frame),m_TabId));
}

void CIEBrowserWnd::Navigate(LPCTSTR url)
{
	Navigate(url,NULL);
}
void CIEBrowserWnd::SetBrowserWndRect()
{
  RECT rect={0};
  GetClientRect(m_hWndParent,&rect);
  CopyRect(&m_rcRect,&rect);

  SetWindowPos(m_hWnd,NULL,m_rcRect.left,m_rcRect.top,m_rcRect.right-m_rcRect.left,m_rcRect.bottom-m_rcRect.top,SWP_SHOWWINDOW);
  UpdateBrowserSize();
  SetBrowserFocus();
}
void CIEBrowserWnd::BindBrowser(int tab_id,HWND hParent)
{
  m_TabId = tab_id;
  m_hWndParent = hParent;

  SetParent(m_hWnd,m_hWndParent);

  SetBrowserWndRect();
}

void CIEBrowserWnd::onUrlChange(LPCTSTR pszUrl)
{
  std::wstring szUrl= pszUrl;
  //if(m_szUrl!=szUrl)
  {
    m_szUrl = szUrl;
    //g_Frame->PostTaskToView(base::Bind(&ISoFrame::OnUpdateURL,base::Unretained(g_Frame),m_TabId));
    if(render_view_)
      render_view_->Send(new ViewMsg_URLCHANGE(render_view_->route_id(), rotate_id_, szUrl));
  }
}

void CIEBrowserWnd::OnTitleChange(LPTSTR pszTitle)
{
  std::wstring szTitle = pszTitle;
  if( m_Title != szTitle)
  {
    m_Title = szTitle;
    //g_Frame->PostTaskToView(base::Bind(&ISoFrame::OnUpdateTitle,base::Unretained(g_Frame),m_TabId));
    if(render_view_) {
      render_view_->Send(new ViewMsg_TITLECHANGE(render_view_->route_id(), rotate_id_, szTitle));
    }
  }
}
void CIEBrowserWnd::OnReload(LPCTSTR url)
{
  int size = (_tcslen(url)+1)*2;
  Navigate(url,NULL);
  VirtualFree((LPVOID)url,size,MEM_RELEASE);
}
void CIEBrowserWnd::OnSetBrowserFocus()
{
  SetBrowserFocus();
}
void CIEBrowserWnd::OnRefresh()
{
  m_pWebBrowser->Refresh();
}
void CIEBrowserWnd::OnBack()
{
  if(m_bCanBack)
    m_pWebBrowser->GoBack();
}
void CIEBrowserWnd::OnForward()
{
  if(m_bCanForward)
    m_pWebBrowser->GoForward();
}
void CIEBrowserWnd::onBackStateChange(BOOL bEnable)
{
  if( bEnable == m_bCanBack )
    return;
  m_bCanBack = bEnable;
  //更新状态
}

void CIEBrowserWnd::onForwardStateChange(BOOL bEnable)
{
  if( bEnable == m_bCanForward )
    return;
  m_bCanForward = bEnable;
  //更新状态
}
void CIEBrowserWnd::OnShowWebDebugger(BOOL bShow)
{
  ShowWebDebugger(bShow);
}
void CIEBrowserWnd::OnIECompatView(DWORD dwValue)
{
  CIECompatibilityViewData::GetInstance().AddSelfList("*.798wiki.com",dwValue);
  //Refresh不生效
  Navigate(m_szUrl.c_str());
}

CIEBrowserWnd* CIEBrowserWnd::Current()
{
  //tls保存
  CIEBrowserWnd* pIEBrowser = (CIEBrowserWnd*)TlsGetValue(g_dwTlsBrowser);
  return pIEBrowser;
}
void CIEBrowserWnd::SetIEBrowserToTls()
{
  TlsSetValue(g_dwTlsBrowser,(LPVOID)this);
}
void CIEBrowserWnd::SetPopupWindow(bool bPopup)
{
  m_bPopupWindow = bPopup;
}

void CIEBrowserWnd::Send(IPC::Message* msg) {
  if(msg) {
    if(::IsWindow(m_hMsgWnd)) 
      ::PostMessage(m_hMsgWnd, BROWSER_TO_CHILDTHREAD_IPC_MESSAGE, 0, (LPARAM)msg);
    else
      delete msg;
  }
}

void CIEBrowserWnd::OnComURLFinished()
{
  LPSTREAM pStream = NULL;
  WGMSG::LPWGMARSHALDATA twMarshalData =(WGMSG::LPWGMARSHALDATA) VirtualAlloc(NULL, sizeof(WGMSG::LWGMARSHALDATA), MEM_COMMIT, PAGE_READWRITE);
  memset(twMarshalData, 0, sizeof(WGMSG::LPWGMARSHALDATA));

  HGLOBAL hGlobal = NULL;
  if(S_OK==CreateStreamOnHGlobal(NULL, TRUE, &pStream))
  {
    if(S_OK==CoMarshalInterface(pStream, IID_IDispatch, m_pDispatch, MSHCTX_LOCAL, NULL, MSHLFLAGS_NORMAL))
    {
      if(S_OK==GetHGlobalFromStream(pStream, &hGlobal))
      {
        LPVOID buffer = GlobalLock(hGlobal);
        int size = GlobalSize(hGlobal);
        twMarshalData->lpStream = (LPVOID)VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_READWRITE);
        memcpy(twMarshalData->lpStream,buffer,size);
        twMarshalData->dwStreamSize = size;
      
        GlobalUnlock(hGlobal);
        if(pStream)
          pStream->Release();
        //异步的话  这里如果超时10s执行OnComWindowReady就会发生崩溃  因此这里改为new
        //这里跨模块传递内存 从SoTrident.dll到SoFrame.dll  不能使用new

        //g_Frame->PostTaskToView(base::Bind(&ISoFrame::OnComWindowReady,base::Unretained(g_Frame),m_TabId,(LPVOID)twMarshalData));
      }
    }
  }
}