// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/browser_navigator.h"

#include <algorithm>

#include "chrome/ui/tabs/tab_strip_model.h"
#include "chrome/ui/browser.h"
#include "chrome/browser/web_contents.h"

using namespace content;

namespace {
  chrome::HostDesktopType GetHostDesktop(Browser* browser) {
    if (browser)
      return browser->host_desktop_type();
    return chrome::GetActiveDesktop();
  }

}

namespace chrome {
  
  NavigateParams::NavigateParams(Browser* a_browser,
    const GURL& a_url,
    content::PageTransition a_transition)
    : url(a_url),
    uses_post(false),
    target_contents(NULL),
    source_contents(NULL),
    disposition(CURRENT_TAB),
    transition(a_transition),
    is_renderer_initiated(false),
    tabstrip_index(-1),
    tabstrip_add_types(TabStripModel::ADD_ACTIVE),
    window_action(NO_ACTION),
    user_gesture(true),
    path_behavior(RESPECT),
    ref_behavior(IGNORE_REF),
    browser(a_browser),
    initiating_profile(NULL),
    host_desktop_type(GetHostDesktop(a_browser)),
    should_replace_current_entry(false),
    should_set_opener(false) {
  }

  NavigateParams::NavigateParams(Browser* a_browser,
    WebContents* a_target_contents)
    : uses_post(false),
    target_contents(a_target_contents),
    source_contents(NULL),
    disposition(CURRENT_TAB),
    transition(content::PAGE_TRANSITION_LINK),
    is_renderer_initiated(false),
    tabstrip_index(-1),
    tabstrip_add_types(TabStripModel::ADD_ACTIVE),
    window_action(NO_ACTION),
    user_gesture(true),
    path_behavior(RESPECT),
    ref_behavior(IGNORE_REF),
    browser(a_browser),
    initiating_profile(NULL),
    host_desktop_type(GetHostDesktop(a_browser)),
    should_replace_current_entry(false),
    should_set_opener(false) {
  }

  NavigateParams::NavigateParams(Profile* a_profile,
    const GURL& a_url,
    content::PageTransition a_transition)
    : url(a_url),
    uses_post(false),
    target_contents(NULL),
    source_contents(NULL),
    disposition(NEW_FOREGROUND_TAB),
    transition(a_transition),
    is_renderer_initiated(false),
    tabstrip_index(-1),
    tabstrip_add_types(TabStripModel::ADD_ACTIVE),
    window_action(SHOW_WINDOW),
    user_gesture(true),
    path_behavior(RESPECT),
    ref_behavior(IGNORE_REF),
    browser(NULL),
    initiating_profile(a_profile),
    host_desktop_type(chrome::GetActiveDesktop()),
    should_replace_current_entry(false),
    should_set_opener(false) {
  }

  NavigateParams::~NavigateParams() {}

  void FillNavigateParamsFromOpenURLParams(chrome::NavigateParams* nav_params,
    const OpenURLParams& params) {
  }

  void Navigate(NavigateParams* params) {
    Browser* source_browser = params->browser;

    if(!source_browser)
      return;

    if(params->disposition == CURRENT_TAB) {
      WebContents* ctn = source_browser->tab_strip_model()->GetActiveWebContents();
      ctn->NavigateUrl(params->url);
    }
    //if (source_browser)
  }

  bool IsURLAllowedInIncognito(const GURL& url,
    content::BrowserContext* browser_context) {
      return true;
  }
}

