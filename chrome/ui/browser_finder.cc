// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/ui/browser_finder.h"

//#include "chrome/browser/profiles/profile.h"
#include "chrome/ui/browser_iterator.h"
#include "chrome/ui/browser_list.h"
#include "chrome/ui/browser_window.h"
#include "chrome/ui/tab_contents/tab_contents_iterator.h"
#include "chrome/ui/tabs/tab_strip_model.h"
//#include "content/public/browser/navigation_controller.h"

using content::WebContents;

namespace {


// Type used to indicate to match anything.
const int kMatchAny                     = 0;

// See BrowserMatches for details.
const int kMatchOriginalProfile         = 1 << 0;
const int kMatchCanSupportWindowFeature = 1 << 1;
const int kMatchTabbed                  = 1 << 2;

// Returns true if the specified |browser| matches the specified arguments.
// |match_types| is a bitmask dictating what parameters to match:
// . If it contains kMatchOriginalProfile then the original profile of the
//   browser must match |profile->GetOriginalProfile()|. This is used to match
//   incognito windows.
// . If it contains kMatchCanSupportWindowFeature
//   |CanSupportWindowFeature(window_feature)| must return true.
// . If it contains kMatchTabbed, the browser must be a tabbed browser.
bool BrowserMatches(Browser* browser,Browser::WindowFeature window_feature,
                    uint32 match_types) {
  if (match_types & kMatchCanSupportWindowFeature &&
      !browser->CanSupportWindowFeature(window_feature)) {
    return false;
  }

  /*if (match_types & kMatchOriginalProfile) {
    if (browser->profile()->GetOriginalProfile() !=
        profile->GetOriginalProfile())
      return false;
  } else if (browser->profile() != profile) {
    return false;
  }*/

  if (match_types & kMatchTabbed)
    return browser->is_type_tabbed();

  return true;
}

// Returns the first browser in the specified iterator that returns true from
// |BrowserMatches|, or null if no browsers match the arguments. See
// |BrowserMatches| for details on the arguments.
template <class T>
Browser* FindBrowserMatching(const T& begin,
                             const T& end,Browser::WindowFeature window_feature,
                             uint32 match_types) {
  for (T i = begin; i != end; ++i) {
    if (BrowserMatches(*i, window_feature, match_types))
      return *i;
  }
  return NULL;
}

Browser* FindBrowserWithTabbedOrAnyType(chrome::HostDesktopType desktop_type,
                                        bool match_tabbed,
                                        bool match_original_profiles) {
  BrowserList* browser_list_impl = BrowserList::GetInstance(desktop_type);
  if (!browser_list_impl)
    return NULL;
  uint32 match_types = kMatchAny;
  if (match_tabbed)
    match_types |= kMatchTabbed;
  if (match_original_profiles)
    match_types |= kMatchOriginalProfile;
  Browser* browser = FindBrowserMatching(browser_list_impl->begin_last_active(),
                                         browser_list_impl->end_last_active(),
                                         Browser::FEATURE_NONE,
                                         match_types);
  // Fall back to a forward scan of all Browsers if no active one was found.
  return browser ? browser : FindBrowserMatching(browser_list_impl->begin(),
                                                 browser_list_impl->end(),
                                                 Browser::FEATURE_NONE,
                                                 match_types);
}

size_t GetBrowserCountImpl(chrome::HostDesktopType desktop_type,
                           uint32 match_types) {
  BrowserList* browser_list_impl = BrowserList::GetInstance(desktop_type);
  size_t count = 0;
  if (browser_list_impl) {
    for (BrowserList::const_iterator i = browser_list_impl->begin();
         i != browser_list_impl->end(); ++i) {
      if (BrowserMatches(*i, Browser::FEATURE_NONE, match_types))
        count++;
    }
  }
  return count;
}

}  // namespace

namespace chrome {

Browser* FindTabbedBrowser(bool match_original_profiles,
                           HostDesktopType type) {
  return FindBrowserWithTabbedOrAnyType(type,
                                        true,
                                        match_original_profiles);
}

Browser* FindOrCreateTabbedBrowser(HostDesktopType type) {
  Browser* browser = FindTabbedBrowser(false, type);
  if (!browser)
    browser = new Browser(Browser::CreateParams(type));
  return browser;
}

Browser* FindAnyBrowser(bool match_original_profiles,
                        HostDesktopType type) {
  return FindBrowserWithTabbedOrAnyType(type,
                                        false,
                                        match_original_profiles);
}

Browser* FindBrowserWithProfile(HostDesktopType desktop_type) {
  return FindBrowserWithTabbedOrAnyType(desktop_type, false, false);
}

Browser* FindBrowserWithID(int desired_id) {
//   for (BrowserIterator it; !it.done(); it.Next()) {
//     if (it->session_id().id() == desired_id)
//       return *it;
//   }
  return NULL;
}

Browser* FindBrowserWithWindow(gfx::NativeWindow window) {
  if (!window)
    return NULL;
  for (BrowserIterator it; !it.done(); it.Next()) {
    Browser* browser = *it;
    if (browser->window() && browser->window()->GetNativeWindow() == window)
      return browser;
  }
  return NULL;
}

Browser* FindBrowserWithWebContents(const WebContents* web_contents) {
  DCHECK(web_contents);
  for (TabContentsIterator it; !it.done(); it.Next()) {
    if (*it == web_contents)
      return it.browser();
  }
  return NULL;
}

Browser* FindLastActiveWithProfile(HostDesktopType type) {
  BrowserList* list = BrowserList::GetInstance(type);
  // We are only interested in last active browsers, so we don't fall back to
  // all browsers like FindBrowserWith* do.
  return FindBrowserMatching(list->begin_last_active(), list->end_last_active(), Browser::FEATURE_NONE, kMatchAny);
}

Browser* FindLastActiveWithHostDesktopType(HostDesktopType type) {
  BrowserList* browser_list_impl = BrowserList::GetInstance(type);
  if (browser_list_impl)
    return browser_list_impl->GetLastActive();
  return NULL;
}

size_t GetTotalBrowserCount() {
  size_t count = 0;
  for (HostDesktopType t = HOST_DESKTOP_TYPE_FIRST; t < HOST_DESKTOP_TYPE_COUNT;
       t = static_cast<HostDesktopType>(t + 1)) {
    count += BrowserList::GetInstance(t)->size();
  }
  return count;
}

size_t GetTotalBrowserCountForProfile() {
  size_t count = 0;
  for (HostDesktopType t = HOST_DESKTOP_TYPE_FIRST; t < HOST_DESKTOP_TYPE_COUNT;
       t = static_cast<HostDesktopType>(t + 1)) {
    count += GetBrowserCount(t);
  }
  return count;
}

size_t GetBrowserCount(HostDesktopType type) {
  return GetBrowserCountImpl(type, kMatchAny);
}

size_t GetTabbedBrowserCount(HostDesktopType type) {
  return GetBrowserCountImpl(type, kMatchTabbed);
}

}  // namespace chrome
