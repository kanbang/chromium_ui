// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_RENDERER_RENDER_VIEW_H_
#define CHROME_RENDERER_RENDER_VIEW_H_

#include <deque>
#include <map>
#include <set>
#include <string>
#include <queue>
#include <vector>

#include "base/basictypes.h"
#include "base/files/file_path.h"
#include "base/id_map.h"
#include "base/memory/linked_ptr.h"
#include "base/memory/shared_memory.h"
#include "base/timer/timer.h"
#include "base/values.h"
#include "base/memory/weak_ptr.h"
#include "base/memory/ref_counted.h"

#include "chrome/iecore/child/render_widget.h"
class CIEBrowserWnd;

// We need to prevent a page from trying to create infinite popups. It is not
// as simple as keeping a count of the number of immediate children
// popups. Having an html file that window.open()s itself would create
// an unlimited chain of RenderViews who only have one RenderView_IE child.
//
// Therefore, each new top level RenderView_IE creates a new counter and shares it
// with all its children and grandchildren popup RenderViews created with
// createView() to have a sort of global limit for the page so no more than
// kMaximumNumberOfPopups popups are created.

// This is a RefCounted holder of an int because I can't say
// scoped_refptr<int>.
typedef base::RefCountedData<int> SharedRenderViewCounter;

//
// RenderView_IE is an object that manages a WebView object, and provides a
// communication interface with an embedding application process
//
class RenderView : public RenderWidget,
                   public base::SupportsWeakPtr<RenderView>{

public:
	int32 from_routing_id_;
	int64 from_session_storage_namespace_id_;

public:
	// CBrowserWnd event
  virtual int get_routing_id();
  virtual void onCreate(){};
  bool SendMsg(IPC::Message* msg);

  int32 route_id();
  int32 page_id();
  void SetBrowserWnd(HWND wnd);
  void SetNewWindowParams(Browser_NewWindow_Params* parmas) { new_window_params_ = parmas; };
  Browser_NewWindow_Params* GetNewWindowParams() { return new_window_params_; }

  virtual bool OnBrowserMessageReceived(const IPC::Message& msg);
  void BindTab(HWND parent, int roateid);
  void didFocus();

public:

  static RenderView* Create(
      content::ChildThread* render_thread,
      HWND parent_hwnd,
      int32 opener_id,
      SharedRenderViewCounter* counter,
      int32 routing_id,
      int64 session_storage_namespace_id, 
      int32 next_page_id, 
      BOOL bNewWindow = FALSE);

  static RenderView* CreateUnbindView(
      content::ChildThread* render_thread,
      HWND parent_hwnd,
      int32 opener_id,
      SharedRenderViewCounter* counter,
      int32 routing_id,
      int64 session_storage_namespace_id, 
      int32 next_page_id, 
      BOOL bNewWindow = FALSE);

  int browser_window_id() const {
    return 0;
  }

  virtual bool OnIPCRedirectMsg_IEThread(const IPC::Message& msg) OVERRIDE;
  virtual bool OnIPCMsg_IOThread(const IPC::Message& msg) OVERRIDE;


  // Do not delete directly.  This class is reference counted.
  virtual ~RenderView();
  static int GetNextRoateId();
  int GetBrowserId(){ return browser_routeid_; };

 protected:
  // RenderWidget overrides:
   virtual void Close(){};
   virtual void DidHandleKeyEvent(){};

  virtual BOOL TranslateMessageBrowser( MSG& msg );
  virtual void OnBrowserSize( WPARAM wParam, LPARAM lParam );
  void Browser_OnNewWindowReady(const Browser_NewWindow_Params& params);

 private:
  explicit RenderView(content::ChildThread* render_thread,
                      int64 session_storage_namespace_id, 
                      int32 next_page_id);
  RenderView(content::ChildThread* render_thread,
                      bool delay_to_bind,
                      int32 next_page_id);

  // Initializes this view with the given parent and ID. The |routing_id| can be
  // set to 'MSG_ROUTING_NONE' if the true ID is not yet known. In this case,
  // CompleteInit must be called later with the true ID.
  void Init(HWND parent_hwnd,
            int32 opener_id,
            SharedRenderViewCounter* counter,
            int32 routing_id, BOOL bNewWindow = FALSE);

  void InitUnbindView(HWND parent,
            int32 opener_id,
            SharedRenderViewCounter* counter,
            int32 routing_id, BOOL bNewWindow = FALSE);
  
  void UpdateSessionHistory();

  virtual void OnMarshalDataResult(void* marshal_data, uint32 size);

  // Unique id to identify the current page between browser and renderer.
  //
  // Note that this is NOT updated for every main frame navigation, only for
  // "regular" navigations that go into session history. In particular, client
  // redirects, like the page cycler uses (document.location.href="foo") do not
  // count as regular navigations and do not increment the page id.
  int32 page_id_;
  int browser_routeid_;
  HWND m_hWnd;
  HWND m_hParent;
  CIEBrowserWnd* m_pWnd;
  Browser_NewWindow_Params* new_window_params_;

  DISALLOW_COPY_AND_ASSIGN(RenderView);

};

#endif  // CHROME_RENDERER_RENDER_VIEW_H_
