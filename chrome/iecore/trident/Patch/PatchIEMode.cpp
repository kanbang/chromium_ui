
#include "chrome/iecore/trident/stdafx.h"
#include "chrome/iecore/trident/preheader.h"
#include "PatchIEMode.h"
#include <mshtml.h>

namespace PatchIEMode{

	void SetHtmlDocumentMode(IWebBrowser2* pWebBrowser,DWORD dwMode)
	{
		if( !pWebBrowser)
			return;
		CComQIPtr<IHTMLDocument2> pDoc;
		CComQIPtr<IDispatch> pDisp;
		pWebBrowser->get_Document(&pDisp);
		if(pDisp)
		{
			HRESULT hr = pDisp->QueryInterface(IID_IHTMLDocument2,(void**)&pDoc);
			if( pDoc)
			{
				CComQIPtr<IOleCommandTarget> spCT;
				hr = pDoc->QueryInterface(IID_IOleCommandTarget, (void**)&spCT);
				if( spCT)
				{
					const  CLSID CLSID_MSHTMLPriv = {0xa11452bc,0xe055,0x4e56,{0xa1,0x51,0x7b,0x16,0xdb,0xb4,0x54,0x4e}};
					//第一次Exec
					VARIANTARG pvarg;
					VariantInit(&pvarg);
					pvarg.vt = 3;
					pvarg.lVal = dwMode;
					hr = spCT->Exec( &CLSID_MSHTMLPriv, 2, 0, &pvarg, 0 );
					if( SUCCEEDED(hr))
					{
						//第二次Exec
						int v6;
						VariantInit((VARIANTARG *)&v6);
						v6 = 11;
						hr = spCT->Exec( &CLSID_MSHTMLPriv, 19, 0, (VARIANTARG *)&v6, 0 );
						if( SUCCEEDED(hr))
							//第三次Exec
							hr = spCT->Exec( &CLSID_MSHTMLPriv, 9, 0, 0, 0 );
					}
				}
			}

		}
	}
	void SetUserAgentMode(DWORD dwMode)
	{
		static HRESULT (__stdcall* CoInternetSetBrowserUserAgentMode)( int ) = NULL;
		if( NULL==CoInternetSetBrowserUserAgentMode  )
			CoInternetSetBrowserUserAgentMode = (HRESULT (__stdcall*)( int ))GetProcAddress(GetModuleHandle(_T("urlmon.dll")), MAKEINTRESOURCEA(447));
		if( CoInternetSetBrowserUserAgentMode )
			CoInternetSetBrowserUserAgentMode(dwMode);
	}
}