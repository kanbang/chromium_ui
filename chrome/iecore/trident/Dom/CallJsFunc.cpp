
#include "chrome/iecore/trident/stdafx.h"
#include "CallJsFunc.h"


CWebPage::CWebPage()
{
}

CWebPage::~CWebPage()
{

}

bool CWebPage::GetJScript(IHTMLDocument2* pDoc,CComPtr<IDispatch>& spDisp)
{
	HRESULT hr = pDoc->get_Script(&spDisp);
	ATLASSERT(SUCCEEDED(hr));
	return SUCCEEDED(hr);
}

bool CWebPage::GetJScripts(IHTMLDocument2* pDoc,CComPtr<IHTMLElementCollection>& spColl)
{
	HRESULT hr = pDoc->get_scripts(&spColl);
	ATLASSERT(SUCCEEDED(hr));
	return SUCCEEDED(hr);
}