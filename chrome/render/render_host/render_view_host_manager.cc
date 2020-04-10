// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/render/render_host/render_view_host_manager.h"

#include <utility>

#include "base/command_line.h"
#include "base/debug/trace_event.h"
#include "base/logging.h"
#if defined (ENABLE_DEVTOOLS)
#include "content/browser/devtools/render_view_devtools_agent_host.h"
#endif
#include "chrome/common/render_process_host_impl.h"
#include "chrome/render/render_host/render_view_host_impl.h"
#include "chrome/render/site_instance_impl.h"
#include "chrome/browser/web_contents_impl.h"
#include "chrome/render/render_host/render_widget_host_view_port.h"
#include "content/public/app/content_browser_client.h"
#include "chrome/render/render_host/render_widget_host_iterator.h"
#include "chrome/browser/web_contents_view.h"
#include "chrome/common/content_switches.h"

namespace content {

RenderViewHost* RenderViewHostFactoryCreate(
    SiteInstance* instance,
    RenderViewHostDelegate* delegate,
    RenderWidgetHostDelegate* widget_delegate,
    int routing_id,
    int main_frame_routing_id,
    bool swapped_out,
    bool hidden) {
      return new RenderViewHostImpl(instance, delegate, widget_delegate, routing_id,
        main_frame_routing_id, swapped_out, hidden);
}

RenderViewHostManager::PendingNavigationParams::PendingNavigationParams() {
}

RenderViewHostManager::PendingNavigationParams::PendingNavigationParams(
    const int& global_request_id)
    : global_request_id(global_request_id) {
}

RenderViewHostManager::RenderViewHostManager(
    RenderViewHostDelegate* render_view_delegate,
    RenderWidgetHostDelegate* render_widget_delegate,
    Delegate* delegate)
    : delegate_(delegate),
      cross_navigation_pending_(false),
      render_view_delegate_(render_view_delegate),
      render_widget_delegate_(render_widget_delegate),
      render_view_host_(NULL),
      pending_render_view_host_(NULL){
}

RenderViewHostManager::~RenderViewHostManager() {
  if (pending_render_view_host_)
    CancelPending();

  // We should always have a main RenderViewHost except in some tests.
  RenderViewHostImpl* render_view_host = render_view_host_;
  render_view_host_ = NULL;
  if (render_view_host)
    render_view_host->Shutdown();

  // Shut down any swapped out RenderViewHosts.
  for (RenderViewHostMap::iterator iter = swapped_out_hosts_.begin();
       iter != swapped_out_hosts_.end();
       ++iter) {
    iter->second->Shutdown();
  }
}

void RenderViewHostManager::Init(BrowserContext* browser_context,
                                 SiteInstance* site_instance,
                                 int routing_id,
                                 int main_frame_routing_id) {
  // Create a RenderViewHost, once we have an instance.  It is important to
  // immediately give this SiteInstance to a RenderViewHost so that it is
  // ref counted.
  if (!site_instance)
    site_instance = SiteInstance::Create(browser_context);
  render_view_host_ = static_cast<RenderViewHostImpl*>(
      RenderViewHostFactoryCreate(
          site_instance, render_view_delegate_, render_widget_delegate_,
          routing_id, main_frame_routing_id, false, delegate_->IsHidden()));

  // Keep track of renderer processes as they start to shut down or are
  // crashed/killed.
  /*registrar_.Add(this, NOTIFICATION_RENDERER_PROCESS_CLOSED,
                 NotificationService::AllSources());
  registrar_.Add(this, NOTIFICATION_RENDERER_PROCESS_CLOSING,
                 NotificationService::AllSources());*/
}

RenderViewHostImpl* RenderViewHostManager::current_host() const {
  return render_view_host_;
}

RenderViewHostImpl* RenderViewHostManager::pending_render_view_host() const {
  return pending_render_view_host_;
}

RenderWidgetHostView* RenderViewHostManager::GetRenderWidgetHostView() const {
//   if (interstitial_page_)
//     return interstitial_page_->GetView();
//   if (!render_view_host_)
//     return NULL;
  return render_view_host_->GetView();
}

void RenderViewHostManager::SetPendingWebUI(const NavigationEntryImpl& entry) {
  /*pending_web_ui_.reset(
      delegate_->CreateWebUIForRenderManager(entry.GetURL()));
  pending_and_current_web_ui_.reset();

  // If we have assigned (zero or more) bindings to this NavigationEntry in the
  // past, make sure we're not granting it different bindings than it had
  // before.  If so, note it and don't give it any bindings, to avoid a
  // potential privilege escalation.
  if (pending_web_ui_.get() &&
      entry.bindings() != NavigationEntryImpl::kInvalidBindings &&
      pending_web_ui_->GetBindings() != entry.bindings()) {
    RecordAction(UserMetricsAction("ProcessSwapBindingsMismatch_RVHM"));
    pending_web_ui_.reset();
  }*/
}

RenderViewHostImpl* RenderViewHostManager::Navigate(
    const NavigationEntryImpl& entry) {
  TRACE_EVENT0("browser", "RenderViewHostManager:Navigate");
  // Create a pending RenderViewHost. It will give us the one we should use
  RenderViewHostImpl* dest_render_view_host =
      static_cast<RenderViewHostImpl*>(UpdateRendererStateForNavigate(entry));
  if (!dest_render_view_host)
    return NULL;  // We weren't able to create a pending render view host.

  // If the current render_view_host_ isn't live, we should create it so
  // that we don't show a sad tab while the dest_render_view_host fetches
  // its first page.  (Bug 1145340)
  if (dest_render_view_host != render_view_host_ &&
      !render_view_host_->IsRenderViewLive()) {
    // Note: we don't call InitRenderView here because we are navigating away
    // soon anyway, and we don't have the NavigationEntry for this host.
    delegate_->CreateRenderViewForRenderManager(render_view_host_,
                                                MSG_ROUTING_NONE);
  }

  // If the renderer crashed, then try to create a new one to satisfy this
  // navigation request.
  if (!dest_render_view_host->IsRenderViewLive()) {
    // Recreate the opener chain.
    int opener_route_id = delegate_->CreateOpenerRenderViewsForRenderManager(
        dest_render_view_host->GetSiteInstance());
    if (!InitRenderView(dest_render_view_host, opener_route_id))
      return NULL;

    // Now that we've created a new renderer, be sure to hide it if it isn't
    // our primary one.  Otherwise, we might crash if we try to call Show()
    // on it later.
    if (dest_render_view_host != render_view_host_ &&
        dest_render_view_host->GetView()) {
      dest_render_view_host->GetView()->Hide();
    } else {
      // This is our primary renderer, notify here as we won't be calling
      // CommitPending (which does the notify).
      /*RenderViewHost* null_rvh = NULL;
      std::pair<RenderViewHost*, RenderViewHost*> details =
          std::make_pair(null_rvh, render_view_host_);
      NotificationService::current()->Notify(
          NOTIFICATION_RENDER_VIEW_HOST_CHANGED,
          Source<NavigationController>(
              &delegate_->GetControllerForRenderManager()),
          Details<std::pair<RenderViewHost*, RenderViewHost*> >(
              &details));*/
    }
  }

  return dest_render_view_host;
}

void RenderViewHostManager::Stop() {
  render_view_host_->Stop();

  // If we are cross-navigating, we should stop the pending renderers.  This
  // will lead to a DidFailProvisionalLoad, which will properly destroy them.
  if (cross_navigation_pending_) {
    //pending_render_view_host_->Send(
    //    new ViewMsg_Stop(pending_render_view_host_->GetRoutingID()));
  }
}

void RenderViewHostManager::SetIsLoading(bool is_loading) {
  render_view_host_->SetIsLoading(is_loading);
  if (pending_render_view_host_)
    pending_render_view_host_->SetIsLoading(is_loading);
}

bool RenderViewHostManager::ShouldCloseTabOnUnresponsiveRenderer() {
  if (!cross_navigation_pending_)
    return true;

  // We should always have a pending RVH when there's a cross-process navigation
  // in progress.  Sanity check this for http://crbug.com/276333.
  CHECK(pending_render_view_host_);

  // If the tab becomes unresponsive during {before}unload while doing a
  // cross-site navigation, proceed with the navigation.  (This assumes that
  // the pending RenderViewHost is still responsive.)
  if (render_view_host_->is_waiting_for_unload_ack()) {
    // The request has been started and paused while we're waiting for the
    // unload handler to finish.  We'll pretend that it did.  The pending
    // renderer will then be swapped in as part of the usual DidNavigate logic.
    // (If the unload handler later finishes, this call will be ignored because
    // the pending_nav_params_ state will already be cleaned up.)
    current_host()->OnSwappedOut(true);
  } else if (render_view_host_->is_waiting_for_beforeunload_ack()) {
    // Haven't gotten around to starting the request, because we're still
    // waiting for the beforeunload handler to finish.  We'll pretend that it
    // did finish, to let the navigation proceed.  Note that there's a danger
    // that the beforeunload handler will later finish and possibly return
    // false (meaning the navigation should not proceed), but we'll ignore it
    // in this case because it took too long.
    if (pending_render_view_host_->are_navigations_suspended())
      pending_render_view_host_->SetNavigationsSuspended(
          false, base::TimeTicks::Now());
  }
  return false;
}

void RenderViewHostManager::SwappedOut(RenderViewHost* render_view_host) {
  // Make sure this is from our current RVH, and that we have a pending
  // navigation from OnCrossSiteResponse.  (There may be no pending navigation
  // for data URLs that don't make network requests, for example.)   If not,
  // just return early and ignore.
  if (render_view_host != render_view_host_ || !pending_nav_params_.get()) {
    pending_nav_params_.reset();
    return;
  }

  // Now that the unload handler has run, we need to resume the paused response.
  /*if (pending_render_view_host_) {
    RenderProcessHostImpl* pending_process =
        static_cast<RenderProcessHostImpl*>(
            pending_render_view_host_->GetProcess());
    pending_process->ResumeDeferredNavigation(
        pending_nav_params_->global_request_id);
  }*/
  pending_nav_params_.reset();
}

void RenderViewHostManager::DidNavigateMainFrame(
    RenderViewHost* render_view_host) {
  if (!cross_navigation_pending_) {
    DCHECK(!pending_render_view_host_);

    // We should only hear this from our current renderer.
    DCHECK(render_view_host == render_view_host_);

    // Even when there is no pending RVH, there may be a pending Web UI.
    //if (pending_web_ui())
    //  CommitPending();
    return;
  }

  if (render_view_host == pending_render_view_host_) {
    // The pending cross-site navigation completed, so show the renderer.
    // If it committed without sending network requests (e.g., data URLs),
    // then we still need to swap out the old RVH first and run its unload
    // handler.  OK for that to happen in the background.
    if (pending_render_view_host_->HasPendingCrossSiteRequest())
      SwapOutOldPage();

    CommitPending();
    cross_navigation_pending_ = false;
  } else if (render_view_host == render_view_host_) {
    // A navigation in the original page has taken place.  Cancel the pending
    // one.
    CancelPending();
    cross_navigation_pending_ = false;
  } else {
    // No one else should be sending us DidNavigate in this state.
    DCHECK(false);
  }
}

void RenderViewHostManager::DidDisownOpener(RenderViewHost* render_view_host) {
  // Notify all swapped out hosts, including the pending RVH.
  for (RenderViewHostMap::iterator iter = swapped_out_hosts_.begin();
       iter != swapped_out_hosts_.end();
       ++iter) {
    DCHECK_NE(iter->second->GetSiteInstance(),
              current_host()->GetSiteInstance());
    iter->second->DisownOpener();
  }
}

void RenderViewHostManager::RendererAbortedProvisionalLoad(
    RenderViewHost* render_view_host) {
  // We used to cancel the pending renderer here for cross-site downloads.
  // However, it's not safe to do that because the download logic repeatedly
  // looks for this WebContents based on a render view ID.  Instead, we just
  // leave the pending renderer around until the next navigation event
  // (Navigate, DidNavigate, etc), which will clean it up properly.
  // TODO(creis): All of this will go away when we move the cross-site logic
  // to ResourceDispatcherHost, so that we intercept responses rather than
  // navigation events.  (That's necessary to support onunload anyway.)  Once
  // we've made that change, we won't create a pending renderer until we know
  // the response is not a download.
}

void RenderViewHostManager::RendererProcessClosing(
    RenderProcessHost* render_process_host) {
  // Remove any swapped out RVHs from this process, so that we don't try to
  // swap them back in while the process is exiting.  Start by finding them,
  // since there could be more than one.
  std::list<int> ids_to_remove;
  for (RenderViewHostMap::iterator iter = swapped_out_hosts_.begin();
       iter != swapped_out_hosts_.end();
       ++iter) {
    if (iter->second->GetProcess() == render_process_host)
      ids_to_remove.push_back(iter->first);
  }

  // Now delete them.
  while (!ids_to_remove.empty()) {
    swapped_out_hosts_[ids_to_remove.back()]->Shutdown();
    swapped_out_hosts_.erase(ids_to_remove.back());
    ids_to_remove.pop_back();
  }
}

void RenderViewHostManager::ShouldClosePage(
    bool for_cross_site_transition,
    bool proceed,
    const base::TimeTicks& proceed_time) {
  if (for_cross_site_transition) {
    // Ignore if we're not in a cross-site navigation.
    if (!cross_navigation_pending_)
      return;

    if (proceed) {
      // Ok to unload the current page, so proceed with the cross-site
      // navigation.  Note that if navigations are not currently suspended, it
      // might be because the renderer was deemed unresponsive and this call was
      // already made by ShouldCloseTabOnUnresponsiveRenderer.  In that case, it
      // is ok to do nothing here.
      if (pending_render_view_host_ &&
          pending_render_view_host_->are_navigations_suspended()) {
        pending_render_view_host_->SetNavigationsSuspended(false, proceed_time);
      }
    } else {
      // Current page says to cancel.
      CancelPending();
      cross_navigation_pending_ = false;
    }
  } else {
    // Non-cross site transition means closing the entire tab.
    bool proceed_to_fire_unload;
    delegate_->BeforeUnloadFiredFromRenderManager(proceed, proceed_time,
                                                  &proceed_to_fire_unload);

    if (proceed_to_fire_unload) {
      // This is not a cross-site navigation, the tab is being closed.
      render_view_host_->ClosePage();
    }
  }
}

void RenderViewHostManager::OnCrossSiteResponse(
    RenderViewHost* pending_render_view_host,
    int global_request_id) {
  // This should be called when the pending RVH is ready to commit.
  DCHECK_EQ(pending_render_view_host_, pending_render_view_host);

  // Remember the request ID until the unload handler has run.
  pending_nav_params_.reset(new PendingNavigationParams(global_request_id));

  // Run the unload handler of the current page.
  SwapOutOldPage();
}

void RenderViewHostManager::SwapOutOldPage() {
  // Should only see this while we have a pending renderer.
  if (!cross_navigation_pending_)
    return;
  DCHECK(pending_render_view_host_);

  // First close any modal dialogs that would prevent us from swapping out.
  // TODO(creis): This is not a guarantee.  The renderer could immediately
  // create another dialog in a loop, potentially causing a renderer crash when
  // we tell it to swap out with a nested message loop and PageGroupLoadDeferrer
  // on the stack.  We should prevent the renderer from showing more dialogs
  // until the SwapOut.  See http://crbug.com/312490.
  delegate_->CancelModalDialogsForRenderManager();

  // Tell the old renderer it is being swapped out.  This will fire the unload
  // handler (without firing the beforeunload handler a second time).  When the
  // unload handler finishes and the navigation completes, we will send a
  // message to the ResourceDispatcherHost, allowing the pending RVH's response
  // to resume.
  render_view_host_->SwapOut();

  // ResourceDispatcherHost has told us to run the onunload handler, which
  // means it is not a download or unsafe page, and we are going to perform the
  // navigation.  Thus, we no longer need to remember that the RenderViewHost
  // is part of a pending cross-site request.
  pending_render_view_host_->SetHasPendingCrossSiteRequest(false);
}

// void RenderViewHostManager::Observe(
//     int type,
//     const NotificationSource& source,
//     const NotificationDetails& details) {
//   switch (type) {
//     case NOTIFICATION_RENDERER_PROCESS_CLOSED:
//     case NOTIFICATION_RENDERER_PROCESS_CLOSING:
//       RendererProcessClosing(
//           Source<RenderProcessHost>(source).ptr());
//       break;
// 
//     default:
//       NOTREACHED();
//   }
// }

bool RenderViewHostManager::ShouldTransitionCrossSite() {
  // False in the single-process mode, as it makes RVHs to accumulate
  // in swapped_out_hosts_.
  // True if we are using process-per-site-instance (default) or
  // process-per-site (kProcessPerSite).
  return
      !CommandLine::ForCurrentProcess()->HasSwitch(switches::kSingleProcess) &&
      !CommandLine::ForCurrentProcess()->HasSwitch(switches::kProcessPerTab);

}
bool RenderViewHostManager::ShouldSwapEngineForNavigation(
  const NavigationEntry* curr_entry,
  const NavigationEntryImpl* new_entry) const{
   DCHECK(new_entry); 
   if( !curr_entry )
     return false;
  
   return false;
}

bool RenderViewHostManager::ShouldSwapProcessesForNavigation(
    const NavigationEntry* curr_entry,
    const NavigationEntryImpl* new_entry) const {
  DCHECK(new_entry);

  return false;
}

bool RenderViewHostManager::ShouldReuseWebUI(
    const NavigationEntry* curr_entry,
    const NavigationEntryImpl* new_entry) const {
  return false;
}

SiteInstance* RenderViewHostManager::GetSiteInstanceForEntry(
    const NavigationEntryImpl& entry,
    SiteInstance* curr_instance) {
      return curr_instance;
}

int RenderViewHostManager::CreateRenderView(
    SiteInstance* instance,
    int opener_route_id,
    bool swapped_out,
    bool hidden) {
  CHECK(instance);
  DCHECK(!swapped_out || hidden); // Swapped out views should always be hidden.

  // Check if we've already created an RVH for this SiteInstance.  If so, try
  // to re-use the existing one, which has already been initialized.  We'll
  // remove it from the list of swapped out hosts if it commits.
  RenderViewHostImpl* new_render_view_host = static_cast<RenderViewHostImpl*>(
      GetSwappedOutRenderViewHost(instance));
  if (new_render_view_host) {
    // Prevent the process from exiting while we're trying to use it.
    if (!swapped_out)
      new_render_view_host->GetProcess()->AddPendingView();
  } else {
    // Create a new RenderViewHost if we don't find an existing one.
    new_render_view_host = static_cast<RenderViewHostImpl*>(
        RenderViewHostFactoryCreate(instance,
                                      render_view_delegate_,
                                      render_widget_delegate_,
                                      MSG_ROUTING_NONE,
                                      MSG_ROUTING_NONE,
                                      swapped_out,
                                      hidden));

    // If the new RVH is swapped out already, store it.  Otherwise prevent the
    // process from exiting while we're trying to navigate in it.
    if (swapped_out) {
      swapped_out_hosts_[instance->GetId()] = new_render_view_host;
    } else {
      new_render_view_host->GetProcess()->AddPendingView();
    }

    bool success = InitRenderView(new_render_view_host, opener_route_id);
    if (success) {
      // Don't show the view until we get a DidNavigate from it.
      new_render_view_host->GetView()->Hide();
    } else if (!swapped_out) {
      CancelPending();
    }
  }

  // Use this as our new pending RVH if it isn't swapped out.
  if (!swapped_out)
    pending_render_view_host_ = new_render_view_host;

  return new_render_view_host->GetRoutingID();
}

bool RenderViewHostManager::InitRenderView(RenderViewHost* render_view_host,
                                           int opener_route_id) {
  // If the pending navigation is to a WebUI and the RenderView is not in a
  // guest process, tell the RenderView about any bindings it will need enabled.
  //if (pending_web_ui() && !render_view_host->GetProcess()->IsGuest())
  //  render_view_host->AllowBindings(pending_web_ui()->GetBindings());

  return delegate_->CreateRenderViewForRenderManager(render_view_host,
                                                     opener_route_id);
}

void RenderViewHostManager::CommitPending() {
  // First check whether we're going to want to focus the location bar after
  // this commit.  We do this now because the navigation hasn't formally
  // committed yet, so if we've already cleared |pending_web_ui_| the call chain
  // this triggers won't be able to figure out what's going on.
  bool will_focus_location_bar = delegate_->FocusLocationBarByDefault();

  // We currently can't guarantee that the renderer isn't showing a new modal
  // dialog, even though we canceled them in SwapOutOldPage.  (It may have
  // created another in the meantime.)  Make sure we run and reset the callback
  // now before we delete its RVH below.
  // TODO(creis): Remove this if we can guarantee that no new dialogs will be
  // shown after SwapOutOldPage.  See http://crbug.com/312490.
  delegate_->CancelModalDialogsForRenderManager();

  // Next commit the Web UI, if any. Either replace |web_ui_| with
  // |pending_web_ui_|, or clear |web_ui_| if there is no pending WebUI, or
  // leave |web_ui_| as is if reusing it.
//   DCHECK(!(pending_web_ui_.get() && pending_and_current_web_ui_.get()));
//   if (pending_web_ui_)
//     web_ui_.reset(pending_web_ui_.release());
//   else if (!pending_and_current_web_ui_.get())
//     web_ui_.reset();

  // It's possible for the pending_render_view_host_ to be NULL when we aren't
  // crossing process boundaries. If so, we just needed to handle the Web UI
  // committing above and we're done.
  if (!pending_render_view_host_) {
    if (will_focus_location_bar)
      delegate_->SetFocusToLocationBar(false);
    return;
  }

  // Remember if the page was focused so we can focus the new renderer in
  // that case.
  bool focus_render_view = !will_focus_location_bar &&
      render_view_host_->GetView() && render_view_host_->GetView()->HasFocus();

  // Swap in the pending view and make it active.
  RenderViewHostImpl* old_render_view_host = render_view_host_;
  render_view_host_ = pending_render_view_host_;
  pending_render_view_host_ = NULL;

  // The process will no longer try to exit, so we can decrement the count.
  render_view_host_->GetProcess()->RemovePendingView();

  // If the view is gone, then this RenderViewHost died while it was hidden.
  // We ignored the RenderProcessGone call at the time, so we should send it now
  // to make sure the sad tab shows up, etc.
  if (!render_view_host_->GetView())
    delegate_->RenderProcessGoneFromRenderManager(render_view_host_);
  else if (!delegate_->IsHidden())
    render_view_host_->GetView()->Show();

  // Hide the old view now that the new one is visible.
  if (old_render_view_host->GetView()) {
    old_render_view_host->GetView()->Hide();
    old_render_view_host->WasSwappedOut();
  }

  // Make sure the size is up to date.  (Fix for bug 1079768.)
  delegate_->UpdateRenderViewSizeForRenderManager();

  if (will_focus_location_bar)
    delegate_->SetFocusToLocationBar(false);
  else if (focus_render_view && render_view_host_->GetView())
    RenderWidgetHostViewPort::FromRWHV(render_view_host_->GetView())->Focus();

  std::pair<RenderViewHost*, RenderViewHost*> details =
      std::make_pair(old_render_view_host, render_view_host_);
  /*NotificationService::current()->Notify(
      NOTIFICATION_RENDER_VIEW_HOST_CHANGED,
      Source<NavigationController>(
          &delegate_->GetControllerForRenderManager()),
      Details<std::pair<RenderViewHost*, RenderViewHost*> >(&details));*/

  // If the pending view was on the swapped out list, we can remove it.
  swapped_out_hosts_.erase(render_view_host_->GetSiteInstance()->GetId());

  // Let the task manager know that we've swapped RenderViewHosts,
  // since it might need to update its process groupings. We do this
  // before shutting down the RVH so that we can clean up
  // RendererResources related to the RVH first.
  delegate_->NotifySwappedFromRenderManager(old_render_view_host);

  // If there are no active RVHs in this SiteInstance, it means that
  // this RVH was the last active one in the SiteInstance. Now that we
  // know that all RVHs are swapped out, we can delete all the RVHs in
  // this SiteInstance.
  if (!static_cast<SiteInstanceImpl*>(old_render_view_host->GetSiteInstance())->
          active_view_count()) {
    ShutdownRenderViewHostsInSiteInstance(
        old_render_view_host->GetSiteInstance()->GetId());
    // This is deleted while cleaning up the SitaInstance's views.
    old_render_view_host = NULL;
  } else if (old_render_view_host->IsRenderViewLive()) {
    // If the old RVH is live, we are swapping it out and should keep track of
    // it in case we navigate back to it.
    DCHECK(old_render_view_host->is_swapped_out());
    // Temp fix for http://crbug.com/90867 until we do a better cleanup to make
    // sure we don't get different rvh instances for the same site instance
    // in the same rvhmgr.
    // TODO(creis): Clean this up.
    int32 old_site_instance_id =
        old_render_view_host->GetSiteInstance()->GetId();
    RenderViewHostMap::iterator iter =
        swapped_out_hosts_.find(old_site_instance_id);
    if (iter != swapped_out_hosts_.end() &&
        iter->second != old_render_view_host) {
      // Shutdown the RVH that will be replaced in the map to avoid a leak.
      iter->second->Shutdown();
    }
    swapped_out_hosts_[old_site_instance_id] = old_render_view_host;
  } else {
    old_render_view_host->Shutdown();
    old_render_view_host = NULL;  // Shutdown() deletes it.
  }
}

void RenderViewHostManager::ShutdownRenderViewHostsInSiteInstance(
    int32 site_instance_id) {
  // First remove any swapped out RVH for this SiteInstance from our
  // list.
  swapped_out_hosts_.erase(site_instance_id);

  /*scoped_ptr<RenderWidgetHostIterator> widgets(
      RenderWidgetHostImpl::GetAllRenderWidgetHosts());
  while (RenderWidgetHost* widget = widgets->GetNextHost()) {
    if (!widget->IsRenderView())
      continue;
    RenderViewHostImpl* rvh =
        static_cast<RenderViewHostImpl*>(RenderViewHost::From(widget));
    if (site_instance_id == rvh->GetSiteInstance()->GetId())
      rvh->Shutdown();
  }*/
}

RenderViewHostImpl* RenderViewHostManager::UpdateRendererStateForNavigate(
    const NavigationEntryImpl& entry) {
  // If we are cross-navigating, then we want to get back to normal and navigate
  // as usual.
   if (cross_navigation_pending_) {
    if (pending_render_view_host_)
      CancelPending();
    cross_navigation_pending_ = false;
  }

  // render_view_host_ will not be deleted before the end of this method, so we
  // don't have to worry about this SiteInstance's ref count dropping to zero.
  SiteInstance* curr_instance = render_view_host_->GetSiteInstance();

  // Determine if we need a new SiteInstance for this entry.
  // Again, new_instance won't be deleted before the end of this method, so it
  // is safe to use a normal pointer here.
  SiteInstance* new_instance = curr_instance;
  const NavigationEntry* curr_entry = NULL;
      //delegate_->GetLastCommittedNavigationEntryForRenderManager();
  bool is_guest_scheme = false;
  bool force_swap = ShouldSwapProcessesForNavigation(curr_entry, &entry);
  if (!is_guest_scheme && (ShouldTransitionCrossSite() || force_swap)) {
    new_instance = GetSiteInstanceForEntry(entry, curr_instance);
  }
 
  if (!is_guest_scheme && (new_instance != curr_instance || force_swap)) {
    // New SiteInstance.
    DCHECK(!cross_navigation_pending_);

    // This will possibly create (set to NULL) a Web UI object for the pending
    // page. We'll use this later to give the page special access. This must
    // happen before the new renderer is created below so it will get bindings.
    // It must also happen after the above conditional call to CancelPending(),
    // otherwise CancelPending may clear the pending_web_ui_ and the page will
    // not have its bindings set appropriately.
    SetPendingWebUI(entry);

    // Ensure that we have created RVHs for the new RVH's opener chain if
    // we are staying in the same BrowsingInstance. This allows the pending RVH
    // to send cross-process script calls to its opener(s).
    int opener_route_id = MSG_ROUTING_NONE;
    if (new_instance->IsRelatedSiteInstance(curr_instance)) {
      opener_route_id =
          delegate_->CreateOpenerRenderViewsForRenderManager(new_instance);
    }

    // Create a non-swapped-out pending RVH with the given opener and navigate
    // it.
    int route_id = CreateRenderView(new_instance, opener_route_id, false,
                                    delegate_->IsHidden());
    if (route_id == MSG_ROUTING_NONE)
      return NULL;

    // Check if our current RVH is live before we set up a transition.
    if (!render_view_host_->IsRenderViewLive()) {
      if (!cross_navigation_pending_) {
        // The current RVH is not live.  There's no reason to sit around with a
        // sad tab or a newly created RVH while we wait for the pending RVH to
        // navigate.  Just switch to the pending RVH now and go back to non
        // cross-navigating (Note that we don't care about on{before}unload
        // handlers if the current RVH isn't live.)
        CommitPending();
        return render_view_host_;
      } else {
        NOTREACHED();
        return render_view_host_;
      }
    }
    // Otherwise, it's safe to treat this as a pending cross-site transition.

    // We need to wait until the beforeunload handler has run, unless we are
    // transferring an existing request (in which case it has already run).
    // Suspend the new render view (i.e., don't let it send the cross-site
    // Navigate message) until we hear back from the old renderer's
    // beforeunload handler.  If the handler returns false, we'll have to
    // cancel the request.
//     DCHECK(!pending_render_view_host_->are_navigations_suspended());
//     bool is_transfer =
//         entry.transferred_global_request_id() != GlobalRequestID();
//     if (!is_transfer) {
//       // Also make sure the old render view stops, in case a load is in
//       // progress.  (We don't want to do this for transfers, since it will
//       // interrupt the transfer with an unexpected DidStopLoading.)
//       render_view_host_->Send(
//           new ViewMsg_Stop(render_view_host_->GetRoutingID()));
// 
//       pending_render_view_host_->SetNavigationsSuspended(true,
//                                                          base::TimeTicks());
//     }

    // Tell the CrossSiteRequestManager that this RVH has a pending cross-site
    // request, so that ResourceDispatcherHost will know to tell us to run the
    // old page's unload handler before it sends the response.
    pending_render_view_host_->SetHasPendingCrossSiteRequest(true);

    // We now have a pending RVH.
    DCHECK(!cross_navigation_pending_);
    cross_navigation_pending_ = true;

    // Unless we are transferring an existing request, we should now
    // tell the old render view to run its beforeunload handler, since it
    // doesn't otherwise know that the cross-site request is happening.  This
    // will trigger a call to ShouldClosePage with the reply.
    //if (!is_transfer)
    //  render_view_host_->FirePageBeforeUnload(true);

    return pending_render_view_host_;
  } else {
    if (ShouldReuseWebUI(curr_entry, &entry)) {
      //pending_web_ui_.reset();
      //pending_and_current_web_ui_ = web_ui_->AsWeakPtr();
    } else {
      SetPendingWebUI(entry);

      // Make sure the new RenderViewHost has the right bindings.
      //if (pending_web_ui())
      //  render_view_host_->AllowBindings(pending_web_ui()->GetBindings());
    }

    //if (pending_web_ui() && render_view_host_->IsRenderViewLive())
    //  pending_web_ui()->GetController()->RenderViewReused(render_view_host_);

    // The renderer can exit view source mode when any error or cancellation
    // happen. We must overwrite to recover the mode.
//     if (entry.IsViewSourceMode()) {
//       render_view_host_->Send(
//           new ViewMsg_EnableViewSourceMode(render_view_host_->GetRoutingID()));
//     }
  }

  // Same SiteInstance can be used.  Navigate render_view_host_ if we are not
  // cross navigating.
  DCHECK(!cross_navigation_pending_);
  return render_view_host_;
}

void RenderViewHostManager::CancelPending() {
  RenderViewHostImpl* pending_render_view_host = pending_render_view_host_;
  pending_render_view_host_ = NULL;
#if defined (ENABLE_DEVTOOLS)
  RenderViewDevToolsAgentHost::OnCancelPendingNavigation(
      pending_render_view_host,
      render_view_host_);
#endif

  // We no longer need to prevent the process from exiting.
  pending_render_view_host->GetProcess()->RemovePendingView();

  // The pending RVH may already be on the swapped out list if we started to
  // swap it back in and then canceled.  If so, make sure it gets swapped out
  // again.  If it's not on the swapped out list (e.g., aborting a pending
  // load), then it's safe to shut down.
  if (IsOnSwappedOutList(pending_render_view_host)) {
    // Any currently suspended navigations are no longer needed.
    pending_render_view_host->CancelSuspendedNavigations();

    pending_render_view_host->SwapOut();
  } else {
    // We won't be coming back, so shut this one down.
    pending_render_view_host->Shutdown();
  }

  //pending_web_ui_.reset();
  //pending_and_current_web_ui_.reset();
}

void RenderViewHostManager::RenderViewDeleted(RenderViewHost* rvh) {
  // We are doing this in order to work around and to track a crasher
  // (http://crbug.com/23411) where it seems that pending_render_view_host_ is
  // deleted (not sure from where) but not NULLed.
  if (rvh == pending_render_view_host_) {
    // If you hit this NOTREACHED, please report it in the following bug
    // http://crbug.com/23411 Make sure to include what you were doing when it
    // happened  (navigating to a new page, closing a tab...) and if you can
    // reproduce.
    NOTREACHED();
    pending_render_view_host_ = NULL;
  }

  // Make sure deleted RVHs are not kept in the swapped out list while we are
  // still alive.  (If render_view_host_ is null, we're already being deleted.)
  if (!render_view_host_)
    return;
  // We can't look it up by SiteInstance ID, which may no longer be valid.
  for (RenderViewHostMap::iterator iter = swapped_out_hosts_.begin();
       iter != swapped_out_hosts_.end();
       ++iter) {
    if (iter->second == rvh) {
      swapped_out_hosts_.erase(iter);
      break;
    }
  }
}

bool RenderViewHostManager::IsOnSwappedOutList(RenderViewHost* rvh) const {
  if (!rvh->GetSiteInstance())
    return false;

  RenderViewHostMap::const_iterator iter = swapped_out_hosts_.find(
      rvh->GetSiteInstance()->GetId());
  if (iter == swapped_out_hosts_.end())
    return false;

  return iter->second == rvh;
}

RenderViewHostImpl* RenderViewHostManager::GetSwappedOutRenderViewHost(
    SiteInstance* instance) {
  RenderViewHostMap::iterator iter = swapped_out_hosts_.find(instance->GetId());
  if (iter != swapped_out_hosts_.end())
    return iter->second;

  return NULL;
}

}  // namespace content
