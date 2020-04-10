// IEBrowser.h: interface for the CIEBrowserWnd class.

#pragma once
#include "XWnd.h"
#include <ObjBase.h>
#include <ExDisp.h>
#include <string>
#include "base/memory/scoped_ptr.h"
#include "base/threading/thread.h"
#include "chrome/iecore/trident/Patch/F12DevTool.h"
#include "base/message_loop/message_loop.h"
#include "ipc/ipc_message.h"
#include "chrome/iecore/trident/WebbrowserControl/WebBrowser.h"

#define WNDPROC_MINIBROWSERWNDPROC _T("Mini_ChromeBrowserWndProc")

struct ViewMsg_Resize_Params;
struct ViewMsg_Navigate_Params;

class RenderView;
class CIEBrowserWnd:
	public XWnd
{
public:
	CIEBrowserWnd();
	virtual ~CIEBrowserWnd();

	static CIEBrowserWnd* CreateBrowserThread(HWND hWndParent ,HWND hWndMsg,
    bool bNewWinodw,int rotaeid, RenderView* render_view);
  static CIEBrowserWnd* Current();
	//ISoIEWnd
	base::MessageLoop* GetMessageLoop(){return browser_message_loop_;};
	void Navigate(LPCTSTR  url);
	void SetBrowserWndRect();
	void DestoryTabPage();
	void BindBrowser(int tab_id,HWND hParent);
	LPCTSTR GetURL(){return m_szUrl.c_str();};
	LPCTSTR GetTitle(){return m_Title.c_str();};
	void OnReload(LPCTSTR url);
  void OnSetBrowserFocus();
  void OnRefresh();
  void OnBack();
  void OnForward();
  void OnShowWebDebugger(BOOL bShow);
  void OnIECompatView(DWORD dwValue);

	void Navigate( LPCTSTR pszUrlPage, LPCTSTR lpszRefererUrl );

	HWND GetWindow(){return m_hWnd;}
	IWebBrowser2* GetIWebBrowser2(){return m_pWebBrowser;}
	void onInitBrowser();
	
	bool ProcessMessage(MSG  msg);
	IDispatch* OnNewWindow( LPCTSTR lpszUrl);
  void onUrlChange(LPCTSTR szUrl);
  void OnTitleChange(LPTSTR pszTitle);
  void OnDocumentComplete();
	bool IsFinishOK(){return m_bCreateFinish;};
	void OnCreate();
  void onBackStateChange(BOOL bEnable);
  void onForwardStateChange(BOOL bEnable);
  void SetPopupWindow(bool bPopup);
  void Send(IPC::Message* msg);
  HWND FindBrowserWnd();

private:
  WNDPROC		m_procIEBrowser;
  static LRESULT CALLBACK IEBrowserWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	static unsigned int __stdcall BrowserThreadProc(void * p);
	void RegisterBrowserThread();
	static void BrowserInitialize();
	static void SetProcessFeatureControl();
	BOOL DoCreate();
	void ConnetEvent();
	void SetBrowserParentWnd(HWND hWnd);
	void CreateContainerWnd();
  void UpdateBrowserSize();
	LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

	void BrowserMessageLoop();
	void SetBrowserFocus();
	void DestroyWebBrowser(); //这个函数关闭的时候 在不同版本IE可能会崩溃
  void OnBrowserReady();

	void UIActivate();
	void OnAccelerator( DWORD identifier );
	void ShowWebDebugger( BOOL bShow );

	void onBeforeNavigate(DISPPARAMS *pDispParams, VARIANT *pVarResult);
	HRESULT GetInterfaceFromStream(REFIID riid, LPVOID lpStream, DWORD dwSize, LPVOID *ppObject);
	void onGetMarshalData();
	bool IsNineTab();
	void SafeReleaseGesture();
	BOOL Fire_onbeforeunload();
  //Tls
  void SetIEBrowserToTls();
  //IPC
  void IPC_DestoryViewHost();
  void IPC_OnBrowserReady();
  void OnComURLFinished();
  bool HandledRedirectIPCMsg(UINT msg, WPARAM wparam, LPARAM lparam);
  bool OnIPCMsg(const IPC::Message& msg);
  void OnResizeMsg(ViewMsg_Resize_Params params);
  void NavigateParams(ViewMsg_Navigate_Params params);
  void OnActiveMsg(bool active);
  void OnSetFocusMsg(bool enable);

protected:
	IStorage*			m_pStorage;
	IOleClientSite*		m_pClientSite;
	IOleObject*			m_pOleObject; //页面

	IOleInPlaceActiveObject* m_pOIPA;  //页面Activex对象，快捷键
	IWebBrowser2*		m_pWebBrowser; //Navigate使用
	IDispatch*			m_pDispatch;   //OnNewWindow使用

	CWebBrowser			m_oWebBrowser;

	vIEDevToolStub*     m_pDevTool;
	int m_TabId;
	HWND				m_hWndParent;
	HWND        m_hBrowserWnd;
	RECT				m_rcRect;
  HWND        m_hMsgWnd;
  int rotate_id_;
  bool m_didFocus;

	std::wstring        m_Title;
	std::wstring        m_szUrl;
	std::wstring        m_szNineTabUrl;
	HANDLE m_hNewWindowEvent;
	HANDLE m_hNewWindowOK;
	//
	scoped_ptr<base::Thread> browser_thread_;
	base::MessageLoop* browser_message_loop_;
	//鼠标手势
	bool m_bSpecialGesrure;
	//PROC_RouteMessageToParent m_FuncRouteMessageToParent;
  bool m_bNewWindow;
  bool m_bCreateFinish;
  BOOL m_bCanBack;
  BOOL m_bCanForward;
  bool m_bPopupWindow;
  bool m_bCom;

  RenderView* render_view_;
};


class RenderUIMessageFilter:public base::MessageLoopForUI::MessageFilter
{
public:
	RenderUIMessageFilter(CIEBrowserWnd* pThis)
	{
		m_pBrowser = pThis;
	}
	~RenderUIMessageFilter()
	{
		m_pBrowser = NULL;
	}
	bool ProcessMessage(const MSG& msg)
	{
		return m_pBrowser->ProcessMessage(msg);
	}

	CIEBrowserWnd* m_pBrowser;
};