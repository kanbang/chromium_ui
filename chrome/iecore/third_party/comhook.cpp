#include "chrome/iecore/third_party/comhook.h"
#include <Windows.h>
#include <atlbase.h>

BOOL HookComInterface(DWORD ** p, int offset, DWORD dwNewVal, DWORD * pOldVal)
{
	DWORD dwSize = 0;
	HANDLE hProcess = GetCurrentProcess();
	VirtualProtect( *p+offset, 4, PAGE_EXECUTE_READWRITE, &dwSize );

	DWORD dwOld = 0;
	if( ReadProcessMemory( hProcess, *p+offset, &dwOld, sizeof(DWORD), &dwSize ) )
	{
		if( dwNewVal == dwOld )
			return FALSE;
		*pOldVal = dwOld;
		if( WriteProcessMemory( hProcess, *p+offset, &dwNewVal, 4, &dwSize ) )
			return TRUE;
	}
    return FALSE;
}

CComAutoCriticalSection g_cs_sound;
BOOL HookComInterfaceSafe(DWORD ** p, int offset,DWORD dwNewVal, DWORD* pOldVal) 
{
	BOOL ok = FALSE;
	if(pOldVal == NULL)
		return FALSE;
	if(*pOldVal == NULL) 
	{
		CComCritSecLock<CComCriticalSection> guard(g_cs_sound);
		if(*pOldVal == NULL)
			ok = HookComInterface(p, offset, dwNewVal, pOldVal);
		if(ok && *pOldVal != NULL) {
			MEMORY_BASIC_INFORMATION mbi={0};
			if(!VirtualQuery((LPCVOID)(*pOldVal), &mbi,sizeof(mbi)) || 0 == mbi.AllocationBase)
				return ok;
			if((HMODULE)mbi.AllocationBase != GetModuleHandle(NULL)) {
				wchar_t szDllName[MAX_PATH + 1] = {L'\0'};
				if(GetModuleFileName((HMODULE)mbi.AllocationBase, szDllName, MAX_PATH) > 0) 
				{
					LoadLibrary(szDllName);//lwg:防止DLL按COM规范卸载，因为每次载入的基地址可能会不同。
				}
			}
		}
	}
	return ok;
}