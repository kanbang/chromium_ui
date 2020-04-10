// ExternalObj.h: interface for the CExternalObj class.

#pragma once
#include <dispex.h>
class CIEBrowserWnd;
class CExternalObj : public IDispatchEx
{
public:
	CExternalObj();
	virtual ~CExternalObj();

public:
	////////////////////////////////////////////////////////////////////
	// IUnKnown
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);
	////////////////////////////////////////////////////////////////////
	// IDispatch
	STDMETHOD(GetTypeInfoCount)( UINT *pctinfo );
	STDMETHOD(GetTypeInfo)( UINT iTInfo, LCID lcid, ITypeInfo **ppTInfo );
	STDMETHOD(GetIDsOfNames)( REFIID riid, LPOLESTR *rgszNames, UINT cNames, LCID lcid, DISPID *rgDispId );
    STDMETHOD(Invoke)( DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr );
	//
	// IDispatchEx
	STDMETHOD(GetDispID)( BSTR bstrName, DWORD grfdex, DISPID *pid);
	STDMETHOD(InvokeEx)(DISPID id, LCID lcid,WORD wFlags,DISPPARAMS *pdp,VARIANT *pvarRes,EXCEPINFO *pei,IServiceProvider *pspCaller);
	STDMETHOD(DeleteMemberByName)( BSTR bstrName,DWORD grfdex){ return E_NOTIMPL;};
	STDMETHOD(DeleteMemberByDispID)( DISPID id ){ return E_NOTIMPL;};
	STDMETHOD(GetMemberProperties)(DISPID id,DWORD grfdexFetch,DWORD *pgrfdex){ return E_NOTIMPL;};
	STDMETHOD(GetMemberName)( DISPID id, BSTR *pbstrName){ return E_NOTIMPL;};
	STDMETHOD(GetNextDispID)( DWORD grfdex, DISPID id,DISPID *pid){ return E_NOTIMPL;};
	STDMETHOD(GetNameSpaceParent)(IUnknown **ppunk){ return E_NOTIMPL;};
public:
	HRESULT InvokeInner(DISPID dispIdMember, DISPPARAMS *pDispParams, VARIANT *pVarResult);
	void SetIEBrowserWnd( CIEBrowserWnd* pIEBrowserWnd ){ m_pIEBrowserWnd = pIEBrowserWnd;};

private:
	CIEBrowserWnd*	m_pIEBrowserWnd;
};
