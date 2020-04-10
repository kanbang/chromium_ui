// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_PUBLIC_BROWSER_WEB_CONTENTS_IMPL_H_
#define CONTENT_PUBLIC_BROWSER_WEB_CONTENTS_IMPL_H_

#include <set>

#include "base/basictypes.h"
#include "base/callback_forward.h"
#include "base/files/file_path.h"
#include "base/process/kill.h"
#include "base/strings/string16.h"
#include "base/supports_user_data.h"
#include "base/observer_list.h"
#include "third_party/skia/include/core/SkColor.h"
#include "ui/base/window_open_disposition.h"
#include "ui/gfx/native_widget_types.h"
#include "ui/gfx/size.h"
#include "chrome/browser/navigation_entry_impl.h"
#include "chrome/browser/gurl.h"
#include "chrome/browser/web_contents.h"
#include "chrome/browser/navigation_controller_impl.h"
#include "chrome/render/web_contents_view_port.h"
#include "chrome/common/render_view_host_delegate.h"
#include "chrome/render/render_host/render_widget_host_delegate.h"
#include "chrome/render/render_host/render_view_host_manager.h"

namespace base {
class TimeTicks;
}

namespace gfx {
class Rect;
class Size;
}

namespace net {
struct LoadStateWithParam;
}

namespace content {

class BrowserContext;
class InterstitialPage;
//class PageState;
class RenderProcessHost;
class RenderViewHost;
class RenderViewHostImpl;
class RenderWidgetHostView;
class SiteInstance;
class WebContentsDelegate;
class WebContentsView;
class WebContentsViewDelegate;
struct RendererPreferences;

WebContentsViewPort* CreateWebContentsView(
  WebContentsImpl* web_contents,
  WebContentsViewDelegate* delegate,
  RenderViewHostDelegateView** render_view_host_delegate_view);

// WebContents is the core class in content/. A WebContents renders web content
// (usually HTML) in a rectangular area.
//
// Instantiating one is simple:
//   scoped_ptr<content::WebContents> web_contents(
//       content::WebContents::Create(
//           content::WebContents::CreateParams(browser_context)));
//   gfx::NativeView view = web_contents->GetView()->GetNativeView();
//   // |view| is an HWND, NSView*, GtkWidget*, etc.; insert it into the view
//   // hierarchy wherever it needs to go.
//
// That's it; go to your kitchen, grab a scone, and chill. WebContents will do
// all the multi-process stuff behind the scenes. More details are at
// http://www.chromium.org/developers/design-documents/multi-process-architecture .
//
// Each WebContents has exactly one NavigationController; each
// NavigationController belongs to one WebContents. The NavigationController can
// be obtained from GetController(), and is used to load URLs into the
// WebContents, navigate it backwards/forwards, etc. See navigation_controller.h
// for more details.
class WebContentsImpl : public WebContents,
                        public RenderViewHostDelegate,
                        public RenderWidgetHostDelegate,
                        public RenderViewHostManager::Delegate{
 public:

  WebContentsImpl(BrowserContext* browser_context);
  ~WebContentsImpl();

  static WebContentsImpl* CreateWithOpener(const content::WebContents::CreateParams& prarams);
  // Intrinsic tab state -------------------------------------------------------

  // Gets/Sets the delegate.
  virtual WebContentsDelegate* GetDelegate() OVERRIDE;
  virtual void SetDelegate(WebContentsDelegate* delegate) OVERRIDE;
  virtual content::BrowserContext* GetBrowserContext() const OVERRIDE;
  virtual const GURL& GetURL() const OVERRIDE;
  virtual const GURL& GetVisibleURL() const OVERRIDE;
  virtual const GURL& GetLastCommittedURL() const OVERRIDE;
  virtual WebContents* GetEmbedderWebContents() const OVERRIDE;
  virtual int GetEmbeddedInstanceID() const OVERRIDE;
  virtual int GetRoutingID() const OVERRIDE;
  virtual RenderWidgetHostView* GetRenderWidgetHostView() const OVERRIDE;
  virtual RenderWidgetHostView* GetFullscreenRenderWidgetHostView() const OVERRIDE;
  virtual WebContentsView* GetView() const OVERRIDE;
  virtual void SetUserAgentOverride(const std::string& override) OVERRIDE;
  virtual const std::string& GetUserAgentOverride() const OVERRIDE;
  virtual RenderViewHost* GetRenderViewHost() const OVERRIDE;

  virtual NavigationController& GetController() OVERRIDE;
  virtual const NavigationController& GetController() const OVERRIDE;

#if defined(OS_WIN) && defined(USE_AURA)
  virtual void SetParentNativeViewAccessible(
      gfx::NativeViewAccessible accessible_parent) OVERRIDE;
#endif

  virtual const string16& GetTitle() const OVERRIDE;
  virtual int32 GetMaxPageID() OVERRIDE;
  virtual bool IsLoading() const OVERRIDE;
  virtual bool IsWaitingForResponse() const OVERRIDE;
  virtual const net::LoadStateWithParam& GetLoadState() const OVERRIDE;
  virtual const string16& GetLoadStateHost() const OVERRIDE;
  virtual uint64 GetUploadSize() const OVERRIDE;
  virtual uint64 GetUploadPosition() const OVERRIDE;
  virtual std::set<GURL> GetSitesInTab() const OVERRIDE;
  virtual const std::string& GetEncoding() const OVERRIDE;
  virtual bool DisplayedInsecureContent() const OVERRIDE;
  virtual void IncrementCapturerCount() OVERRIDE;
  virtual void DecrementCapturerCount() OVERRIDE;
  virtual int GetCapturerCount() const OVERRIDE;
  virtual bool IsCrashed() const  OVERRIDE;
  virtual void SetIsCrashed(base::TerminationStatus status, int error_code) OVERRIDE;
  virtual base::TerminationStatus GetCrashedStatus() const OVERRIDE;
  virtual bool IsBeingDestroyed() const OVERRIDE;
  virtual void NotifyNavigationStateChanged(unsigned changed_flags) OVERRIDE;
  virtual base::TimeTicks GetLastSelectedTime() const OVERRIDE;
  virtual void WasShown() OVERRIDE;
  virtual void WasHidden() OVERRIDE;
  virtual bool NeedToFireBeforeUnload() OVERRIDE;
  virtual void Stop() OVERRIDE;
  virtual WebContents* Clone() OVERRIDE;
  virtual void FocusThroughTabTraversal(bool reverse) OVERRIDE;
  virtual bool ShowingInterstitialPage() const OVERRIDE;
  virtual bool IsSavable() OVERRIDE;
  virtual void OnSavePage() OVERRIDE;
  virtual void GenerateMHTML(
      const base::FilePath& file,
      const base::Callback<void(
          int64 /* size of the file */)>& callback) OVERRIDE;
  virtual bool IsActiveEntry(int32 page_id) OVERRIDE;
  virtual const std::string& GetContentsMimeType() const OVERRIDE;
  virtual bool WillNotifyDisconnection() const OVERRIDE;
  virtual void SetOverrideEncoding(const std::string& encoding) OVERRIDE;
  virtual void ResetOverrideEncoding() OVERRIDE;
  virtual content::RendererPreferences* GetMutableRendererPrefs() OVERRIDE;
  virtual void Close() OVERRIDE;
  virtual void SystemDragEnded() OVERRIDE;
  virtual void UserGestureDone() OVERRIDE;
  virtual void SetClosedByUserGesture(bool value) OVERRIDE;
  virtual bool GetClosedByUserGesture() const OVERRIDE;
  virtual double GetZoomLevel() const OVERRIDE;
  virtual int GetZoomPercent(bool* enable_increment,
                             bool* enable_decrement) const OVERRIDE;
  virtual void ViewSource() OVERRIDE;
  virtual int GetMinimumZoomPercent() const OVERRIDE;
  virtual int GetMaximumZoomPercent() const OVERRIDE;
  virtual gfx::Size GetPreferredSize() const OVERRIDE;
  virtual bool GotResponseToLockMouseRequest(bool allowed) OVERRIDE;
  virtual void DidChooseColorInColorChooser(SkColor color) OVERRIDE;
  virtual void DidEndColorChooser() OVERRIDE;
  virtual bool FocusLocationBarByDefault() OVERRIDE;
  virtual bool HasOpener() const OVERRIDE;
  virtual void NavigateUrl(const GURL& url) OVERRIDE;

  // render_view_host_delegate
  virtual void CreateNewWindow(int route_id) OVERRIDE;
  virtual void UpdateTitle(RenderViewHost* render_view_host,
    int32 page_id,
    const string16& title) OVERRIDE;
  virtual void UpdateUrl(RenderViewHost* render_view_host,
    int32 page_id,
    const string16& Url) OVERRIDE;

   virtual RenderViewHostDelegateView* GetDelegateView() OVERRIDE;

  // manage delegate
  virtual bool CreateRenderViewForRenderManager(
    RenderViewHost* render_view_host, int opener_route_id) OVERRIDE;
  virtual void BeforeUnloadFiredFromRenderManager(
    bool proceed, const base::TimeTicks& proceed_time,
    bool* proceed_to_fire_unload) OVERRIDE;
  virtual void RenderProcessGoneFromRenderManager(
    RenderViewHost* render_view_host) OVERRIDE;
  virtual void UpdateRenderViewSizeForRenderManager() OVERRIDE;
  virtual void CancelModalDialogsForRenderManager() OVERRIDE;
  virtual void NotifySwappedFromRenderManager(
    RenderViewHost* old_render_view_host) OVERRIDE;
  virtual NavigationControllerImpl&
    GetControllerForRenderManager() OVERRIDE;

  // Create swapped out RenderViews in the given SiteInstance for each tab in
  // the opener chain of this tab, if any.  This allows the current tab to
  // make cross-process script calls to its opener(s).  Returns the route ID
  // of the immediate opener, if one exists (otherwise MSG_ROUTING_NONE).
  virtual int CreateOpenerRenderViewsForRenderManager(
    SiteInstance* instance) OVERRIDE;

  // Creates a WebUI object for the given URL if one applies. Ownership of the
  // returned pointer will be passed to the caller. If no WebUI applies,
  // returns NULL.
  //virtual WebUIImpl* CreateWebUIForRenderManager(const GURL& url) = 0;

  // Returns the navigation entry of the current navigation, or NULL if there
  // is none.
  //virtual NavigationEntry*
  //  GetLastCommittedNavigationEntryForRenderManager() OVERRIDE;

  // Returns true if the location bar should be focused by default rather than
  // the page contents. The view calls this function when the tab is focused
  // to see what it should do.
  //virtual bool FocusLocationBarByDefault() OVERRIDE;

  // Focuses the location bar.
  virtual void SetFocusToLocationBar(bool select_all) OVERRIDE;

  // Creates a view and sets the size for the specified RVH.
  virtual void CreateViewAndSetSizeForRVH(RenderViewHost* rvh) OVERRIDE;

  // Returns true if views created for this delegate should be created in a
  // hidden state.
  virtual bool IsHidden() OVERRIDE;

 private:
  friend class WebContentsObserver;
  friend class WebContents;  // To implement factory methods.

  void AddObserver(WebContentsObserver* observer);
  void RemoveObserver(WebContentsObserver* observer);
  void InitView(const content::WebContents::CreateParams& prarams);
  void Init(RenderViewHost* source, int browserid);

  bool NavigateToEntry(const NavigationEntryImpl& entry,
    int reload_type);
  // Helper function to invoke WebContentsDelegate::GetSizeForNewRenderView().
  gfx::Size GetSizeForNewRenderView() const;
  NavigationControllerImpl controller_;
  BrowserContext* browser_context_;

  // The corresponding view.
  scoped_ptr<WebContentsViewPort> view_;
  scoped_ptr<RenderViewHost>  rvh_;
  //scoped_ptr<RenderProcessHost> render_process_host_;

  // Manages creation and swapping of render views.
  RenderViewHostManager render_manager_;

  RenderViewHostDelegateView* render_view_host_delegate_view_;

  ObserverList<WebContentsObserver> observers_;

  WebContentsDelegate* delegate_;
  GURL url_;
  GURL visible_url_;
  std::string user_agent_;
  string16 title_;

  net::LoadStateWithParam load_state_;
  std::string encoding_;
  base::TimeTicks time_ticks_;
  std::string mime_type_;
  int opener_id_;
};

}  // namespace content

#endif  // CONTENT_PUBLIC_BROWSER_WEB_CONTENTS_IMPL_H_
