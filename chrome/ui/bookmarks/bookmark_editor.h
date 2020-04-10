// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_BOOKMARKS_BOOKMARK_EDITOR_H_
#define CHROME_BROWSER_UI_BOOKMARKS_BOOKMARK_EDITOR_H_

#include <utility>
#include <vector>

#include "base/strings/string16.h"
//#include "chrome/browser/bookmarks/bookmark_model.h"
#include "ui/gfx/native_widget_types.h"
#include "chrome/browser/gurl.h"

// Small, cross platform interface that shows the correct platform specific
// bookmark editor dialog.
class BookmarkEditor {
 public:
  // An enumeration of the possible configurations offered.
  enum Configuration {
    // If Configuration is SHOW_TREE, a tree is shown allowing the user to
    // choose the parent of the node.
    SHOW_TREE,
    NO_TREE
  };

  // Describes what the user is editing.
  class EditDetails {
   public:
    // Returns the type of the existing or new node.
    /*BookmarkNode::Type*/int GetNodeType() const;

    // Returns the resource id for the string resource to use on the window
    // title for this edit operation.
    int GetWindowTitleId() const;

    // Returns an EditDetails instance for the user editing the given bookmark.
    static EditDetails EditNode(/*const BookmarkNode* node*/);

    // Returns an EditDetails instance for the user adding a bookmark within
    // a given parent node with a specified index.
    static EditDetails AddNodeInFolder(int index,
                                       const GURL& url,
                                       const string16& title);

    // Returns an EditDetails instance for the user adding a folder within a
    // given parent node with a specified index.
    static EditDetails AddFolder(int index);

    enum Type {
      // The user is editing an existing node in the model. The node the user
      // is editing is set in |existing_node|.
      EXISTING_NODE,

      // A new bookmark should be created if the user accepts the edit.
      // |existing_node| is null in this case.
      NEW_URL,

      // A new folder bookmark should be created if the user accepts the edit.
      // The contents of the folder should be that of |urls|.
      // |existing_node| is null in this case.
      NEW_FOLDER
    };

    ~EditDetails();

    // See description of enum value for details.
    const Type type;

    // If type == EXISTING_NODE this gives the existing node.
    //const BookmarkNode* existing_node;

    // If type == NEW_URL or type == NEW_FOLDER this gives the initial parent
    // node to place the new node in.
    //const BookmarkNode* parent_node;

    // If type == NEW_URL or type == NEW_FOLDER this gives the index to insert
    // the new node at.
    int index;

    // If type == NEW_URL this gives the URL/title.
    GURL url;
    string16 title;

    // If type == NEW_FOLDER, this is the urls/title pairs to add to the
    // folder.
    std::vector<std::pair<GURL, string16> > urls;

   private:
    explicit EditDetails(Type node_type);
  };

  // Shows the bookmark editor. The bookmark editor allows editing an existing
  // node or creating a new bookmark node (as determined by |details.type|).
  // |details.parent_node| is only used if |details.existing_node| is null.
  static void Show(gfx::NativeWindow parent_window,
                   const EditDetails& details);

  // Modifies a bookmark node (assuming that there's no magic that needs to be
  // done regarding moving from one folder to another).  If a new node is
  // explicitly being added, returns a pointer to the new node that was created.
  // Otherwise the return value is identically |node|.
//   static const BookmarkNode* ApplyEditsWithNoFolderChange(
//       const EditDetails& details,
//       const base::string16& new_title,
//       const GURL& new_url);

  // Modifies a bookmark node assuming that the parent of the node may have
  // changed and the node will need to be removed and reinserted.  If a new node
  // is explicitly being added, returns a pointer to the new node that was
  // created.  Otherwise the return value is identically |node|.
//   static const BookmarkNode* ApplyEditsWithPossibleFolderChange(
//       const EditDetails& details,
//       const base::string16& new_title,
//       const GURL& new_url);
};

#endif  // CHROME_BROWSER_UI_BOOKMARKS_BOOKMARK_EDITOR_H_
