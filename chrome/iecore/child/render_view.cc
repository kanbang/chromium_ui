// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/iecore/child/render_view.h"

#include <windows.h>
#include <xutility>
#include <algorithm>
#include <time.h>

#include "base/callback.h"
#include "base/command_line.h"
#include "base/compiler_specific.h"
#include "base/file_util.h"
#include "base/lazy_instance.h"
#include "base/metrics/field_trial.h"
#include "base/metrics/histogram.h"
#include "base/memory/singleton.h"
#include "base/strings/string_piece.h"
#include "base/strings/string_util.h"
#include "base/time/time.h"
#include "base/strings/utf_string_conversions.h"
#include "base/strings/string_number_conversions.h"
#include "base/win/scoped_bstr.h"
#include "base/win/scoped_comptr.h"
#include "ui/base/window_open_disposition.h"
#include "build/build_config.h"
#include "ui/base/window_open_disposition.h"
#include "ui/gfx/canvas.h"
#include "ui/gfx/gfx_paths.h"
#include "ui/gfx/icon_util.h"
#include "ui/gfx/size.h"
#include "ui/gfx/color_utils.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/resource/resource_bundle.h"
#include "base/memory/shared_memory.h"
#include "base/files/file_path.h"
#include "base/base64.h"
#include "ipc/ipc_logging.h"
#include "ipc/ipc_switches.h"
#include "ipc/ipc_sync_channel.h"
#include "ipc/ipc_sync_message_filter.h"

#include "chrome/iecore/trident/WebBrowserHost/IEBrowserWnd.h"
#include "chrome/iecore/iecore_message.h"

using base::Time;
using base::TimeDelta;

///////////////////////////////////////////////////////////////////////////////
RenderView::RenderView(content::ChildThread* render_thread,
                       int64 session_storage_namespace_id,
                       int32 next_page_id)
    : RenderWidget(render_thread),
      m_hWnd(NULL),
      browser_routeid_(0),
      m_pWnd(NULL),
      new_window_params_(NULL),
      m_hParent(NULL){
}

RenderView::RenderView(content::ChildThread* render_thread,
                       bool delay_to_bind,
                       int32 next_page_id)
   : RenderWidget(render_thread),
    m_hWnd(NULL),
    browser_routeid_(0),
    m_pWnd(NULL),
    new_window_params_(NULL),
    m_hParent(NULL){
   
}

RenderView::~RenderView() {
}

/*static*/
RenderView* RenderView::Create(
    content::ChildThread* render_thread,
    HWND parent_hwnd,
    int32 opener_id,
    SharedRenderViewCounter* counter,
    int32 routing_id,
    int64 session_storage_namespace_id, 
    int32 next_page_id,
    BOOL bNewWindow) {
  DCHECK(routing_id != MSG_ROUTING_NONE);
  scoped_refptr<RenderView> view = 
          new RenderView(render_thread,
                         session_storage_namespace_id, next_page_id);
  view->Init(parent_hwnd,
             opener_id,
             counter,
             routing_id,
			        bNewWindow);  // adds reference
  return view;
}

RenderView* RenderView::CreateUnbindView(
    content::ChildThread* render_thread,
    HWND parent_hwnd,
    int32 opener_id,
    SharedRenderViewCounter* counter,
    int32 routing_id,
    int64 session_storage_namespace_id, 
    int32 next_page_id, 
    BOOL bNewWindow) {
  scoped_refptr<RenderView> view = 
          new RenderView(render_thread,true, next_page_id);
  view->InitUnbindView(parent_hwnd,
             opener_id,
             counter,
             routing_id,
			       TRUE);  // adds reference
  return view;
}

int RenderView::GetNextRoateId() {
  static int i = 1;
  return i++;
}

void RenderView::InitUnbindView(HWND parent_hwnd,
                      int32 opener_id,
                      SharedRenderViewCounter* counter,
                      int32 routing_id, BOOL bNewWindow) {

  if (opener_id != MSG_ROUTING_NONE)
    opener_id_ = opener_id;

  routing_id_ = routing_id;
  // Take a reference on behalf of the RenderThread.  This will be balanced
  // when we receive ViewMsg_Close.
  AddRef();

  // todo
}


void RenderView::Init(HWND parent_hwnd,
                      int32 opener_id,
                      SharedRenderViewCounter* counter,
                      int32 routing_id, 
					  BOOL bNewWindow) {

  if (opener_id != MSG_ROUTING_NONE)
    opener_id_ = opener_id;

  routing_id_ = routing_id;
  if(routing_id != 0)
    render_thread_->AddRoute(routing_id_, this);
  // Take a reference on behalf of the RenderThread.  This will be balanced
  // when we receive ViewMsg_Close.
  AddRef();
  browser_routeid_ = GetNextRoateId();
  render_thread_->AddRenderViewRoute(browser_routeid_, this);
  //kk ²âÊÔ²»´´½¨wnd

//   m_pWnd = CIEBrowserWnd::CreateBrowserThread(parent_hwnd,
//     render_thread_->m_hWnd,
//     bNewWindow ? true : false,
//     browser_routeid_, this);
}

bool RenderView::OnIPCRedirectMsg_IEThread(const IPC::Message& message){
  bool handled =true;
  handled = true;
  return handled;
}
bool RenderView::OnIPCMsg_IOThread(const IPC::Message& message){
  bool handled = false;
  return handled;
}

void RenderView::OnBrowserSize( WPARAM wParam, LPARAM lParam ){
  
}

int RenderView::get_routing_id(){
  return routing_id();
}

int32 RenderView::route_id(){
   return routing_id();
}
int32 RenderView::page_id(){
   return page_id_;
}

void RenderView::SetBrowserWnd(HWND wnd) {
 m_hWnd = wnd; 
 InitIPCMsgHwnd(wnd);
 if(m_hParent) 
   ::PostMessage(m_hWnd, CHILDTHREAD_TO_BROWSER_SET_PARENT, 0, (LPARAM)m_hParent);
}

bool  RenderView::SendMsg(IPC::Message* msg){
  return Send(msg);
}

BOOL RenderView::TranslateMessageBrowser( MSG& msg ){
	if( msg.message >= WM_KEYFIRST && msg.message <= WM_KEYLAST ){
		if((msg.wParam=='F' && HIBYTE(GetKeyState(VK_CONTROL))) ||
       (msg.message == WM_SYSKEYDOWN && msg.wParam == VK_F4)) {
			PostMessage(GetParent(m_hWnd), msg.message, msg.wParam, msg.lParam);
			return TRUE;
		}
		if( msg.wParam == VK_MENU 
			|| msg.wParam == VK_CONTROL 
			|| msg.wParam == VK_SHIFT)
			return FALSE;

		PostMessage( GetParent(m_hWnd), msg.message, msg.wParam, msg.lParam );
    if(HIBYTE(GetKeyState(VK_CONTROL)))
    if(  VK_OEM_PLUS == msg.wParam || VK_OEM_MINUS == msg.wParam  ||
         VK_ADD ==msg.wParam || VK_SUBTRACT == msg.wParam   )
      return TRUE;
	}
	return FALSE;
}

void RenderView::OnMarshalDataResult(void* marshal_data, uint32 size) {
  /*if (marshal_data && 0 != size) {
      base::BinaryValue* value = base::BinaryValue::CreateWithCopiedBuffer((const char*)marshal_data,size);
      if (value) {
        base::ListValue result;
        result.Set(0, value);
        Send(new ViewHostMsg_MarshalDataResult(routing_id(), result));
      }
  }*/
}

bool RenderView::OnBrowserMessageReceived(const IPC::Message& msg) {
  bool handle = false;
  IPC_BEGIN_MESSAGE_MAP(RenderView, msg)
    IPC_MESSAGE_HANDLER(Browser_NewWindowReady, Browser_OnNewWindowReady)
    IPC_MESSAGE_UNHANDLED(handle = false)
  IPC_END_MESSAGE_MAP()
  return false;
}

void RenderView::Browser_OnNewWindowReady(const Browser_NewWindow_Params& params) {
  if(new_window_params_) {
    new_window_params_->pStream = params.pStream;
    new_window_params_->dwSize = params.dwSize;
    new_window_params_->hNewEvent = params.hNewEvent;
    SetEvent(new_window_params_->hEvent);
    SetNewWindowParams(NULL);
  }
}

void RenderView::BindTab(HWND parent, int roateid) {
  if(routing_id_ <= 0)
    render_thread_->AddRoute(roateid, this);

  routing_id_ = roateid;
  m_hParent = parent;
  if(m_hWnd) {
    //::SetParent(m_hWnd, parent);
   ::PostMessage(m_hWnd, CHILDTHREAD_TO_BROWSER_SET_PARENT, 0, (LPARAM)parent);
  }
}

void RenderView::didFocus() {
  Send(new ViewHostMsg_Focus(routing_id()));
}