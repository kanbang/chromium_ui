// DocHostUIHandler.cpp: implementation of the CWebBrowserEx class.
//
//////////////////////////////////////////////////////////////////////

#include "chrome/iecore/trident/stdafx.h"
#include "SecurityManager.h"
#include "chrome/iecore/trident/webbrowserhost/IEBrowserWnd.h"

STDMETHODIMP_(ULONG)CSecurityManager::AddRef(THIS)
{
	return 1;
}

STDMETHODIMP_(ULONG)CSecurityManager::Release(THIS)
{
	return 1;
}

STDMETHODIMP CSecurityManager::QueryInterface(REFIID riid, LPVOID FAR* ppvObj)
{
	*ppvObj = NULL;

	if( IsEqualIID(riid, IID_IUnknown) )
		*ppvObj = this;
	else if( IsEqualIID(riid, IID_IInternetSecurityManager) )
		*ppvObj = (IInternetSecurityManager *)this;
	else if( IsEqualIID(riid, IID_IHttpSecurity) )
		*ppvObj = (IHttpSecurity *)this;
	else
	{
		*ppvObj = 0;
		return E_NOINTERFACE;
	}

	return S_OK;	
}


STDMETHODIMP CSecurityManager::ProcessUrlAction(LPCWSTR pwszUrl,DWORD dwAction,BYTE *pPolicy,DWORD cbPolicy,BYTE *pContext,DWORD cbContext,DWORD dwFlags,DWORD dwReserved)
{	
  //URLACTION参考：http://msdn.microsoft.com/en-us/library/ms537178.aspx
	//URLACTION_HTML_MIXED_CONTENT 混合内容显示
	//URLACTION_SCRIPT_PASTE  允许对剪贴板进行编程访问
  //URLACTION_AUTOMATIC_ACTIVEX_UI:Determines whether to display the Information Bar for ActiveX control installations rather than the ActiveX control prompt. 安装Activex弹出Inforamation Bar
  //bug:http://pap.cpipec.com 无法安装Activex控件。因为安装控件会触发Navigate事件，然后页面就会跳转到登陆页
  //这里不作处理的话，会通过CSecurityManager::ProcessUrlActionEx2Internal这个接口去读取注册表中的FeatureControl的设置
	if(dwAction == URLACTION_HTML_MIXED_CONTENT||dwAction==URLACTION_SCRIPT_PASTE ||dwAction==URLACTION_AUTOMATIC_ACTIVEX_UI)
	{
		if(pwszUrl && pPolicy && (cbPolicy >= sizeof(DWORD)))
		{
			*(DWORD*)pPolicy = URLPOLICY_ALLOW;
			return S_OK;
		}
	}
	return INET_E_DEFAULT_ACTION;
}


STDMETHODIMP CSecurityManager::GetWindow(REFGUID rguidReason,HWND *phwnd)
{
		*phwnd = m_pIEBrowserWnd->m_hWnd;
		return S_OK;
}

STDMETHODIMP CSecurityManager::OnSecurityProblem(DWORD dwProblem)
{
// 	switch (dwProblem) {
// 	case ERROR_HTTP_REDIRECT_NEEDS_CONFIRMATION:
// 	case ERROR_INTERNET_CLIENT_AUTH_CERT_NEEDED:
// 		return S_FALSE;
// 	case ERROR_INTERNET_SEC_CERT_REV_FAILED:
// 		return S_OK;
// 	case ERROR_INTERNET_SEC_CERT_ERRORS:
// 	case ERROR_INTERNET_INVALID_CA:
// 	case ERROR_INTERNET_SEC_CERT_CN_INVALID:
// 	case ERROR_INTERNET_SEC_CERT_DATE_INVALID:
// 	case ERROR_INTERNET_SEC_CERT_NO_REV:
// 		return S_OK;
// 	}
	return S_FALSE;
}