// ExternalObj.cpp: implementation of the CExternalObj class.
//
//////////////////////////////////////////////////////////////////////
#include "chrome/iecore/trident/stdafx.h"
#include "ExternalObj.h"
#include "chrome/iecore/trident/webbrowserhost/IEBrowserWnd.h"

#define	DISPID_DHTMLEXTEND_NOINTERFACE		    0   // 所有不支持的方法都返0
#define DISPID_DHTMLEXTEND_CALLAPI				1	// 传输播放ID
#define DISPID_DHTMLEXTEND_AutoCompleteSaveForm				2	// 传输播放ID

typedef struct strInvkName
{
	WCHAR	wszName[MAX_PATH];
	LONG	DispId;
}InvkName;

InvkName stName[] = { L"CALL_API",				DISPID_DHTMLEXTEND_CALLAPI,
					  L"AutoCompleteSaveForm",  DISPID_DHTMLEXTEND_AutoCompleteSaveForm,
};
//////////////////////////////////////////////////////////////////////////

CExternalObj::CExternalObj()
{
	m_pIEBrowserWnd = NULL;
}

CExternalObj::~CExternalObj()
{

}

STDMETHODIMP_(ULONG)CExternalObj::AddRef(THIS)
{
	return 1;
}

STDMETHODIMP_(ULONG)CExternalObj::Release(THIS)
{
	return 1;
}

STDMETHODIMP CExternalObj::QueryInterface(REFIID riid, LPVOID FAR* ppvObj)
{
	*ppvObj = NULL;
	
	if( IsEqualIID(riid, IID_IUnknown) )
		*ppvObj = this;
	else if( IsEqualIID(riid, IID_IDispatch) )
		*ppvObj = (IDispatch *)this;
// 	else if( IsEqualIID(riid, IID_IDispatchEx) )
// 		*ppvObj = (IDispatchEx *)this;
	else
	{
		*ppvObj = 0;
		return E_NOINTERFACE;
	}
	
	return S_OK;
}

STDMETHODIMP CExternalObj::GetTypeInfoCount( UINT *pctinfo )
{
	return E_NOINTERFACE;
}

STDMETHODIMP CExternalObj::GetTypeInfo( UINT iTInfo, LCID lcid, ITypeInfo **ppTInfo )
{
	return E_NOINTERFACE;
}

STDMETHODIMP CExternalObj::GetIDsOfNames( REFIID riid, LPOLESTR *rgszNames, UINT cNames, LCID lcid, DISPID *rgDispId )
{
	int i;
	int nCount = sizeof(stName)/sizeof(InvkName);
	for( i=0; i<nCount; i++ )
	{
		if( _wcsicmp(rgszNames[0], stName[i].wszName) == 0 )
		{
			rgDispId[0] = stName[i].DispId;
			return S_OK;
		}
	}

	rgDispId[0] = DISPID_DHTMLEXTEND_NOINTERFACE;
	return DISP_E_UNKNOWNNAME;
}

STDMETHODIMP CExternalObj::Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pvarResult, EXCEPINFO *pexcepinfo, UINT *puArgErr)
{
	return InvokeInner(dispidMember, pDispParams, pvarResult);
}
HRESULT CExternalObj::InvokeInner(DISPID dispidMember, DISPPARAMS *pDispParams, VARIANT *pvarResult)
{
	USES_CONVERSION;
	HRESULT hr = S_OK;
	if( 0 == dispidMember )
		return hr;

	switch( dispidMember )
	{
	case DISPID_DHTMLEXTEND_CALLAPI:
		{
			if ( 2 == pDispParams->cArgs 
				&& VT_BSTR == pDispParams->rgvarg[0].vt && NULL != pDispParams->rgvarg[0].bstrVal
				&& VT_BSTR == pDispParams->rgvarg[1].vt && NULL != pDispParams->rgvarg[1].bstrVal )
			{
				LPCTSTR lpszFunc = pDispParams->rgvarg[1].bstrVal;
				LPCTSTR lpszParam = pDispParams->rgvarg[0].bstrVal;
			}
		}
		break;   
	case DISPID_DHTMLEXTEND_AutoCompleteSaveForm:
		return S_OK;
	}

	return hr;
}


HRESULT CExternalObj::GetDispID( BSTR bstrName, DWORD grfdex, DISPID *pid)
{
	int i;
	int nCount = sizeof(stName)/sizeof(InvkName);
	for( i=0; i<nCount; i++ )
	{
		if( _wcsicmp(bstrName, stName[i].wszName) == 0 )
		{
			pid[0] = stName[i].DispId;
			return S_OK;
		}
	}

	pid[0] = DISPID_DHTMLEXTEND_NOINTERFACE;
	return DISP_E_UNKNOWNNAME;
}

HRESULT CExternalObj::InvokeEx( DISPID id, LCID lcid,WORD wFlags,DISPPARAMS *pdp,VARIANT *pvarRes,EXCEPINFO *pei,IServiceProvider *pspCaller )
{
	if( id==0)
		return E_NOTIMPL;
	return InvokeInner(id, pdp, pvarRes);
}