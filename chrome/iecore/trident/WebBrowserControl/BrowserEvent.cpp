#include "chrome/iecore/trident/stdafx.h"
#include "BrowserEvent.h"
#include "chrome/iecore/trident/webbrowserhost/IEBrowserWnd.h"
#include "WebBrowser.h"
#include <ExDispid.h>

CBrowserEvent::CBrowserEvent()
{
	Clear();
}

CBrowserEvent::~CBrowserEvent()
{
	Clear();
}

void CBrowserEvent::Clear()
{
	m_pIEBrowserWnd = NULL;
	m_pCPContainer = NULL;
	m_pCP = NULL;
	m_dwCookie=0;
}

STDMETHODIMP_(ULONG)CBrowserEvent::AddRef(THIS)
{
	return 1;
}

STDMETHODIMP_(ULONG)CBrowserEvent::Release(THIS)
{
	return 1;
}

STDMETHODIMP CBrowserEvent::QueryInterface(REFIID riid, LPVOID FAR* ppvObj)
{
	*ppvObj = NULL;

	if( IsEqualIID(riid, IID_IUnknown) )
		*ppvObj = this;
	else if( IsEqualIID(riid, IID_IDispatch) )
		*ppvObj = (IDispatch *)this;
	else
	{
		*ppvObj = 0;
		return E_NOINTERFACE;
	}

	return S_OK;
}

STDMETHODIMP CBrowserEvent::GetTypeInfoCount( UINT *pctinfo )
{
	return E_NOINTERFACE;
}

STDMETHODIMP CBrowserEvent::GetTypeInfo( UINT iTInfo, LCID lcid, ITypeInfo **ppTInfo )
{
	return E_NOINTERFACE;
}

STDMETHODIMP CBrowserEvent::GetIDsOfNames( REFIID riid, LPOLESTR *rgszNames, UINT cNames, LCID lcid, DISPID *rgDispId )
{
	return E_NOINTERFACE;
}

HRESULT CBrowserEvent::Connect()
{
	HRESULT hr = S_FALSE;
	if( m_pIEBrowserWnd->GetIWebBrowser2() )
	{
		m_pIEBrowserWnd->GetIWebBrowser2()->QueryInterface( IID_IConnectionPointContainer, (void**)&m_pCPContainer );
		if( m_pCPContainer )
		{
			m_pCPContainer->FindConnectionPoint(DIID_DWebBrowserEvents2, &m_pCP);
			if( m_pCP )
				hr = m_pCP->Advise( this, &m_dwCookie );
		}
	}
	return hr;
}

HRESULT CBrowserEvent::DisConnect()
{
	assert( m_pIEBrowserWnd != NULL );

	HRESULT hr = S_FALSE;
	if( m_pCPContainer )
	{
		if( m_pCP )
		{
			hr = m_pCP->Unadvise( m_dwCookie );
			m_pCP->Release();
		}
		else
		{
			assert( FALSE );
		}
		m_pCPContainer->Release();
	}

	Clear();
	return hr;
}

STDMETHODIMP CBrowserEvent::Invoke( DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, 
	VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr )
{
 
	DWORD dwID = 0;
	assert( m_pIEBrowserWnd );

	if (!pDispParams )
		return E_INVALIDARG;

	switch( dispIdMember )
	{
	case DISPID_SETSECURELOCKICON:
		{
      if(m_pIEBrowserWnd)
        m_pIEBrowserWnd->FindBrowserWnd();

			//https level icon maybe have no use
// 			int nSecurityIcon = pDispParams->rgvarg[0].lVal;
// 			if( nSecurityIcon == secureLockIconMixed )
// 				int k=0;
		}break;
	case DISPID_TITLECHANGE:
		{
			OnTitleChange(pDispParams);
		}break;
	case DISPID_DOCUMENTCOMPLETE:
		{
      OnDocumentComplete(pDispParams);
		}break;
	case DISPID_NEWWINDOW2:
		{

			int k=0;
		}break;
	case DISPID_NEWWINDOW3:
		{
		  OnNewWindow3(pDispParams);
		}break;
	case DISPID_BEFORENAVIGATE2:
		{
			//处理九宫格
			//m_pIEBrowserWnd->onBeforeNavigate(pDispParams, pVarResult);
		}break;
  case DISPID_NAVIGATECOMPLETE2:
    {
      OnNaviagateComplete2(pDispParams);
    }break;
  case DISPID_COMMANDSTATECHANGE:
    {
      OnCommandStateChange(pDispParams);
    }break;
	default:
		return DISP_E_MEMBERNOTFOUND;
	}

	return S_OK;
}


void CBrowserEvent::OnTitleChange(DISPPARAMS *pDispParams)
{
  if( pDispParams->rgvarg[0].bstrVal )
  {
    LPTSTR pszTitle;
    pszTitle = OLE2T(pDispParams->rgvarg[0].bstrVal);
    m_pIEBrowserWnd->OnTitleChange( pszTitle );	
  }
}

void CBrowserEvent::OnNewWindow3(DISPPARAMS *pDispParams)
{
  IDispatch* pDispatch=NULL;
  LPTSTR pszUrl=NULL;
  if(pDispParams->rgvarg[0].vt=VT_BSTR && pDispParams->rgvarg[0].bstrVal)
  {
    USES_CONVERSION;
    pszUrl=W2T(pDispParams->rgvarg[0].bstrVal);
  }
  pDispatch = m_pIEBrowserWnd->OnNewWindow(pszUrl);
  *(pDispParams->rgvarg[4].ppdispVal)=pDispatch;
  *(pDispParams->rgvarg[3].pboolVal)=pDispatch?VARIANT_FALSE:VARIANT_TRUE;
}
void CBrowserEvent::OnNaviagateComplete2(DISPPARAMS *pDispParams)
{
  //判断是否是顶层的WebBrowser
  if( pDispParams->rgvarg[1].pdispVal && pDispParams->rgvarg[1].vt == VT_DISPATCH )
  {
    if( IsTopWebBrowser( pDispParams->rgvarg[1].pdispVal ))
    {

      TCHAR szUrl[MAX_URL_LEN]=_T("");
      if(GetBrowserUrl(szUrl, _ARRAYSIZE(szUrl)-1) && _T('\0')!=szUrl[0])
      {	
        m_pIEBrowserWnd->onUrlChange(szUrl);
      }
    }
  }
}

BOOL CBrowserEvent::IsTopWebBrowser(IDispatch *pDisp)
{
  if( pDisp )
  {
    IWebBrowser2Ptr pWeb = NULL;
    pWeb = pDisp;
    if( pWeb && pWeb == m_pIEBrowserWnd->GetIWebBrowser2())
    {
        return TRUE;
    }
  }
  return FALSE;
}

BOOL CBrowserEvent::GetBrowserUrl(LPTSTR lpszUrl, DWORD dwSize)
{
  BOOL bRet = FALSE;
  BSTR bstrUrl=NULL;
  IWebBrowser2Ptr pWeb = NULL;
  pWeb = m_pIEBrowserWnd->GetIWebBrowser2();
  pWeb->get_LocationURL( &bstrUrl );
  if( bstrUrl && bstrUrl[0] )
  {
    USES_CONVERSION;
    _tcsncpy_s(lpszUrl, dwSize, OLE2T(bstrUrl), _TRUNCATE);
    SysFreeString( bstrUrl );
    bRet = TRUE;
  }
  return bRet;
}
void CBrowserEvent::OnDocumentComplete(DISPPARAMS *pDispParams)
{
  m_pIEBrowserWnd->OnDocumentComplete();
}
void CBrowserEvent::OnCommandStateChange(DISPPARAMS *pDispParams)
{
  long lCommand = pDispParams->rgvarg[1].lVal;
  BOOL bEnable = pDispParams->rgvarg[0].boolVal;

  if(lCommand == CSC_NAVIGATEBACK) 
    m_pIEBrowserWnd->onBackStateChange(bEnable);
  else if(lCommand == CSC_NAVIGATEFORWARD)
    m_pIEBrowserWnd->onForwardStateChange(bEnable);
}