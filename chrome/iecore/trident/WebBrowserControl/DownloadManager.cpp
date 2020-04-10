// DocHostUIHandler.cpp: implementation of the CWebBrowserEx class.
//
//////////////////////////////////////////////////////////////////////

#include "chrome/iecore/trident/stdafx.h"
#include "DownloadManager.h"
#include "chrome/iecore/trident/webbrowserhost/IEBrowserWnd.h"

CDownloadManager::CDownloadManager()
{

}

CDownloadManager::~CDownloadManager()
{

}

STDMETHODIMP_(ULONG)CDownloadManager::AddRef(THIS)
{
	return 1;
}

STDMETHODIMP_(ULONG)CDownloadManager::Release(THIS)
{
	return 1;
}

STDMETHODIMP CDownloadManager::QueryInterface(REFIID riid, LPVOID FAR* ppvObj)
{
	*ppvObj = NULL;

	if( IsEqualIID(riid, IID_IUnknown) )
		*ppvObj = this;
	else if( IsEqualIID(riid, IID_IDownloadManager) )
		*ppvObj = (IDownloadManager *)this;
	else
	{
		*ppvObj = 0;
		return E_NOINTERFACE;
	}

	return S_OK;	
}
