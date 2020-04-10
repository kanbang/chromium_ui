#pragma once
#include "chrome/iecore/trident/preheader.h"

class CIEBrowserWnd;
class CBrowserEvent : 
  public IDispatch
{
public:
	CBrowserEvent();
	~CBrowserEvent();

	// IUnKnown
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	// IDispatch
	STDMETHOD(GetTypeInfoCount)( UINT *pctinfo );
	STDMETHOD(GetTypeInfo)( UINT iTInfo, LCID lcid, ITypeInfo **ppTInfo );
	STDMETHOD(GetIDsOfNames)( REFIID riid, LPOLESTR *rgszNames, UINT cNames, LCID lcid, DISPID *rgDispId );
	STDMETHOD(Invoke)( DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr );

public:
	void SetIEBrowserWnd( CIEBrowserWnd* pIEBrowserWnd ){ m_pIEBrowserWnd = pIEBrowserWnd;};
	HRESULT Connect();
	HRESULT DisConnect();
  //
  void OnNewWindow3(DISPPARAMS *pDispParams);
  void OnBeforeNavigate2(DISPPARAMS *pDispParams);
  void OnNaviagateComplete2(DISPPARAMS *pDispParams);
  void OnDocumentComplete(DISPPARAMS *pDispParams);
  void OnTitleChange(DISPPARAMS *pDispParams);
  void OnCommandStateChange(DISPPARAMS *pDispParams);
protected:
	void Clear();
  BOOL IsTopWebBrowser(IDispatch *pDisp);
  BOOL GetBrowserUrl(LPTSTR lpszUrl, DWORD dwSize);
private:
	IConnectionPointContainer *m_pCPContainer;
	IConnectionPoint *m_pCP;
	DWORD m_dwCookie;
	CIEBrowserWnd*	m_pIEBrowserWnd;
};
