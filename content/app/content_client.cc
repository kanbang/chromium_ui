// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/public/app/content_client.h"

#include "base/logging.h"
#include "base/strings/string_piece.h"
#include "ui/gfx/image/image.h"

namespace content {

static ContentClient* g_client;

class InternalTestInitializer {
 public:
  static ContentBrowserClient* SetBrowser(ContentBrowserClient* b) {
    ContentBrowserClient* rv = g_client->browser_;
    g_client->browser_ = b;
    return rv;
  }
};

void SetContentClient(ContentClient* client) {
  g_client = client;

  // Set the default user agent as provided by the client. We need to make
  // sure this is done before webkit_glue::GetUserAgent() is called (so that
  // the UA doesn't change).
}

ContentClient* GetContentClient() {
  return g_client;
}

ContentBrowserClient* SetBrowserClientForTesting(ContentBrowserClient* b) {
  return InternalTestInitializer::SetBrowser(b);
}

ContentClient::ContentClient()
    : browser_(NULL) {
}

ContentClient::~ContentClient() {
}

string16 ContentClient::GetLocalizedString(int message_id) const {
  return string16();
}

base::StringPiece ContentClient::GetDataResource(
    int resource_id,
    ui::ScaleFactor scale_factor) const {
  return base::StringPiece();
}

base::RefCountedStaticMemory* ContentClient::GetDataResourceBytes(
    int resource_id) const {
  return NULL;
}

gfx::Image& ContentClient::GetNativeImageNamed(int resource_id) const {
  CR_DEFINE_STATIC_LOCAL(gfx::Image, kEmptyImage, ());
  return kEmptyImage;
}

std::string ContentClient::GetProcessTypeNameInEnglish(int type) {
  NOTIMPLEMENTED();
  return std::string();
}

#if defined(OS_MACOSX) && !defined(OS_IOS)
bool ContentClient::GetSandboxProfileForSandboxType(
    int sandbox_type,
    int* sandbox_profile_resource_id) const {
  return false;
}

std::string ContentClient::GetCarbonInterposePath() const {
  return std::string();
}
#endif

}  // namespace content
