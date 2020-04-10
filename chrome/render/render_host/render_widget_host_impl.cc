// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/render/render_host/render_widget_host_impl.h"

#include <math.h>
#include <set>
#include <utility>

#include "base/auto_reset.h"
#include "base/bind.h"
#include "base/command_line.h"
#include "base/containers/hash_tables.h"
#include "base/debug/trace_event.h"
#include "base/lazy_instance.h"
#include "base/message_loop/message_loop.h"
#include "base/metrics/field_trial.h"
#include "base/metrics/histogram.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/utf_string_conversions.h"
#include "base/logging.h"
#include "skia/ext/image_operations.h"
#include "skia/ext/platform_canvas.h"
#include "ui/events/event.h"
#include "ui/events/keycodes/keyboard_codes.h"
#include "ui/gfx/size_conversions.h"
#include "ui/gfx/skbitmap_operations.h"
#include "ui/gfx/vector2d_conversions.h"
#include "ui/snapshot/snapshot.h"
#include "chrome/common/render_process_host.h"
#include "chrome/common/view_messages.h"
#include "chrome/browser/browser_thread.h"
#include "chrome/render/render_host/render_widget_host_view_port.h"

#if defined(TOOLKIT_GTK)
#include "content/browser/renderer_host/backing_store_gtk.h"
#elif defined(OS_MACOSX)
#include "content/browser/renderer_host/backing_store_mac.h"
#elif defined(OS_WIN)

#endif

using base::Time;
using base::TimeDelta;
using base::TimeTicks;

namespace content {
namespace {

bool g_check_for_pending_resize_ack = true;

// How long to (synchronously) wait for the renderer to respond with a
// PaintRect message, when our backing-store is invalid, before giving up and
// returning a null or incorrectly sized backing-store from GetBackingStore.
// This timeout impacts the "choppiness" of our window resize perf.
const int kPaintMsgTimeoutMS = 50;

base::LazyInstance<std::vector<RenderWidgetHost::CreatedCallback> >
g_created_callbacks = LAZY_INSTANCE_INITIALIZER;

typedef std::pair<int32, int32> RenderWidgetHostID;
typedef base::unordered_map<RenderWidgetHostID, RenderWidgetHostImpl*>
    RoutingIDWidgetMap;

base::LazyInstance<RoutingIDWidgetMap> g_routing_id_widget_map =
    LAZY_INSTANCE_INITIALIZER;

// Implements the RenderWidgetHostIterator interface. It keeps a list of
// RenderWidgetHosts, and makes sure it returns a live RenderWidgetHost at each
// iteration (or NULL if there isn't any left).
// class RenderWidgetHostIteratorImpl : public RenderWidgetHostIterator {
//  public:
//   RenderWidgetHostIteratorImpl()
//       : current_index_(0) {
//   }
// 
//   virtual ~RenderWidgetHostIteratorImpl() {
//   }
// 
//   void Add(RenderWidgetHost* host) {
//     hosts_.push_back(RenderWidgetHostID(host->GetProcess()->GetID(),
//                                         host->GetRoutingID()));
//   }
// 
//   // RenderWidgetHostIterator:
//   virtual RenderWidgetHost* GetNextHost() OVERRIDE {
//     RenderWidgetHost* host = NULL;
//     while (current_index_ < hosts_.size() && !host) {
//       RenderWidgetHostID id = hosts_[current_index_];
//       host = RenderWidgetHost::FromID(id.first, id.second);
//       ++current_index_;
//     }
//     return host;
//   }
// 
//  private:
//   std::vector<RenderWidgetHostID> hosts_;
//   size_t current_index_;
// 
//   DISALLOW_COPY_AND_ASSIGN(RenderWidgetHostIteratorImpl);
// };

}  // namespace


// static
void RenderWidgetHost::RemoveAllBackingStores() {
  //BackingStoreManager::RemoveAllBackingStores();
}

// static
size_t RenderWidgetHost::BackingStoreMemorySize() {
  return 0;//return BackingStoreManager::MemorySize();
}

///////////////////////////////////////////////////////////////////////////////
// RenderWidgetHostImpl

RenderWidgetHostImpl::RenderWidgetHostImpl(RenderWidgetHostDelegate* delegate,
                                           RenderProcessHost* process,
                                           int routing_id,
                                           bool hidden)
    : view_(NULL),
      renderer_initialized_(false),
      hung_renderer_delay_ms_(/*kHungRendererDelayMs*/1000),
      delegate_(delegate),
      process_(process),
      routing_id_(routing_id),
      surface_id_(0),
      is_loading_(false),
      is_hidden_(hidden),
      is_fullscreen_(false),
      is_accelerated_compositing_active_(false),
      repaint_ack_pending_(false),
      resize_ack_pending_(false),
      screen_info_out_of_date_(false),
      overdraw_bottom_height_(0.f),
      should_auto_resize_(false),
      waiting_for_screen_rects_ack_(false),
      //accessibility_mode_(AccessibilityModeOff),
      needs_repainting_on_restore_(false),
      is_unresponsive_(false),
      in_flight_event_count_(0),
      in_get_backing_store_(false),
      abort_get_backing_store_(false),
      view_being_painted_(false),
      ignore_input_events_(false),
      input_method_active_(false),
      text_direction_updated_(false),
      //text_direction_(WebKit::WebTextDirectionLeftToRight),
      text_direction_canceled_(false),
      suppress_next_char_events_(false),
      pending_mouse_lock_request_(false),
      allow_privileged_mouse_lock_(false),
      has_touch_handler_(false),
      weak_factory_(this),
      last_input_number_(0) {
  //CHECK(delegate_);
  if (routing_id_ == MSG_ROUTING_NONE) {
    routing_id_ = process_->GetNextRoutingID();
//     surface_id_ = GpuSurfaceTracker::Get()->AddSurfaceForRenderer(
//         process_->GetID(),
//         routing_id_);
  } else {
    // TODO(piman): This is a O(N) lookup, where we could forward the
    // information from the RenderWidgetHelper. The problem is that doing so
    // currently leaks outside of content all the way to chrome classes, and
    // would be a layering violation. Since we don't expect more than a few
    // hundreds of RWH, this seems acceptable. Revisit if performance become a
    // problem, for example by tracking in the RenderWidgetHelper the routing id
    // (and surface id) that have been created, but whose RWH haven't yet.
//     surface_id_ = GpuSurfaceTracker::Get()->LookupSurfaceForRenderer(
//         process_->GetID(),
//         routing_id_);
 //   DCHECK(surface_id_);
  }

  is_threaded_compositing_enabled_ = false;

//kk
//   g_routing_id_widget_map.Get().insert(std::make_pair(
//       RenderWidgetHostID(process->GetID(), routing_id_), this));
//   process_->AddRoute(routing_id_, this);

  // If we're initially visible, tell the process host that we're alive.
  // Otherwise we'll notify the process host when we are first shown.
  if (!hidden)
    process_->WidgetRestored();

//   accessibility_mode_ =
//       BrowserAccessibilityStateImpl::GetInstance()->accessibility_mode();

  for (size_t i = 0; i < g_created_callbacks.Get().size(); i++)
    g_created_callbacks.Get().at(i).Run(this);

//   input_router_.reset(
//       new ImmediateInputRouter(process_, this, this, routing_id_));

#if defined(USE_AURA)
  bool overscroll_enabled = CommandLine::ForCurrentProcess()->
      GetSwitchValueASCII(switches::kOverscrollHistoryNavigation) != "0";
  SetOverscrollControllerEnabled(overscroll_enabled);
#endif
}

RenderWidgetHostImpl::~RenderWidgetHostImpl() {
  SetView(NULL);

  // Clear our current or cached backing store if either remains.
//   BackingStoreManager::RemoveBackingStore(this);
// 
//   GpuSurfaceTracker::Get()->RemoveSurface(surface_id_);
  surface_id_ = 0;

  process_->RemoveRoute(routing_id_);

  //kk
  //   g_routing_id_widget_map.Get().erase(
//       RenderWidgetHostID(process_->GetID(), routing_id_));


// 
//   if (delegate_)
//     delegate_->RenderWidgetDeleted(this);
}

// static
RenderWidgetHost* RenderWidgetHost::FromID(
    int32 process_id,
    int32 routing_id) {
  return RenderWidgetHostImpl::FromID(process_id, routing_id);
}

// static
RenderWidgetHostImpl* RenderWidgetHostImpl::FromID(
    int32 process_id,
    int32 routing_id) {

    //kk
    return nullptr;
    //   DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
//   RoutingIDWidgetMap* widgets = g_routing_id_widget_map.Pointer();
// 
//   RoutingIDWidgetMap::iterator it = widgets->find(
//       RenderWidgetHostID(process_id, routing_id));
// 
//   return it == widgets->end() ? NULL : it->second;
}

// static
// scoped_ptr<RenderWidgetHostIterator> RenderWidgetHost::GetRenderWidgetHosts() {
//   RenderWidgetHostIteratorImpl* hosts = new RenderWidgetHostIteratorImpl();
//   RoutingIDWidgetMap* widgets = g_routing_id_widget_map.Pointer();
//   for (RoutingIDWidgetMap::const_iterator it = widgets->begin();
//        it != widgets->end();
//        ++it) {
//     RenderWidgetHost* widget = it->second;
// 
//     if (!widget->IsRenderView()) {
//       hosts->Add(widget);
//       continue;
//     }
// 
//     // Add only active RenderViewHosts.
//     RenderViewHost* rvh = RenderViewHost::From(widget);
//     if (!static_cast<RenderViewHostImpl*>(rvh)->is_swapped_out())
//       hosts->Add(widget);
//   }
// 
//   return scoped_ptr<RenderWidgetHostIterator>(hosts);
// }

// static
// scoped_ptr<RenderWidgetHostIterator>
// RenderWidgetHostImpl::GetAllRenderWidgetHosts() {
//   RenderWidgetHostIteratorImpl* hosts = new RenderWidgetHostIteratorImpl();
//   RoutingIDWidgetMap* widgets = g_routing_id_widget_map.Pointer();
//   for (RoutingIDWidgetMap::const_iterator it = widgets->begin();
//        it != widgets->end();
//        ++it) {
//     hosts->Add(it->second);
//   }
// 
//   return scoped_ptr<RenderWidgetHostIterator>(hosts);
// }

// static
RenderWidgetHostImpl* RenderWidgetHostImpl::From(RenderWidgetHost* rwh) {
  return rwh->AsRenderWidgetHostImpl();
}

// static
void RenderWidgetHost::AddCreatedCallback(const CreatedCallback& callback) {
  g_created_callbacks.Get().push_back(callback);
}

// static
void RenderWidgetHost::RemoveCreatedCallback(const CreatedCallback& callback) {
  for (size_t i = 0; i < g_created_callbacks.Get().size(); ++i) {
    if (g_created_callbacks.Get().at(i).Equals(callback)) {
      g_created_callbacks.Get().erase(g_created_callbacks.Get().begin() + i);
      return;
    }
  }
}

void RenderWidgetHostImpl::SetView(RenderWidgetHostView* view) {
  view_ = RenderWidgetHostViewPort::FromRWHV(view);

//   if (!view_) {
//     GpuSurfaceTracker::Get()->SetSurfaceHandle(
//         surface_id_, gfx::GLSurfaceHandle());
//   }
}

RenderProcessHost* RenderWidgetHostImpl::GetProcess() const {
  return process_;
}

int RenderWidgetHostImpl::GetRoutingID() const {
  return routing_id_;
}

RenderWidgetHostView* RenderWidgetHostImpl::GetView() const {
  return view_;
}

RenderWidgetHostImpl* RenderWidgetHostImpl::AsRenderWidgetHostImpl() {
  return this;
}

gfx::NativeViewId RenderWidgetHostImpl::GetNativeViewId() const {
   if (view_)
     return view_->GetNativeViewId();
  return 0;
}

gfx::GLSurfaceHandle RenderWidgetHostImpl::GetCompositingSurface() {
   if (view_)
     return view_->GetCompositingSurface();
  return gfx::GLSurfaceHandle();
}

void RenderWidgetHostImpl::CompositingSurfaceUpdated() {
//   GpuSurfaceTracker::Get()->SetSurfaceHandle(
//       surface_id_, GetCompositingSurface());
  process_->SurfaceUpdated(surface_id_);
}

void RenderWidgetHostImpl::ResetSizeAndRepaintPendingFlags() {
  resize_ack_pending_ = false;
  if (repaint_ack_pending_) {
    TRACE_EVENT_ASYNC_END0(
        "renderer_host", "RenderWidgetHostImpl::repaint_ack_pending_", this);
  }
  repaint_ack_pending_ = false;
  last_requested_size_.SetSize(0, 0);
}

void RenderWidgetHostImpl::SendScreenRects() {
  if (!renderer_initialized_ || waiting_for_screen_rects_ack_)
    return;

  if (is_hidden_) {
    // On GTK, this comes in for backgrounded tabs. Ignore, to match what
    // happens on Win & Mac, and when the view is shown it'll call this again.
    return;
  }

  if (!view_)
    return;

  last_view_screen_rect_ = view_->GetViewBounds();
  last_window_screen_rect_ = view_->GetBoundsInRootWindow();
//   Send(new ViewMsg_UpdateScreenRects(
//       GetRoutingID(), last_view_screen_rect_, last_window_screen_rect_));
//   if (delegate_)
//     delegate_->DidSendScreenRects(this);
  waiting_for_screen_rects_ack_ = true;
}

base::TimeDelta
    RenderWidgetHostImpl::GetSyntheticGestureMessageInterval() const {
  return base::TimeDelta();//synthetic_gesture_controller_.GetSyntheticGestureMessageInterval();
}


void RenderWidgetHostImpl::SuppressNextCharEvents() {
  suppress_next_char_events_ = true;
}

void RenderWidgetHostImpl::FlushInput() {
  //input_router_->Flush();
}

void RenderWidgetHostImpl::Init() {
  DCHECK(process_->HasConnection());

  renderer_initialized_ = true;
// 
//   GpuSurfaceTracker::Get()->SetSurfaceHandle(
//       surface_id_, GetCompositingSurface());
// 
//   // Send the ack along with the information on placement.
//   Send(new ViewMsg_CreatingNew_ACK(routing_id_));
  GetProcess()->ResumeRequestsForView(routing_id_);

  WasResized();
}

void RenderWidgetHostImpl::Shutdown() {
  RejectMouseLockOrUnlockIfNecessary();

  if (process_->HasConnection()) {
    // Tell the renderer object to close.
    //bool rv = Send(new ViewMsg_Close(routing_id_));
    //DCHECK(rv);
  }

  Destroy();
}

bool RenderWidgetHostImpl::IsLoading() const {
  return is_loading_;
}

bool RenderWidgetHostImpl::IsRenderView() const {
  return false;
}

bool RenderWidgetHostImpl::OnMessageReceived(const IPC::Message &msg) {
  bool handled = true;
  bool msg_is_ok = true;

  return handled;
}

bool RenderWidgetHostImpl::Send(IPC::Message* msg) {
//   if (IPC_MESSAGE_ID_CLASS(msg->type()) == InputMsgStart)
//     return input_router_->SendInput(make_scoped_ptr(msg));

  return process_->Send(msg);
}

void RenderWidgetHostImpl::WasHidden() {
  if (is_hidden_)
    return;

  is_hidden_ = true;

  // Don't bother reporting hung state when we aren't active.
  StopHangMonitorTimeout();

  // If we have a renderer, then inform it that we are being hidden so it can
  // reduce its resource utilization.
  //Send(new ViewMsg_WasHidden(routing_id_));

  // Tell the RenderProcessHost we were hidden.
  process_->WidgetHidden();
}

void RenderWidgetHostImpl::WasShown() {
  if (!is_hidden_)
    return;
  is_hidden_ = false;

  SendScreenRects();

//   BackingStore* backing_store = BackingStoreManager::Lookup(this);
//   // If we already have a backing store for this widget, then we don't need to
//   // repaint on restore _unless_ we know that our backing store is invalid.
//   // When accelerated compositing is on, we must always repaint, even when
//   // the backing store exists.
//   bool needs_repainting;
//   if (needs_repainting_on_restore_ || !backing_store ||
//       is_accelerated_compositing_active()) {
//     needs_repainting = true;
//     needs_repainting_on_restore_ = false;
//   } else {
//     needs_repainting = false;
//   }
//   Send(new ViewMsg_WasShown(routing_id_, needs_repainting));

  process_->WidgetRestored();

  // It's possible for our size to be out of sync with the renderer. The
  // following is one case that leads to this:
  // 1. WasResized -> Send ViewMsg_Resize to render
  // 2. WasResized -> do nothing as resize_ack_pending_ is true
  // 3. WasHidden
  // 4. OnUpdateRect from (1) processed. Does NOT invoke WasResized as view
  //    is hidden. Now renderer/browser out of sync with what they think size
  //    is.
  // By invoking WasResized the renderer is updated as necessary. WasResized
  // does nothing if the sizes are already in sync.
  //
  // TODO: ideally ViewMsg_WasShown would take a size. This way, the renderer
  // could handle both the restore and resize at once. This isn't that big a
  // deal as RenderWidget::WasShown delays updating, so that the resize from
  // WasResized is usually processed before the renderer is painted.
  WasResized();
}

void RenderWidgetHostImpl::WasResized() {
  // Skip if the |delegate_| has already been detached because
  // it's web contents is being deleted.
  if (/*resize_ack_pending_ || */!process_->HasConnection() || !view_ ||
    /*!renderer_initialized_ || should_auto_resize_ || */!delegate_) {
      return;
  }

  gfx::Rect view_bounds = view_->GetViewBounds();
  gfx::Size new_size(view_bounds.size());

  gfx::Size old_physical_backing_size = physical_backing_size_;
  physical_backing_size_ = view_->GetPhysicalBackingSize();
  bool was_fullscreen = is_fullscreen_;
  is_fullscreen_ = IsFullscreen();
  float old_overdraw_bottom_height = overdraw_bottom_height_;
  overdraw_bottom_height_ = view_->GetOverdrawBottomHeight();

  bool size_changed = new_size != last_requested_size_;
  bool side_payload_changed =
    screen_info_out_of_date_ ||
    old_physical_backing_size != physical_backing_size_ ||
    was_fullscreen != is_fullscreen_ ||
    old_overdraw_bottom_height != overdraw_bottom_height_;

  if (!size_changed && !side_payload_changed)
    return;

  // We don't expect to receive an ACK when the requested size or the physical
  // backing size is empty, or when the main viewport size didn't change.
  if (!new_size.IsEmpty() && !physical_backing_size_.IsEmpty() && size_changed)
    resize_ack_pending_ = g_check_for_pending_resize_ack;

  ViewMsg_Resize_Params params;
  //params.screen_info = *screen_info_;
  params.w = new_size.width();
  params.h = new_size.height();
  params.is_fullscreen = is_fullscreen_;
  if (!Send(new ViewMsg_Resize(routing_id_, params))) {
    resize_ack_pending_ = false;
  } else {
    last_requested_size_ = new_size;
  }
}

void RenderWidgetHostImpl::ResizeRectChanged(const gfx::Rect& new_rect) {
  //Send(new ViewMsg_ChangeResizeRect(routing_id_, new_rect));
}

void RenderWidgetHostImpl::GotFocus() {
  Focus();
}

void RenderWidgetHostImpl::Focus() {
  Send(new InputMsg_SetFocus(routing_id_, true));
}

void RenderWidgetHostImpl::Blur() {
  // If there is a pending mouse lock request, we don't want to reject it at
  // this point. The user can switch focus back to this view and approve the
  // request later.
  if (IsMouseLocked())
    view_->UnlockMouse();
// 
//   // If there is a pending overscroll, then that should be cancelled.
//   if (overscroll_controller_)
//     overscroll_controller_->Cancel();
// 
  Send(new InputMsg_SetFocus(routing_id_, false));
}

void RenderWidgetHostImpl::LostCapture() {
  //Send(new InputMsg_MouseCaptureLost(routing_id_));
}

void RenderWidgetHostImpl::SetActive(bool active) {
  Send(new ViewMsg_SetActive(routing_id_, active));
}

void RenderWidgetHostImpl::LostMouseLock() {
  //Send(new ViewMsg_MouseLockLost(routing_id_));
}

void RenderWidgetHostImpl::ViewDestroyed() {
  RejectMouseLockOrUnlockIfNecessary();

  // TODO(evanm): tracking this may no longer be necessary;
  // eliminate this function if so.
  SetView(NULL);
}

void RenderWidgetHostImpl::SetIsLoading(bool is_loading) {
  is_loading_ = is_loading;
  if (!view_)
    return;
  view_->SetIsLoading(is_loading);
}

void RenderWidgetHostImpl::CopyFromBackingStore(
    const gfx::Rect& src_subrect,
    const gfx::Size& accelerated_dst_size,
    const base::Callback<void(bool, const SkBitmap&)>& callback) {
//   if (view_ && is_accelerated_compositing_active_) {
//     TRACE_EVENT0("browser",
//         "RenderWidgetHostImpl::CopyFromBackingStore::FromCompositingSurface");
//     gfx::Rect accelerated_copy_rect = src_subrect.IsEmpty() ?
//         gfx::Rect(view_->GetViewBounds().size()) : src_subrect;
//     view_->CopyFromCompositingSurface(accelerated_copy_rect,
//                                       accelerated_dst_size,
//                                       callback);
//     return;
//   }
// 
//   BackingStore* backing_store = GetBackingStore(false);
//   if (!backing_store) {
//     callback.Run(false, SkBitmap());
//     return;
//   }
// 
//   TRACE_EVENT0("browser",
//       "RenderWidgetHostImpl::CopyFromBackingStore::FromBackingStore");
//   gfx::Rect copy_rect = src_subrect.IsEmpty() ?
//       gfx::Rect(backing_store->size()) : src_subrect;
//   // When the result size is equal to the backing store size, copy from the
//   // backing store directly to the output canvas.
//   skia::PlatformBitmap output;
//   bool result = backing_store->CopyFromBackingStore(copy_rect, &output);
//   callback.Run(result, output.GetBitmap());
}

#if defined(TOOLKIT_GTK)
bool RenderWidgetHostImpl::CopyFromBackingStoreToGtkWindow(
    const gfx::Rect& dest_rect, GdkWindow* target) {
  BackingStore* backing_store = GetBackingStore(false);
  if (!backing_store)
    return false;
  (static_cast<BackingStoreGtk*>(backing_store))->PaintToRect(
      dest_rect, target);
  return true;
}
#elif defined(OS_MACOSX)
gfx::Size RenderWidgetHostImpl::GetBackingStoreSize() {
  BackingStore* backing_store = GetBackingStore(false);
  return backing_store ? backing_store->size() : gfx::Size();
}

bool RenderWidgetHostImpl::CopyFromBackingStoreToCGContext(
    const CGRect& dest_rect, CGContextRef target) {
  BackingStore* backing_store = GetBackingStore(false);
  if (!backing_store)
    return false;
  (static_cast<BackingStoreMac*>(backing_store))->
      CopyFromBackingStoreToCGContext(dest_rect, target);
  return true;
}
#endif

void RenderWidgetHostImpl::PaintAtSize(TransportDIB::Handle dib_handle,
                                       int tag,
                                       const gfx::Size& page_size,
                                       const gfx::Size& desired_size) {
  // Ask the renderer to create a bitmap regardless of whether it's
  // hidden, being resized, redrawn, etc.  It resizes the web widget
  // to the page_size and then scales it to the desired_size.
//   Send(new ViewMsg_PaintAtSize(routing_id_, dib_handle, tag,
//                                page_size, desired_size));
}

void RenderWidgetHostImpl::DonePaintingToBackingStore() {
  //Send(new ViewMsg_UpdateRect_ACK(GetRoutingID()));
}

bool RenderWidgetHostImpl::ScheduleComposite() {
  if (is_hidden_ || !is_accelerated_compositing_active_ ||
      current_size_.IsEmpty() || repaint_ack_pending_ ||
      resize_ack_pending_ || view_being_painted_) {
    return false;
  }

  // Send out a request to the renderer to paint the view if required.
  repaint_start_time_ = TimeTicks::Now();
  repaint_ack_pending_ = true;
  TRACE_EVENT_ASYNC_BEGIN0(
      "renderer_host", "RenderWidgetHostImpl::repaint_ack_pending_", this);
  //Send(new ViewMsg_Repaint(routing_id_, current_size_));
  return true;
}

void RenderWidgetHostImpl::StartHangMonitorTimeout(TimeDelta delay) {
//   if (!GetProcess()->IsGuest() && CommandLine::ForCurrentProcess()->HasSwitch(
//           switches::kDisableHangMonitor)) {
//     return;
//   }

  // Set time_when_considered_hung_ if it's null. Also, update
  // time_when_considered_hung_ if the caller's request is sooner than the
  // existing one. This will have the side effect that the existing timeout will
  // be forgotten.
  Time requested_end_time = Time::Now() + delay;
  if (time_when_considered_hung_.is_null() ||
      time_when_considered_hung_ > requested_end_time)
    time_when_considered_hung_ = requested_end_time;

  // If we already have a timer with the same or shorter duration, then we can
  // wait for it to finish.
  if (hung_renderer_timer_.IsRunning() &&
      hung_renderer_timer_.GetCurrentDelay() <= delay) {
    // If time_when_considered_hung_ was null, this timer may fire early.
    // CheckRendererIsUnresponsive handles that by calling
    // StartHangMonitorTimeout with the remaining time.
    // If time_when_considered_hung_ was non-null, it means we still haven't
    // heard from the renderer so we leave time_when_considered_hung_ as is.
    return;
  }

  // Either the timer is not yet running, or we need to adjust the timer to
  // fire sooner.
  time_when_considered_hung_ = requested_end_time;
  hung_renderer_timer_.Stop();
  hung_renderer_timer_.Start(FROM_HERE, delay, this,
      &RenderWidgetHostImpl::CheckRendererIsUnresponsive);
}

void RenderWidgetHostImpl::RestartHangMonitorTimeout() {
  // Setting to null will cause StartHangMonitorTimeout to restart the timer.
  time_when_considered_hung_ = Time();
  StartHangMonitorTimeout(
      TimeDelta::FromMilliseconds(hung_renderer_delay_ms_));
}

void RenderWidgetHostImpl::StopHangMonitorTimeout() {
  time_when_considered_hung_ = Time();
  RendererIsResponsive();
  // We do not bother to stop the hung_renderer_timer_ here in case it will be
  // started again shortly, which happens to be the common use case.
}

void RenderWidgetHostImpl::EnableFullAccessibilityMode() {
  //SetAccessibilityMode(AccessibilityModeComplete);
}

// void RenderWidgetHostImpl::ForwardMouseEvent(const WebMouseEvent& mouse_event) {
// //   ForwardMouseEventWithLatencyInfo(
// //       MouseEventWithLatencyInfo(mouse_event,
// //                                 CreateRWHLatencyInfoIfNotExist(NULL)));
// }
// 
// void RenderWidgetHostImpl::ForwardMouseEventWithLatencyInfo(
//     const MouseEventWithLatencyInfo& mouse_event) {
//   TRACE_EVENT2("input", "RenderWidgetHostImpl::ForwardMouseEvent",
//                "x", mouse_event.event.x, "y", mouse_event.event.y);
// 
//   for (size_t i = 0; i < mouse_event_callbacks_.size(); ++i) {
//     if (mouse_event_callbacks_[i].Run(mouse_event.event))
//       return;
//   }
// 
//   if (IgnoreInputEvents())
//     return;
// 
//   input_router_->SendMouseEvent(mouse_event);
// }

void RenderWidgetHostImpl::OnPointerEventActivate() {
}
// 
// void RenderWidgetHostImpl::ForwardWheelEvent(
//     const WebMouseWheelEvent& wheel_event) {
//   ForwardWheelEventWithLatencyInfo(
//       MouseWheelEventWithLatencyInfo(wheel_event,
//                                      CreateRWHLatencyInfoIfNotExist(NULL)));
// }
// 
// void RenderWidgetHostImpl::ForwardWheelEventWithLatencyInfo(
//     const MouseWheelEventWithLatencyInfo& wheel_event) {
//   TRACE_EVENT0("input", "RenderWidgetHostImpl::ForwardWheelEvent");
//   if (IgnoreInputEvents())
//     return;
// 
//   if (delegate_->PreHandleWheelEvent(wheel_event.event))
//     return;
// 
//   input_router_->SendWheelEvent(wheel_event);
// }

// void RenderWidgetHostImpl::ForwardGestureEvent(
//     const WebKit::WebGestureEvent& gesture_event) {
//   ForwardGestureEventWithLatencyInfo(gesture_event, ui::LatencyInfo());
// }
// 
// void RenderWidgetHostImpl::ForwardGestureEventWithLatencyInfo(
//     const WebKit::WebGestureEvent& gesture_event,
//     const ui::LatencyInfo& ui_latency) {
//   TRACE_EVENT0("input", "RenderWidgetHostImpl::ForwardGestureEvent");
//   // Early out if necessary, prior to performing latency logic.
//   if (IgnoreInputEvents())
//     return;
// 
//   ui::LatencyInfo latency_info = CreateRWHLatencyInfoIfNotExist(&ui_latency);
// 
//   if (gesture_event.type == WebKit::WebInputEvent::GestureScrollUpdate) {
//     latency_info.AddLatencyNumber(
//         ui::INPUT_EVENT_LATENCY_SCROLL_UPDATE_RWH_COMPONENT,
//         GetLatencyComponentId(),
//         ++last_input_number_);
// 
//     // Make a copy of the INPUT_EVENT_LATENCY_ORIGINAL_COMPONENT with a
//     // different name INPUT_EVENT_LATENCY_SCROLL_UPDATE_ORIGINAL_COMPONENT.
//     // So we can track the latency specifically for scroll update events.
//     ui::LatencyInfo::LatencyComponent original_component;
//     if (latency_info.FindLatency(ui::INPUT_EVENT_LATENCY_ORIGINAL_COMPONENT,
//                                  0,
//                                  &original_component)) {
//       latency_info.AddLatencyNumberWithTimestamp(
//           ui::INPUT_EVENT_LATENCY_SCROLL_UPDATE_ORIGINAL_COMPONENT,
//           GetLatencyComponentId(),
//           original_component.sequence_number,
//           original_component.event_time,
//           original_component.event_count,
//           true);
//     }
//   }
// 
//   GestureEventWithLatencyInfo gesture_with_latency(gesture_event, latency_info);
//   input_router_->SendGestureEvent(gesture_with_latency);
// }
// 
// void RenderWidgetHostImpl::ForwardTouchEventWithLatencyInfo(
//       const WebKit::WebTouchEvent& touch_event,
//       const ui::LatencyInfo& ui_latency) {
//   TRACE_EVENT0("input", "RenderWidgetHostImpl::ForwardTouchEvent");
//   if (IgnoreInputEvents())
//     return;
// 
//   ui::LatencyInfo latency_info = CreateRWHLatencyInfoIfNotExist(&ui_latency);
//   TouchEventWithLatencyInfo touch_with_latency(touch_event, latency_info);
//   input_router_->SendTouchEvent(touch_with_latency);
// }
// 
// void RenderWidgetHostImpl::ForwardKeyboardEvent(
//     const NativeWebKeyboardEvent& key_event) {
//   TRACE_EVENT0("input", "RenderWidgetHostImpl::ForwardKeyboardEvent");
//   if (IgnoreInputEvents())
//     return;
// 
//   if (!process_->HasConnection())
//     return;
// 
//   // First, let keypress listeners take a shot at handling the event.  If a
//   // listener handles the event, it should not be propagated to the renderer.
//   if (KeyPressListenersHandleEvent(key_event)) {
//     // Some keypresses that are accepted by the listener might have follow up
//     // char events, which should be ignored.
//     if (key_event.type == WebKeyboardEvent::RawKeyDown)
//       suppress_next_char_events_ = true;
//     return;
//   }
// 
//   if (key_event.type == WebKeyboardEvent::Char &&
//       (key_event.windowsKeyCode == ui::VKEY_RETURN ||
//        key_event.windowsKeyCode == ui::VKEY_SPACE)) {
//     OnUserGesture();
//   }
// 
//   // Double check the type to make sure caller hasn't sent us nonsense that
//   // will mess up our key queue.
//   if (!WebInputEvent::isKeyboardEventType(key_event.type))
//     return;
// 
//   if (suppress_next_char_events_) {
//     // If preceding RawKeyDown event was handled by the browser, then we need
//     // suppress all Char events generated by it. Please note that, one
//     // RawKeyDown event may generate multiple Char events, so we can't reset
//     // |suppress_next_char_events_| until we get a KeyUp or a RawKeyDown.
//     if (key_event.type == WebKeyboardEvent::Char)
//       return;
//     // We get a KeyUp or a RawKeyDown event.
//     suppress_next_char_events_ = false;
//   }
// 
//   bool is_shortcut = false;
// 
//   // Only pre-handle the key event if it's not handled by the input method.
//   if (delegate_ && !key_event.skip_in_browser) {
//     // We need to set |suppress_next_char_events_| to true if
//     // PreHandleKeyboardEvent() returns true, but |this| may already be
//     // destroyed at that time. So set |suppress_next_char_events_| true here,
//     // then revert it afterwards when necessary.
//     if (key_event.type == WebKeyboardEvent::RawKeyDown)
//       suppress_next_char_events_ = true;
// 
//     // Tab switching/closing accelerators aren't sent to the renderer to avoid
//     // a hung/malicious renderer from interfering.
//     if (delegate_->PreHandleKeyboardEvent(key_event, &is_shortcut))
//       return;
// 
//     if (key_event.type == WebKeyboardEvent::RawKeyDown)
//       suppress_next_char_events_ = false;
//   }
// 
//   input_router_->SendKeyboardEvent(key_event,
//                                    CreateRWHLatencyInfoIfNotExist(NULL),
//                                    is_shortcut);
// }

void RenderWidgetHostImpl::SendCursorVisibilityState(bool is_visible) {
  //Send(new InputMsg_CursorVisibilityChange(GetRoutingID(), is_visible));
}

int64 RenderWidgetHostImpl::GetLatencyComponentId() {
  return GetRoutingID() | (static_cast<int64>(GetProcess()->GetID()) << 32);
}

// static
void RenderWidgetHostImpl::DisableResizeAckCheckForTesting() {
  g_check_for_pending_resize_ack = false;
}

ui::LatencyInfo RenderWidgetHostImpl::CreateRWHLatencyInfoIfNotExist(
    const ui::LatencyInfo* original) {
  ui::LatencyInfo info;
  if (original)
    info = *original;
  // In Aura, gesture event will already carry its original touch event's
  // INPUT_EVENT_LATENCY_RWH_COMPONENT.
  if (!info.FindLatency(ui::INPUT_EVENT_LATENCY_BEGIN_RWH_COMPONENT,
                        GetLatencyComponentId(),
                        NULL)) {
    info.AddLatencyNumber(ui::INPUT_EVENT_LATENCY_BEGIN_RWH_COMPONENT,
                          GetLatencyComponentId(),
                          ++last_input_number_);
  }
  return info;
}

// 
// void RenderWidgetHostImpl::AddKeyPressEventCallback(
//     const KeyPressEventCallback& callback) {
//   key_press_event_callbacks_.push_back(callback);
// }
// 
// void RenderWidgetHostImpl::RemoveKeyPressEventCallback(
//     const KeyPressEventCallback& callback) {
//   for (size_t i = 0; i < key_press_event_callbacks_.size(); ++i) {
//     if (key_press_event_callbacks_[i].Equals(callback)) {
//       key_press_event_callbacks_.erase(
//           key_press_event_callbacks_.begin() + i);
//       return;
//     }
//   }
// }
// 
// void RenderWidgetHostImpl::AddMouseEventCallback(
//     const MouseEventCallback& callback) {
//   mouse_event_callbacks_.push_back(callback);
// }
// 
// void RenderWidgetHostImpl::RemoveMouseEventCallback(
//     const MouseEventCallback& callback) {
//   for (size_t i = 0; i < mouse_event_callbacks_.size(); ++i) {
//     if (mouse_event_callbacks_[i].Equals(callback)) {
//       mouse_event_callbacks_.erase(mouse_event_callbacks_.begin() + i);
//       return;
//     }
//   }
// }
// 
// void RenderWidgetHostImpl::GetWebScreenInfo(WebKit::WebScreenInfo* result) {
//   TRACE_EVENT0("renderer_host", "RenderWidgetHostImpl::GetWebScreenInfo");
//   if (GetView())
//     static_cast<RenderWidgetHostViewPort*>(GetView())->GetScreenInfo(result);
//   else
//     RenderWidgetHostViewPort::GetDefaultScreenInfo(result);
//   screen_info_out_of_date_ = false;
// }
// 
// const NativeWebKeyboardEvent*
//     RenderWidgetHostImpl::GetLastKeyboardEvent() const {
//   return input_router_->GetLastKeyboardEvent();
// }

void RenderWidgetHostImpl::NotifyScreenInfoChanged() {
  // The resize message (which may not happen immediately) will carry with it
  // the screen info as well as the new size (if the screen has changed scale
  // factor).
  InvalidateScreenInfo();
  WasResized();
}

void RenderWidgetHostImpl::InvalidateScreenInfo() {
  screen_info_out_of_date_ = true;
  //screen_info_.reset();
}

void RenderWidgetHostImpl::GetSnapshotFromRenderer(
    const gfx::Rect& src_subrect,
    const base::Callback<void(bool, const SkBitmap&)>& callback) {
  TRACE_EVENT0("browser", "RenderWidgetHostImpl::GetSnapshotFromRenderer");
  pending_snapshots_.push(callback);
// 
//   gfx::Rect copy_rect = src_subrect.IsEmpty() ?
//       gfx::Rect(view_->GetViewBounds().size()) : src_subrect;
// 
//   gfx::Rect copy_rect_in_pixel = ConvertViewRectToPixel(view_, copy_rect);
//   Send(new ViewMsg_Snapshot(GetRoutingID(), copy_rect_in_pixel));
}

void RenderWidgetHostImpl::OnSnapshot(bool success,
                                    const SkBitmap& bitmap) {
  if (pending_snapshots_.size() == 0) {
    LOG(ERROR) << "RenderWidgetHostImpl::OnSnapshot: "
                  "Received a snapshot that was not requested.";
    return;
  }

  base::Callback<void(bool, const SkBitmap&)> callback =
      pending_snapshots_.front();
  pending_snapshots_.pop();

  if (!success) {
    callback.Run(success, SkBitmap());
    return;
  }

  callback.Run(success, bitmap);
}

void RenderWidgetHostImpl::UpdateVSyncParameters(base::TimeTicks timebase,
                                                 base::TimeDelta interval) {
  //Send(new ViewMsg_UpdateVSyncParameters(GetRoutingID(), timebase, interval));
}

void RenderWidgetHostImpl::RendererExited(base::TerminationStatus status,
                                          int exit_code) {
//   // Clearing this flag causes us to re-create the renderer when recovering
//   // from a crashed renderer.
//   renderer_initialized_ = false;
// 
//   waiting_for_screen_rects_ack_ = false;
// 
//   // Reset to ensure that input routing works with a new renderer.
//   input_router_.reset(
//       new ImmediateInputRouter(process_, this, this, routing_id_));
// 
//   if (overscroll_controller_)
//     overscroll_controller_->Reset();
// 
//  // Must reset these to ensure that keyboard events work with a new renderer.
//   suppress_next_char_events_ = false;
// 
//   // Reset some fields in preparation for recovering from a crash.
//   ResetSizeAndRepaintPendingFlags();
//   current_size_.SetSize(0, 0);
//   is_hidden_ = false;
//   is_accelerated_compositing_active_ = false;
// 
//   // Reset this to ensure the hung renderer mechanism is working properly.
//   in_flight_event_count_ = 0;
// 
//   if (view_) {
//     GpuSurfaceTracker::Get()->SetSurfaceHandle(surface_id_,
//                                                gfx::GLSurfaceHandle());
//     view_->RenderProcessGone(status, exit_code);
//     view_ = NULL;  // The View should be deleted by RenderProcessGone.
//   }
// 
//   BackingStoreManager::RemoveBackingStore(this);
}
// 
// void RenderWidgetHostImpl::UpdateTextDirection(WebTextDirection direction) {
//   text_direction_updated_ = true;
//   text_direction_ = direction;
// }

void RenderWidgetHostImpl::CancelUpdateTextDirection() {
  if (text_direction_updated_)
    text_direction_canceled_ = true;
}

// void RenderWidgetHostImpl::NotifyTextDirection() {
//   if (text_direction_updated_) {
//     if (!text_direction_canceled_)
//       Send(new ViewMsg_SetTextDirection(GetRoutingID(), text_direction_));
//     text_direction_updated_ = false;
//     text_direction_canceled_ = false;
//   }
// }

void RenderWidgetHostImpl::SetInputMethodActive(bool activate) {
  input_method_active_ = activate;
  //Send(new ViewMsg_SetInputMethodActive(GetRoutingID(), activate));
}

// void RenderWidgetHostImpl::ImeSetComposition(
//     const string16& text,
//     const std::vector<WebKit::WebCompositionUnderline>& underlines,
//     int selection_start,
//     int selection_end) {
// //   Send(new ViewMsg_ImeSetComposition(
// //             GetRoutingID(), text, underlines, selection_start, selection_end));
// }

void RenderWidgetHostImpl::ImeConfirmComposition(
    const string16& text,
    const gfx::Range& replacement_range,
    bool keep_selection) {
//   Send(new ViewMsg_ImeConfirmComposition(
//         GetRoutingID(), text, replacement_range, keep_selection));
}

void RenderWidgetHostImpl::ImeCancelComposition() {
//   Send(new ViewMsg_ImeSetComposition(GetRoutingID(), string16(),
//             std::vector<WebKit::WebCompositionUnderline>(), 0, 0));
}

void RenderWidgetHostImpl::ExtendSelectionAndDelete(
    size_t before,
    size_t after) {
/*  Send(new ViewMsg_ExtendSelectionAndDelete(GetRoutingID(), before, after));*/
}

gfx::Rect RenderWidgetHostImpl::GetRootWindowResizerRect() const {
  return gfx::Rect();
}

void RenderWidgetHostImpl::RequestToLockMouse(bool user_gesture,
                                              bool last_unlocked_by_target) {
  // Directly reject to lock the mouse. Subclass can override this method to
  // decide whether to allow mouse lock or not.
  GotResponseToLockMouseRequest(false);
}

void RenderWidgetHostImpl::RejectMouseLockOrUnlockIfNecessary() {
  DCHECK(!pending_mouse_lock_request_ || !IsMouseLocked());
//   if (pending_mouse_lock_request_) {
//     pending_mouse_lock_request_ = false;
//     Send(new ViewMsg_LockMouse_ACK(routing_id_, false));
//   } else if (IsMouseLocked()) {
//     view_->UnlockMouse();
//   }
}

bool RenderWidgetHostImpl::IsMouseLocked() const {
  return view_ ? view_->IsMouseLocked() : false;
}

bool RenderWidgetHostImpl::IsFullscreen() const {
  return false;
}

void RenderWidgetHostImpl::SetShouldAutoResize(bool enable) {
  should_auto_resize_ = enable;
}

void RenderWidgetHostImpl::Destroy() {
//   NotificationService::current()->Notify(
//       NOTIFICATION_RENDER_WIDGET_HOST_DESTROYED,
//       Source<RenderWidgetHost>(this),
//       NotificationService::NoDetails());

  // Tell the view to die.
  // Note that in the process of the view shutting down, it can call a ton
  // of other messages on us.  So if you do any other deinitialization here,
  // do it after this call to view_->Destroy().
  if (view_)
    view_->Destroy();

  delete this;
}

void RenderWidgetHostImpl::CheckRendererIsUnresponsive() {
  // If we received a call to StopHangMonitorTimeout.
  if (time_when_considered_hung_.is_null())
    return;

  // If we have not waited long enough, then wait some more.
  Time now = Time::Now();
  if (now < time_when_considered_hung_) {
    StartHangMonitorTimeout(time_when_considered_hung_ - now);
    return;
  }
  is_unresponsive_ = true;
  NotifyRendererUnresponsive();
}

void RenderWidgetHostImpl::RendererIsResponsive() {
  if (is_unresponsive_) {
    is_unresponsive_ = false;
    NotifyRendererResponsive();
  }
}

void RenderWidgetHostImpl::OnRenderViewReady() {
  SendScreenRects();
  WasResized();
}

void RenderWidgetHostImpl::OnRenderProcessGone(int status, int exit_code) {
  // TODO(evanm): This synchronously ends up calling "delete this".
  // Is that really what we want in response to this message?  I'm matching
  // previous behavior of the code here.
  Destroy();
}

void RenderWidgetHostImpl::OnClose() {
  Shutdown();
}


void RenderWidgetHostImpl::OnUpdateScreenRectsAck() {
  waiting_for_screen_rects_ack_ = false;
  if (!view_)
    return;

  if (view_->GetViewBounds() == last_view_screen_rect_ &&
      view_->GetBoundsInRootWindow() == last_window_screen_rect_) {
    return;
  }

  SendScreenRects();
}

void RenderWidgetHostImpl::OnRequestMove(const gfx::Rect& pos) {
  // Note that we ignore the position.
  if (view_) {
     view_->SetBounds(pos);
//     Send(new ViewMsg_Move_ACK(routing_id_));
  }
}

void RenderWidgetHostImpl::OnPaintAtSizeAck(int tag, const gfx::Size& size) {
  std::pair<int, gfx::Size> details = std::make_pair(tag, size);
 
}

#if defined(OS_MACOSX)
void RenderWidgetHostImpl::OnCompositorSurfaceBuffersSwapped(
      const ViewHostMsg_CompositorSurfaceBuffersSwapped_Params& params) {
  TRACE_EVENT0("renderer_host",
               "RenderWidgetHostImpl::OnCompositorSurfaceBuffersSwapped");
  if (!view_) {
    AcceleratedSurfaceMsg_BufferPresented_Params ack_params;
    ack_params.sync_point = 0;
    RenderWidgetHostImpl::AcknowledgeBufferPresent(params.route_id,
                                                   params.gpu_process_host_id,
                                                   ack_params);
    return;
  }
  GpuHostMsg_AcceleratedSurfaceBuffersSwapped_Params gpu_params;
  gpu_params.surface_id = params.surface_id;
  gpu_params.surface_handle = params.surface_handle;
  gpu_params.route_id = params.route_id;
  gpu_params.size = params.size;
  gpu_params.scale_factor = params.scale_factor;
  gpu_params.latency_info = params.latency_info;
  view_->AcceleratedSurfaceBuffersSwapped(gpu_params,
                                          params.gpu_process_host_id);
  view_->DidReceiveRendererFrame();
}
#endif  // OS_MACOSX

bool RenderWidgetHostImpl::OnSwapCompositorFrame(
    const IPC::Message& message) {

  return true;
}

void RenderWidgetHostImpl::OnOverscrolled(
    gfx::Vector2dF accumulated_overscroll,
    gfx::Vector2dF current_fling_velocity) {

}

void RenderWidgetHostImpl::OnUpdateIsDelayed() {
  if (in_get_backing_store_)
    abort_get_backing_store_ = true;
}

void RenderWidgetHostImpl::OnFocus() {
  // Only RenderViewHost can deal with that message.
  //RecordAction(UserMetricsAction("BadMessageTerminate_RWH4"));
  GetProcess()->ReceivedBadMessage();
}

void RenderWidgetHostImpl::OnBlur() {
  // Only RenderViewHost can deal with that message.
  //RecordAction(UserMetricsAction("BadMessageTerminate_RWH5"));
  GetProcess()->ReceivedBadMessage();
}

void RenderWidgetHostImpl::OnSetCursor(const WebCursor& cursor) {
  if (!view_) {
    return;
  }
  view_->UpdateCursor(cursor);
}

void RenderWidgetHostImpl::OnTextInputTypeChanged(
    ui::TextInputType type,
    ui::TextInputMode input_mode,
    bool can_compose_inline) {
   if (view_)
     view_->TextInputTypeChanged(type, input_mode, can_compose_inline);
}

#if defined(OS_MACOSX) || defined(OS_WIN) || defined(USE_AURA)
void RenderWidgetHostImpl::OnImeCompositionRangeChanged(
    const gfx::Range& range,
    const std::vector<gfx::Rect>& character_bounds) {
   if (view_)
     view_->ImeCompositionRangeChanged(range, character_bounds);
}
#endif

void RenderWidgetHostImpl::OnImeCancelComposition() {
   if (view_)
     view_->ImeCancelComposition();
}

void RenderWidgetHostImpl::OnDidActivateAcceleratedCompositing(bool activated) {
  TRACE_EVENT1("renderer_host",
               "RenderWidgetHostImpl::OnDidActivateAcceleratedCompositing",
               "activated", activated);
//   is_accelerated_compositing_active_ = activated;
   if (view_)
     view_->OnAcceleratedCompositingStateChange();
}

void RenderWidgetHostImpl::OnLockMouse(bool user_gesture,
                                       bool last_unlocked_by_target,
                                       bool privileged) {

}

void RenderWidgetHostImpl::OnUnlockMouse() {
  RejectMouseLockOrUnlockIfNecessary();
}


#if defined(OS_WIN)

#endif


bool RenderWidgetHostImpl::IgnoreInputEvents() const {
  return ignore_input_events_ || process_->IgnoreInputEvents();
}

void RenderWidgetHostImpl::StartUserGesture() {
  OnUserGesture();
}

void RenderWidgetHostImpl::Stop() {
  //Send(new ViewMsg_Stop(GetRoutingID()));
}

void RenderWidgetHostImpl::SetBackground(const SkBitmap& background) {
  //Send(new ViewMsg_SetBackground(GetRoutingID(), background));
}

void RenderWidgetHostImpl::SetEditCommandsForNextKeyEvent(
    const std::vector<EditCommand>& commands) {
  //Send(new InputMsg_SetEditCommandsForNextKeyEvent(GetRoutingID(), commands));
}

// void RenderWidgetHostImpl::SetAccessibilityMode(AccessibilityMode mode) {
//   //accessibility_mode_ = mode;
//   Send(new ViewMsg_SetAccessibilityMode(GetRoutingID(), mode));
// }

void RenderWidgetHostImpl::AccessibilityDoDefaultAction(int object_id) {
  //Send(new AccessibilityMsg_DoDefaultAction(GetRoutingID(), object_id));
}

void RenderWidgetHostImpl::AccessibilitySetFocus(int object_id) {
  //Send(new AccessibilityMsg_SetFocus(GetRoutingID(), object_id));
}

void RenderWidgetHostImpl::AccessibilitySetTextSelection(
    int object_id, int start_offset, int end_offset) {
  //Send(new AccessibilityMsg_SetTextSelection(
  //    GetRoutingID(), object_id, start_offset, end_offset));
}

void RenderWidgetHostImpl::FatalAccessibilityTreeError() {
  //Send(new AccessibilityMsg_FatalError(GetRoutingID()));
}

#if defined(OS_WIN) && defined(USE_AURA)
void RenderWidgetHostImpl::SetParentNativeViewAccessible(
    gfx::NativeViewAccessible accessible_parent) {
  if (view_)
    view_->SetParentNativeViewAccessible(accessible_parent);
}

gfx::NativeViewAccessible
RenderWidgetHostImpl::GetParentNativeViewAccessible() const {
  return delegate_->GetParentNativeViewAccessible();
}
#endif

void RenderWidgetHostImpl::ExecuteEditCommand(const std::string& command,
                                              const std::string& value) {
  //Send(new InputMsg_ExecuteEditCommand(GetRoutingID(), command, value));
}

void RenderWidgetHostImpl::ScrollFocusedEditableNodeIntoRect(
    const gfx::Rect& rect) {
  //Send(new InputMsg_ScrollFocusedEditableNodeIntoRect(GetRoutingID(), rect));
}

void RenderWidgetHostImpl::SelectRange(const gfx::Point& start,
                                       const gfx::Point& end) {
  //Send(new InputMsg_SelectRange(GetRoutingID(), start, end));
}

void RenderWidgetHostImpl::MoveCaret(const gfx::Point& point) {
  //Send(new InputMsg_MoveCaret(GetRoutingID(), point));
}

void RenderWidgetHostImpl::Undo() {
  //Send(new InputMsg_Undo(GetRoutingID()));
  //RecordAction(UserMetricsAction("Undo"));
}

void RenderWidgetHostImpl::Redo() {
//   Send(new InputMsg_Redo(GetRoutingID()));
//   RecordAction(UserMetricsAction("Redo"));
}

void RenderWidgetHostImpl::Cut() {
//   Send(new InputMsg_Cut(GetRoutingID()));
//   RecordAction(UserMetricsAction("Cut"));
}

void RenderWidgetHostImpl::Copy() {
//   Send(new InputMsg_Copy(GetRoutingID()));
//   RecordAction(UserMetricsAction("Copy"));
}

void RenderWidgetHostImpl::CopyToFindPboard() {
#if defined(OS_MACOSX)
  // Windows/Linux don't have the concept of a find pasteboard.
  Send(new InputMsg_CopyToFindPboard(GetRoutingID()));
  RecordAction(UserMetricsAction("CopyToFindPboard"));
#endif
}

void RenderWidgetHostImpl::Paste() {
//   Send(new InputMsg_Paste(GetRoutingID()));
//   RecordAction(UserMetricsAction("Paste"));
}

void RenderWidgetHostImpl::PasteAndMatchStyle() {
//   Send(new InputMsg_PasteAndMatchStyle(GetRoutingID()));
//   RecordAction(UserMetricsAction("PasteAndMatchStyle"));
}

void RenderWidgetHostImpl::Delete() {
//   Send(new InputMsg_Delete(GetRoutingID()));
//   RecordAction(UserMetricsAction("DeleteSelection"));
}

void RenderWidgetHostImpl::SelectAll() {
//   Send(new InputMsg_SelectAll(GetRoutingID()));
//   RecordAction(UserMetricsAction("SelectAll"));
}

void RenderWidgetHostImpl::Unselect() {
//   Send(new InputMsg_Unselect(GetRoutingID()));
//   RecordAction(UserMetricsAction("Unselect"));
}

bool RenderWidgetHostImpl::GotResponseToLockMouseRequest(bool allowed) {
//   if (!allowed) {
//     RejectMouseLockOrUnlockIfNecessary();
//     return false;
//   } else {
//     if (!pending_mouse_lock_request_) {
//       // This is possible, e.g., the plugin sends us an unlock request before
//       // the user allows to lock to mouse.
//       return false;
//     }
// 
//     pending_mouse_lock_request_ = false;
//     if (!view_ || !view_->HasFocus()|| !view_->LockMouse()) {
//       Send(new ViewMsg_LockMouse_ACK(routing_id_, false));
//       return false;
//     } else {
//       Send(new ViewMsg_LockMouse_ACK(routing_id_, true));
//       return true;
//     }
//   }
  return false;
}

// static
// void RenderWidgetHostImpl::AcknowledgeBufferPresent(
//     int32 route_id, int gpu_host_id,
//     const AcceleratedSurfaceMsg_BufferPresented_Params& params) {
//   GpuProcessHostUIShim* ui_shim = GpuProcessHostUIShim::FromID(gpu_host_id);
//   if (ui_shim) {
//     ui_shim->Send(new AcceleratedSurfaceMsg_BufferPresented(route_id,
//                                                             params));
//   }
// }

// static
void RenderWidgetHostImpl::SendSwapCompositorFrameAck(
    int32 route_id,
    uint32 output_surface_id,
    int renderer_host_id,
    const cc::CompositorFrameAck& ack) {
  RenderProcessHost* host = RenderProcessHost::FromID(renderer_host_id);
  if (!host)
    return;
//   host->Send(new ViewMsg_SwapCompositorFrameAck(
//       route_id, output_surface_id, ack));
}

// static
void RenderWidgetHostImpl::SendReclaimCompositorResources(
    int32 route_id,
    uint32 output_surface_id,
    int renderer_host_id,
    const cc::CompositorFrameAck& ack) {
  RenderProcessHost* host = RenderProcessHost::FromID(renderer_host_id);
  if (!host)
    return;
//   host->Send(
//       new ViewMsg_ReclaimCompositorResources(route_id, output_surface_id, ack));
}

void RenderWidgetHostImpl::AcknowledgeSwapBuffersToRenderer() {
//   if (!is_threaded_compositing_enabled_)
//     Send(new ViewMsg_SwapBuffers_ACK(routing_id_));
}

#if defined(USE_AURA)

void RenderWidgetHostImpl::ParentChanged(gfx::NativeViewId new_parent) {
#if defined(OS_WIN)
  HWND hwnd = reinterpret_cast<HWND>(new_parent);
  if (!hwnd)
    hwnd = GetDesktopWindow();
  for (std::list<HWND>::iterator i = dummy_windows_for_activation_.begin();
        i != dummy_windows_for_activation_.end(); ++i) {
    SetParent(*i, hwnd);
  }
#endif
}

#endif

void RenderWidgetHostImpl::DelayedAutoResized() {
  gfx::Size new_size = new_auto_size_;
  // Clear the new_auto_size_ since the empty value is used as a flag to
  // indicate that no callback is in progress (i.e. without this line
  // DelayedAutoResized will not get called again).
  new_auto_size_.SetSize(0, 0);
  if (!should_auto_resize_)
    return;

  OnRenderAutoResized(new_size);
}

void RenderWidgetHostImpl::DetachDelegate() {
  delegate_ = NULL;
}

void RenderWidgetHostImpl::DidReceiveRendererFrame() {
  view_->DidReceiveRendererFrame();
}

}  // namespace content
