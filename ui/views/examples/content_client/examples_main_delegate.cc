// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/views/examples/content_client/examples_main_delegate.h"
#include "ui/views/examples/content_client/examples_browser_main_parts.h"

#include <string>

#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "base/message_loop/message_loop.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/base/ui_base_paths.h"
#include "ui/views/examples/content_client/examples_content_browser_client.h"

#if defined(OS_WIN)
#include "base/logging_win.h"
#include <Ole2.h>
#endif

namespace views {
namespace examples {
namespace {

#if defined(OS_WIN)
// {83FAC8EE-7A0E-4dbb-A3F6-6F500D7CAB1A}
const GUID kViewsExamplesProviderName =
    { 0x83fac8ee, 0x7a0e, 0x4dbb,
        { 0xa3, 0xf6, 0x6f, 0x50, 0xd, 0x7c, 0xab, 0x1a } };
#endif

}  // namespace

ExamplesMainDelegate::ExamplesMainDelegate() {
  examples_browser_main_parts_ = NULL;
}

ExamplesMainDelegate::~ExamplesMainDelegate() {
}

bool ExamplesMainDelegate::BasicStartupComplete(int* exit_code) {
  const CommandLine& command_line = *CommandLine::ForCurrentProcess();
  //std::string process_type =
  //    command_line.GetSwitchValueASCII(switches::kProcessType);

  //content::SetContentClient(&content_client_);

  logging::LoggingSettings settings;
  settings.logging_dest = logging::LOG_TO_SYSTEM_DEBUG_LOG;
  bool success = logging::InitLogging(settings);
  CHECK(success);
#if defined(OS_WIN)
  logging::LogEventProvider::Initialize(kViewsExamplesProviderName);
#endif

  return false;
}

int ExamplesMainDelegate::RunProcess(const std::string& process_type)
{
  base::MessageLoop main_message_loop(base::MessageLoop::TYPE_UI);

  ::OleInitialize(NULL);
  examples_browser_main_parts_ =  new ExamplesBrowserMainParts();
  int ext_code = 0;
  examples_browser_main_parts_->PreMainMessageLoopRun();
  examples_browser_main_parts_->MainMessageLoopRun(&ext_code);
  examples_browser_main_parts_->PostMainMessageLoopRun();
  return 0;
}

void ExamplesMainDelegate::PreSandboxStartup() {
  InitializeResourceBundle();
}

content::ContentBrowserClient*
    ExamplesMainDelegate::CreateContentBrowserClient() {
  browser_client_.reset(new ExamplesContentBrowserClient);
  return browser_client_.get();
}

void ExamplesMainDelegate::InitializeResourceBundle() {
  ui::ResourceBundle::InitSharedInstanceWithLocale("en-US", NULL);
  ui::ResourceBundle::InitCssSharedInstanceWithPakPath(L"C:\\work\\frame\\myapp\\src\\ui\\res\\skin", L"theme.css");
}

}  // namespace examples
}  // namespace views
