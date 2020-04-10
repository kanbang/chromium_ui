// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/chrome_content/chrome_content_content_browser_client.h"
#include "chrome/chrome_content/chrome_content_browser_main_parts.h"

namespace views {
namespace chromemain {

ChromeContentBrowserClient::ChromeContentBrowserClient()
    : examples_browser_main_parts_(NULL) {
}

ChromeContentBrowserClient::~ChromeContentBrowserClient() {
}

content::BrowserMainParts* ChromeContentBrowserClient::CreateBrowserMainParts() {
  examples_browser_main_parts_ =  new ChromeBrowserMainParts();
  return examples_browser_main_parts_;
}

}  // namespace examples
}  // namespace views
