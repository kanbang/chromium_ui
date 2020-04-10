// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_SHELL_BROWSER_SHELL_BROWSER_CONTEXT_H_
#define CONTENT_SHELL_BROWSER_SHELL_BROWSER_CONTEXT_H_

#include "base/compiler_specific.h"
#include "base/files/file_path.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "chrome/render/browser_context.h"

class DownloadManagerDelegate;
class ShellDownloadManagerDelegate;
class ShellURLRequestContextGetter;

class Profile : public content::BrowserContext {
 public:
  Profile(bool off_the_record);
  virtual ~Profile();

  // BrowserContext implementation.
  virtual base::FilePath GetPath() const OVERRIDE;
  virtual bool IsOffTheRecord() const OVERRIDE;

 private:
  
  // Performs initialization of the ShellBrowserContext while IO is still
  // allowed on the current thread.
  void InitWhileIOAllowed();

  bool off_the_record_;

  bool ignore_certificate_errors_;
  base::FilePath path_;


  DISALLOW_COPY_AND_ASSIGN(Profile);
};

#endif  // CONTENT_SHELL_BROWSER_SHELL_BROWSER_CONTEXT_H_
