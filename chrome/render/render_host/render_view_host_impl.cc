// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/render/render_host/render_view_host_impl.h"

#include <set>
#include <string>
#include <utility>
#include <vector>

#include "base/callback.h"
#include "base/command_line.h"
#include "base/debug/trace_event.h"
#include "base/json/json_reader.h"
#include "base/lazy_instance.h"
#include "base/message_loop/message_loop.h"
#include "base/metrics/histogram.h"
#include "base/stl_util.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/time/time.h"
#include "base/values.h"
#include "third_party/skia/include/core/SkBitmap.h"
#include "ui/gfx/image/image_skia.h"
#include "ui/gfx/native_widget_types.h"
#include "ui/shell_dialogs/selected_file_info.h"

#include "chrome/render/render_host/render_widget_host_view.h"
#include "chrome/render/render_host/render_view_host_delegate_view.h"
#include "chrome/common/render_view_host_delegate.h"

#if defined(OS_MACOSX)
#include "content/browser/renderer_host/popup_menu_helper_mac.h"
#elif defined(OS_ANDROID)
#include "content/browser/media/android/browser_media_player_manager.h"
#endif

using base::TimeDelta;


namespace content {
namespace {

// Delay to wait on closing the WebContents for a beforeunload/unload handler to
// fire.
const int kUnloadTimeoutMS = 1000;

// Translate a WebKit text direction into a base::i18n one.

base::LazyInstance<std::vector<RenderViewHost::CreatedCallback> >
g_created_callbacks = LAZY_INSTANCE_INITIALIZER;

}  // namespace

///////////////////////////////////////////////////////////////////////////////
// RenderViewHost, public:

// static
RenderViewHost* RenderViewHost::FromID(int render_process_id,
                                       int render_view_id) {
  return RenderViewHostImpl::FromID(render_process_id, render_view_id);
}

// static
RenderViewHost* RenderViewHost::From(RenderWidgetHost* rwh) {
  DCHECK(rwh->IsRenderView());
  return static_cast<RenderViewHostImpl*>(RenderWidgetHostImpl::From(rwh));
}

// static
void RenderViewHost::FilterURL(const RenderProcessHost* process,
                               bool empty_allowed,
                               GURL* url) {
//   RenderViewHostImpl::FilterURL(ChildProcessSecurityPolicyImpl::GetInstance(),
//                                 process, empty_allowed, url);
}

///////////////////////////////////////////////////////////////////////////////
// RenderViewHostImpl, public:

// static
RenderViewHostImpl* RenderViewHostImpl::FromID(int render_process_id,
                                               int render_view_id) {
  RenderWidgetHost* widget =
      RenderWidgetHost::FromID(render_process_id, render_view_id);
  if (!widget || !widget->IsRenderView())
    return NULL;
  return static_cast<RenderViewHostImpl*>(RenderWidgetHostImpl::From(widget));
}

RenderViewHostImpl::RenderViewHostImpl(
    SiteInstance* instance,
    RenderViewHostDelegate* delegate,
    RenderWidgetHostDelegate* widget_delegate,
    int routing_id,
    int main_frame_routing_id,
    bool swapped_out,
    bool hidden)
    : RenderWidgetHostImpl(widget_delegate,
                           instance->GetProcess(),
                           routing_id,
                           hidden),
      delegate_(delegate),
      instance_(static_cast<SiteInstanceImpl*>(instance)),
      waiting_for_drag_context_response_(false),
      enabled_bindings_(0),
      navigations_suspended_(false),
      has_accessed_initial_document_(false),
      is_swapped_out_(swapped_out),
      is_subframe_(false),
      main_frame_id_(-1),
      run_modal_reply_msg_(NULL),
      run_modal_opener_id_(MSG_ROUTING_NONE),
      is_waiting_for_beforeunload_ack_(false),
      is_waiting_for_unload_ack_(false),
      has_timed_out_on_unload_(false),
      unload_ack_is_for_cross_site_transition_(false),
      are_javascript_messages_suppressed_(false),
      sudden_termination_allowed_(false),
      render_view_termination_status_(base::TERMINATION_STATUS_STILL_RUNNING) {
  DCHECK(instance_.get());
  //CHECK(delegate_);  // http://crbug.com/82827

  if (main_frame_routing_id == MSG_ROUTING_NONE)
    main_frame_routing_id = GetProcess()->GetNextRoutingID();

//   main_render_frame_host_.reset(
//       new RenderFrameHostImpl(this, delegate_->GetFrameTree(),
//                               main_frame_routing_id, is_swapped_out_));

  GetProcess()->EnableSendQueue();

  for (size_t i = 0; i < g_created_callbacks.Get().size(); i++)
    g_created_callbacks.Get().at(i).Run(this);

  if (!swapped_out)
    instance_->increment_active_view_count();

#if defined(OS_ANDROID)
  media_player_manager_ = BrowserMediaPlayerManager::Create(this);
#endif
}

RenderViewHostImpl::~RenderViewHostImpl() {
//   FOR_EACH_OBSERVER(
//       RenderViewHostObserver, observers_, RenderViewHostDestruction());
// 
//   GetDelegate()->RenderViewDeleted(this);
// 
//   // Be sure to clean up any leftover state from cross-site requests.
//   CrossSiteRequestManager::GetInstance()->SetHasPendingCrossSiteRequest(
//       GetProcess()->GetID(), GetRoutingID(), false);

  // If this was swapped out, it already decremented the active view
  // count of the SiteInstance it belongs to.
  if (!is_swapped_out_)
    instance_->decrement_active_view_count();
}

RenderViewHostDelegate* RenderViewHostImpl::GetDelegate() const {
  return delegate_;
}

SiteInstance* RenderViewHostImpl::GetSiteInstance() const {
  return instance_.get();
}

bool RenderViewHostImpl::CreateRenderView(
    const string16& frame_name,
    int opener_route_id,
    int32 max_page_id, gfx::NativeView parent) {
  TRACE_EVENT0("renderer_host", "RenderViewHostImpl::CreateRenderView");
  DCHECK(!IsRenderViewLive()) << "Creating view twice";

  // The process may (if we're sharing a process with another host that already
  // initialized it) or may not (we have our own process or the old process
  // crashed) have been initialized. Calling Init multiple times will be
  // ignored, so this is safe.
  if (!GetProcess()->Init())
    return false;
  DCHECK(GetProcess()->HasConnection());
  DCHECK(GetProcess()->GetBrowserContext());

  renderer_initialized_ = true;

//   GpuSurfaceTracker::Get()->SetSurfaceHandle(
//       surface_id(), GetCompositingSurface());

  // Ensure the RenderView starts with a next_page_id larger than any existing
  // page ID it might be asked to render.
  int32 next_page_id = 1;
  if (max_page_id > -1)
    next_page_id = max_page_id + 1;

  ViewMsg_New_Params params;
  params.view_id = GetRoutingID();
  params.main_frame_routing_id = GetRoutingID();
  params.surface_id = surface_id();
  params.frame_name = frame_name;
  // Ensure the RenderView sets its opener correctly.
  params.opener_route_id = opener_route_id;
  params.swapped_out = is_swapped_out_;
  params.hidden = is_hidden();
  params.next_page_id = next_page_id;
  params.hwnd = parent;

  Send(new ViewMsg_New(params));
  return true;
}

bool RenderViewHostImpl::IsRenderViewLive() const {
  return GetProcess()->HasConnection() && renderer_initialized_;
}

bool RenderViewHostImpl::IsSubframe() const {
  return is_subframe_;
}

void RenderViewHostImpl::SyncRendererPrefs() {

}

void RenderViewHostImpl::Navigate(const ViewMsg_Navigate_Params& params) {
  Send(new ViewMsg_Navigate(GetRoutingID(), params));
}

void RenderViewHostImpl::NavigateToURL(const GURL& url) {
  ViewMsg_Navigate_Params params;
  params.url = url.spec();
//   params.page_id = -1;
//   params.pending_history_list_offset = -1;
//   params.current_history_list_offset = -1;
//   params.current_history_list_length = 0;
//   params.url = url;
//   params.transition = PAGE_TRANSITION_LINK;
//   params.navigation_type = ViewMsg_Navigate_Type::NORMAL;
  Navigate(params);
}

void RenderViewHostImpl::SetNavigationsSuspended(
    bool suspend,
    const base::TimeTicks& proceed_time) {

}

void RenderViewHostImpl::CancelSuspendedNavigations() {
  // Clear any state if a pending navigation is canceled or pre-empted.
//   if (suspended_nav_params_)
//     suspended_nav_params_.reset();
  navigations_suspended_ = false;
}

void RenderViewHostImpl::FirePageBeforeUnload(bool for_cross_site_transition) {

}

void RenderViewHostImpl::SwapOut() {
 
}

void RenderViewHostImpl::OnSwapOutACK() {
  OnSwappedOut(false);
}

void RenderViewHostImpl::OnSwappedOut(bool timed_out) {

}

void RenderViewHostImpl::WasSwappedOut() {

}

void RenderViewHostImpl::AllowBindings(int binding_flags) {

}

void RenderViewHostImpl::ClearFocusedNode() {

}

void RenderViewHostImpl::ClosePage() {

}

void RenderViewHostImpl::ClosePageIgnoringUnloadEvents() {

}

bool RenderViewHostImpl::HasPendingCrossSiteRequest() {
  return false;
}

void RenderViewHostImpl::SetHasPendingCrossSiteRequest(
    bool has_pending_request) {

}

void RenderViewHostImpl::DragTargetDragLeave() {

}

void RenderViewHostImpl::DragTargetDrop(
    const gfx::Point& client_pt,
    const gfx::Point& screen_pt,
    int key_modifiers) {
//   Send(new DragMsg_TargetDrop(GetRoutingID(), client_pt, screen_pt,
//                               key_modifiers));
}

void RenderViewHostImpl::DesktopNotificationPermissionRequestDone(
    int callback_context) {
//   Send(new DesktopNotificationMsg_PermissionRequestDone(
//       GetRoutingID(), callback_context));
}

void RenderViewHostImpl::DesktopNotificationPostDisplay(int callback_context) {
//   Send(new DesktopNotificationMsg_PostDisplay(GetRoutingID(),
//                                               callback_context));
}

void RenderViewHostImpl::DesktopNotificationPostError(int notification_id,
                                                      const string16& message) {
//   Send(new DesktopNotificationMsg_PostError(
//       GetRoutingID(), notification_id, message));
}

void RenderViewHostImpl::DesktopNotificationPostClose(int notification_id,
                                                      bool by_user) {
//   Send(new DesktopNotificationMsg_PostClose(
//       GetRoutingID(), notification_id, by_user));
}

void RenderViewHostImpl::DesktopNotificationPostClick(int notification_id) {
  //Send(new DesktopNotificationMsg_PostClick(GetRoutingID(), notification_id));
}

void RenderViewHostImpl::ExecuteJavascriptInWebFrame(
    const string16& frame_xpath,
    const string16& jscript) {
  //Send(new ViewMsg_ScriptEvalRequest(GetRoutingID(), frame_xpath, jscript,
  //                                   0, false));
}

void RenderViewHostImpl::ExecuteJavascriptInWebFrameCallbackResult(
      const string16& frame_xpath,
      const string16& jscript,
     const JavascriptResultCallback& callback) {
}
//   static int next_id = 1;
//   int key = next_id++;
//   Send(new ViewMsg_ScriptEvalRequest(GetRoutingID(), frame_xpath, jscript,
//                                      key, true));
//   javascript_callbacks_.insert(std::make_pair(key, callback));
// }

// void RenderViewHostImpl::JavaScriptDialogClosed(IPC::Message* reply_msg,
//                                                 bool success,
//                                                 const string16& user_input) {
//   GetProcess()->SetIgnoreInputEvents(false);
//   bool is_waiting =
//       is_waiting_for_beforeunload_ack_ || is_waiting_for_unload_ack_;
// 
//   // If we are executing as part of (before)unload event handling, we don't
//   // want to use the regular hung_renderer_delay_ms_ if the user has agreed to
//   // leave the current page. In this case, use the regular timeout value used
//   // during the (before)unload handling.
//   if (is_waiting) {
//     StartHangMonitorTimeout(TimeDelta::FromMilliseconds(
//         success ? kUnloadTimeoutMS : hung_renderer_delay_ms_));
//   }
// 
//   ViewHostMsg_RunJavaScriptMessage::WriteReplyParams(reply_msg,
//                                                      success, user_input);
//   Send(reply_msg);
// 
//   // If we are waiting for an unload or beforeunload ack and the user has
//   // suppressed messages, kill the tab immediately; a page that's spamming
//   // alerts in onbeforeunload is presumably malicious, so there's no point in
//   // continuing to run its script and dragging out the process.
//   // This must be done after sending the reply since RenderView can't close
//   // correctly while waiting for a response.
//   if (is_waiting && are_javascript_messages_suppressed_)
//     delegate_->RendererUnresponsive(
//         this, is_waiting_for_beforeunload_ack_, is_waiting_for_unload_ack_);
// }

// void RenderViewHostImpl::DragSourceEndedAt(
//     int client_x, int client_y, int screen_x, int screen_y,
//     WebDragOperation operation) {
//   Send(new DragMsg_SourceEndedOrMoved(
//       GetRoutingID(),
//       gfx::Point(client_x, client_y),
//       gfx::Point(screen_x, screen_y),
//       true, operation));
// }
// 
// void RenderViewHostImpl::DragSourceMovedTo(
//     int client_x, int client_y, int screen_x, int screen_y) {
//   Send(new DragMsg_SourceEndedOrMoved(
//       GetRoutingID(),
//       gfx::Point(client_x, client_y),
//       gfx::Point(screen_x, screen_y),
//       false, WebDragOperationNone));
// }
// 
// void RenderViewHostImpl::DragSourceSystemDragEnded() {
//   Send(new DragMsg_SourceSystemDragEnded(GetRoutingID()));
// }

// void RenderViewHostImpl::AllowBindings(int bindings_flags) {
//   // Never grant any bindings to browser plugin guests.
//   if (GetProcess()->IsGuest()) {
//     NOTREACHED() << "Never grant bindings to a guest process.";
//     return;
//   }
// 
//   // Ensure we aren't granting WebUI bindings to a process that has already
//   // been used for non-privileged views.
//   if (bindings_flags & BINDINGS_POLICY_WEB_UI &&
//       GetProcess()->HasConnection() &&
//       !ChildProcessSecurityPolicyImpl::GetInstance()->HasWebUIBindings(
//           GetProcess()->GetID())) {
//     // This process has no bindings yet. Make sure it does not have more
//     // than this single active view.
//     RenderProcessHostImpl* process =
//         static_cast<RenderProcessHostImpl*>(GetProcess());
//     if (process->GetActiveViewCount() > 1)
//       return;
//   }
// 
//   if (bindings_flags & BINDINGS_POLICY_WEB_UI) {
//     ChildProcessSecurityPolicyImpl::GetInstance()->GrantWebUIBindings(
//         GetProcess()->GetID());
//   }
// 
//   enabled_bindings_ |= bindings_flags;
//   if (renderer_initialized_)
//     Send(new ViewMsg_AllowBindings(GetRoutingID(), enabled_bindings_));
// }

int RenderViewHostImpl::GetEnabledBindings() const {
  return enabled_bindings_;
}

void RenderViewHostImpl::SetWebUIProperty(const std::string& name,
                                          const std::string& value) {
  // This is a sanity check before telling the renderer to enable the property.
  // It could lie and send the corresponding IPC messages anyway, but we will
  // not act on them if enabled_bindings_ doesn't agree. If we get here without
  // WebUI bindings, kill the renderer process.
//   if (enabled_bindings_ & BINDINGS_POLICY_WEB_UI) {
//     Send(new ViewMsg_SetWebUIProperty(GetRoutingID(), name, value));
//   } else {
//     RecordAction(UserMetricsAction("BindingsMismatchTerminate_RVH_WebUI"));
//     base::KillProcess(
//         GetProcess()->GetHandle(), content::RESULT_CODE_KILLED, false);
//   }
}

void RenderViewHostImpl::GotFocus() {
  RenderWidgetHostImpl::GotFocus();  // Notifies the renderer it got focus.

  RenderViewHostDelegateView* view = delegate_->GetDelegateView();
  if (view)
    view->GotFocus();
}

void RenderViewHostImpl::LostCapture() {
  RenderWidgetHostImpl::LostCapture();
  //delegate_->LostCapture();
}

void RenderViewHostImpl::LostMouseLock() {
  RenderWidgetHostImpl::LostMouseLock();
  //delegate_->LostMouseLock();
}

void RenderViewHostImpl::SetInitialFocus(bool reverse) {
  //Send(new ViewMsg_SetInitialFocus(GetRoutingID(), reverse));
}

// void RenderViewHostImpl::FilesSelectedInChooser(
//     const std::vector<ui::SelectedFileInfo>& files,
//     FileChooserParams::Mode permissions) {
//   // Grant the security access requested to the given files.
//   for (size_t i = 0; i < files.size(); ++i) {
//     const ui::SelectedFileInfo& file = files[i];
//     if (permissions == FileChooserParams::Save) {
//       ChildProcessSecurityPolicyImpl::GetInstance()->GrantCreateReadWriteFile(
//           GetProcess()->GetID(), file.local_path);
//     } else {
//       ChildProcessSecurityPolicyImpl::GetInstance()->GrantReadFile(
//           GetProcess()->GetID(), file.local_path);
//     }
//   }
//   Send(new ViewMsg_RunFileChooserResponse(GetRoutingID(), files));
// }

void RenderViewHostImpl::DirectoryEnumerationFinished(
    int request_id,
    const std::vector<base::FilePath>& files) {
  // Grant the security access requested to the given files.
//   for (std::vector<base::FilePath>::const_iterator file = files.begin();
//        file != files.end(); ++file) {
//     ChildProcessSecurityPolicyImpl::GetInstance()->GrantReadFile(
//         GetProcess()->GetID(), *file);
//   }
//   Send(new ViewMsg_EnumerateDirectoryResponse(GetRoutingID(),
//                                               request_id,
//                                               files));
}

// void RenderViewHostImpl::LoadStateChanged(
//     const GURL& url,
//     const net::LoadStateWithParam& load_state,
//     uint64 upload_position,
//     uint64 upload_size) {
//   delegate_->LoadStateChanged(url, load_state, upload_position, upload_size);
// }

bool RenderViewHostImpl::SuddenTerminationAllowed() const {
  return sudden_termination_allowed_ ||
      GetProcess()->SuddenTerminationAllowed();
}

///////////////////////////////////////////////////////////////////////////////
// RenderViewHostImpl, IPC message handlers:

bool RenderViewHostImpl::OnMessageReceived(const IPC::Message& msg) {
  // TODO(jochen): Consider removing message handlers that only add a this
  // pointer and forward the messages to the RenderViewHostDelegate. The
  // respective delegates can handle the messages themselves in their
  // OnMessageReceived implementation.
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(RenderViewHostImpl, msg)
    IPC_MESSAGE_HANDLER(ViewMsg_CreateNewWindow, OnCreateNewWindow)
    IPC_MESSAGE_HANDLER(ViewMsg_URLCHANGE, OnUrlChange)
    IPC_MESSAGE_HANDLER(ViewMsg_TITLECHANGE, OnTitleChange)
    IPC_MESSAGE_HANDLER(ViewHostMsg_Focus, OnGotFocus)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()

    if (handled)
      return true;
  bool msg_is_ok = true;

  return handled;
}

void RenderViewHostImpl::Init() {
  RenderWidgetHostImpl::Init();
  //main_render_frame_host()->Init();
}

void RenderViewHostImpl::Shutdown() {
  // If we are being run modally (see RunModal), then we need to cleanup.
  if (run_modal_reply_msg_) {
    Send(run_modal_reply_msg_);
    run_modal_reply_msg_ = NULL;
    RenderViewHostImpl* opener =
        RenderViewHostImpl::FromID(GetProcess()->GetID(), run_modal_opener_id_);
    if (opener) {
      opener->StartHangMonitorTimeout(TimeDelta::FromMilliseconds(
          hung_renderer_delay_ms_));
      // Balance out the decrement when we got created.
      opener->increment_in_flight_event_count();
    }
    run_modal_opener_id_ = MSG_ROUTING_NONE;
  }

  RenderWidgetHostImpl::Shutdown();
}

bool RenderViewHostImpl::IsRenderView() const {
  return true;
}

void RenderViewHostImpl::CreateNewWindow(
    int route_id,
    int main_frame_route_id,
    const ViewHostMsg_CreateWindow_Params& params,
    SessionStorageNamespace* session_storage_namespace) {
//   ViewHostMsg_CreateWindow_Params validated_params(params);
//   ChildProcessSecurityPolicyImpl* policy =
//       ChildProcessSecurityPolicyImpl::GetInstance();
//   FilterURL(policy, GetProcess(), false, &validated_params.target_url);
//   FilterURL(policy, GetProcess(), false, &validated_params.opener_url);
//   FilterURL(policy, GetProcess(), true,
//             &validated_params.opener_security_origin);
// 
//   delegate_->CreateNewWindow(route_id, main_frame_route_id,
//                              validated_params, session_storage_namespace);
}

// void RenderViewHostImpl::CreateNewWidget(int route_id,
//                                      WebKit::WebPopupType popup_type) {
//   delegate_->CreateNewWidget(route_id, popup_type);
// }

void RenderViewHostImpl::CreateNewFullscreenWidget(int route_id) {
  //delegate_->CreateNewFullscreenWidget(route_id);
}

// void RenderViewHostImpl::OnShowView(int route_id,
//   WindowOpenDisposition disposition,
//   const gfx::Rect& initial_pos,
//   bool user_gesture) {
//     if (!is_swapped_out_) {
//       delegate_->ShowCreatedWindow(
//         route_id, disposition, initial_pos, user_gesture);
//     }
//     Send(new ViewMsg_Move_ACK(route_id));
// }

void RenderViewHostImpl::OnShowWidget(int route_id,
                                      const gfx::Rect& initial_pos) {
//   if (!is_swapped_out_)
//     delegate_->ShowCreatedWidget(route_id, initial_pos);
//   Send(new ViewMsg_Move_ACK(route_id));
}

void RenderViewHostImpl::OnShowFullscreenWidget(int route_id) {
//   if (!is_swapped_out_)
//     delegate_->ShowCreatedFullscreenWidget(route_id);
//   Send(new ViewMsg_Move_ACK(route_id));
}

void RenderViewHostImpl::OnRunModal(int opener_id, IPC::Message* reply_msg) {

}

void RenderViewHostImpl::OnRenderViewReady() {

}

void RenderViewHostImpl::OnRenderProcessGone(int status, int exit_code) {
//   // Keep the termination status so we can get at it later when we
//   // need to know why it died.
//   render_view_termination_status_ =
//       static_cast<base::TerminationStatus>(status);
// 
//   // Reset state.
//   main_frame_id_ = -1;
// 
//   // Our base class RenderWidgetHost needs to reset some stuff.
//   RendererExited(render_view_termination_status_, exit_code);
// 
//   delegate_->RenderViewTerminated(this,
//                                   static_cast<base::TerminationStatus>(status),
//                                   exit_code);
}

void RenderViewHostImpl::OnDidStartProvisionalLoadForFrame(
    int64 frame_id,
    int64 parent_frame_id,
    bool is_main_frame,
    const GURL& url) {
//   delegate_->DidStartProvisionalLoadForFrame(
//       this, frame_id, parent_frame_id, is_main_frame, url);
}

void RenderViewHostImpl::OnDidRedirectProvisionalLoad(
    int32 page_id,
    const GURL& source_url,
    const GURL& target_url) {
//   delegate_->DidRedirectProvisionalLoad(
//       this, page_id, source_url, target_url);
}

// void RenderViewHostImpl::OnDidFailProvisionalLoadWithError(
//     const ViewHostMsg_DidFailProvisionalLoadWithError_Params& params) {
//   //delegate_->DidFailProvisionalLoadWithError(this, params);
// }

// Called when the renderer navigates.  For every frame loaded, we'll get this
// notification containing parameters identifying the navigation.
//
// Subframes are identified by the page transition type.  For subframes loaded
// as part of a wider page load, the page_id will be the same as for the top
// level frame.  If the user explicitly requests a subframe navigation, we will
// get a new page_id because we need to create a new navigation entry for that
// action.
void RenderViewHostImpl::OnNavigate(const IPC::Message& msg) {

}

void RenderViewHostImpl::OnUpdateState(int32 page_id, const PageState& state) {
//   Without this check, the renderer can trick the browser into using
//     // filenames it can't access in a future session restore.
//     if (!CanAccessFilesOfPageState(state)) {
//       GetProcess()->ReceivedBadMessage();
//       return;
//     }
//   
//     delegate_->UpdateState(this, page_id, state);
}

// void RenderViewHostImpl::OnUpdateTitle(
//     int32 page_id,
//     const string16& title,
//     WebKit::WebTextDirection title_direction) {
//   if (title.length() > kMaxTitleChars) {
//     NOTREACHED() << "Renderer sent too many characters in title.";
//     return;
//   }
// 
//   delegate_->UpdateTitle(this, page_id, title,
//                          WebTextDirectionToChromeTextDirection(
//                              title_direction));
// }

void RenderViewHostImpl::OnUpdateEncoding(const std::string& encoding_name) {

}

void RenderViewHostImpl::OnUpdateTargetURL(int32 page_id, const GURL& url) {

}

void RenderViewHostImpl::OnUpdateInspectorSetting(
    const std::string& key, const std::string& value) {

}

void RenderViewHostImpl::OnClose() {
  // If the renderer is telling us to close, it has already run the unload
  // events, and we can take the fast path.
  ClosePageIgnoringUnloadEvents();
}

void RenderViewHostImpl::OnRequestMove(const gfx::Rect& pos) {

}

void RenderViewHostImpl::OnDidStartLoading() {
  //delegate_->DidStartLoading(this);
}

void RenderViewHostImpl::OnDidStopLoading() {
  //delegate_->DidStopLoading(this);
}

void RenderViewHostImpl::OnDidChangeLoadProgress(double load_progress) {
  //delegate_->DidChangeLoadProgress(load_progress);
}

void RenderViewHostImpl::OnDidDisownOpener() {
  //delegate_->DidDisownOpener(this);
}

void RenderViewHostImpl::OnDocumentAvailableInMainFrame() {
  //delegate_->DocumentAvailableInMainFrame(this);
}

void RenderViewHostImpl::OnDocumentOnLoadCompletedInMainFrame(
    int32 page_id) {
  //delegate_->DocumentOnLoadCompletedInMainFrame(this, page_id);
}

// void RenderViewHostImpl::OnContextMenu(const ContextMenuParams& params) {
//  
// }

void RenderViewHostImpl::OnToggleFullscreen(bool enter_fullscreen) {
//   DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
//   delegate_->ToggleFullscreenMode(enter_fullscreen);
//   // We need to notify the contents that its fullscreen state has changed. This
//   // is done as part of the resize message.
//   WasResized();
}

// void RenderViewHostImpl::OnOpenURL(
//     const ViewHostMsg_OpenURL_Params& params) {
// //   GURL validated_url(params.url);
// //   FilterURL(ChildProcessSecurityPolicyImpl::GetInstance(),
// //             GetProcess(), false, &validated_url);
// // 
// //   delegate_->RequestOpenURL(
// //       this, validated_url, params.referrer, params.disposition, params.frame_id,
// //       params.should_replace_current_entry, params.user_gesture);
// }

void RenderViewHostImpl::OnDidContentsPreferredSizeChange(
    const gfx::Size& new_size) {
  //delegate_->UpdatePreferredSize(new_size);
}

void RenderViewHostImpl::OnRenderAutoResized(const gfx::Size& new_size) {
  //delegate_->ResizeDueToAutoResize(new_size);
}

// void RenderViewHostImpl::OnDidChangeScrollOffset() {
//   if (view_)
//     view_->ScrollOffsetChanged();
// }

void RenderViewHostImpl::OnDidChangeScrollbarsForMainFrame(
    bool has_horizontal_scrollbar, bool has_vertical_scrollbar) {
//   if (view_)
//     view_->SetHasHorizontalScrollbar(has_horizontal_scrollbar);
}

void RenderViewHostImpl::OnDidChangeScrollOffsetPinningForMainFrame(
    bool is_pinned_to_left, bool is_pinned_to_right) {
//   if (view_)
//     view_->SetScrollOffsetPinning(is_pinned_to_left, is_pinned_to_right);
}

void RenderViewHostImpl::OnDidChangeNumWheelEvents(int count) {
}

void RenderViewHostImpl::OnSelectionChanged(const string16& text,
                                            size_t offset,
                                            const gfx::Range& range) {
//   if (view_)
//     view_->SelectionChanged(text, offset, range);
}

// void RenderViewHostImpl::OnSelectionBoundsChanged(
//     const ViewHostMsg_SelectionBounds_Params& params) {
// //   if (view_) {
// //     view_->SelectionBoundsChanged(params);
// //   }
// }

void RenderViewHostImpl::OnRouteCloseEvent() {
  // Have the delegate route this to the active RenderViewHost.
  //delegate_->RouteCloseEvent(this);
}

// void RenderViewHostImpl::OnRouteMessageEvent(
//     const ViewMsg_PostMessage_Params& params) {
//   // Give to the delegate to route to the active RenderViewHost.
//   //delegate_->RouteMessageEvent(this, params);
// }

// void RenderViewHostImpl::OnRunJavaScriptMessage(
//     const string16& message,
//     const string16& default_prompt,
//     const GURL& frame_url,
//     JavaScriptMessageType type,
//     IPC::Message* reply_msg) {
//   // While a JS message dialog is showing, tabs in the same process shouldn't
//   // process input events.
//   GetProcess()->SetIgnoreInputEvents(true);
//   StopHangMonitorTimeout();
//   delegate_->RunJavaScriptMessage(this, message, default_prompt, frame_url,
//                                   type, reply_msg,
//                                   &are_javascript_messages_suppressed_);
// }

void RenderViewHostImpl::OnRunBeforeUnloadConfirm(const GURL& frame_url,
                                                  const string16& message,
                                                  bool is_reload,
                                                  IPC::Message* reply_msg) {
  // While a JS before unload dialog is showing, tabs in the same process
  // shouldn't process input events.
  GetProcess()->SetIgnoreInputEvents(true);
  StopHangMonitorTimeout();
  //delegate_->RunBeforeUnloadConfirm(this, message, is_reload, reply_msg);
}

void RenderViewHostImpl::OnAddMessageToConsole(
    int32 level,
    const string16& message,
    int32 line_no,
    const string16& source_id) {
//   if (delegate_->AddMessageToConsole(level, message, line_no, source_id))
//     return;
// 
//   // Pass through log level only on WebUI pages to limit console spew.
//   int32 resolved_level = HasWebUIScheme(delegate_->GetURL()) ? level : 0;
// 
//   if (resolved_level >= ::logging::GetMinLogLevel()) {
//     logging::LogMessage("CONSOLE", line_no, resolved_level).stream() << "\"" <<
//         message << "\", source: " << source_id << " (" << line_no << ")";
//   }
}

// void RenderViewHostImpl::AddObserver(RenderViewHostObserver* observer) {
//   observers_.AddObserver(observer);
// }
// 
// void RenderViewHostImpl::RemoveObserver(RenderViewHostObserver* observer) {
//   observers_.RemoveObserver(observer);
// }

void RenderViewHostImpl::OnUserGesture() {
  //delegate_->OnUserGesture();
}

void RenderViewHostImpl::OnShouldCloseACK(
    bool proceed,
    const base::TimeTicks& renderer_before_unload_start_time,
    const base::TimeTicks& renderer_before_unload_end_time) {
  decrement_in_flight_event_count();
  StopHangMonitorTimeout();

}

void RenderViewHostImpl::OnClosePageACK() {
  decrement_in_flight_event_count();
  ClosePageIgnoringUnloadEvents();
}

void RenderViewHostImpl::NotifyRendererUnresponsive() {
  //delegate_->RendererUnresponsive(
  //    this, is_waiting_for_beforeunload_ack_, is_waiting_for_unload_ack_);
}

void RenderViewHostImpl::NotifyRendererResponsive() {
  //delegate_->RendererResponsive(this);
}

void RenderViewHostImpl::RequestToLockMouse(bool user_gesture,
                                            bool last_unlocked_by_target) {
  //delegate_->RequestToLockMouse(user_gesture, last_unlocked_by_target);
}

bool RenderViewHostImpl::IsFullscreen() const {
  return false;//delegate_->IsFullscreenForCurrentTab();
}

void RenderViewHostImpl::OnFocus() {
  // Note: We allow focus and blur from swapped out RenderViewHosts, even when
  // the active RenderViewHost is in a different BrowsingInstance (e.g., WebUI).
  //delegate_->Activate();
}

void RenderViewHostImpl::OnBlur() {
  //delegate_->Deactivate();
}

gfx::Rect RenderViewHostImpl::GetRootWindowResizerRect() const {
  return gfx::Rect();//delegate_->GetRootWindowResizerRect();
}

void RenderViewHostImpl::OnPointerEventActivate() {
  //delegate_->HandlePointerActivate();
}
// 
// void RenderViewHostImpl::ForwardKeyboardEvent(
//     const NativeWebKeyboardEvent& key_event) {
//   if (ignore_input_events()) {
//     if (key_event.type == WebInputEvent::RawKeyDown)
//       delegate_->OnIgnoredUIEvent();
//     return;
//   }
//   RenderWidgetHostImpl::ForwardKeyboardEvent(key_event);
// }

void RenderViewHostImpl::SendOrientationChangeEvent(int orientation) {
  //Send(new ViewMsg_OrientationChangeEvent(GetRoutingID(), orientation));
}

void RenderViewHostImpl::ToggleSpeechInput() {
  //Send(new InputTagSpeechMsg_ToggleSpeechInput(GetRoutingID()));
}

bool RenderViewHostImpl::CanCommitURL(const GURL& url) {
  // TODO(creis): We should also check for WebUI pages here.  Also, when the
  // out-of-process iframes implementation is ready, we should check for
  // cross-site URLs that are not allowed to commit in this process.

  // Give the client a chance to disallow URLs from committing.
  return false;//GetContentClient()->browser()->CanCommitURL(GetProcess(), url);
}

void RenderViewHostImpl::FilterURL(ChildProcessSecurityPolicyImpl* policy,
                                   const RenderProcessHost* process,
                                   bool empty_allowed,
                                   GURL* url) {
  if (empty_allowed && url->is_empty())
    return;
}

void RenderViewHost::AddCreatedCallback(const CreatedCallback& callback) {
  g_created_callbacks.Get().push_back(callback);
}

void RenderViewHost::RemoveCreatedCallback(const CreatedCallback& callback) {
  for (size_t i = 0; i < g_created_callbacks.Get().size(); ++i) {
    if (g_created_callbacks.Get().at(i).Equals(callback)) {
      g_created_callbacks.Get().erase(g_created_callbacks.Get().begin() + i);
      return;
    }
  }
}

void RenderViewHostImpl::SetAltErrorPageURL(const GURL& url) {
  //Send(new ViewMsg_SetAltErrorPageURL(GetRoutingID(), url));
}

void RenderViewHostImpl::ExitFullscreen() {
  RejectMouseLockOrUnlockIfNecessary();
  // Notify delegate_ and renderer of fullscreen state change.
  OnToggleFullscreen(false);
}

// WebPreferences RenderViewHostImpl::GetWebkitPreferences() {
//   return delegate_->GetWebkitPrefs();
// }

void RenderViewHostImpl::DisownOpener() {
  // This should only be called when swapped out.
  DCHECK(is_swapped_out_);

  //Send(new ViewMsg_DisownOpener(GetRoutingID()));
}
// 
// void RenderViewHostImpl::SetAccessibilityCallbackForTesting(
//     const base::Callback<void(WebKit::WebAXEvent)>& callback) {
//   accessibility_testing_callback_ = callback;
// }

// void RenderViewHostImpl::UpdateWebkitPreferences(const WebPreferences& prefs) {
//   Send(new ViewMsg_UpdateWebPreferences(GetRoutingID(), prefs));
// }
// 
void RenderViewHostImpl::NotifyTimezoneChange() {
//   Send(new ViewMsg_TimezoneChange(GetRoutingID()));
}


void RenderViewHostImpl::SetZoomLevel(double level) {
//   Send(new ViewMsg_SetZoomLevel(GetRoutingID(), level));
}

void RenderViewHostImpl::ReloadFrame() {
//   Send(new ViewMsg_ReloadFrame(GetRoutingID()));
 }

void RenderViewHostImpl::Find(int request_id,
                              const string16& search_text,
                              const WebKit::WebFindOptions& options) {
  //Send(new ViewMsg_Find(GetRoutingID(), request_id, search_text, options));
}

void RenderViewHostImpl::InsertCSS(const string16& frame_xpath,
                                   const std::string& css) {
  //Send(new ViewMsg_CSSInsertRequest(GetRoutingID(), frame_xpath, css));
}

void RenderViewHostImpl::DisableScrollbarsForThreshold(const gfx::Size& size) {
  //Send(new ViewMsg_DisableScrollbarsForSmallWindows(GetRoutingID(), size));
}

void RenderViewHostImpl::EnablePreferredSizeMode() {
  //Send(new ViewMsg_EnablePreferredSizeChangedMode(GetRoutingID()));
}

void RenderViewHostImpl::EnableAutoResize(const gfx::Size& min_size,
                                          const gfx::Size& max_size) {
  SetShouldAutoResize(true);
  //Send(new ViewMsg_EnableAutoResize(GetRoutingID(), min_size, max_size));
}

void RenderViewHostImpl::DisableAutoResize(const gfx::Size& new_size) {
  SetShouldAutoResize(false);
  //Send(new ViewMsg_DisableAutoResize(GetRoutingID(), new_size));
}

void RenderViewHostImpl::ExecuteCustomContextMenuCommand(
    int action, const CustomContextMenuContext& context) {
  //Send(new ViewMsg_CustomContextMenuAction(GetRoutingID(), context, action));
}

void RenderViewHostImpl::NotifyContextMenuClosed(
    const CustomContextMenuContext& context) {
  //Send(new ViewMsg_ContextMenuClosed(GetRoutingID(), context));
}

void RenderViewHostImpl::CopyImageAt(int x, int y) {
  //Send(new ViewMsg_CopyImageAt(GetRoutingID(), x, y));
}

void RenderViewHostImpl::ExecuteMediaPlayerActionAtLocation(
  const gfx::Point& location, const WebKit::WebMediaPlayerAction& action) {
  //Send(new ViewMsg_MediaPlayerActionAt(GetRoutingID(), location, action));
}

void RenderViewHostImpl::ExecutePluginActionAtLocation(
  const gfx::Point& location, const WebKit::WebPluginAction& action) {
  //Send(new ViewMsg_PluginActionAt(GetRoutingID(), location, action));
}

void RenderViewHostImpl::NotifyMoveOrResizeStarted() {
  //Send(new ViewMsg_MoveOrResizeStarted(GetRoutingID()));
}

// void RenderViewHostImpl::StopFinding(StopFindAction action) {
//   //Send(new ViewMsg_StopFinding(GetRoutingID(), action));
// }

// void RenderViewHostImpl::OnAccessibilityEvents(
//     const std::vector<AccessibilityHostMsg_EventParams>& params) {
// //   if (view_ && !is_swapped_out_)
// //     view_->OnAccessibilityEvents(params);
// // 
// //   // Always send an ACK or the renderer can be in a bad state.
// //   Send(new AccessibilityMsg_Events_ACK(GetRoutingID()));
// // 
// //   // The rest of this code is just for testing; bail out if we're not
// //   // in that mode.
// //   if (accessibility_testing_callback_.is_null())
// //     return;
// // 
// //   for (unsigned i = 0; i < params.size(); i++) {
// //     const AccessibilityHostMsg_EventParams& param = params[i];
// //     WebKit::WebAXEvent src_type = param.event_type;
// //     if (src_type == WebKit::WebAXEventLayoutComplete ||
// //         src_type == WebKit::WebAXEventLoadComplete) {
// //       MakeAccessibilityNodeDataTree(param.nodes, &accessibility_tree_);
// //     }
// //     accessibility_testing_callback_.Run(src_type);
// //   }
// }

void RenderViewHostImpl::OnScriptEvalResponse(int id,
                                              const base::ListValue& result) {
  const base::Value* result_value;
  if (!result.Get(0, &result_value)) {
    // Programming error or rogue renderer.
    NOTREACHED() << "Got bad arguments for OnScriptEvalResponse";
    return;
  }

  std::map<int, JavascriptResultCallback>::iterator it =
      javascript_callbacks_.find(id);
  if (it != javascript_callbacks_.end()) {
    // ExecuteJavascriptInWebFrameCallbackResult was used; do callback.
    it->second.Run(result_value);
    javascript_callbacks_.erase(it);
  } else {
    NOTREACHED() << "Received script response for unknown request";
  }
}

void RenderViewHostImpl::OnDidZoomURL(double zoom_level,
                                      bool remember,
                                      const GURL& url) {
//   HostZoomMapImpl* host_zoom_map = static_cast<HostZoomMapImpl*>(
//       HostZoomMap::GetForBrowserContext(GetProcess()->GetBrowserContext()));
//   if (remember) {
//     host_zoom_map->
//         SetZoomLevelForHost(net::GetHostOrSpecFromURL(url), zoom_level);
//   } else {
//     host_zoom_map->SetTemporaryZoomLevel(
//         GetProcess()->GetID(), GetRoutingID(), zoom_level);
//   }
}

void RenderViewHostImpl::OnRequestDesktopNotificationPermission(
    const GURL& source_origin, int callback_context) {
//   GetContentClient()->browser()->RequestDesktopNotificationPermission(
//       source_origin, callback_context, GetProcess()->GetID(), GetRoutingID());
}

// void RenderViewHostImpl::OnShowDesktopNotification(
//     const ShowDesktopNotificationHostMsgParams& params) {
//   // Disallow HTML notifications from javascript: and file: schemes as this
//   // allows unwanted cross-domain access.
// //   GURL url = params.contents_url;
// //   if (params.is_html &&
// //       (url.SchemeIs(kJavaScriptScheme) ||
// //        url.SchemeIs(chrome::kFileScheme))) {
// //     return;
// //   }
// // 
// //   GetContentClient()->browser()->ShowDesktopNotification(
// //       params, GetProcess()->GetID(), GetRoutingID(), false);
// }

void RenderViewHostImpl::OnCancelDesktopNotification(int notification_id) {
//   GetContentClient()->browser()->CancelDesktopNotification(
//       GetProcess()->GetID(), GetRoutingID(), notification_id);
}

// void RenderViewHostImpl::OnRunFileChooser(const FileChooserParams& params) {
//   //delegate_->RunFileChooser(this, params);
// }

void RenderViewHostImpl::OnDidAccessInitialDocument() {
  has_accessed_initial_document_ = true;
  //delegate_->DidAccessInitialDocument();
}

void RenderViewHostImpl::OnDomOperationResponse(
    const std::string& json_string, int automation_id) {
//   DomOperationNotificationDetails details(json_string, automation_id);
//   NotificationService::current()->Notify(
//       NOTIFICATION_DOM_OPERATION_RESPONSE,
//       Source<RenderViewHost>(this),
//       Details<DomOperationNotificationDetails>(&details));
}

#if defined(OS_MACOSX) || defined(OS_ANDROID)
void RenderViewHostImpl::OnShowPopup(
    const ViewHostMsg_ShowPopup_Params& params) {
  RenderViewHostDelegateView* view = delegate_->GetDelegateView();
  if (view) {
    view->ShowPopupMenu(params.bounds,
                        params.item_height,
                        params.item_font_size,
                        params.selected_item,
                        params.popup_items,
                        params.right_aligned,
                        params.allow_multiple_selection);
  }
}
#endif

void RenderViewHostImpl::SetSwappedOut(bool is_swapped_out) {
  // We update the number of RenderViews in a SiteInstance when the
  // swapped out status of this RenderView gets flipped.
  if (is_swapped_out_ && !is_swapped_out)
    instance_->increment_active_view_count();
  else if (!is_swapped_out_ && is_swapped_out)
    instance_->decrement_active_view_count();

  is_swapped_out_ = is_swapped_out;

  // Whenever we change swap out state, we should not be waiting for
  // beforeunload or unload acks.  We clear them here to be safe, since they
  // can cause navigations to be ignored in OnNavigate.
  is_waiting_for_beforeunload_ack_ = false;
  is_waiting_for_unload_ack_ = false;
  has_timed_out_on_unload_ = false;
}

bool RenderViewHostImpl::CanAccessFilesOfPageState(
    const PageState& state) const {
//   ChildProcessSecurityPolicyImpl* policy =
//       ChildProcessSecurityPolicyImpl::GetInstance();
// 
//   const std::vector<base::FilePath>& file_paths = state.GetReferencedFiles();
//   for (std::vector<base::FilePath>::const_iterator file = file_paths.begin();
//        file != file_paths.end(); ++file) {
//     if (!policy->CanReadFile(GetProcess()->GetID(), *file))
//       return false;
//   }
  return true;
}

void RenderViewHostImpl::AttachToFrameTree() {
//   FrameTree* frame_tree = delegate_->GetFrameTree();
// 
//   frame_tree->SwapMainFrame(main_render_frame_host_.get());
//   if (main_frame_id() != FrameTreeNode::kInvalidFrameId) {
//     frame_tree->OnFirstNavigationAfterSwap(main_frame_id());
//   }
}

void RenderViewHostImpl::OnCreateNewWindow(int browserid) {
  if(delegate_)
    delegate_->CreateNewWindow(browserid);
}

void RenderViewHostImpl::OnUrlChange(int pageid, const string16& url) {
   if(delegate_)
     delegate_->UpdateUrl(this, pageid, url);
}

void RenderViewHostImpl::OnTitleChange(int pageid, const string16& title) {
  if(delegate_)
    delegate_->UpdateTitle(this, pageid, title);
}

void RenderViewHostImpl::OnGotFocus() {
  GetView()->Focus();
}

}  // namespace content
