// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/ui/views/tab_contents/web_drag_bookmark_handler_win.h"

//#include "chrome/browser/bookmarks/bookmark_node_data.h"
#include "chrome/ui/browser.h"
#include "chrome/ui/browser_finder.h"
#include "chrome/ui/browser_window.h"
#include "chrome/ui/bookmarks/bookmark_tab_helper.h"
//#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/web_contents.h"
//#include "content/public/common/drop_data.h"
//#include "content/public/common/url_constants.h"
#include "ui/base/dragdrop/os_exchange_data.h"
#include "ui/base/dragdrop/os_exchange_data_provider_win.h"

using content::WebContents;

WebDragBookmarkHandlerWin::WebDragBookmarkHandlerWin() {
}

WebDragBookmarkHandlerWin::~WebDragBookmarkHandlerWin() {
}
