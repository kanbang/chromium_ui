// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_WEB_CONTENTS_NAVIGATION_ENTRY_IMPL_H_
#define CONTENT_BROWSER_WEB_CONTENTS_NAVIGATION_ENTRY_IMPL_H_

#include "base/basictypes.h"
#include "base/memory/ref_counted.h"
#include "chrome/chrome_export.h"
#include "chrome/render/site_instance_impl.h"
#include "chrome/browser/navigation_entry.h"

namespace content {
class CONTENT_EXPORT NavigationEntryImpl
    : public NON_EXPORTED_BASE(NavigationEntry) {
 public:
  static NavigationEntryImpl* FromNavigationEntry(NavigationEntry* entry);

  // The value of bindings() before it is set during commit.
  static int kInvalidBindings;

  NavigationEntryImpl();
  NavigationEntryImpl(SiteInstanceImpl* instance,
                      int page_id,
                      const GURL& url,
                      const GURL& referrer,
                      const string16& title,
                      int transition_type,
                      bool is_renderer_initiated);
  virtual ~NavigationEntryImpl();

  // NavigationEntry implementation:
  virtual int GetUniqueID() const OVERRIDE;
  virtual void SetURL(const GURL& url) OVERRIDE;
  virtual const GURL& GetURL() const OVERRIDE;
  virtual void SetBaseURLForDataURL(const GURL& url) OVERRIDE;
  virtual const GURL& GetBaseURLForDataURL() const OVERRIDE;
  virtual void SetReferrer(const GURL& referrer) OVERRIDE;
  virtual const GURL& GetReferrer() const OVERRIDE;
  virtual void SetVirtualURL(const GURL& url) OVERRIDE;
  virtual const GURL& GetVirtualURL() const OVERRIDE;
  virtual void SetTitle(const string16& title) OVERRIDE;
  virtual const string16& GetTitle() const OVERRIDE;
  virtual void SetPageID(int page_id) OVERRIDE;
  virtual int32 GetPageID() const OVERRIDE;
  virtual const string16& GetTitleForDisplay(
      const std::string& languages) const OVERRIDE;
  virtual bool IsViewSourceMode() const OVERRIDE;
  virtual const GURL& GetUserTypedURL() const OVERRIDE;
  virtual void SetTimestamp(base::Time timestamp) OVERRIDE;
  virtual base::Time GetTimestamp() const OVERRIDE;
  virtual void SetCanLoadLocalResources(bool allow) OVERRIDE;
  virtual bool GetCanLoadLocalResources() const OVERRIDE;

  void set_unique_id(int unique_id) {
    unique_id_ = unique_id;
  }

  // The SiteInstance tells us how to share sub-processes. This is a reference
  // counted pointer to a shared site instance.
  //
  // Note that the SiteInstance should usually not be changed after it is set,
  // but this may happen if the NavigationEntry was cloned and needs to use a
  // different SiteInstance.
  void set_site_instance(SiteInstanceImpl* site_instance);
  SiteInstanceImpl* site_instance() const {
    return site_instance_.get();
  }

  // Remember the set of bindings granted to this NavigationEntry at the time
  // of commit, to ensure that we do not grant it additional bindings if we
  // navigate back to it in the future.  This can only be changed once.
  void SetBindings(int bindings);
  int bindings() const {
    return bindings_;
  }

  void set_page_type(int page_type) {
    page_type_ = page_type;
  }

  bool has_virtual_url() const {
    return !virtual_url_.is_empty();
  }

  bool update_virtual_url_with_url() const {
    return update_virtual_url_with_url_;
  }
  void set_update_virtual_url_with_url(bool update) {
    update_virtual_url_with_url_ = update;
  }

  // Extra headers (separated by \n) to send during the request.
  void set_extra_headers(const std::string& extra_headers) {
    extra_headers_ = extra_headers;
  }
  const std::string& extra_headers() const {
    return extra_headers_;
  }

  // Whether this (pending) navigation is renderer-initiated.  Resets to false
  // for all types of navigations after commit.
  void set_is_renderer_initiated(bool is_renderer_initiated) {
    is_renderer_initiated_ = is_renderer_initiated;
  }
  bool is_renderer_initiated() const {
    return is_renderer_initiated_;
  }

  void set_user_typed_url(const GURL& user_typed_url) {
    user_typed_url_ = user_typed_url;
  }

  // Enumerations of the possible restore types.
  enum RestoreType {
    // Restore from the previous session.
    RESTORE_LAST_SESSION_EXITED_CLEANLY,
    RESTORE_LAST_SESSION_CRASHED,

    // The entry has been restored from the current session. This is used when
    // the user issues 'reopen closed tab'.
    RESTORE_CURRENT_SESSION,

    // The entry was not restored.
    RESTORE_NONE
  };

  // The RestoreType for this entry. This is set if the entry was retored. This
  // is set to RESTORE_NONE once the entry is loaded.
  void set_restore_type(RestoreType type) {
    restore_type_ = type;
  }
  RestoreType restore_type() const {
    return restore_type_;
  }


  // Whether this (pending) navigation needs to replace current entry.
  // Resets to false after commit.
  bool should_replace_entry() const {
    return should_replace_entry_;
  }

  void set_should_replace_entry(bool should_replace_entry) {
    should_replace_entry_ = should_replace_entry;
  }

  void SetScreenshotPNGData(scoped_refptr<base::RefCountedBytes> png_data);
  const scoped_refptr<base::RefCountedBytes> screenshot() const {
    return screenshot_;
  }

  // Whether this (pending) navigation should clear the session history. Resets
  // to false after commit.
  bool should_clear_history_list() const {
    return should_clear_history_list_;
  }
  void set_should_clear_history_list(bool should_clear_history_list) {
    should_clear_history_list_ = should_clear_history_list;
  }

 private:
  // WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
  // Session/Tab restore save portions of this class so that it can be recreated
  // later. If you add a new field that needs to be persisted you'll have to
  // update SessionService/TabRestoreService and Android WebView
  // state_serializer.cc appropriately.
  // WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING

  // See the accessors above for descriptions.
  int unique_id_;
  scoped_refptr<SiteInstanceImpl> site_instance_;
  // TODO(creis): Persist bindings_. http://crbug.com/173672.
  int bindings_;
  int page_type_;
  GURL url_;
  GURL referrer_;
  GURL virtual_url_;
  bool update_virtual_url_with_url_;
  string16 title_;
  int32 page_id_;
  GURL user_typed_url_;
  bool has_post_data_;
  int64 post_id_;
  RestoreType restore_type_;
  GURL original_request_url_;
  bool is_overriding_user_agent_;
  base::Time timestamp_;
  int http_status_code_;

  // This member is not persisted with session restore because it is transient.
  // If the post request succeeds, this field is cleared since the same
  // information is stored in |content_state_| above. It is also only shallow
  // copied with compiler provided copy constructor.
  scoped_refptr<const base::RefCountedMemory> browser_initiated_post_data_;

  // This is also a transient member (i.e. is not persisted with session
  // restore). The screenshot of a page is taken when navigating away from the
  // page. This screenshot is displayed during an overscroll-navigation
  // gesture. |screenshot_| will be NULL when the screenshot is not available
  // (e.g. after a session restore, or if taking the screenshot of a page
  // failed). The UI is responsible for dealing with missing screenshots
  // appropriately (e.g. display a placeholder image instead).
  scoped_refptr<base::RefCountedBytes> screenshot_;

  // This member is not persisted with session restore.
  std::string extra_headers_;

  // Used for specifying base URL for pages loaded via data URLs. Only used and
  // persisted by Android WebView.
  GURL base_url_for_data_url_;

  // Whether the entry, while loading, was created for a renderer-initiated
  // navigation.  This dictates whether the URL should be displayed before the
  // navigation commits.  It is cleared on commit and not persisted.
  bool is_renderer_initiated_;

  // This is a cached version of the result of GetTitleForDisplay. It prevents
  // us from having to do URL formatting on the URL every time the title is
  // displayed. When the URL, virtual URL, or title is set, this should be
  // cleared to force a refresh.
  mutable string16 cached_display_title_;

  // In case a navigation is transferred to a new RVH but the request has
  // been generated in the renderer already, this identifies the old request so
  // that it can be resumed. The old request is stored until the
  // ResourceDispatcher receives the navigation from the renderer which
  // carries this |transferred_global_request_id_| annotation. Once the request
  // is transferred to the new process, this is cleared and the request
  // continues as normal.
  //GlobalRequestID transferred_global_request_id_;

  // This is set to true when this entry is being reloaded and due to changes in
  // the state of the URL, it has to be reloaded in a different site instance.
  // In such case, we must treat it as an existing navigation in the new site
  // instance, instead of a new navigation. This value should not be persisted
  // and is not needed after the entry commits.
  //
  // We also use this flag for cross-process redirect navigations, so that the
  // browser will replace the current navigation entry (which is the page
  // doing the redirect).
  bool should_replace_entry_;

  // This is set to true when this entry's navigation should clear the session
  // history both on the renderer and browser side. The browser side history
  // won't be cleared until the renderer has committed this navigation. This
  // entry is not persisted by the session restore system, as it is always
  // reset to false after commit.
  bool should_clear_history_list_;

  // Set when this entry should be able to access local file:// resources. This
  // value is not needed after the entry commits and is not persisted.
  bool can_load_local_resources_;

  // If not empty, the name of the frame to navigate. This field is not
  // persisted, because it is currently only used in tests.
  std::string frame_to_navigate_;

  // Used to store extra data to support browser features. This member is not
  // persisted, unless specific data is taken out/put back in at save/restore
  // time (see TabNavigation for an example of this).
  std::map<std::string, string16> extra_data_;

  // Copy and assignment is explicitly allowed for this class.
};

}  // namespace content

#endif  // CONTENT_BROWSER_WEB_CONTENTS_NAVIGATION_ENTRY_IMPL_H_
