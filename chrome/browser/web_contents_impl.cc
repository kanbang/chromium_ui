#include "chrome/browser/web_contents_impl.h"

#include "base/process/kill.h"
#include "chrome/browser/web_contents_observer.h"
#include "chrome/browser/web_contents_view_delegate.h"
#include "chrome/browser/web_contents_delegate.h"
#include "chrome/ui/invalidate_type.h"
#include "chrome/ui/views/tab_contents/chrome_web_contents_view_delegate_views.h"
#include "chrome/common/render_process_host_impl.h"
#include "chrome/render/render_host/render_view_host_impl.h"
#include "chrome/render/render_host/render_widget_host_view_port.h"
#include "chrome/render/site_instance_impl.h"
#include "chrome/render/browsing_instance.h"
#include "chrome/browser/gurl.h"

namespace content {
  WebContents* WebContents::Create(const WebContents::CreateParams& params) {
    return WebContentsImpl::CreateWithOpener(params);
  }

WebContentsImpl* WebContentsImpl::CreateWithOpener(const content::WebContents::CreateParams& prarams) {
  WebContentsImpl* pImpl = new WebContentsImpl(prarams.browser_context);
  if(pImpl)
    pImpl->InitView(prarams);
  
  return pImpl;
}

WebContentsImpl::WebContentsImpl(BrowserContext* browser_context) 
  : render_manager_(this, this, this),
  controller_(this, browser_context),
  browser_context_(browser_context){
  title_ = L"New Tab Test";
  delegate_ = NULL;
  opener_id_ = MSG_ROUTING_NONE;
  //url_ = GURL("http://www.so.com/");
  render_view_host_delegate_view_ = NULL;
}

WebContentsImpl::~WebContentsImpl() {
  FOR_EACH_OBSERVER(WebContentsObserver,
    observers_,
    WebContentsImplDestroyed());
}

WebContentsDelegate* WebContentsImpl::GetDelegate() {
  return delegate_;
}

void WebContentsImpl::SetDelegate(WebContentsDelegate* delegate) {
  delegate_ = delegate;
}

BrowserContext* WebContentsImpl::GetBrowserContext() const {
  return render_manager_.current_host()->GetSiteInstance()->GetBrowserContext();
}

const GURL& WebContentsImpl::GetURL() const {
  return url_;
}

const GURL& WebContentsImpl::GetVisibleURL() const {
  return visible_url_;
}

const GURL& WebContentsImpl::GetLastCommittedURL() const {
  return url_;
}

WebContents* WebContentsImpl::GetEmbedderWebContents() const {
  return NULL;
}

int WebContentsImpl::GetEmbeddedInstanceID() const {
  return 0;
}

int WebContentsImpl::GetRoutingID() const {
  if (!GetRenderViewHost())
    return MSG_ROUTING_NONE;

  return GetRenderViewHost()->GetRoutingID();
}

WebContentsView* WebContentsImpl::GetView() const {
  return view_.get();
}

RenderWidgetHostView* WebContentsImpl::GetRenderWidgetHostView() const {
  return render_manager_.GetRenderWidgetHostView();
}

RenderWidgetHostView* WebContentsImpl::GetFullscreenRenderWidgetHostView() const {
  return RenderWidgetHostViewPort::FromRWHV(GetRenderWidgetHostView());
}

void WebContentsImpl::SetUserAgentOverride(const std::string& override) {
  user_agent_ = override;
}

const std::string& WebContentsImpl::GetUserAgentOverride() const {
  return user_agent_;
}

 RenderViewHost* WebContentsImpl::GetRenderViewHost() const {
   return render_manager_.current_host();
 }

#if defined(OS_WIN) && defined(USE_AURA)
  void WebContentsImpl::SetParentNativeViewAccessible(
      gfx::NativeViewAccessible accessible_parent) {

  }
#endif

  const string16& WebContentsImpl::GetTitle() const {
    return title_;
  }

  int32 WebContentsImpl::GetMaxPageID() {
    return 0;
  }

  bool WebContentsImpl::IsLoading() const {
    return false;
  }

  bool WebContentsImpl::IsWaitingForResponse() const {
    return false;
  }

  const net::LoadStateWithParam& WebContentsImpl::GetLoadState() const {
    return load_state_;
  }

  const string16& WebContentsImpl::GetLoadStateHost() const {
    return load_state_.host_;
  }

  uint64 WebContentsImpl::GetUploadSize() const {
    return 0;
  }

  uint64 WebContentsImpl::GetUploadPosition() const {
    return 0;
  }

  std::set<GURL> WebContentsImpl::GetSitesInTab() const {
    return std::set<GURL>();
  }

  const std::string& WebContentsImpl::GetEncoding() const {
    return encoding_;
  }

  bool WebContentsImpl::DisplayedInsecureContent() const {
    return false;
  }

  void WebContentsImpl::IncrementCapturerCount() {

  }

  void WebContentsImpl::DecrementCapturerCount() {

  }

  int WebContentsImpl::GetCapturerCount() const {
    return 0;
  }

  bool WebContentsImpl::IsCrashed() const {
    return false;
  }

  void WebContentsImpl::SetIsCrashed(base::TerminationStatus status, int error_code) {

  }

  base::TerminationStatus WebContentsImpl::GetCrashedStatus() const {
    return base::TERMINATION_STATUS_NORMAL_TERMINATION;
  }

  bool WebContentsImpl::IsBeingDestroyed() const {
    return false;
  }

  void WebContentsImpl::NotifyNavigationStateChanged(unsigned changed_flags) {

  }

  base::TimeTicks WebContentsImpl::GetLastSelectedTime() const {
    return time_ticks_;
  }

  void WebContentsImpl::WasShown() {

  }

  void WebContentsImpl::WasHidden() {

  }

  bool WebContentsImpl::NeedToFireBeforeUnload() {
    return false;
  }

  void WebContentsImpl::Stop() {

  }

  WebContents* WebContentsImpl::Clone() {
    return NULL;
  }

  void WebContentsImpl::FocusThroughTabTraversal(bool reverse) {

  }

  bool WebContentsImpl::ShowingInterstitialPage() const {
    return false;
  }

  bool WebContentsImpl::IsSavable() {
    return false;
  }

  void WebContentsImpl::OnSavePage() {

  }

  void WebContentsImpl::GenerateMHTML(
      const base::FilePath& file,
      const base::Callback<void(
          int64 /* size of the file */)>& callback) {

  }

  bool WebContentsImpl::IsActiveEntry(int32 page_id) {
    return false;
  }

  const std::string& WebContentsImpl::GetContentsMimeType() const {
    return mime_type_;
  }

  bool WebContentsImpl::WillNotifyDisconnection() const {
    return false;
  }

  void WebContentsImpl::SetOverrideEncoding(const std::string& encoding) {

  }

  void WebContentsImpl::ResetOverrideEncoding() {

  }

  content::RendererPreferences* WebContentsImpl::GetMutableRendererPrefs() {
    return NULL;
  }

  void WebContentsImpl::Close() {

  }

  void WebContentsImpl::SystemDragEnded() {

  }

  void WebContentsImpl::UserGestureDone() {

  }

  void WebContentsImpl::SetClosedByUserGesture(bool value) {

  }

  bool WebContentsImpl::GetClosedByUserGesture() const {
    return false;
  }

  double WebContentsImpl::GetZoomLevel() const {
    return 1.0;
  }

  int WebContentsImpl::GetZoomPercent(bool* enable_increment,
                             bool* enable_decrement) const {
    *enable_decrement = true;
    *enable_increment = true;
    return 100;
  }

  void WebContentsImpl::ViewSource() {

  }

  int WebContentsImpl::GetMinimumZoomPercent() const {
    return 10;
  }

  int WebContentsImpl:: GetMaximumZoomPercent() const {
    return 1000;
  }

  gfx::Size WebContentsImpl::GetPreferredSize() const {
    return gfx::Size();
  }

  bool WebContentsImpl::GotResponseToLockMouseRequest(bool allowed) {
    return false;
  }

  void WebContentsImpl::DidChooseColorInColorChooser(SkColor color) {

  }

  void WebContentsImpl::DidEndColorChooser() {

  }

  bool WebContentsImpl::FocusLocationBarByDefault() {
    return false;
  }

  bool WebContentsImpl::HasOpener() const {
    return false;
  }

  void WebContentsImpl::NavigateUrl(const GURL& url) {
    NavigationEntryImpl NaviEntry;
    NaviEntry.SetURL(url);

    NavigateToEntry(NaviEntry, 0);
  }

  void WebContentsImpl::AddObserver(WebContentsObserver* observer) {
    observers_.AddObserver(observer);
  }

  void WebContentsImpl::RemoveObserver(WebContentsObserver* observer) {
    observers_.RemoveObserver(observer);
  }

  void WebContentsImpl::InitView(const content::WebContents::CreateParams& params) {
    WebContentsViewDelegate* delegate = chrome::CreateWebContentsViewDelegate(this);
  
    //if(params.opener)
    //  opener_id_ = params.opener->GetRoutingID();

    render_manager_.Init(
      params.browser_context, params.site_instance, params.routing_id,
      params.main_frame_routing_id);

    // Regular WebContentsView.
    view_.reset(CreateWebContentsView(
      this, delegate, &render_view_host_delegate_view_));

    CHECK(view_.get());
    gfx::Size initial_size = params.initial_size;
    gfx::NativeView context = NULL;
    view_->CreateView(initial_size, context);
    
    NavigationEntryImpl NaviEntry;
    
    GURL url("http://hao.360.cn/");
    NaviEntry.SetURL(url);

    NavigateToEntry(NaviEntry, 0);

    /*gfx::NativeView native_view = view_->GetNativeView();
    SiteInstanceImpl* impl = new SiteInstanceImpl(new BrowsingInstance(params.browser_context));
    rvh_.reset(new RenderViewHostImpl(impl, this, NULL, MSG_ROUTING_NONE, MSG_ROUTING_NONE, false, false));
    if(rvh_)
      (static_cast<RenderViewHostImpl*>(rvh_.get()))->CreateRenderView(string16(), MSG_ROUTING_NONE, -1, native_view);*/
  }

  void WebContentsImpl::Init(RenderViewHost* source, int browserid) {
    WebContentsViewDelegate* delegate = chrome::CreateWebContentsViewDelegate(this);

    // Regular WebContentsView.
    view_.reset(CreateWebContentsView(
      this, delegate, &render_view_host_delegate_view_));

    CHECK(view_.get());
    gfx::Size initial_size;// = params.initial_size;
    gfx::NativeView context = NULL;
    view_->CreateView(initial_size, context);

    gfx::NativeView native_view = view_->GetNativeView();
    rvh_.reset(new RenderViewHostImpl(source->GetSiteInstance(), this, NULL, MSG_ROUTING_NONE, MSG_ROUTING_NONE, false, false));
    if(rvh_)
      (static_cast<RenderViewHostImpl*>(rvh_.get()))->CreateRenderView(string16(), browserid, -1, native_view);
    gfx::Rect initial_rect;// = params.initial_size;
    if(delegate_) 
      delegate_->AddNewContents(NULL, this, (WindowOpenDisposition)0, initial_rect, false, NULL);
  }

   void WebContentsImpl::CreateNewWindow(int route_id) {
     // We usually create the new window in the same BrowsingInstance (group of
     // script-related windows), by passing in the current SiteInstance.  However,
     // if the opener is being suppressed (in a non-guest), we create a new
     // SiteInstance in its own BrowsingInstance.
     bool is_guest = false;

     // Create the new web contents. This will automatically create the new
     // WebContentsView. In the future, we may want to create the view separately.
     WebContentsImpl* new_contents =
       new WebContentsImpl(GetBrowserContext());

     new_contents->opener_id_ = route_id;
     CreateParams create_params(GetBrowserContext(), render_manager_.current_host()->GetSiteInstance());
     create_params.opener = this;
     create_params.routing_id = MSG_ROUTING_NONE;
     create_params.main_frame_routing_id = MSG_ROUTING_NONE;
     create_params.context = view_->GetNativeView();
     create_params.initial_size = view_->GetContainerSize();
     new_contents->InitView(create_params);

     gfx::Rect initial_rect;
     if(delegate_)
       delegate_->AddNewContents(this, new_contents, (WindowOpenDisposition)0, initial_rect, false, NULL);
     WebContentsViewPort* new_view = new_contents->view_.get();
     new_view->CreateViewForWidget(new_contents->GetRenderViewHost());

     /*WebContentsImpl* pImpl = new WebContentsImpl();
     if(pImpl) {
       pImpl->SetDelegate(delegate_);
       pImpl->Init(rvh_.get(), route_id);
     }*/
   }

   void WebContentsImpl::UpdateTitle(RenderViewHost* render_view_host,
     int32 page_id,
     const string16& title) {
       title_ = title;
       if(delegate_) {
         delegate_->NavigationStateChanged(this, content::INVALIDATE_TYPE_TITLE);
       }
   }

   void WebContentsImpl::UpdateUrl(RenderViewHost* render_view_host,
     int32 page_id, const string16& Url) {
       url_ = GURL(Url);
       if(delegate_)
         delegate_->NavigationStateChanged(this, content::INVALIDATE_TYPE_URL);
   }

   // manage delegate
   bool WebContentsImpl::CreateRenderViewForRenderManager(
     RenderViewHost* render_view_host, int opener_route_id) {
       TRACE_EVENT0("browser", "WebContentsImpl::CreateRenderViewForRenderManager");
       // Can be NULL during tests.
       RenderWidgetHostView* rwh_view = view_->CreateViewForWidget(render_view_host);

       // Now that the RenderView has been created, we need to tell it its size.
       if (rwh_view)
         rwh_view->SetSize(GetSizeForNewRenderView());

       if (!static_cast<RenderViewHostImpl*>(
         render_view_host)->CreateRenderView(string16(),
         opener_route_id,
         10000000, rwh_view->GetNativeView())) {
           return false;
       }

#if defined(OS_POSIX) && !defined(OS_MACOSX) && !defined(OS_ANDROID)
       // Force a ViewMsg_Resize to be sent, needed to make plugins show up on
       // linux. See crbug.com/83941.
       if (rwh_view) {
         if (RenderWidgetHost* render_widget_host = rwh_view->GetRenderWidgetHost())
           render_widget_host->WasResized();
       }
#endif

       return true;
   }

   void WebContentsImpl::BeforeUnloadFiredFromRenderManager(
     bool proceed, const base::TimeTicks& proceed_time,
     bool* proceed_to_fire_unload) {

   }

   void WebContentsImpl::RenderProcessGoneFromRenderManager(
     RenderViewHost* render_view_host) {

   }

   void WebContentsImpl::UpdateRenderViewSizeForRenderManager() {

   }

   void WebContentsImpl::CancelModalDialogsForRenderManager() {

   }

   void WebContentsImpl::NotifySwappedFromRenderManager(
     RenderViewHost* old_render_view_host) {

   }

   NavigationControllerImpl&
     WebContentsImpl::GetControllerForRenderManager() {
       return controller_;
   }

   NavigationController& WebContentsImpl::GetController() {
     return controller_;
   }

   const NavigationController& WebContentsImpl::GetController() const {
     return controller_;
   }

   int WebContentsImpl::CreateOpenerRenderViewsForRenderManager(
     SiteInstance* instance) {
       return opener_id_;
   }

//    bool WebContentsImpl::FocusLocationBarByDefault() {
//      return false;
//    }

   void WebContentsImpl::SetFocusToLocationBar(bool select_all) {

   }

   void WebContentsImpl::CreateViewAndSetSizeForRVH(RenderViewHost* rvh) {

   }

   bool WebContentsImpl::IsHidden() {
     return false;
   }

   gfx::Size WebContentsImpl::GetSizeForNewRenderView() const {
     gfx::Size size;
     if (delegate_)
       size = delegate_->GetSizeForNewRenderView(this);
     if (size.IsEmpty())
       size = view_->GetContainerSize();
     return size;
   }

   bool WebContentsImpl::NavigateToEntry(const NavigationEntryImpl& entry,
     int reload_type) {
       const size_t kMaxURLChars = 2 * 1024 * 1024;
       // The renderer will reject IPC messages with URLs longer than
       // this limit, so don't attempt to navigate with a longer URL.
       if (entry.GetURL().spec().size() > kMaxURLChars) {
         LOG(WARNING) << "Refusing to load URL as it exceeds " << kMaxURLChars
           << " characters.";
         return false;
       }

       RenderViewHostImpl* dest_render_view_host =
         static_cast<RenderViewHostImpl*>(render_manager_.Navigate(entry));
       if (!dest_render_view_host)
         return false;  // Unable to create the desired render view host.

       // For security, we should never send non-Web-UI URLs to a Web UI renderer.
       // Double check that here.
       int enabled_bindings = dest_render_view_host->GetEnabledBindings();
       bool data_urls_allowed = delegate_ && delegate_->CanLoadDataURLsInWebUI();
       bool is_allowed_in_web_ui_renderer = false;
      
       // Notify observers that we will navigate in this RV.
       FOR_EACH_OBSERVER(WebContentsObserver,
         observers_,
         AboutToNavigateRenderView(dest_render_view_host));

       // Used for page load time metrics.
       //current_load_start_ = base::TimeTicks::Now();

       // Delay load
       /*BrowserThread::PostDelayedTask(BrowserThread::UI, FROM_HERE,
         base::Bind(&WebContentsImpl::DelayLoad,
         GetWeakPtr()), base::TimeDelta::FromSeconds(kDelayLoadSeconds));*/

       // Navigate in the desired RenderViewHost.
       ViewMsg_Navigate_Params navigate_params;
       navigate_params.url = entry.GetURL().spec();
       dest_render_view_host->Navigate(navigate_params);

       if (entry.GetPageID() == -1) {
         // HACK!!  This code suppresses javascript: URLs from being added to
         // session history, which is what we want to do for javascript: URLs that
         // do not generate content.  What we really need is a message from the
         // renderer telling us that a new page was not created.  The same message
         // could be used for mailto: URLs and the like.
         if (entry.GetURL().SchemeIs("javascript"))
           return false;
       }

       // Notify observers about navigation.
       FOR_EACH_OBSERVER(WebContentsObserver,
         observers_,
         NavigateToPendingEntry(entry.GetURL(), reload_type));

       if (delegate_)
         delegate_->DidNavigateToPendingEntry(this);

       return true;
   }

   RenderViewHostDelegateView* WebContentsImpl::GetDelegateView() {
     return render_view_host_delegate_view_;
   }
}