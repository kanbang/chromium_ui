// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/chrome_content/chrome_content_browser_main_parts.h"

#include "base/bind.h"
#include "base/command_line.h"
#include "base/message_loop/message_loop.h"
#include "base/run_loop.h"
#include "base/strings/string_number_conversions.h"
#include "base/threading/thread.h"
#include "base/threading/thread_restrictions.h"
#include "ui/views/examples/examples_window_with_content.h"
#include "ui/views/focus/accelerator_handler.h"
#include "ui/views/test/desktop_test_views_delegate.h"
#include "chrome/ui/tabs/tab_strip_model.h"
#include "chrome/ui/browser_tabstrip.h"

#if defined(USE_AURA)
#include "ui/aura/env.h"
#include "ui/gfx/screen.h"
#include "ui/views/widget/desktop_aura/desktop_screen.h"
#include "ui/views/widget/native_widget_aura.h"
#endif

#if defined(OS_CHROMEOS)
#include "ui/aura/root_window.h"
#include "ui/aura/test/test_screen.h"
#include "ui/shell/minimal_shell.h"
#endif

#include "chrome/ui/browser.h"
#include "chrome/ui/browser_window.h"
#include "chrome/ui/host_desktop.h"

namespace views {
namespace chromemain {

ChromeBrowserMainParts::ChromeBrowserMainParts() {
	browser_ = NULL;
  Profile_ = NULL;
}

ChromeBrowserMainParts::~ChromeBrowserMainParts() {
}

void ChromeBrowserMainParts::PreMainMessageLoopRun() {
  //browser_context_.reset(new content::ShellBrowserContext(false, NULL));

  gfx::NativeView window_context = NULL;
#if defined(OS_CHROMEOS)
  gfx::Screen::SetScreenInstance(
      gfx::SCREEN_TYPE_NATIVE, aura::TestScreen::Create());
  // Set up basic pieces of views::corewm.
  minimal_shell_.reset(new shell::MinimalShell(gfx::Size(800, 600)));
  // Ensure the X window gets mapped.
  minimal_shell_->root_window()->ShowRootWindow();
  // Ensure Aura knows where to open new windows.
  window_context = minimal_shell_->root_window();
#elif defined(USE_AURA)
  gfx::Screen::SetScreenInstance(
      gfx::SCREEN_TYPE_NATIVE, CreateDesktopScreen());
#endif
  //views_delegate_.reset(new DesktopTestViewsDelegate);

  //ShowExamplesWindowWithContent(
  //    QUIT_ON_CLOSE, /*browser_context_.get()*/NULL, window_context);
  Profile_ = new Profile(false);
  Browser::CreateParams params(chrome::HOST_DESKTOP_TYPE_NATIVE);
  params.profile = Profile_;
  browser_ = new Browser(params);
  if (!browser_->tab_strip_model()->GetActiveWebContents()) {
    // TODO: this is a work around for 110909. Figure out why it's needed.
    if (!browser_->tab_strip_model()->count())
      chrome::AddBlankTabAt(browser_, -1, true);
    else
      browser_->tab_strip_model()->ActivateTabAt(0, false);
  }

  browser_->window()->Show();
}

void ChromeBrowserMainParts::PostMainMessageLoopRun() {
  //browser_context_.reset();
#if defined(OS_CHROMEOS)
  minimal_shell_.reset();
#endif
  views_delegate_.reset();
#if defined(USE_AURA)
  aura::Env::DeleteInstance();
#endif
}

bool ChromeBrowserMainParts::MainMessageLoopRun(int* result_code) {
  // xxx: Hax here because this kills event handling.
#if !defined(USE_AURA)
  AcceleratorHandler accelerator_handler;
  base::RunLoop run_loop(&accelerator_handler);
#else
  base::RunLoop run_loop;
#endif
  run_loop.Run();
  return true;
}

}  // namespace examples
}  // namespace views
