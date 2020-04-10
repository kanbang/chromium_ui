#pragma once
#include <MsHTML.h>
#include <ExDisp.h>
/*
模拟实现对IE内核打开新窗口的支持
其中window_open事件的features对应着新窗口的属性。
window_moveto--Window_ResizeBy是为了支持对新窗口位置的修改。
测试URL：
www.ccb.com
http://www.cnbeta.com/articles/267434.htm 上面的Qzone关注按钮
*/

typedef HRESULT (__stdcall *Window_Open)(IHTMLWindow2* pThis, BSTR url, BSTR name, BSTR features, VARIANT_BOOL replace, IHTMLWindow2 **pomWindowResult);
typedef HRESULT (__stdcall *Window_MoveTo)(IHTMLWindow2* pThis, long x, long y);
typedef HRESULT (__stdcall *Window_MoveBy)(IHTMLWindow2* pThis, long x, long y);
typedef HRESULT (__stdcall *Window_ResizeTo)(IHTMLWindow2* pThis, long x, long y);
typedef HRESULT (__stdcall *Window_ResizeBy)(IHTMLWindow2* pThis, long x, long y);

enum
{
  WINDOW_POS_MOVETO,
  WINDOW_POS_MOVEBY,
  WINDOW_POS_RESIZETO,
  WINDOW_POS_RESIZEBY,
};

struct POSITIONADJUST
{
  int nType;
  long x;
  long y;
};

class CHtmlWindowHook
{
public:
  CHtmlWindowHook(void);
  virtual ~CHtmlWindowHook(void);
  
  enum WINDOWOPEN_POLICY 
  {
    WINDOWOPEN_SUGGEST = 0,
    WINDOWOPEN_NEWWINDOW,
    WINDOWOPEN_NEWTAB,
    WINDOWOPEN_UNKNOWN = -1,
  };

public:
  static Window_Open orig_window_open;
  static Window_MoveTo orig_window_moveTo;
  static Window_MoveBy orig_window_moveBy;
  static Window_ResizeTo orig_window_resizeTo;
  static Window_ResizeBy orig_window_resizeBy;

  static HRESULT __stdcall Hooked_window_open( IHTMLWindow2* pThis, BSTR url, BSTR name, BSTR features, VARIANT_BOOL replace, IHTMLWindow2 **pomWindowResult );
  static HRESULT __stdcall Hooked_window_moveTo( IHTMLWindow2* pThis, long x, long y );
  static HRESULT __stdcall Hooked_window_moveBy( IHTMLWindow2* pThis, long x, long y );
  static HRESULT __stdcall Hooked_window_resizeTo( IHTMLWindow2* pThis, long x, long y );
  static HRESULT __stdcall Hooked_window_resizeBy( IHTMLWindow2* pThis, long x, long y );

  static void StartAPIHook();

  // 获得ie中window.open打开的方式
  static WINDOWOPEN_POLICY GetWindowOpenPilicy();
};
