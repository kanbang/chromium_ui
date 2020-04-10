// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// When each service is created, we set a flag indicating this. At this point,
// the service initialization could fail or succeed. This allows us to remember
// if we tried to create a service, and not try creating it over and over if
// the creation failed.

#ifndef CHROME_BROWSER_BROWSER_PROCESS_IMPL_H_
#define CHROME_BROWSER_BROWSER_PROCESS_IMPL_H_

#include <string>

#include "base/basictypes.h"
#include "base/debug/stack_trace.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "base/prefs/pref_change_registrar.h"
#include "base/threading/non_thread_safe.h"
#include "base/timer/timer.h"
#include "chrome/browser/browser_process.h"

class ChromeNetLog;
class ChromeResourceDispatcherHostDelegate;
class CommandLine;
class RemoteDebuggingServer;
class PrefRegistrySimple;
class PromoResourceService;

#if defined(ENABLE_PLUGIN_INSTALLATION)
class PluginsResourceService;
#endif

namespace base {
class SequencedTaskRunner;
}

namespace policy {
class BrowserPolicyConnector;
class PolicyService;
};

// Real implementation of BrowserProcess that creates and returns the services.
class BrowserProcessImpl : public BrowserProcess,
                           public base::NonThreadSafe {
 public:
  // |local_state_task_runner| must be a shutdown-blocking task runner.
  BrowserProcessImpl(base::SequencedTaskRunner* local_state_task_runner,
                     const CommandLine& command_line);
  virtual ~BrowserProcessImpl();

  // Called before the browser threads are created.
  void PreCreateThreads();

  // Called after the threads have been created but before the message loops
  // starts running. Allows the browser process to do any initialization that
  // requires all threads running.
  void PreMainMessageLoopRun();

  // Most cleanup is done by these functions, driven from
  // ChromeBrowserMain based on notifications from the content
  // framework, rather than in the destructor, so that we can
  // interleave cleanup with threads being stopped.
  void StartTearDown();
  void PostDestroyThreads();

  // BrowserProcess implementation.
//   virtual void ResourceDispatcherHostCreated() OVERRIDE;
//   virtual void EndSession() OVERRIDE;
//   virtual MetricsService* metrics_service() OVERRIDE;
//   virtual IOThread* io_thread() OVERRIDE;
//   virtual WatchDogThread* watchdog_thread() OVERRIDE;
//   virtual ProfileManager* profile_manager() OVERRIDE;
//   virtual PrefService* local_state() OVERRIDE;
//   virtual net::URLRequestContextGetter* system_request_context() OVERRIDE;
//   virtual chrome_variations::VariationsService* variations_service() OVERRIDE;
//   virtual BrowserProcessPlatformPart* platform_part() OVERRIDE;
//   virtual extensions::EventRouterForwarder*
//         extension_event_router_forwarder() OVERRIDE;
//   virtual NotificationUIManager* notification_ui_manager() OVERRIDE;
//   virtual message_center::MessageCenter* message_center() OVERRIDE;
//   virtual policy::BrowserPolicyConnector* browser_policy_connector() OVERRIDE;
//   virtual policy::PolicyService* policy_service() OVERRIDE;
//   virtual IconManager* icon_manager() OVERRIDE;
//   virtual GLStringManager* gl_string_manager() OVERRIDE;
//   virtual GpuModeManager* gpu_mode_manager() OVERRIDE;
//   virtual RenderWidgetSnapshotTaker* GetRenderWidgetSnapshotTaker() OVERRIDE;
//   virtual AutomationProviderList* GetAutomationProviderList() OVERRIDE;
//   virtual void CreateDevToolsHttpProtocolHandler(
//       chrome::HostDesktopType host_desktop_type,
//       const std::string& ip,
//       int port,
//       const std::string& frontend_url) OVERRIDE;
//   virtual unsigned int AddRefModule() OVERRIDE;
//   virtual unsigned int ReleaseModule() OVERRIDE;
  virtual bool IsShuttingDown() OVERRIDE;
//   virtual printing::PrintJobManager* print_job_manager() OVERRIDE;
//   virtual printing::PrintPreviewDialogController*
//       print_preview_dialog_controller() OVERRIDE;
//   virtual printing::BackgroundPrintingManager*
//       background_printing_manager() OVERRIDE;
//   virtual IntranetRedirectDetector* intranet_redirect_detector() OVERRIDE;
//   virtual const std::string& GetApplicationLocale() OVERRIDE;
//   virtual void SetApplicationLocale(const std::string& locale) OVERRIDE;
//   virtual DownloadStatusUpdater* download_status_updater() OVERRIDE;
//   virtual DownloadRequestLimiter* download_request_limiter() OVERRIDE;
//   virtual BackgroundModeManager* background_mode_manager() OVERRIDE;
//   virtual void set_background_mode_manager_for_test(
//       scoped_ptr<BackgroundModeManager> manager) OVERRIDE;
//   virtual StatusTray* status_tray() OVERRIDE;
//   virtual SafeBrowsingService* safe_browsing_service() OVERRIDE;
//   virtual safe_browsing::ClientSideDetectionService*
//       safe_browsing_detection_service() OVERRIDE;

#if (defined(OS_WIN) || defined(OS_LINUX)) && !defined(OS_CHROMEOS)
  virtual void StartAutoupdateTimer() OVERRIDE{};
#endif

//   virtual ChromeNetLog* net_log() OVERRIDE;
//   virtual prerender::PrerenderTracker* prerender_tracker() OVERRIDE;
//   virtual ComponentUpdateService* component_updater() OVERRIDE;
//   virtual CRLSetFetcher* crl_set_fetcher() OVERRIDE;
//   virtual PnaclComponentInstaller* pnacl_component_installer() OVERRIDE;
//   virtual BookmarkPromptController* bookmark_prompt_controller() OVERRIDE;
//   virtual StorageMonitor* storage_monitor() OVERRIDE;
//   void set_storage_monitor_for_test(scoped_ptr<StorageMonitor> monitor);
//   virtual MediaFileSystemRegistry* media_file_system_registry() OVERRIDE;
//   virtual bool created_local_state() const OVERRIDE;
#if defined(ENABLE_WEBRTC)
  //virtual WebRtcLogUploader* webrtc_log_uploader() OVERRIDE;
#endif

  static void RegisterPrefs(PrefRegistrySimple* registry);

 private:
  

#if (defined(OS_WIN) || defined(OS_LINUX)) && !defined(OS_CHROMEOS)
  base::RepeatingTimer<BrowserProcessImpl> autoupdate_timer_;

  // Gets called by autoupdate timer to see if browser needs restart and can be
  // restarted, and if that's the case, restarts the browser.
  void OnAutoupdateTimer();
  bool CanAutorestartForUpdate() const{return false;};
  void RestartBackgroundInstance(){};
#endif  // defined(OS_WIN) || defined(OS_LINUX) && !defined(OS_CHROMEOS)

  DISALLOW_COPY_AND_ASSIGN(BrowserProcessImpl);
};

#endif  // CHROME_BROWSER_BROWSER_PROCESS_IMPL_H_
