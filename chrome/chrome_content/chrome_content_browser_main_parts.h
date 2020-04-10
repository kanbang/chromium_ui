// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_EXAMPLES_CONTENT_CLIENT_EXAMPLES_BROWSER_MAIN_PARTS_H_
#define UI_VIEWS_EXAMPLES_CONTENT_CLIENT_EXAMPLES_BROWSER_MAIN_PARTS_H_

#include "base/basictypes.h"
#include "base/memory/scoped_ptr.h"
#include "content/public/app/browser_main_parts.h"
#include "chrome/ui/browser.h"
#include "chrome/browser/profile.h"


namespace views {
class ViewsDelegate;

namespace chromemain {

class ChromeBrowserMainParts : public content::BrowserMainParts {
 public:
  explicit ChromeBrowserMainParts();
  virtual ~ChromeBrowserMainParts();

  // Overridden from content::BrowserMainParts:
  virtual void PreMainMessageLoopRun() OVERRIDE;
  virtual bool MainMessageLoopRun(int* result_code) OVERRIDE;
  virtual void PostMainMessageLoopRun() OVERRIDE;


 private:

  scoped_ptr<ViewsDelegate> views_delegate_;
  Browser *browser_;
  Profile *Profile_;

#if defined(OS_CHROMEOS)
  // Enable a minimal set of views::corewm to be initialized.
  scoped_ptr<shell::MinimalShell> minimal_shell_;
#endif

  DISALLOW_COPY_AND_ASSIGN(ChromeBrowserMainParts);
};

}  // namespace examples
}  // namespace views

#endif  // UI_VIEWS_EXAMPLES_CONTENT_CLIENT_EXAMPLES_BROWSER_MAIN_PARTS_H_
