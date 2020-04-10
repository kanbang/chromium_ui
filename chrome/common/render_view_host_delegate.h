// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_RENDERER_HOST_RENDER_VIEW_HOST_DELEGATE_H_
#define CONTENT_BROWSER_RENDERER_HOST_RENDER_VIEW_HOST_DELEGATE_H_

#include <string>

#include "base/basictypes.h"
#include "base/callback.h"
#include "base/process/kill.h"
#include "base/strings/string16.h"
#include "chrome/chrome_export.h"
#include "ui/base/window_open_disposition.h"

namespace base {
class ListValue;
class TimeTicks;
}

namespace IPC {
class Message;
}

namespace gfx {
class Point;
class Rect;
class Size;
}

class GURL;
namespace content {

class BrowserContext;
class FrameTree;
class PageState;
class RenderViewHost;
class RenderViewHostDelegateView;
class SessionStorageNamespace;
class SiteInstance;
class WebContents;
class WebContentsImpl;
struct ContextMenuParams;
struct FileChooserParams;
struct GlobalRequestID;
struct NativeWebKeyboardEvent;
struct Referrer;
struct RendererPreferences;
struct ResourceRedirectDetails;
struct ResourceRequestDetails;

//
// RenderViewHostDelegate
//
//  An interface implemented by an object interested in knowing about the state
//  of the RenderViewHost.
//
//  This interface currently encompasses every type of message that was
//  previously being sent by WebContents itself. Some of these notifications
//  may not be relevant to all users of RenderViewHost and we should consider
//  exposing a more generic Send function on RenderViewHost and a response
//  listener here to serve that need.
class CONTENT_EXPORT RenderViewHostDelegate {
 public:

   // RendererManagerment -------------------------------------------------------
   // Functions for managing switching of Renderers. For WebContents, this is
   // implemented by the RenderViewHostManager.

   class CONTENT_EXPORT RendererManagement {
   public:
     // Notification whether we should close the page, after an explicit call to
     // AttemptToClosePage.  This is called before a cross-site request or before
     // a tab/window is closed (as indicated by the first parameter) to allow the
     // appropriate renderer to approve or deny the request.  |proceed| indicates
     // whether the user chose to proceed.  |proceed_time| is the time when the
     // request was allowed to proceed.
     virtual void ShouldClosePage(
       bool for_cross_site_transition,
       bool proceed,
       const base::TimeTicks& proceed_time) = 0;

     // The |pending_render_view_host| is ready to commit a page.  The delegate
     // should ensure that the old RenderViewHost runs its unload handler first.
     virtual void OnCrossSiteResponse(
       RenderViewHost* pending_render_view_host,
       int global_request_id) = 0;

   protected:
     virtual ~RendererManagement() {}
   };

  // ---------------------------------------------------------------------------

  // Returns the current delegate associated with a feature. May return NULL if
  // there is no corresponding delegate.
  virtual RenderViewHostDelegateView* GetDelegateView();

  // This is used to give the delegate a chance to filter IPC messages.
  virtual bool OnMessageReceived(RenderViewHost* render_view_host,
                                 const IPC::Message& message);

  // Gets the URL that is currently being displayed, if there is one.
  //virtual const GURL& GetURL() const;

  // Return this object cast to a WebContents, if it is one. If the object is
  // not a WebContents, returns NULL. DEPRECATED: Be sure to include brettw or
  // jam as reviewers before you use this method. http://crbug.com/82582
  virtual WebContents* GetAsWebContents();

  // The RenderView is being constructed (message sent to the renderer process
  // to construct a RenderView).  Now is a good time to send other setup events
  // to the RenderView.  This precedes any other commands to the RenderView.
  virtual void RenderViewCreated(RenderViewHost* render_view_host) {}

  // The RenderView has been constructed.
  virtual void RenderViewReady(RenderViewHost* render_view_host) {}

  // The RenderView died somehow (crashed or was killed by the user).
  virtual void RenderViewTerminated(RenderViewHost* render_view_host,
                                    base::TerminationStatus status,
                                    int error_code) {}

  // The RenderView is going to be deleted. This is called when each
  // RenderView is going to be destroyed
  virtual void RenderViewDeleted(RenderViewHost* render_view_host) {}

  // The RenderView started a provisional load for a given frame.
  virtual void DidStartProvisionalLoadForFrame(
      RenderViewHost* render_view_host,
      int64 frame_id,
      int64 parent_frame_id,
      bool main_frame,
      const GURL& url) {}

  // The RenderView processed a redirect during a provisional load.
  //
  // TODO(creis): Remove this method and have the pre-rendering code listen to
  // WebContentsObserver::DidGetRedirectForResourceRequest instead.
  // See http://crbug.com/78512.
  virtual void DidRedirectProvisionalLoad(
      RenderViewHost* render_view_host,
      int32 page_id,
      const GURL& source_url,
      const GURL& target_url) {}

  // The state for the page changed and should be updated.
  virtual void UpdateState(RenderViewHost* render_view_host,
                           int32 page_id,
                           const PageState& state) {}

  // The page's title was changed and should be updated.
  virtual void UpdateTitle(RenderViewHost* render_view_host,
                           int32 page_id,
                           const string16& title) {}

  virtual void UpdateUrl(RenderViewHost* render_view_host,
    int32 page_id,
    const string16& Url) {}

  // The page is trying to close the RenderView's representation in the client.
  virtual void Close(RenderViewHost* render_view_host) {}

  // The RenderView began loading a new page. This corresponds to WebKit's
  // notion of the throbber starting.
  virtual void DidStartLoading(RenderViewHost* render_view_host) {}

  // The RenderView stopped loading a page. This corresponds to WebKit's
  // notion of the throbber stopping.
  virtual void DidStopLoading(RenderViewHost* render_view_host) {}

  // The pending page load was canceled.
  virtual void DidCancelLoading() {}

  // The RenderView made progress loading a page's top frame.
  // |progress| is a value between 0 (nothing loaded) to 1.0 (top frame
  // entirely loaded).
  virtual void DidChangeLoadProgress(double progress) {}

  // The RenderView set its opener to null, disowning it for the lifetime of
  // the window.
  virtual void DidDisownOpener(RenderViewHost* rvh) {}

  // Notification the user has made a gesture while focus was on the
  // page. This is used to avoid uninitiated user downloads (aka carpet
  // bombing), see DownloadRequestLimiter for details.
  virtual void OnUserGesture() {}

  // Notification from the renderer host that blocked UI event occurred.
  // This happens when there are tab-modal dialogs. In this case, the
  // notification is needed to let us draw attention to the dialog (i.e.
  // refocus on the modal dialog, flash title etc).
  virtual void OnIgnoredUIEvent() {}

  // The page wants the hosting window to activate/deactivate itself (it
  // called the JavaScript window.focus()/blur() method).
  virtual void Activate() {}
  virtual void Deactivate() {}

  // Notification that the view has lost capture.
  virtual void LostCapture() {}

  // Notifications about mouse events in this view.  This is useful for
  // implementing global 'on hover' features external to the view.
  virtual void HandleMouseMove() {}
  virtual void HandleMouseDown() {}
  virtual void HandleMouseLeave() {}
  virtual void HandleMouseUp() {}
  virtual void HandlePointerActivate() {}
  virtual void HandleGestureBegin() {}
  virtual void HandleGestureEnd() {}

  // Notification that the page wants to go into or out of fullscreen mode.
  virtual void ToggleFullscreenMode(bool enter_fullscreen) {}
  virtual bool IsFullscreenForCurrentTab() const;

  virtual void CreateNewWindow(
      int route_id) {}

  // Show a previously created page with the specified disposition and bounds.
  // The window is identified by the route_id passed to CreateNewWindow.
  //
  // Note: this is not called "ShowWindow" because that will clash with
  // the Windows function which is actually a #define.
  virtual void ShowCreatedWindow(int route_id,
                                 WindowOpenDisposition disposition,
                                 const gfx::Rect& initial_pos,
                                 bool user_gesture) {}

  // Show the newly created widget with the specified bounds.
  // The widget is identified by the route_id passed to CreateNewWidget.
  virtual void ShowCreatedWidget(int route_id,
                                 const gfx::Rect& initial_pos) {}

  // Show the newly created full screen widget. Similar to above.
  virtual void ShowCreatedFullscreenWidget(int route_id) {}

  // A context menu should be shown, to be built using the context information
  // provided in the supplied params.
  virtual void ShowContextMenu(const ContextMenuParams& params) {}

 protected:
  virtual ~RenderViewHostDelegate() {}
};

}  // namespace content

#endif  // CONTENT_BROWSER_RENDERER_HOST_RENDER_VIEW_HOST_DELEGATE_H_
