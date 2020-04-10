// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/ui/browser_toolbar_model_delegate.h"

#include "chrome/ui/browser.h"
#include "chrome/ui/tabs/tab_strip_model.h"

BrowserToolbarModelDelegate::BrowserToolbarModelDelegate(Browser* browser)
    : browser_(browser) {
}

BrowserToolbarModelDelegate::~BrowserToolbarModelDelegate() {
}

content::WebContents*
BrowserToolbarModelDelegate::GetActiveWebContents() const {
  return browser_->tab_strip_model()->GetActiveWebContents();
}
