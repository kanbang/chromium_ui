
#pragma once

namespace SystemInfo
{
	enum WindowType
	{
		LOW=0,
		EQUAL=1,
		ABOVE=2,
	};
	void GetWindowInfo();
	void GetIEVersion();
	BOOL IsWindows64();
	BOOL IsWindows8();
	BOOL IsWindows81();
	void GetOsVer(DWORD& dwMajor, DWORD& dwMinor, DWORD& dwBuildNumber, DWORD& dwPlatformId, wchar_t * lpszCSDVersion, DWORD dwCSDVersionLen);
	BOOL GetFileVersion( LPCTSTR strFile, LPTSTR pszVersion, int nVersionLen );
	LPCTSTR GetIEPath();
	BOOL GetIEVersion(LPTSTR lpszVersion, DWORD dwSize) ;
}