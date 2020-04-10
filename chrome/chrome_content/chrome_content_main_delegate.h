// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_EXAMPLES_CONTENT_CLIENT_EXAMPLES_MAIN_DELEGATE_H_
#define UI_VIEWS_EXAMPLES_CONTENT_CLIENT_EXAMPLES_MAIN_DELEGATE_H_

#include <string>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "content/public/app/content_main_delegate.h"
#include "chrome/browser/browser_thread.h"
#include "chrome/browser/browser_thread_impl.h"
#include "chrome/browser/browser_process_sub_thread.h"
#include "base/message_loop/message_loop.h"

namespace views {
namespace chromemain {

class ChromeContentBrowserClient;
class ChromeBrowserMainParts;

class ChromeMainDelegate : public content::ContentMainDelegate{
 public:
  ChromeMainDelegate();
  virtual ~ChromeMainDelegate();

  // content::ContentMainDelegate implementation
	virtual int RunProcess(const std::string& process_type) OVERRIDE;
	virtual void ProcessExiting(const std::string& process_type) OVERRIDE;
	virtual bool BasicStartupComplete(int* exit_code) OVERRIDE;
	virtual void PreSandboxStartup() OVERRIDE;
	virtual content::ContentBrowserClient* CreateContentBrowserClient() OVERRIDE;

 private:
  void InitializeResourceBundle();
  void InitializeMainThread();
  int CreateThread();

   scoped_ptr<ChromeContentBrowserClient> browser_client_;
   ChromeBrowserMainParts* examples_browser_main_parts_;
   scoped_ptr<content::BrowserThreadImpl> main_thread_;
   scoped_ptr<base::MessageLoop> main_message_loop_;

// 
//   // sub thread
  scoped_ptr<content::BrowserProcessSubThread> db_thread_;
  scoped_ptr<content::BrowserProcessSubThread> file_user_blocking_thread_;
  scoped_ptr<content::BrowserProcessSubThread> file_thread_;
  scoped_ptr<content::BrowserProcessSubThread> process_launcher_thread_;
  scoped_ptr<content::BrowserProcessSubThread> cache_thread_;
  scoped_ptr<content::BrowserProcessSubThread> io_thread_;


  DISALLOW_COPY_AND_ASSIGN(ChromeMainDelegate);
};

}  // namespace examples
}  // namespace views

#endif  // UI_VIEWS_EXAMPLES_CONTENT_CLIENT_EXAMPLES_MAIN_DELEGATE_H_
