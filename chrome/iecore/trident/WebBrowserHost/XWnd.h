// XWnd.h: interface for the XWnd class.
//
//////////////////////////////////////////////////////////////////////
#pragma once
#include <windows.h>

class XWndProcThunk
{
public:
	#pragma pack(push,1)
	struct _WndProcThunk
	{
		DWORD   m_mov;          // mov dword ptr [esp+0x4], pThis (esp+0x4 is hWnd)
		DWORD   m_this;         //
		BYTE    m_jmp;          // jmp WndProc
		DWORD   m_relproc;      // relative jmp
	};
	#pragma pack(pop)
	struct XWndCreateWndData
	{
		void* m_pThis;
		DWORD m_dwThreadID;
		WNDPROC m_pProc;
		XWndCreateWndData* m_pNext;
	};
	_WndProcThunk* pThunk;

	XWndProcThunk()
	{
		pThunk = NULL;
	};
	virtual ~XWndProcThunk()
	{
		if( pThunk )
		{
			VirtualFree(pThunk, 0, MEM_RELEASE);
		}
	};
	XWndCreateWndData m_data; 
	static XWndCreateWndData* m_pCreateWndList;
	static CRITICAL_SECTION m_csWindowCreate;
	void Init( WNDPROC proc, void* pThis)
	{
		pThunk = (_WndProcThunk*)VirtualAlloc(NULL, sizeof(_WndProcThunk), MEM_COMMIT|MEM_RESERVE, PAGE_EXECUTE_READWRITE);
		pThunk->m_mov = 0x042444C7;  //C7 44 24 0C
		pThunk->m_this = (DWORD)pThis;
		pThunk->m_jmp = 0xe9;
		pThunk->m_relproc = (int)m_data.m_pProc - ((int)pThunk+sizeof(_WndProcThunk));

		// write block from data cache and
		//  flush from instruction cache
		FlushInstructionCache(GetCurrentProcess(), pThunk, sizeof(_WndProcThunk));
	};
	inline void __stdcall XWndThunkAddCreateWndData( WNDPROC pProc, void* pObject )
	{
		m_data.m_pThis = pObject;
		m_data.m_dwThreadID = ::GetCurrentThreadId();
		m_data.m_pProc = pProc;
		::EnterCriticalSection(&m_csWindowCreate);
		m_data.m_pNext = m_pCreateWndList;
		m_pCreateWndList = &m_data;
		::LeaveCriticalSection(&m_csWindowCreate);
	}
	static inline void* __stdcall XWndThunkExtractCreateWndData()
	{
		void* pv = NULL;
		::EnterCriticalSection(&m_csWindowCreate);
		XWndCreateWndData* pEntry = m_pCreateWndList;
		if(pEntry != NULL)
		{
			DWORD dwThreadID = ::GetCurrentThreadId();
			XWndCreateWndData* pPrev = NULL;
			while(pEntry != NULL)
			{
				if(pEntry->m_dwThreadID == dwThreadID)
				{
					if(pPrev == NULL)
						m_pCreateWndList = pEntry->m_pNext;
					else
						pPrev->m_pNext = pEntry->m_pNext;
					pv = pEntry->m_pThis;
					break;
				}
				pPrev = pEntry;
				pEntry = pEntry->m_pNext;
			}
		}
		::LeaveCriticalSection(&m_csWindowCreate);
		return pv;
	}
};


/////////////////////////////////////////////////////////////////////////////
class XWnd  
{
public:
	XWnd();
	virtual ~XWnd();

	operator HWND(){ return m_hWnd; };
	HWND Create( DWORD dwStyleEx, LPCTSTR pszClassName, LPCTSTR pszTitle, DWORD dwStyle, LPRECT lpRect, HWND hParent, DWORD wID );
public:
	HWND m_hWnd;

protected:
	static void InitWindowClass( LPCTSTR lpszName, DWORD dwStyle, HBRUSH hbrBackground=NULL);
	static void UnInitWindowClass( LPCTSTR lpszName );
	static void Init()
	{
		static BOOL bInit = FALSE;
		if( bInit )
			return;
		bInit = TRUE;
		XWndProcThunk::m_pCreateWndList = NULL;
		InitializeCriticalSection( &XWndProcThunk::m_csWindowCreate );
	};

	static LRESULT __stdcall StartWindowProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
	static LRESULT __stdcall ReflectWndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
	virtual LRESULT __stdcall WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );

	DWORD m_dwThreadID;

	XWndProcThunk m_thunk;
};

