// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/public/app/content_browser_client.h"

#include "base/files/file_path.h"
#include "ui/gfx/image/image_skia.h"

namespace content {

BrowserMainParts* ContentBrowserClient::CreateBrowserMainParts() {
  return NULL;
}
}  // namespace content
