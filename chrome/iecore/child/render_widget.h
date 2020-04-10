// Copyright (c) 2009 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_RENDERER_RENDER_WIDGET_H_
#define CHROME_RENDERER_RENDER_WIDGET_H_

#include <vector>

#include "base/basictypes.h"
#include "base/memory/ref_counted.h"
#include "base/memory/shared_memory.h"
#include "ipc/ipc_channel.h"
#include "third_party/skia/include/core/SkBitmap.h"
#include "ui/gfx/native_widget_types.h"
#include "ui/gfx/point.h"
#include "ui/gfx/rect.h"
#include "ui/gfx/size.h"
#include "ui/events/latency_info.h"

#include "chrome/iecore/child/child_thread.h"

struct ViewHostMsg_ShowPopup_Params;
namespace content {
  class ChildThread;
}

namespace WebKit {
class WebInputEvent;
}

#define WM_IPCMSG_REDIRECT  (WM_USER + 110)
// RenderWidget_IE provides a communication bridge between a WebWidget and
// a RenderWidgetHost, the latter of which lives in a different process.
class RenderWidget : public IPC::Listener,
                     public IPC::Sender,
                     public base::RefCounted<RenderWidget> {
 public:
  // Creates a new RenderWidget_IE.  The opener_id is the routing ID of the
  // RenderView that this widget lives inside. The render_thread is any
  // ChildThread_IE implementation, mostly commonly RenderThread::current().
  static RenderWidget* Create(int32 opener_id,
                              content::ChildThread* render_thread);

  // The routing ID assigned by the RenderProcess. Will be MSG_ROUTING_NONE if
  // not yet assigned a view ID, in which case, the process MUST NOT send
  // messages with this ID to the parent.
  int32 routing_id() const {
    return routing_id_;
  }
  gfx::NativeViewId host_window() const {
    return host_window_;
  }

  void    InitIPCMsgHwnd(HWND hWnd);
  virtual bool OnIPCRedirectMsg_IEThread(const IPC::Message& msg){return false;};
  virtual bool OnIPCMsg_IOThread(const IPC::Message& msg);
  // IPC::Listener
  virtual bool OnMessageReceived(const IPC::Message& msg);
  virtual bool OnBrowserMessageReceived(const IPC::Message& msg){return false;};

  // IPC::Message::Sender
  virtual bool Send(IPC::Message* msg);

  // Close the underlying WebWidget.
  virtual void Close();

 protected:
  // Friend RefCounted so that the dtor can be non-public. Using this class
  // without ref-counting is an error.
  friend class base::RefCounted<RenderWidget>;

  RenderWidget(content::ChildThread* render_thread);
  virtual ~RenderWidget();

  // Initializes this view with the given opener.  CompleteInit must be called
  // later.
  void Init(int32 opener_id);
  void OnClose();

  // Routing ID that allows us to communicate to the parent browser process
  // RenderWidgetHost. When MSG_ROUTING_NONE, no messages may be sent.
  int32 routing_id_;

  // Set to the ID of the view that initiated creating this view, if any. When
  // the view was initiated by the browser (the common case), this will be
  // MSG_ROUTING_NONE. This is used in determining ownership when opening
  // child tabs. See RenderWidget_IE::createWebViewWithRequest.
  //
  // This ID may refer to an invalid view if that view is closed before this
  // view is.
  int32 opener_id_;
  content::ChildThread* render_thread_;

  // The position where this view should be initially shown.
  gfx::Rect initial_pos_;

  // The window we are embedded within.  TODO(darin): kill this.
  gfx::NativeViewId host_window_;

  // The size of the RenderWidget_IE.
  gfx::Size size_;

  // The area that must be reserved for drawing the resize corner.
  gfx::Rect resizer_rect_;

  // True if we are expecting an UpdateRect_ACK message (i.e., that a
  // UpdateRect message has been sent).
  bool update_reply_pending_;

  // Set to true if we should ignore RenderWidget_IE::Show calls.
  bool did_show_;

  // Indicates that we shouldn't bother generated paint events.
  bool is_hidden_;

  // Indicates whether we have been focused/unfocused by the browser.
  bool has_focus_;

  // True if we have requested this widget be closed.  No more messages will
  // be sent, except for a Close.
  bool closing_;

  // The screen rects of the view and the window that contains it.
  gfx::Rect view_screen_rect_;

  // Indicates if the next sequence of Char events should be suppressed or not.
  bool suppress_next_char_events_;

  // Whether this RenderWidget is currently swapped out, such that the view is
  // being rendered by another process.  If all RenderWidgets in a process are
  // swapped out, the process can exit.
  bool is_swapped_out_;

  HWND  hWndIPCReDirect_;
  std::vector<IPC::Message*> delay_msg_;
  DISALLOW_COPY_AND_ASSIGN(RenderWidget);
};

#endif  // CHROME_RENDERER_RENDER_WIDGET_H_
