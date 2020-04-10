// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_VIEWS_TAB_CONTENTS_WEB_DRAG_BOOKMARK_HANDLER_WIN_H_
#define CHROME_BROWSER_UI_VIEWS_TAB_CONTENTS_WEB_DRAG_BOOKMARK_HANDLER_WIN_H_

#include "base/compiler_specific.h"

//#include "chrome/browser/bookmarks/bookmark_node_data.h"
//#include "chrome/browser/web_drag_dest_delegate.h"

class BookmarkTabHelper;

namespace content {
class WebContents;
}

// Chrome needs to intercept content drag events so it can dispatch them to the
// bookmarks and extensions system.
class WebDragBookmarkHandlerWin/* : public content::WebDragDestDelegate*/ {
 public:
  WebDragBookmarkHandlerWin();
  virtual ~WebDragBookmarkHandlerWin();

private:
};

#endif  // CHROME_BROWSER_UI_VIEWS_TAB_CONTENTS_WEB_DRAG_BOOKMARK_HANDLER_WIN_H_
