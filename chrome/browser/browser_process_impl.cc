// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/browser_process_impl.h"

#include <algorithm>
#include <map>
#include <vector>

#include "base/bind.h"
#include "base/bind_helpers.h"
#include "base/command_line.h"
#include "base/debug/alias.h"
#include "base/debug/leak_annotations.h"
#include "base/path_service.h"
#include "base/prefs/json_pref_store.h"
#include "base/prefs/pref_registry_simple.h"
#include "base/prefs/pref_service.h"
#include "base/synchronization/waitable_event.h"
#include "base/threading/thread.h"
#include "base/threading/thread_restrictions.h"
#include "chrome/ui/bookmarks/bookmark_prompt_controller.h"
#include "chrome/ui/browser_finder.h"



#if defined(OS_WIN)
#include "base/win/windows_version.h"
#include "ui/views/focus/view_storage.h"
#elif defined(OS_MACOSX)
#include "chrome/browser/chrome_browser_main_mac.h"
#endif

#if defined(USE_AURA)
#include "ui/aura/env.h"
#endif

#if (defined(OS_WIN) || defined(OS_LINUX)) && !defined(OS_CHROMEOS)
// How often to check if the persistent instance of Chrome needs to restart
// to install an update.
static const int kUpdateCheckIntervalHours = 6;
#endif

#if defined(OS_WIN)
// Attest to the fact that the call to the file thread to save preferences has
// run, and it is safe to terminate.  This avoids the potential of some other
// task prematurely terminating our waiting message loop by posting a
// QuitTask().
static bool g_end_session_file_thread_has_completed = false;
#endif

#if defined(USE_X11)
// How long to wait for the File thread to complete during EndSession, on
// Linux. We have a timeout here because we're unable to run the UI messageloop
// and there's some deadlock risk. Our only option is to exit anyway.
static const int kEndSessionTimeoutSeconds = 10;
#endif


BrowserProcessImpl::BrowserProcessImpl(
    base::SequencedTaskRunner* local_state_task_runner,
    const CommandLine& command_line) {
  g_browser_process = this;
  

#if defined(OS_MACOSX)
  InitIdleMonitor();
#endif


  //message_center::MessageCenter::Initialize();
}

BrowserProcessImpl::~BrowserProcessImpl() {

  g_browser_process = NULL;
}

void BrowserProcessImpl::StartTearDown() {

  //message_center::MessageCenter::Shutdown();
}

void BrowserProcessImpl::PostDestroyThreads() {

}

#if defined(OS_WIN)
// Send a QuitTask to the given MessageLoop when the (file) thread has processed
// our (other) recent requests (to save preferences).
// Change the boolean so that the receiving thread will know that we did indeed
// send the QuitTask that terminated the message loop.
static void PostQuit(base::MessageLoop* message_loop) {
  g_end_session_file_thread_has_completed = true;
  message_loop->PostTask(FROM_HERE, base::MessageLoop::QuitClosure());
}
#elif defined(USE_X11)
static void Signal(base::WaitableEvent* event) {
  event->Signal();
}
#endif

bool BrowserProcessImpl::IsShuttingDown() {
  DCHECK(CalledOnValidThread());
  return false;
}


// static
void BrowserProcessImpl::RegisterPrefs(PrefRegistrySimple* registry) {
}

void BrowserProcessImpl::OnAutoupdateTimer() {
  if (CanAutorestartForUpdate()) {
    DLOG(WARNING) << "Detected update.  Restarting browser.";
    RestartBackgroundInstance();
  }
}

//#endif  // (defined(OS_WIN) || defined(OS_LINUX)) && !defined(OS_CHROMEOS)
