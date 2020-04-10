// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_BROWSER_COMMANDS_H_
#define CHROME_BROWSER_UI_BROWSER_COMMANDS_H_

#include "ui/base/window_open_disposition.h"
class Browser;
class CommandObserver;

namespace chrome {

  bool IsCommandEnabled(Browser* browser, int command);
  bool SupportsCommand(Browser* browser, int command);
  bool ExecuteCommand(Browser* browser, int command);
  bool ExecuteCommandWithDisposition(Browser* browser,
    int command,
    WindowOpenDisposition disposition);
  void UpdateCommandEnabled(Browser* browser, int command, bool enabled);

  void AddCommandObserver(Browser*, int command, CommandObserver* observer);
  bool CanGoBack(const Browser* browser);
  void GoBack(Browser* browser, WindowOpenDisposition disposition);
  bool CanGoForward(const Browser* browser);
  void GoForward(Browser* browser, WindowOpenDisposition disposition);
  void Reload(Browser* browser, WindowOpenDisposition disposition);
  void ReloadIgnoringCache(Browser* browser, WindowOpenDisposition disposition);
  bool CanReload(const Browser* browser);
  void Home(Browser* browser, WindowOpenDisposition disposition);
  void OpenCurrentURL(Browser* browser);
  void Stop(Browser* browser);
  void ClearCache(Browser* browser);
} //namespace chrome
#endif  // CHROME_BROWSER_UI_BROWSER_COMMANDS_H_