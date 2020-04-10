// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_PUBLIC_BROWSER_NAVIGATION_ENTRY_H_
#define CONTENT_PUBLIC_BROWSER_NAVIGATION_ENTRY_H_

#include <string>

#include "base/memory/ref_counted_memory.h"
#include "base/strings/string16.h"
#include "base/time/time.h"
#include "chrome/chrome_export.h"

class GURL;

namespace content {

// A NavigationEntry is a data structure that captures all the information
// required to recreate a browsing state. This includes some opaque binary
// state as provided by the WebContentsImpl as well as some clear text title and
// URL which is used for our user interface.
class NavigationEntry {
 public:
  virtual ~NavigationEntry() {}

  CONTENT_EXPORT static NavigationEntry* Create();
  CONTENT_EXPORT static NavigationEntry* Create(const NavigationEntry& copy);

  // Page-related stuff --------------------------------------------------------

  // A unique ID is preserved across commits and redirects, which means that
  // sometimes a NavigationEntry's unique ID needs to be set (e.g. when
  // creating a committed entry to correspond to a to-be-deleted pending entry,
  // the pending entry's ID must be copied).
  virtual int GetUniqueID() const = 0;

  // The page type tells us if this entry is for an interstitial or error page.
  //virtual int GetPageType() const = 0;

  // The actual URL of the page. For some about pages, this may be a scary
  // data: URL or something like that. Use GetVirtualURL() below for showing to
  // the user.
  virtual void SetURL(const GURL& url) = 0;
  virtual const GURL& GetURL() const = 0;

  // Used for specifying a base URL for pages loaded via data URLs.
  virtual void SetBaseURLForDataURL(const GURL& url) = 0;
  virtual const GURL& GetBaseURLForDataURL() const = 0;

  // The referring URL. Can be empty.
  virtual void SetReferrer(const GURL& referrer) = 0;
  virtual const GURL& GetReferrer() const = 0;

  // The virtual URL, when nonempty, will override the actual URL of the page
  // when we display it to the user. This allows us to have nice and friendly
  // URLs that the user sees for things like about: URLs, but actually feed
  // the renderer a data URL that results in the content loading.
  //
  // GetVirtualURL() will return the URL to display to the user in all cases, so
  // if there is no overridden display URL, it will return the actual one.
  virtual void SetVirtualURL(const GURL& url) = 0;
  virtual const GURL& GetVirtualURL() const = 0;

  // The title as set by the page. This will be empty if there is no title set.
  // The caller is responsible for detecting when there is no title and
  // displaying the appropriate "Untitled" label if this is being displayed to
  // the user.
  virtual void SetTitle(const string16& title) = 0;
  virtual const string16& GetTitle() const = 0;

  // Describes the current page that the tab represents. This is the ID that the
  // renderer generated for the page and is how we can tell new versus
  // renavigations.
  virtual void SetPageID(int page_id) = 0;
  virtual int32 GetPageID() const = 0;

  // Page-related helpers ------------------------------------------------------

  // Returns the title to be displayed on the tab. This could be the title of
  // the page if it is available or the URL. |languages| is the list of
  // accpeted languages (e.g., prefs::kAcceptLanguages) or empty if proper
  // URL formatting isn't needed (e.g., unit tests).
  virtual const string16& GetTitleForDisplay(
      const std::string& languages) const = 0;

  // Returns true if the current tab is in view source mode. This will be false
  // if there is no navigation.
  virtual bool IsViewSourceMode() const = 0;

  // Tracking stuff ------------------------------------------------------------

  // The user typed URL was the URL that the user initiated the navigation
  // with, regardless of any redirects. This is used to generate keywords, for
  // example, based on "what the user thinks the site is called" rather than
  // what it's actually called. For example, if the user types "foo.com", that
  // may redirect somewhere arbitrary like "bar.com/foo", and we want to use
  // the name that the user things of the site as having.
  //
  // This URL will be is_empty() if the URL was navigated to some other way.
  // Callers should fall back on using the regular or display URL in this case.
  virtual const GURL& GetUserTypedURL() const = 0;

  // The time at which the last known local navigation has
  // completed. (A navigation can be completed more than once if the
  // page is reloaded.)
  //
  // If GetTimestamp() returns a null time, that means that either:
  //
  //   - this navigation hasn't completed yet;
  //   - this navigation was restored and for some reason the
  //     timestamp wasn't available;
  //   - or this navigation was copied from a foreign session.
  virtual void SetTimestamp(base::Time timestamp) = 0;
  virtual base::Time GetTimestamp() const = 0;

  // Used to specify if this entry should be able to access local file://
  // resources.
  virtual void SetCanLoadLocalResources(bool allow) = 0;
  virtual bool GetCanLoadLocalResources() const = 0;
};

}  // namespace content

#endif  // CONTENT_PUBLIC_BROWSER_NAVIGATION_ENTRY_H_
