// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/iecore/child/render_widget.h"

#include "base/bind.h"
#include "base/command_line.h"
#include "base/logging.h"
#include "base/message_loop/message_loop.h"
#include "base/memory/scoped_ptr.h"
#include "build/build_config.h"
#include "chrome/iecore/child/child_process.h"
#include "chrome/iecore/child/child_thread.h"


RenderWidget::RenderWidget(content::ChildThread* render_thread)
    : routing_id_(MSG_ROUTING_NONE),
      opener_id_(MSG_ROUTING_NONE),
      render_thread_(render_thread),
      host_window_(0),
      update_reply_pending_(false),
      did_show_(false),
      is_hidden_(false),
      is_swapped_out_(false),
      has_focus_(false),
      closing_(false),
      hWndIPCReDirect_(NULL){
  DCHECK(render_thread_);
  content::ChildProcess::current()->AddRefProcess();
}

RenderWidget::~RenderWidget() {
  if(render_thread_ && base::MessageLoop::current() == render_thread_->message_loop()){
    if (routing_id_ != MSG_ROUTING_NONE) {
      if(render_thread_->ResolveRoute(routing_id_)){
        render_thread_->RemoveRoute(routing_id_);
      }
    }    
  }

  // If we are swapped out, we have released already.
  if (!is_swapped_out_)
    content::ChildProcess::current()->ReleaseProcess();
}

/*static*/
RenderWidget* RenderWidget::Create(int32 opener_id,
                                   content::ChildThread* render_thread ) {
  DCHECK(opener_id != MSG_ROUTING_NONE);
  scoped_refptr<RenderWidget> widget = new RenderWidget(render_thread);
  widget->Init(opener_id);  // adds reference
  return widget;
}
void RenderWidget::InitIPCMsgHwnd(HWND hWnd){
  DCHECK(NULL == hWndIPCReDirect_);
  hWndIPCReDirect_ = hWnd;
  DCHECK(::IsWindow(hWndIPCReDirect_));
  if(delay_msg_.size() > 0) {
    for (size_t index = 0; index < delay_msg_.size(); index++) {
      ::PostMessage(hWndIPCReDirect_,WM_IPCMSG_REDIRECT,(WPARAM)delay_msg_[index],(LPARAM)this);
    }
    delay_msg_.clear();
  }
}
void RenderWidget::Init(int32 opener_id) {
  if (opener_id != MSG_ROUTING_NONE)
    opener_id_ = opener_id;

  /*bool result = render_thread_->Send(
      new ViewHostMsg_CreateWidget(opener_id, popup_type_, &routing_id_, 
      &surface_id_));
  if (result) {
    render_thread_->AddRoute(routing_id_, this);
    // Take a reference on behalf of the RenderThread.  This will be balanced
    // when we receive ViewMsg_Close.
    AddRef();
  } else {
    DCHECK(false);
  }*/
}

bool RenderWidget::OnMessageReceived(const IPC::Message& msg) {
   bool handled = OnIPCMsg_IOThread(msg);
   if( false == handled ){
     //DCHECK(hWndIPCReDirect_);
     if(hWndIPCReDirect_)
      ::PostMessage(hWndIPCReDirect_,WM_IPCMSG_REDIRECT,(WPARAM)new IPC::Message(msg),(LPARAM)this);
     else
       delay_msg_.push_back(new IPC::Message(msg));

     handled = true;
  }
  return handled;
}

bool RenderWidget::OnIPCMsg_IOThread(const IPC::Message& msg){
  bool handled = false;
  /*IPC_BEGIN_MESSAGE_MAP(RenderWidget, msg)
    IPC_MESSAGE_UNHANDLED(handled = false)
    IPC_END_MESSAGE_MAP()*/
    return handled;
}
bool RenderWidget::Send(IPC::Message* message) {
  // Don't send any messages after the browser has told us to close.
  if (closing_) {
    delete message;
    return false;
  }

  // If given a messsage without a routing ID, then assign our routing ID.
  if (message->routing_id() == MSG_ROUTING_NONE)
    message->set_routing_id(routing_id_);

  return render_thread_->Send(message);
}

void RenderWidget::OnClose() {
  // Browser correspondence is no longer needed at this point.
  if (routing_id_ != MSG_ROUTING_NONE) {
    render_thread_->RemoveRoute(routing_id_);
  }
}

void RenderWidget::Close() {
}

