#include "chrome/iecore/trident/stdafx.h"
#include "chrome/iecore/trident/preheader.h"
#include "SystemInfo.h"

SystemInfo::WindowType _bIsWindows9X = SystemInfo::LOW;
SystemInfo::WindowType _bIsWindowsXP = SystemInfo::LOW;//FALSE:XP以下  TRUE：XP及XP以上
SystemInfo::WindowType _bIsWindowsVista =SystemInfo::LOW;
SystemInfo::WindowType _bIsWindows2000 = SystemInfo::LOW;
SystemInfo::WindowType _bIsWindows7 =SystemInfo::LOW;
SystemInfo::WindowType _bIsWindows8 = SystemInfo::LOW;
SystemInfo::WindowType _bIsWindows81 = SystemInfo::LOW;
BOOL _bIsWindows64 = FALSE;
BOOL _bIE7	= FALSE;
BOOL _bIE6	= FALSE;
BOOL _bIE8	= FALSE;
BOOL _bIE9	= FALSE;
BOOL _bIE10	= FALSE;
BOOL _bIE11	= FALSE;

BOOL SystemInfo::IsWindows8()
{
	if( _bIsWindows8 == EQUAL)
		return TRUE;
	return FALSE;
}

BOOL SystemInfo::IsWindows81()
{
	if( _bIsWindows81 == EQUAL)
		return TRUE;
	return FALSE;
}

BOOL SystemInfo::IsWindows64()
{
	typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
	LPFN_ISWOW64PROCESS fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(GetModuleHandleA("kernel32"),"IsWow64Process");
	BOOL bIsWow64 = FALSE;
	if (NULL != fnIsWow64Process)
	{
		if (!fnIsWow64Process(GetCurrentProcess(),&bIsWow64))
			return FALSE;
	}
	return bIsWow64;
}


void SystemInfo::GetOsVer(DWORD& dwMajor, DWORD& dwMinor, DWORD& dwBuildNumber, DWORD& dwPlatformId, wchar_t * lpszCSDVersion, DWORD dwCSDVersionLen)
{	

	BOOL bOld=FALSE;
	WCHAR szSystem32Dir[MAX_PATH] = {0};
	GetSystemDirectory(szSystem32Dir, _countof(szSystem32Dir)) ;
	WCHAR szDll[MAX_PATH] = {0} ;
//kk	_snwprintf_s(szDll, _countof(szDll)-1, L"%s\\%s", szSystem32Dir, L"ntdll.dll", _TRUNCATE);
	_snwprintf_s(szDll, _countof(szDll) - 1, L"%s\\%s", szSystem32Dir, L"ntdll.dll");
	HMODULE hMod = GetModuleHandle(szDll);
	if(!hMod)		
		hMod = LoadLibrary(szDll);
	if(hMod)
	{
		typedef int (__stdcall * pFunRtlGetVersion)(PRTL_OSVERSIONINFOW);
		pFunRtlGetVersion pFunc = NULL;
		pFunc = (pFunRtlGetVersion)GetProcAddress(hMod, "RtlGetVersion");
		if(pFunc)
		{
			RTL_OSVERSIONINFOW osVersion = {0};
			osVersion.dwOSVersionInfoSize = sizeof(osVersion);
			LONG lRet = pFunc(&osVersion);
			if(osVersion.dwMajorVersion)
			{
				dwMajor = osVersion.dwMajorVersion;
				dwMinor = osVersion.dwMinorVersion;				
				dwBuildNumber = osVersion.dwBuildNumber;				
				dwPlatformId = osVersion.dwPlatformId;
				if( lpszCSDVersion)
					wcsncpy_s(lpszCSDVersion, dwCSDVersionLen-1, osVersion.szCSDVersion, _TRUNCATE);
			}
		}
	}
}


void SystemInfo::GetWindowInfo()
{
	DWORD dwPlatformId=-1;
	DWORD dwMajorVersion=-1;
	DWORD dwMinorVersion=-1;
	DWORD dwBuildNumber=-1;
	GetOsVer(dwMajorVersion,dwMinorVersion,dwBuildNumber,dwPlatformId,NULL,0);
	_bIsWindows64 = IsWindows64();
	if( VER_PLATFORM_WIN32_WINDOWS == dwPlatformId )
	{
		_bIsWindows9X = EQUAL;
	}
	else if( ( VER_PLATFORM_WIN32_NT == dwPlatformId ) )
	{
		if(5 == dwMajorVersion)
		{
			if( 0 == dwMinorVersion )
			{
				_bIsWindows9X = ABOVE;
				_bIsWindows2000 = EQUAL;
			}
			else if(dwMinorVersion>=1)
			{
				_bIsWindows9X =ABOVE;
				_bIsWindows2000 = ABOVE;
				_bIsWindowsXP = EQUAL;
			}
		}
		else if( (6 == dwMajorVersion) && (0 == dwMinorVersion || 1 ==dwMinorVersion || 2 == dwMinorVersion ||3==dwMinorVersion) )
		{
			_bIsWindows9X = ABOVE;
			_bIsWindows2000 = ABOVE;
			_bIsWindowsXP = ABOVE;
			_bIsWindowsVista = EQUAL;
			if( 1 == dwMinorVersion )
			{
				_bIsWindowsVista = ABOVE;
				_bIsWindows7 = EQUAL;
			}
			if( 2 == dwMinorVersion )
			{
				_bIsWindows7 = ABOVE;
				_bIsWindows8 = EQUAL;
			}
			if( 3== dwMinorVersion)
			{
				//win8.1 rtm build number 9600
				_bIsWindows8 = ABOVE;
				_bIsWindows81 = EQUAL;
			}
		}
		else if(6 < dwMajorVersion)
		{
			_bIsWindows9X = ABOVE;
			_bIsWindows2000 = ABOVE;
			_bIsWindowsXP = ABOVE;
			_bIsWindowsVista = ABOVE;
			_bIsWindows7 = ABOVE;
			_bIsWindows8 = ABOVE;
			_bIsWindows81 = EQUAL;
		}
	}
}

void* GetProcAddressFromModule(LPCTSTR module_name, LPCSTR proc_name)
{
	HMODULE module = GetModuleHandle(module_name);
	if (!module)
		module = LoadLibrary(module_name);
	if (module)
		return GetProcAddress(module, proc_name);
	return NULL;
}

BOOL VerQueryValueW_DLL(LPCVOID pBlock, LPCWSTR lpSubBlock, LPVOID * lplpBuffer, PUINT puLen)
{
	typedef BOOL (WINAPI *LPVerQueryValueW)(LPCVOID pBlock, LPCWSTR lpSubBlock, LPVOID * lplpBuffer, PUINT puLen);
	static LPVerQueryValueW lpVerQueryValueW = (LPVerQueryValueW)GetProcAddressFromModule(_T("version.dll"), "VerQueryValueW");
	if (lpVerQueryValueW)
		return lpVerQueryValueW(pBlock, lpSubBlock, lplpBuffer, puLen);
	return FALSE;
}

BOOL GetFileVersionInfoW_DLL(LPCWSTR lptstrFilename, DWORD dwHandle, DWORD dwLen, LPVOID lpData)
{
	typedef BOOL (WINAPI *LPGetFileVersionInfoW)(LPCWSTR lptstrFilename, DWORD dwHandle, DWORD dwLen, LPVOID lpData);
	static LPGetFileVersionInfoW lpGetFileVersionInfoW = (LPGetFileVersionInfoW)GetProcAddressFromModule(_T("version.dll"), "GetFileVersionInfoW");
	if (lpGetFileVersionInfoW)
		return lpGetFileVersionInfoW(lptstrFilename, dwHandle, dwLen, lpData);
	return FALSE;
}

DWORD GetFileVersionInfoSizeW_DLL(LPCWSTR lptstrFilename, LPDWORD lpdwHandle)
{
	typedef DWORD (WINAPI *LPGetFileVersionInfoSizeW)(LPCWSTR lptstrFilename, LPDWORD lpdwHandle);
	static LPGetFileVersionInfoSizeW lpGetFileVersionInfoSizeW = (LPGetFileVersionInfoSizeW)GetProcAddressFromModule(_T("version.dll"), "GetFileVersionInfoSizeW");
	if (lpGetFileVersionInfoSizeW)
		return lpGetFileVersionInfoSizeW(lptstrFilename, lpdwHandle);
	return 0;
}

BOOL SystemInfo::GetFileVersion( LPCTSTR strFile, LPTSTR pszVersion, int nVersionLen )
{
	TCHAR  szVersionBuffer[8192] = {0};
	DWORD dwVerSize;
	DWORD dwHandle;

	dwVerSize = GetFileVersionInfoSizeW_DLL( strFile, &dwHandle );
	if( dwVerSize == 0 || dwVerSize > (sizeof(szVersionBuffer)-1) )
		return FALSE;

	if( GetFileVersionInfoW_DLL( strFile,0,dwVerSize,szVersionBuffer) )
	{
		VS_FIXEDFILEINFO * pInfo;
		unsigned int nInfoLen;

		if( VerQueryValueW_DLL( szVersionBuffer, _T("\\"), (void**)&pInfo, &nInfoLen ) )
		{
			_stprintf_s(pszVersion, nVersionLen, _T("%d.%d.%d.%d"), 
				HIWORD( pInfo->dwFileVersionMS ), LOWORD( pInfo->dwFileVersionMS ), 
				HIWORD( pInfo->dwFileVersionLS ), LOWORD( pInfo->dwFileVersionLS ) );
			pszVersion[nVersionLen-1] = _T('\0');
			return TRUE;
		}
	}

	return FALSE;
}

LPCTSTR SystemInfo::GetIEPath()
{
	static TCHAR szIEPath[MAX_PATH]=_T("");
	if(_T('\0')==szIEPath[0])
	{
		SHGetSpecialFolderPath( NULL, szIEPath, CSIDL_PROGRAM_FILES, FALSE );
		_tcscat_s( szIEPath, _T("\\internet explorer\\iexplore.exe") );
	}
	return szIEPath;
}

BOOL SystemInfo::GetIEVersion(LPTSTR lpszVersion, DWORD dwSize) 
{
	DWORD dwType = REG_DWORD;
	DWORD dwValueSize = (dwSize - 1) * sizeof(TCHAR);
	if (ERROR_SUCCESS == SHGetValue(HKEY_LOCAL_MACHINE,
		_T("Software\\Microsoft\\Internet Explorer"), _T("Version"), &dwType,
		lpszVersion, &dwValueSize) && _T('\0') != lpszVersion[0])
		return TRUE;

	lpszVersion[0] = _T('\0');
	return GetFileVersion(GetIEPath(), lpszVersion, dwSize);
}

void SystemInfo::GetIEVersion()
{
	TCHAR szVersion[ MAX_PATH ] = {0};

	if(  _bIsWindows7==EQUAL )
	{
		GetFileVersion( GetIEPath(), szVersion,  _ARRAYSIZE( szVersion ) - 1 );
	}
	else
	{
		//读取注册表 速度快
		GetIEVersion( szVersion, _ARRAYSIZE( szVersion ) - 1 );
	}
	if ( _T( '7' ) == szVersion[0] )
		_bIE7 = TRUE;
	else if ( _T( '6' ) == szVersion[0] )
		_bIE6 = TRUE;
	else if ( _T( '8' ) == szVersion[0] )
		_bIE8 = TRUE;
	else if ( _T( '9' ) == szVersion[0] )
		_bIE9 = TRUE;
	else if(_T( '1' ) == szVersion[0] && _T( '0' ) == szVersion[1] )
		_bIE10 = TRUE;
	else if(_T( '1' ) == szVersion[0] && _T( '1' ) == szVersion[1] )
		_bIE11 = TRUE;
}