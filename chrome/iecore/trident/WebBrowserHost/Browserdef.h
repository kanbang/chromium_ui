#pragma once 
#include <WinInet.h>

enum TabType
{
  TAB_HOME =0,
  TAB_NEWTAB,
  TAB_OTHER,
};

namespace WGMSG
{
	typedef struct LWGMARSHALDATA
	{
		HANDLE hEvent;
		HANDLE hEventOK;
		LPVOID lpStream;
		DWORD dwStreamSize;
		TCHAR szUrl[INTERNET_MAX_URL_LENGTH];
    bool bPopWindow;
	}*LPWGMARSHALDATA;
}


#define WG_BROWSER_NEEDNEWWND 0x5000
#define WG_BROWSER_MARSHARESULT 0x5001

#define WG_UI_ADDTAB          0x6001
#define WG_UI_ACTIVE_CHANGE   0x6002