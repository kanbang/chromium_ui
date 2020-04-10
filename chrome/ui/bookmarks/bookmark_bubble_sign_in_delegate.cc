// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/ui/bookmarks/bookmark_bubble_sign_in_delegate.h"

//#include "chrome/browser/signin/signin_promo.h"
#include "chrome/ui/browser.h"
#include "chrome/ui/browser_finder.h"
#include "chrome/ui/browser_list.h"
#include "chrome/ui/browser_window.h"
//#include "chrome/ui/chrome_pages.h"
#include "chrome/ui/tabs/tab_strip_model.h"

BookmarkBubbleSignInDelegate::BookmarkBubbleSignInDelegate(Browser* browser)
    : browser_(browser),
      desktop_type_(browser->host_desktop_type()) {
  BrowserList::AddObserver(this);
}

BookmarkBubbleSignInDelegate::~BookmarkBubbleSignInDelegate() {
  BrowserList::RemoveObserver(this);
}

void BookmarkBubbleSignInDelegate::OnSignInLinkClicked() {
  EnsureBrowser();
  //chrome::ShowBrowserSignin(browser_, signin::SOURCE_BOOKMARK_BUBBLE);
}

void BookmarkBubbleSignInDelegate::OnBrowserRemoved(Browser* browser) {
  if (browser == browser_)
    browser_ = NULL;
}

void BookmarkBubbleSignInDelegate::EnsureBrowser() {
//   if (!browser_) {
//     Profile* original_profile = profile_->GetOriginalProfile();
//     browser_ = chrome::FindLastActiveWithProfile(original_profile,
//                                                  desktop_type_);
//     if (!browser_) {
//       browser_ = new Browser(Browser::CreateParams(original_profile,
//                                                    desktop_type_));
//     }
//   }
}
