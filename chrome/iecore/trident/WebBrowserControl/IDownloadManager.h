#ifndef __IDownloadManger__
#define __IDownloadManger__

#pragma once 
typedef interface IDownloadManager IDownloadManager;
typedef IDownloadManager *LPIDOWNLOADMANAGER;

DEFINE_GUID(IID_IDownloadManager,
0x988934A4, 0x064B, 0x11D3, 0xBB, 0x80, 0x00, 0x10, 0x4B, 0x35, 0xE7, 0xF9);
EXTERN_C const IID IID_IDownloadManager;

MIDL_INTERFACE("988934A4-064B-11D3-BB80-00104B35E7F9")
IDownloadManager : public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE Download(
			IMoniker *pmk, // Identifies the object to be downloaded
			IBindCtx *pbc, // Stores information used by the moniker to bind
			DWORD dwBindVerb, // The action to be performed during the bind
			LONG grfBINDF, // Determines the use of URL encoding during the bind
			BINDINFO *pBindInfo, // Used to implement IBindStatusCallback::GetBindInfo
			LPCOLESTR pszHeaders, // Additional headers to use with IHttpNegotiate
			LPCOLESTR pszRedir, // The URL that the moniker is redirected to
			UINT uiCP // The code page of the object's display name
			) = 0;
};

#endif