// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/ui/browser_tabstrip.h"

#include "base/command_line.h"
//#include "chrome/browser/profiles/profile.h"
#include "chrome/ui/browser.h"
//#include "chrome/ui/browser_navigator.h"
#include "chrome/ui/tab_contents/core_tab_helper.h"
#include "chrome/ui/tabs/tab_strip_model.h"
//#include "chrome/common/chrome_switches.h"
//#include "chrome/common/url_constants.h"
//#include "content/public/browser/navigation_entry.h"
//#include "content/public/browser/render_view_host.h"
#include "chrome/browser/web_contents.h"
#include "chrome/browser/web_contents_impl.h"
#include "chrome/ui/browser.h"
#include "chrome/ui/tabs/tab_strip_model.h"
namespace chrome {

void AddBlankTabAt(Browser* browser, int index, bool foreground) {
  // Time new tab page creation time.  We keep track of the timing data in
  // WebContents, but we want to include the time it takes to create the
  // WebContents object too.
  base::TimeTicks new_tab_start_time = base::TimeTicks::Now();
  //chrome::NavigateParams params(browser, GURL(chrome::kChromeUINewTabURL),
  //                              content::PAGE_TRANSITION_TYPED);
  //params.disposition = foreground ? NEW_FOREGROUND_TAB : NEW_BACKGROUND_TAB;
  //params.tabstrip_index = index;
  //chrome::Navigate(&params);
  //CoreTabHelper* core_tab_helper =
  //    CoreTabHelper::FromWebContents(params.target_contents);
  //core_tab_helper->set_new_tab_start_time(new_tab_start_time);
  content::WebContents::CreateParams params(browser->profile());
  content::WebContents* content = content::WebContentsImpl::CreateWithOpener(params);
  content->SetDelegate(browser);
  browser->tab_strip_model()->AddWebContents(
    content,
    0,
    0,
    TabStripModel::ADD_ACTIVE);

}

content::WebContents* AddSelectedTabWithURL(
    Browser* browser,
    const GURL& url,
    int transition) {
  /*NavigateParams params(browser, url, transition);
  params.disposition = NEW_FOREGROUND_TAB;
  Navigate(&params);
  return params.target_contents;*/
      return NULL;
}

void AddWebContents(Browser* browser,
                    content::WebContents* source_contents,
                    content::WebContents* new_contents,
                    WindowOpenDisposition disposition,
                    const gfx::Rect& initial_pos,
                    bool user_gesture,
                    bool* was_blocked) {
  // No code for this yet.
  DCHECK(disposition != SAVE_TO_DISK);
  // Can't create a new contents for the current tab - invalid case.
  DCHECK(disposition != CURRENT_TAB);

  /*NavigateParams params(browser, new_contents);
  params.source_contents = source_contents;
  params.disposition = disposition;
  params.window_bounds = initial_pos;
  params.window_action = NavigateParams::SHOW_WINDOW;
  params.user_gesture = user_gesture;
  Navigate(&params);*/
  browser->tab_strip_model()->AddWebContents(
    new_contents,
    -1,
    0,
    0);
}

void CloseWebContents(Browser* browser,
                      content::WebContents* contents,
                      bool add_to_history) {
  int index = browser->tab_strip_model()->GetIndexOfWebContents(contents);
  if (index == TabStripModel::kNoTab) {
    NOTREACHED() << "CloseWebContents called for tab not in our strip";
    return;
  }

  browser->tab_strip_model()->CloseWebContentsAt(
      index,
      add_to_history ? TabStripModel::CLOSE_CREATE_HISTORICAL_TAB
                     : TabStripModel::CLOSE_NONE);
}

}  // namespace chrome
