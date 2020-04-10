#pragma once
#include "chrome/iecore/trident/preheader.h"
#include "IDownloadManager.h"
class CIEBrowserWnd;

class CDownloadManager : public IDownloadManager
{
public:
	CDownloadManager();
	virtual ~CDownloadManager();

	//	IUnKnown
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);
	//	IDownloadManager
	STDMETHOD(Download)( IMoniker *pmk, IBindCtx *pbc, DWORD dwBindVerb, LONG grfBINDF, BINDINFO *pBindInfo, LPCOLESTR pszHeaders, LPCOLESTR pszRedir, UINT uiCP ){return E_NOTIMPL;};
public:
	void SetIEBrowserWnd( CIEBrowserWnd* pIEBrowserWnd ){ m_pIEBrowserWnd = pIEBrowserWnd;};

private:
	CIEBrowserWnd*	m_pIEBrowserWnd;
};
