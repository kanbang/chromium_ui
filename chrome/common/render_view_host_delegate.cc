// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/common/render_view_host_delegate.h"
#include "chrome/browser/gurl.h"

namespace content {

RenderViewHostDelegateView* RenderViewHostDelegate::GetDelegateView() {
  return NULL;
}

bool RenderViewHostDelegate::OnMessageReceived(RenderViewHost* render_view_host,
                                               const IPC::Message& message) {
  return false;
}
// 
// const GURL& RenderViewHostDelegate::GetURL() const {
//   return GURL();
// }

WebContents* RenderViewHostDelegate::GetAsWebContents() {
  return NULL;
}

bool RenderViewHostDelegate::IsFullscreenForCurrentTab() const {
  return false;
}
}  // namespace content
