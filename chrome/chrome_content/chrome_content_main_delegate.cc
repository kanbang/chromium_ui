// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/chrome_content/chrome_content_main_delegate.h"
#include "chrome/chrome_content/chrome_content_browser_main_parts.h"

#include <string>

#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "base/debug/trace_event.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/base/ui_base_paths.h"
#include "chrome/chrome_content/chrome_content_content_browser_client.h"
#include "chrome/theme_resources_browser.h"

#if defined(OS_WIN)
#include "base/logging_win.h"
#include <Ole2.h>
#endif

extern "C" int WINAPI IECore_wWinMain(HINSTANCE hInstance, 
  HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow);

namespace views {
namespace chromemain {
namespace {

#if defined(OS_WIN)
// {83FAC8EE-7A0E-4dbb-A3F6-6F500D7CAB1A}
const GUID kViewsExamplesProviderName =
    { 0x83fac8ee, 0x7a0e, 0x4dbb,
        { 0xa3, 0xf6, 0x6f, 0x50, 0xd, 0x7c, 0xab, 0x1a } };
#endif

}  // namespace

ChromeMainDelegate::ChromeMainDelegate() {
  examples_browser_main_parts_ = NULL;
}

ChromeMainDelegate::~ChromeMainDelegate() {
}

bool ChromeMainDelegate::BasicStartupComplete(int* exit_code) {
  const CommandLine& command_line = *CommandLine::ForCurrentProcess();
  //std::string process_type =
  //    command_line.GetSwitchValueASCII(switches::kProcessType);

  //content::SetContentClient(&content_client_);

  logging::LoggingSettings settings;
  settings.logging_dest = logging::LOG_TO_SYSTEM_DEBUG_LOG;
  bool success = logging::InitLogging(settings);
  CHECK(success);
#if defined(OS_WIN)
  logging::LogEventProvider::Initialize(kViewsExamplesProviderName);
#endif

  return false;
}

int ChromeMainDelegate::RunProcess(const std::string& process_type)
{
  if(process_type == "renderer")
    return IECore_wWinMain(NULL, NULL, NULL, 0);
 
  InitializeResourceBundle();
  if (!base::MessageLoop::current())
    main_message_loop_.reset(new base::MessageLoop(base::MessageLoop::TYPE_UI));

  InitializeMainThread();

  ::OleInitialize(NULL);
  examples_browser_main_parts_ =  new ChromeBrowserMainParts();
  int ext_code = 0;

  examples_browser_main_parts_->PreMainMessageLoopRun();
  examples_browser_main_parts_->MainMessageLoopRun(&ext_code);
  examples_browser_main_parts_->PostMainMessageLoopRun();

  return ext_code;
}

void ChromeMainDelegate::ProcessExiting(const std::string& process_type) {
// 	if (SubprocessNeedsResourceBundle(process_type))
// 		ResourceBundle::CleanupSharedInstance();
#if !defined(OS_ANDROID)
//	logging::CleanupChromeLogging();
#else
	// Android doesn't use InitChromeLogging, so we close the log file manually.
	logging::CloseLogFile();
#endif  // !defined(OS_ANDROID)
}

void ChromeMainDelegate::PreSandboxStartup() {
  
}

content::ContentBrowserClient*
    ChromeMainDelegate::CreateContentBrowserClient() {
  browser_client_.reset(new ChromeContentBrowserClient);
  return browser_client_.get();
}

void ChromeMainDelegate::InitializeResourceBundle() {
  themeresoure::ThemeRes* pData = new themeresoure::ThemeRes();
  ui::ResourceBundle::InitSharedInstanceWithLocale("en-US", pData);
}

void ChromeMainDelegate::InitializeMainThread() {
  const char* kThreadName = "CrBrowserMain";
  base::PlatformThread::SetName(kThreadName);
  if (main_message_loop_)
    main_message_loop_->set_thread_name(kThreadName);

  // Register the main thread by instantiating it, but don't call any methods.
  main_thread_.reset(
    new content::BrowserThreadImpl(content::BrowserThread::UI, base::MessageLoop::current()));

  CreateThread();
}

int ChromeMainDelegate::CreateThread() {
  TRACE_EVENT0("startup", "BrowserMainLoop::CreateThreads");

  base::Thread::Options default_options;
  base::Thread::Options io_message_loop_options;
  io_message_loop_options.message_loop_type = base::MessageLoop::TYPE_IO;
  base::Thread::Options ui_message_loop_options;
  ui_message_loop_options.message_loop_type = base::MessageLoop::TYPE_UI;

  // Start threads in the order they occur in the BrowserThread::ID
  // enumeration, except for BrowserThread::UI which is the main
  // thread.
  //
  // Must be size_t so we can increment it.
  for (size_t thread_id = content::BrowserThread::UI + 1;
    thread_id < content::BrowserThread::ID_COUNT;
    ++thread_id) {
      scoped_ptr<content::BrowserProcessSubThread>* thread_to_start = NULL;
      base::Thread::Options* options = &default_options;

      switch (thread_id) {
      case content::BrowserThread::DB:
        TRACE_EVENT_BEGIN1("startup",
          "BrowserMainLoop::CreateThreads:start",
          "Thread", "BrowserThread::DB");
        thread_to_start = &db_thread_;
        break;
      case content::BrowserThread::FILE_USER_BLOCKING:
        TRACE_EVENT_BEGIN1("startup",
          "BrowserMainLoop::CreateThreads:start",
          "Thread", "BrowserThread::FILE_USER_BLOCKING");
        thread_to_start = &file_user_blocking_thread_;
        break;
      case content::BrowserThread::FILE:
        TRACE_EVENT_BEGIN1("startup",
          "BrowserMainLoop::CreateThreads:start",
          "Thread", "BrowserThread::FILE");
        thread_to_start = &file_thread_;
#if defined(OS_WIN)
        // On Windows, the FILE thread needs to be have a UI message loop
        // which pumps messages in such a way that Google Update can
        // communicate back to us.
        options = &ui_message_loop_options;
#else
        options = &io_message_loop_options;
#endif
        break;
      case content::BrowserThread::PROCESS_LAUNCHER:
        TRACE_EVENT_BEGIN1("startup",
          "BrowserMainLoop::CreateThreads:start",
          "Thread", "BrowserThread::PROCESS_LAUNCHER");
        thread_to_start = &process_launcher_thread_;
        break;
      case content::BrowserThread::CACHE:
        TRACE_EVENT_BEGIN1("startup",
          "BrowserMainLoop::CreateThreads:start",
          "Thread", "BrowserThread::CACHE");
        thread_to_start = &cache_thread_;
        options = &io_message_loop_options;
        break;
      case content::BrowserThread::IO:
        TRACE_EVENT_BEGIN1("startup",
          "BrowserMainLoop::CreateThreads:start",
          "Thread", "BrowserThread::IO");
        thread_to_start = &io_thread_;
        options = &io_message_loop_options;
        break;
      case content::BrowserThread::UI:
      case content::BrowserThread::ID_COUNT:
      default:
        NOTREACHED();
        break;
      }

      content::BrowserThread::ID id = static_cast<content::BrowserThread::ID>(thread_id);

      if (thread_to_start) {
        (*thread_to_start).reset(new content::BrowserProcessSubThread(id));
        (*thread_to_start)->StartWithOptions(*options);
      } else {
        NOTREACHED();
      }

      TRACE_EVENT_END0("startup", "BrowserMainLoop::CreateThreads:start");

  }
  return 0;
}

}  // namespace examples
}  // namespace views
