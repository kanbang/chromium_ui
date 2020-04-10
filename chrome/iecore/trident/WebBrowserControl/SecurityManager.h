#pragma once

#include <urlmon.h>
class CIEBrowserWnd;
class CSecurityManager : public IInternetSecurityManager,
						 public IHttpSecurity  
{
public:
	CSecurityManager()
	{
		m_pIEBrowserWnd=NULL;
	}
	virtual ~CSecurityManager()
	{
		m_pIEBrowserWnd=NULL;
	}

	//	IUnKnown
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);
	//	IInternetSecurityManager
	STDMETHOD(GetSecurityId)( LPCWSTR pwszUrl,BYTE *pbSecurityId,DWORD *pcbSecurityId,DWORD_PTR dwReserved){return INET_E_DEFAULT_ACTION;};
	STDMETHOD(GetSecuritySite)( IInternetSecurityMgrSite **ppSite){return INET_E_DEFAULT_ACTION;};
	STDMETHOD(GetZoneMappings)( DWORD dwZone,IEnumString **ppenumString,DWORD dwFlags){return INET_E_DEFAULT_ACTION;};
	STDMETHOD(MapUrlToZone)( LPCWSTR pwszUrl,DWORD *pdwZone,DWORD dwFlags){return INET_E_DEFAULT_ACTION;};
	STDMETHOD(ProcessUrlAction)(LPCWSTR pwszUrl,DWORD dwAction,BYTE *pPolicy,DWORD cbPolicy,BYTE *pContext,DWORD cbContext,DWORD dwFlags,DWORD dwReserved);
	STDMETHOD(QueryCustomPolicy)( LPCWSTR pwszUrl,REFGUID guidKey,BYTE **ppPolicy,DWORD *pcbPolicy,BYTE *pContext,DWORD cbContext,DWORD dwReserved){return INET_E_DEFAULT_ACTION;};
	STDMETHOD(SetSecuritySite)( IInternetSecurityMgrSite *pSite){return INET_E_DEFAULT_ACTION;};
	STDMETHOD(SetZoneMapping)(  DWORD dwZone,LPCWSTR lpszPattern,DWORD dwFlags){return INET_E_DEFAULT_ACTION;};
	//	IHttpSecurity
	STDMETHOD(GetWindow)(  REFGUID rguidReason,HWND *phwnd);
	STDMETHOD(OnSecurityProblem)( DWORD dwProblem);
public:
	void SetIEBrowserWnd( CIEBrowserWnd* pIEBrowserWnd ){ m_pIEBrowserWnd = pIEBrowserWnd;};

private:
	CIEBrowserWnd*	m_pIEBrowserWnd;
};
