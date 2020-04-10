// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/ui/tabs/tab_menu_model.h"

//#include "base/command_line.h"
#include "chrome/ui/tabs/tab_strip_model.h"
#include "chrome/ui/tabs/tab_strip_model_delegate.h"
//#include "chrome/common/chrome_switches.h"
//#include "grit/generated_resources.h"

TabMenuModel::TabMenuModel(ui::SimpleMenuModel::Delegate* delegate,
                           TabStripModel* tab_strip,
                           int index)
    : ui::SimpleMenuModel(delegate) {
  Build(tab_strip, index);
}

void TabMenuModel::Build(TabStripModel* tab_strip, int index) {
  bool affects_multiple_tabs =
      (tab_strip->IsTabSelected(index) &&
       tab_strip->selection_model().selected_indices().size() > 1);
  AddItemWithStringId(TabStripModel::CommandNewTab, /*IDS_TAB_CXMENU_NEWTAB*/0);
  AddSeparator(ui::NORMAL_SEPARATOR);
  AddItemWithStringId(TabStripModel::CommandReload, /*IDS_TAB_CXMENU_RELOAD*/0);
  AddItemWithStringId(TabStripModel::CommandDuplicate,
                      /*IDS_TAB_CXMENU_DUPLICATE*/0);
  bool will_pin = tab_strip->WillContextMenuPin(index);
  if (affects_multiple_tabs) {
    AddItemWithStringId(
        TabStripModel::CommandTogglePinned,
        will_pin ? /*IDS_TAB_CXMENU_PIN_TABS*/0 : /*IDS_TAB_CXMENU_UNPIN_TABS*/0);
  } else {
    AddItemWithStringId(
        TabStripModel::CommandTogglePinned,
        will_pin ? /*IDS_TAB_CXMENU_PIN_TAB*/0 : /*IDS_TAB_CXMENU_UNPIN_TAB*/0);
  }
  AddSeparator(ui::NORMAL_SEPARATOR);
  if (affects_multiple_tabs) {
    AddItemWithStringId(TabStripModel::CommandCloseTab,
                        /*IDS_TAB_CXMENU_CLOSETABS*/0);
  } else {
    AddItemWithStringId(TabStripModel::CommandCloseTab,
                        /*IDS_TAB_CXMENU_CLOSETAB*/0);
  }
  AddItemWithStringId(TabStripModel::CommandCloseOtherTabs,
                      /*IDS_TAB_CXMENU_CLOSEOTHERTABS*/0);
  AddItemWithStringId(TabStripModel::CommandCloseTabsToRight,
                      /*IDS_TAB_CXMENU_CLOSETABSTORIGHT*/0);
  AddSeparator(ui::NORMAL_SEPARATOR);
  const bool is_window = tab_strip->delegate()->GetRestoreTabType() ==
      TabStripModelDelegate::RESTORE_WINDOW;
  AddItemWithStringId(TabStripModel::CommandRestoreTab,
                      is_window ? /*IDS_RESTORE_WINDOW*/0 : /*IDS_RESTORE_TAB*/0);
  AddItemWithStringId(TabStripModel::CommandBookmarkAllTabs,
                      /*IDS_TAB_CXMENU_BOOKMARK_ALL_TABS*/0);
  if (1/*CommandLine::ForCurrentProcess()->HasSwitch(
    switches::kEnableTabGroupsContextMenu)*/) {
    AddSeparator(ui::NORMAL_SEPARATOR);
    AddItemWithStringId(TabStripModel::CommandSelectByDomain,
                        /*IDS_TAB_CXMENU_SELECT_BY_DOMAIN*/0);
    AddItemWithStringId(TabStripModel::CommandSelectByOpener,
                        /*IDS_TAB_CXMENU_SELECT_BY_OPENER*/0);
  }
}
