// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/ui/views/frame/system_menu_model_builder.h"

#include "base/command_line.h"
#include "base/strings/utf_string_conversions.h"
//#include "chrome/app/chrome_command_ids.h"
//#include "chrome/ui/browser_commands.h"
#include "chrome/ui/host_desktop.h"
#include "chrome/ui/toolbar/wrench_menu_model.h"
//#include "chrome/common/chrome_switches.h"
//#include "grit/generated_resources.h"
#include "ui/base/accelerators/accelerator.h"
#include "ui/base/models/simple_menu_model.h"

SystemMenuModelBuilder::SystemMenuModelBuilder(
    ui::AcceleratorProvider* provider,
    Browser* browser)
    : menu_delegate_(provider, browser) {
}

SystemMenuModelBuilder::~SystemMenuModelBuilder() {
}

void SystemMenuModelBuilder::Init() {
  ui::SimpleMenuModel* model = new ui::SimpleMenuModel(&menu_delegate_);
  menu_model_.reset(model);
  BuildMenu(model);
#if defined(OS_WIN)
  // On Windows with HOST_DESKTOP_TYPE_NATIVE we put the menu items in the
  // system menu (not at the end). Doing this necessitates adding a trailing
  // separator.
  if (browser()->host_desktop_type() == chrome::HOST_DESKTOP_TYPE_NATIVE)
    model->AddSeparator(ui::NORMAL_SEPARATOR);
#endif
}

void SystemMenuModelBuilder::BuildMenu(ui::SimpleMenuModel* model) {
  // We add the menu items in reverse order so that insertion_index never needs
  // to change.
  if (browser()->is_type_tabbed())
    BuildSystemMenuForBrowserWindow(model);
  else
    BuildSystemMenuForAppOrPopupWindow(model);
  AddFrameToggleItems(model);
}

void SystemMenuModelBuilder::BuildSystemMenuForBrowserWindow(
    ui::SimpleMenuModel* model) {
  model->AddItemWithStringId(/*IDC_NEW_TAB*/0, /*IDS_NEW_TAB*/0);
  model->AddItemWithStringId(/*IDC_RESTORE_TAB*/0, /*IDS_RESTORE_TAB*/0);
  if (/*chrome::CanOpenTaskManager()*/0) {
    model->AddSeparator(ui::NORMAL_SEPARATOR);
    model->AddItemWithStringId(/*IDC_TASK_MANAGER*/0, /*IDS_TASK_MANAGER*/0);
  }
  // If it's a regular browser window with tabs, we don't add any more items,
  // since it already has menus (Page, Chrome).
}

void SystemMenuModelBuilder::BuildSystemMenuForAppOrPopupWindow(
    ui::SimpleMenuModel* model) {
  model->AddItemWithStringId(/*IDC_BACK*/0, /*IDS_CONTENT_CONTEXT_BACK*/0);
  model->AddItemWithStringId(/*IDC_FORWARD*/0, /*IDS_CONTENT_CONTEXT_FORWARD*/0);
  model->AddItemWithStringId(/*IDC_RELOAD*/0, /*IDS_APP_MENU_RELOAD*/0);
  model->AddSeparator(ui::NORMAL_SEPARATOR);
  if (browser()->is_app())
    model->AddItemWithStringId(/*IDC_NEW_TAB*/0, /*IDS_APP_MENU_NEW_WEB_PAGE*/0);
  else
    model->AddItemWithStringId(/*IDC_SHOW_AS_TAB*/0, /*IDS_SHOW_AS_TAB*/0);
  model->AddSeparator(ui::NORMAL_SEPARATOR);
  model->AddItemWithStringId(/*IDC_CUT*/0, /*IDS_CUT*/0);
  model->AddItemWithStringId(/*IDC_COPY*/0, /*IDS_COPY*/0);
  model->AddItemWithStringId(/*IDC_PASTE*/0, /*IDS_PASTE*/0);
  model->AddSeparator(ui::NORMAL_SEPARATOR);
  model->AddItemWithStringId(/*IDC_FIND*/0, /*IDS_FIND*/0);
  model->AddItemWithStringId(/*IDC_PRINT*/0, /*IDS_PRINT*/0);
  zoom_menu_contents_.reset(new ZoomMenuModel(&menu_delegate_));
  model->AddSubMenuWithStringId(/*IDC_ZOOM_MENU*/0, /*IDS_ZOOM_MENU*/0,
                                zoom_menu_contents_.get());
  encoding_menu_contents_.reset(new EncodingMenuModel(browser()));
  model->AddSubMenuWithStringId(/*IDC_ENCODING_MENU*/0,
                                /*IDS_ENCODING_MENU*/0,
                                encoding_menu_contents_.get());
  if (browser()->is_app() /*&& chrome::CanOpenTaskManager()*/) {
    model->AddSeparator(ui::NORMAL_SEPARATOR);
    model->AddItemWithStringId(/*IDC_TASK_MANAGER*/0, /*IDS_TASK_MANAGER*/0);
  }
}

void SystemMenuModelBuilder::AddFrameToggleItems(ui::SimpleMenuModel* model) {
  /*if (CommandLine::ForCurrentProcess()->HasSwitch(
          switches::kDebugEnableFrameToggle)) {
    model->AddSeparator(ui::NORMAL_SEPARATOR);
    model->AddItem(IDC_DEBUG_FRAME_TOGGLE, ASCIIToUTF16("Toggle Frame Type"));
  }*/
}

