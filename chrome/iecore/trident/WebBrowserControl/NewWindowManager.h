#pragma once

#include <ShObjIdl.h>
class CIEBrowserWnd;
class CNewWindowManager : public INewWindowManager
{
public:
	CNewWindowManager();
	virtual ~CNewWindowManager();

	//	IUnKnown
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);
	//	INewWindowManager
	STDMETHOD(EvaluateNewWindow)( LPCWSTR pszUrl,LPCWSTR pszName,LPCWSTR pszUrlContext,LPCWSTR pszFeatures,BOOL fReplace,DWORD dwFlags,DWORD dwUserActionTime);
public:
	void SetIEBrowserWnd( CIEBrowserWnd* pIEBrowserWnd ){ m_pIEBrowserWnd = pIEBrowserWnd;};

private:
	CIEBrowserWnd*	m_pIEBrowserWnd;
};
