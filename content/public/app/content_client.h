// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_PUBLIC_COMMON_CONTENT_CLIENT_H_
#define CONTENT_PUBLIC_COMMON_CONTENT_CLIENT_H_

#include <string>
#include <vector>

#include "base/basictypes.h"
#include "base/strings/string16.h"
#include "base/strings/string_piece.h"
#include "build/build_config.h"
#include "ui/base/layout.h"

class CommandLine;
class GURL;

namespace base {
class RefCountedStaticMemory;
}

namespace IPC {
class Message;
}

namespace gfx {
class Image;
}

namespace gpu {
struct GPUInfo;
}

namespace sandbox {
class TargetPolicy;
}

namespace content {

class ContentBrowserClient;
class ContentClient;

// Setter and getter for the client.  The client should be set early, before any
// content code is called.
void SetContentClient(ContentClient* client);

ContentClient* GetContentClient();

// Used for tests to override the relevant embedder interfaces. Each method
// returns the old value.
ContentBrowserClient* SetBrowserClientForTesting(
    ContentBrowserClient* b);

// Returns the user agent string being used by the browser. SetContentClient()
// must be called prior to calling this, and this routine must be used
// instead of webkit_glue::GetUserAgent() in order to ensure that we use
// the same user agent string everywhere.
// TODO(dpranke): This is caused by webkit_glue being a library that can
// get linked into multiple linkable objects, causing us to have multiple
// static values of the user agent. This will be fixed when we clean up
// webkit_glue.

// Interface that the embedder implements.
class ContentClient {
 public:
  ContentClient();
  virtual ~ContentClient();

  ContentBrowserClient* browser() { return browser_; }


  // Returns a string resource given its id.
  virtual string16 GetLocalizedString(int message_id) const;

  // Return the contents of a resource in a StringPiece given the resource id.
  virtual base::StringPiece GetDataResource(
      int resource_id,
      ui::ScaleFactor scale_factor) const;

  // Returns the raw bytes of a scale independent data resource.
  virtual base::RefCountedStaticMemory* GetDataResourceBytes(
      int resource_id) const;

  // Returns a native image given its id.
  virtual gfx::Image& GetNativeImageNamed(int resource_id) const;

  // Called by content::GetProcessTypeNameInEnglish for process types that it
  // doesn't know about because they're from the embedder.
  virtual std::string GetProcessTypeNameInEnglish(int type);

#if defined(OS_MACOSX) && !defined(OS_IOS)
  // Allows the embedder to define a new |sandbox_type| by mapping it to the
  // resource ID corresponding to the sandbox profile to use. The legal values
  // for |sandbox_type| are defined by the embedder and should start with
  // SandboxType::SANDBOX_TYPE_AFTER_LAST_TYPE. Returns false if no sandbox
  // profile for the given |sandbox_type| exists. Otherwise,
  // |sandbox_profile_resource_id| is set to the resource ID corresponding to
  // the sandbox profile to use and true is returned.
  virtual bool GetSandboxProfileForSandboxType(
      int sandbox_type,
      int* sandbox_profile_resource_id) const;

  // Gets the Carbon interposing path to give to DYLD. Returns an empty string
  // if the embedder doesn't bundle it.
  virtual std::string GetCarbonInterposePath() const;
#endif

 private:
  friend class ContentClientInitializer;  // To set these pointers.
  friend class InternalTestInitializer;

  // The embedder API for participating in browser logic.
  ContentBrowserClient* browser_;
};

}  // namespace content

#endif  // CONTENT_PUBLIC_COMMON_CONTENT_CLIENT_H_
