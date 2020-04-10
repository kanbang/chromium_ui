// DocHostUIHandler.cpp: implementation of the CWebBrowserEx class.
//
//////////////////////////////////////////////////////////////////////

#include "chrome/iecore/trident/stdafx.h"
#include "NewWindowManager.h"
#include "chrome/iecore/trident/webbrowserhost/IEBrowserWnd.h"

CNewWindowManager::CNewWindowManager()
{
	m_pIEBrowserWnd=NULL;
}

CNewWindowManager::~CNewWindowManager()
{
	m_pIEBrowserWnd=NULL;
}

STDMETHODIMP_(ULONG)CNewWindowManager::AddRef(THIS)
{
	return 1;
}

STDMETHODIMP_(ULONG)CNewWindowManager::Release(THIS)
{
	return 1;
}

STDMETHODIMP CNewWindowManager::QueryInterface(REFIID riid, LPVOID FAR* ppvObj)
{
	*ppvObj = NULL;

	if( IsEqualIID(riid, IID_IUnknown) )
		*ppvObj = this;
	else if( IsEqualIID(riid, IID_INewWindowManager) )
		*ppvObj = (INewWindowManager *)this;
	else
	{
		*ppvObj = 0;
		return E_NOINTERFACE;
	}

	return S_OK;	
}


STDMETHODIMP CNewWindowManager::EvaluateNewWindow( LPCWSTR pszUrl,LPCWSTR pszName,LPCWSTR pszUrlContext,LPCWSTR pszFeatures,BOOL fReplace,DWORD dwFlags,DWORD dwUserActionTime)
{
  return E_NOTIMPL;
}