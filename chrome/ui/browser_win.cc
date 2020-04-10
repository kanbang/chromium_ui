// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/ui/browser.h"

#include "base/strings/utf_string_conversions.h"
#include "base/win/metro.h"
//#include "chrome/browser/bookmarks/bookmark_utils.h"
//#include "chrome/browser/profiles/profile.h"
//#include "chrome/ui/browser_commands.h"
#include "chrome/ui/browser_finder.h"
#include "chrome/ui/browser_tabstrip.h"
#include "chrome/ui/fullscreen/fullscreen_controller.h"
#include "win8/util/win8_util.h"

namespace {

void NewWindowMaybeMetro(Browser* source_browser) {
  chrome::HostDesktopType host_desktop_type =
      source_browser->host_desktop_type();
  if (win8::IsSingleWindowMetroMode()) {
    typedef void (*FlipFrameWindows)();

    static FlipFrameWindows flip_window_fn = reinterpret_cast<FlipFrameWindows>(
        ::GetProcAddress(base::win::GetMetroModule(), "FlipFrameWindows"));
    DCHECK(flip_window_fn);

    Browser* browser = chrome::FindTabbedBrowser(false,
                                                 host_desktop_type);

    if (!browser) {
      //chrome::OpenEmptyWindow(profile, host_desktop_type);
      return;
    }

    //chrome::NewTab(browser);

    if (browser != source_browser) {
      // Tell the metro_driver to flip our window. This causes the current
      // browser window to be hidden and the next window to be shown.
      flip_window_fn();
    }
  } else {
    //NewEmptyWindow(profile, host_desktop_type);
  }
}

}  // namespace

namespace chrome {

void NewWindow(Browser* browser) {
  NewWindowMaybeMetro(browser);
}

void NewIncognitoWindow(Browser* browser) {
  NewWindowMaybeMetro(browser);
}

}  // namespace chrome

void Browser::SetMetroSnapMode(bool enable) {
  fullscreen_controller_->SetMetroSnapMode(enable);
}
