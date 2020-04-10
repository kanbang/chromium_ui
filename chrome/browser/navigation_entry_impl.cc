// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/navigation_entry_impl.h"

#include "base/metrics/histogram.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "ui/gfx/text_elider.h"

// Use this to get a new unique ID for a NavigationEntry during construction.
// The returned ID is guaranteed to be nonzero (which is the "no ID" indicator).
static int GetUniqueIDInConstructor() {
  static int unique_id_counter = 0;
  return ++unique_id_counter;
}

namespace content {

int NavigationEntryImpl::kInvalidBindings = -1;

NavigationEntry* NavigationEntry::Create() {
  return new NavigationEntryImpl();
}

NavigationEntry* NavigationEntry::Create(const NavigationEntry& copy) {
  return new NavigationEntryImpl(static_cast<const NavigationEntryImpl&>(copy));
}

NavigationEntryImpl* NavigationEntryImpl::FromNavigationEntry(
    NavigationEntry* entry) {
  return static_cast<NavigationEntryImpl*>(entry);
}

NavigationEntryImpl::NavigationEntryImpl()
    : unique_id_(GetUniqueIDInConstructor()),
      site_instance_(NULL),
      bindings_(kInvalidBindings),
      page_type_(0),
      update_virtual_url_with_url_(false),
      page_id_(-1),
      has_post_data_(false),
      post_id_(-1),
      restore_type_(RESTORE_NONE),
      is_overriding_user_agent_(false),
      http_status_code_(0),
      is_renderer_initiated_(false),
      should_replace_entry_(false),
      should_clear_history_list_(false),
      can_load_local_resources_(false){
}

NavigationEntryImpl::NavigationEntryImpl(SiteInstanceImpl* instance,
                                         int page_id,
                                         const GURL& url,
                                         const GURL& referrer,
                                         const string16& title,
                                         int transition_type,
                                         bool is_renderer_initiated)
    : unique_id_(GetUniqueIDInConstructor()),
      site_instance_(instance),
      bindings_(kInvalidBindings),
      page_type_(0),
      url_(url),
      referrer_(referrer),
      update_virtual_url_with_url_(false),
      title_(title),
      page_id_(page_id),
      has_post_data_(false),
      post_id_(-1),
      restore_type_(RESTORE_NONE),
      is_overriding_user_agent_(false),
      http_status_code_(0),
      is_renderer_initiated_(is_renderer_initiated),
      should_replace_entry_(false),
      should_clear_history_list_(false),
      can_load_local_resources_(false) {
}

NavigationEntryImpl::~NavigationEntryImpl() {
}

int NavigationEntryImpl::GetUniqueID() const {
  return unique_id_;
}

void NavigationEntryImpl::SetURL(const GURL& url) {
  url_ = url;
  cached_display_title_.clear();
}

const GURL& NavigationEntryImpl::GetURL() const {
  return url_;
}

void NavigationEntryImpl::SetBaseURLForDataURL(const GURL& url) {
  base_url_for_data_url_ = url;
}

const GURL& NavigationEntryImpl::GetBaseURLForDataURL() const {
  return base_url_for_data_url_;
}

void NavigationEntryImpl::SetReferrer(const GURL& referrer) {
  referrer_ = referrer;
}

const GURL& NavigationEntryImpl::GetReferrer() const {
  return referrer_;
}

void NavigationEntryImpl::SetVirtualURL(const GURL& url) {
  virtual_url_ = (url == url_) ? GURL() : url;
  cached_display_title_.clear();
}

const GURL& NavigationEntryImpl::GetVirtualURL() const {
  return virtual_url_.is_empty() ? url_ : virtual_url_;
}

void NavigationEntryImpl::SetTitle(const string16& title) {
  title_ = title;
  cached_display_title_.clear();
}

const string16& NavigationEntryImpl::GetTitle() const {
  return title_;
}

void NavigationEntryImpl::SetPageID(int page_id) {
  page_id_ = page_id;
}

int32 NavigationEntryImpl::GetPageID() const {
  return page_id_;
}

void NavigationEntryImpl::set_site_instance(SiteInstanceImpl* site_instance) {
  site_instance_ = site_instance;
}

void NavigationEntryImpl::SetBindings(int bindings) {
  // Ensure this is set to a valid value, and that it stays the same once set.
  CHECK_NE(bindings, kInvalidBindings);
  CHECK(bindings_ == kInvalidBindings || bindings_ == bindings);
  bindings_ = bindings;
}

const string16& NavigationEntryImpl::GetTitleForDisplay(
    const std::string& languages) const {
  // Most pages have real titles. Don't even bother caching anything if this is
  // the case.
  if (!title_.empty())
    return title_;

  // More complicated cases will use the URLs as the title. This result we will
  // cache since it's more complicated to compute.
  if (!cached_display_title_.empty())
    return cached_display_title_;

//   // Use the virtual URL first if any, and fall back on using the real URL.
//   string16 title;
//   if (!virtual_url_.is_empty()) {
//     title = net::FormatUrl(virtual_url_, languages);
//   } else if (!url_.is_empty()) {
//     if (GURL("chrome://embedding/") == url_)
//       return title_;
//     title = net::FormatUrl(url_, languages);
//   }
// 
//   // For file:// URLs use the filename as the title, not the full path.
//   if (url_.SchemeIsFile()) {
//     string16::size_type slashpos = title.rfind('/');
//     if (slashpos != string16::npos)
//       title = title.substr(slashpos + 1);
//   }
// 
//   gfx::ElideString(title, kMaxTitleChars, &cached_display_title_);
  return cached_display_title_;
}

bool NavigationEntryImpl::IsViewSourceMode() const {
  return virtual_url_.SchemeIs("chrome:://viewsource/");
}

const GURL& NavigationEntryImpl::GetUserTypedURL() const {
  return user_typed_url_;
}

void NavigationEntryImpl::SetTimestamp(base::Time timestamp) {
  timestamp_ = timestamp;
}

base::Time NavigationEntryImpl::GetTimestamp() const {
  return timestamp_;
}

void NavigationEntryImpl::SetCanLoadLocalResources(bool allow) {
  can_load_local_resources_ = allow;
}

bool NavigationEntryImpl::GetCanLoadLocalResources() const {
  return can_load_local_resources_;
}

void NavigationEntryImpl::SetScreenshotPNGData(
    scoped_refptr<base::RefCountedBytes> png_data) {
  screenshot_ = png_data;
  if (screenshot_.get())
    UMA_HISTOGRAM_MEMORY_KB("Overscroll.ScreenshotSize", screenshot_->size());
}

}  // namespace content
