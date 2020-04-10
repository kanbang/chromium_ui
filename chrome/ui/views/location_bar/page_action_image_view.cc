// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/ui/views/location_bar/page_action_image_view.h"

#include "base/strings/utf_string_conversions.h"
//#include "chrome/browser/extensions/api/commands/command_service.h"
//#include "chrome/browser/extensions/extension_action.h"
//#include "chrome/browser/extensions/extension_action_icon_factory.h"
//#include "chrome/browser/extensions/extension_action_manager.h"
//#include "chrome/browser/extensions/extension_context_menu_model.h"
//#include "chrome/browser/extensions/extension_service.h"
//#include "chrome/browser/extensions/extension_tab_util.h"
//#include "chrome/browser/extensions/location_bar_controller.h"
//#include "chrome/browser/extensions/tab_helper.h"
#include "chrome/browser/platform_util.h"
//#include "chrome/browser/profiles/profile.h"
//#include "chrome/browser/sessions/session_id.h"
#include "chrome/ui/browser_list.h"
#include "chrome/ui/views/frame/browser_view.h"
#include "chrome/ui/views/location_bar/location_bar_view.h"
//#include "chrome/ui/webui/extensions/extension_info_ui.h"
//#include "chrome/common/extensions/extension.h"
#include "ui/base/accessibility/accessible_view_state.h"
#include "ui/events/event.h"
#include "ui/gfx/canvas.h"
#include "ui/gfx/image/image.h"
#include "ui/views/controls/menu/menu_item_view.h"
#include "ui/views/controls/menu/menu_runner.h"

using content::WebContents;


PageActionImageView::PageActionImageView(LocationBarView* owner,
                                         Browser* browser)
    : owner_(owner),
      browser_(browser),
      current_tab_id_(-1),
      preview_enabled_(false) {
          /*page_action->GetIconAnimation(
              SessionID::IdForTab(owner->GetWebContents())),
          this) {
  const Extension* extension = owner_->profile()->GetExtensionService()->
      GetExtensionById(page_action->extension_id(), false);
  DCHECK(extension);

  icon_factory_.reset(
      new ExtensionActionIconFactory(
          owner_->profile(), extension, page_action, this));

  set_accessibility_focusable(true);
  set_context_menu_controller(this);

  extensions::CommandService* command_service =
      extensions::CommandService::Get(browser_->profile());
  extensions::Command page_action_command;
  if (command_service->GetPageActionCommand(
          extension->id(),
          extensions::CommandService::ACTIVE_ONLY,
          &page_action_command,
          NULL)) {
    page_action_keybinding_.reset(
        new ui::Accelerator(page_action_command.accelerator()));
    owner_->GetFocusManager()->RegisterAccelerator(
        *page_action_keybinding_.get(),
        ui::AcceleratorManager::kHighPriority,
        this);
  }

  extensions::Command script_badge_command;
  if (command_service->GetScriptBadgeCommand(
          extension->id(),
          extensions::CommandService::ACTIVE_ONLY,
          &script_badge_command,
          NULL)) {
    script_badge_keybinding_.reset(
        new ui::Accelerator(script_badge_command.accelerator()));
    owner_->GetFocusManager()->RegisterAccelerator(
        *script_badge_keybinding_.get(),
        ui::AcceleratorManager::kHighPriority,
        this);
  }*/
}

PageActionImageView::~PageActionImageView() {
  if (owner_->GetFocusManager()) {
    if (page_action_keybinding_.get()) {
      owner_->GetFocusManager()->UnregisterAccelerator(
          *page_action_keybinding_.get(), this);
    }

    if (script_badge_keybinding_.get()) {
      owner_->GetFocusManager()->UnregisterAccelerator(
          *script_badge_keybinding_.get(), this);
    }
  }

  HidePopup();
}



void PageActionImageView::GetAccessibleState(ui::AccessibleViewState* state) {
  state->role = ui::AccessibilityTypes::ROLE_PUSHBUTTON;
  state->name = UTF8ToUTF16(tooltip_);
}

bool PageActionImageView::OnMousePressed(const ui::MouseEvent& event) {
  // We want to show the bubble on mouse release; that is the standard behavior
  // for buttons.  (Also, triggering on mouse press causes bugs like
  // http://crbug.com/33155.)
  return true;
}

void PageActionImageView::OnMouseReleased(const ui::MouseEvent& event) {
  if (!HitTestPoint(event.location()))
    return;

  if (event.IsRightMouseButton()) {
    // Don't show a menu here, its handled in View::ProcessMouseReleased. We
    // show the context menu by way of being the ContextMenuController.
    return;
  }

}

bool PageActionImageView::OnKeyPressed(const ui::KeyEvent& event) {
  if (event.key_code() == ui::VKEY_SPACE ||
      event.key_code() == ui::VKEY_RETURN) {
   
    return true;
  }
  return false;
}

void PageActionImageView::ShowContextMenuForView(
    View* source,
    const gfx::Point& point,
    ui::MenuSourceType source_type) {
  
}

bool PageActionImageView::AcceleratorPressed(
    const ui::Accelerator& accelerator) {
  DCHECK(visible());  // Should not have happened due to CanHandleAccelerator.

  return true;
}

bool PageActionImageView::CanHandleAccelerators() const {
  // While visible, we don't handle accelerators and while so we also don't
  // count as a priority accelerator handler.
  return visible();
}

void PageActionImageView::UpdateVisibility(WebContents* contents,
                                           const GURL& url) {
  SetVisible(true);
}

void PageActionImageView::PaintChildren(gfx::Canvas* canvas) {
  View::PaintChildren(canvas);
}

void PageActionImageView::HidePopup() {

}
