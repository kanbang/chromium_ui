#include "chrome/iecore/trident/stdafx.h"
#include "XWnd.h"

CRITICAL_SECTION XWndProcThunk::m_csWindowCreate;
XWndProcThunk::XWndCreateWndData* XWndProcThunk::m_pCreateWndList;

XWnd::XWnd()
{
	m_hWnd = NULL;
	m_dwThreadID = 0;
}

XWnd::~XWnd()
{

}

void XWnd::InitWindowClass( LPCTSTR lpszName, DWORD dwStyle, HBRUSH hbrBackground)
{
	WNDCLASSEX wcex;
	memset( &wcex, 0, sizeof(wcex) );
	wcex.cbSize = sizeof(WNDCLASSEX); 
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.lpszMenuName	= (LPCTSTR)NULL;
	wcex.hbrBackground	= hbrBackground;
	wcex.style			= dwStyle;
	wcex.hIcon			= NULL;
	wcex.hIconSm		= NULL;

	wcex.lpszClassName	= lpszName;
	wcex.lpfnWndProc	= (WNDPROC)XWnd::StartWindowProc;
	ATOM atom = RegisterClassEx(&wcex); // Don't care register failed, only care create window failed.
}

void XWnd::UnInitWindowClass( LPCTSTR lpszName )
{
	::UnregisterClass( lpszName, NULL );
}

HWND XWnd::Create( DWORD dwStyleEx, LPCTSTR pszClassName, LPCTSTR pszTitle, DWORD dwStyle, 
				  LPRECT lpRect, HWND hParent, DWORD wID )
{
	RECT rect={0};
	if( NULL == lpRect)
		lpRect = &rect;
	Init();
	InitWindowClass(pszClassName, CS_DBLCLKS|CS_BYTEALIGNWINDOW,(HBRUSH)(COLOR_WINDOW+1));
	m_dwThreadID = ::GetCurrentThreadId();

	m_thunk.XWndThunkAddCreateWndData( ReflectWndProc, this );
	CreateWindowEx( dwStyleEx, pszClassName, pszTitle, dwStyle, 
		lpRect->left, lpRect->top, lpRect->right-lpRect->left, lpRect->bottom-lpRect->top, 
		hParent, (HMENU)wID, NULL, this );
	
	return m_hWnd;
}

LRESULT __stdcall XWnd::WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	return DefWindowProc(hWnd, message, wParam, lParam);
}

LRESULT __stdcall XWnd::StartWindowProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	XWnd* pThis = (XWnd*)XWndProcThunk::XWndThunkExtractCreateWndData();
	pThis->m_hWnd = hWnd;
	pThis->m_thunk.Init( ReflectWndProc, pThis );
	WNDPROC pProc = (WNDPROC)(pThis->m_thunk.pThunk);
	WNDPROC pOldProc = (WNDPROC)::SetWindowLong(hWnd, GWL_WNDPROC, (LONG)pProc);
	return pProc( hWnd, message, wParam, lParam );
}

LRESULT __stdcall XWnd::ReflectWndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	XWnd * pThis = (XWnd *)hWnd;
	hWnd = pThis->m_hWnd;
	return pThis->WndProc(hWnd, message, wParam, lParam);
}

