// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/views/window/new_custom_frame_view.h"

#include <algorithm>

#include "base/strings/utf_string_conversions.h"
#include "ui/base/hit_test.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/gfx/canvas.h"
#include "ui/gfx/font.h"
#include "ui/gfx/image/image.h"
#include "ui/gfx/path.h"
#include "ui/gfx/gdi_util.h"
#include "ui/views/color_constants.h"
#include "ui/views/controls/button/image_button.h"
#include "ui/views/controls/button/new_button.h"
#include "ui/views/widget/widget.h"
#include "ui/views/widget/widget_delegate.h"
#include "ui/views/window/client_view.h"
#include "ui/views/window/frame_background.h"
#include "ui/views/window/window_resources.h"
#include "ui/views/window/window_shape.h"

#if defined(USE_AURA)
#include "ui/views/widget/native_widget_aura.h"
#elif defined(OS_WIN)
#include "ui/views/widget/native_widget_win.h"
#endif

namespace views {

namespace {


// In the window corners, the resize areas don't actually expand bigger, but the
// 16 px at the end of each edge triggers diagonal resizing.
const int kResizeAreaCornerSize = 16;

#if defined(USE_AURA)
const SkColor kDefaultColorFrame = SkColorSetRGB(109, 109, 109);
const SkColor kDefaultColorFrameInactive = SkColorSetRGB(176, 176, 176);
#else
const SkColor kDefaultColorFrame = SkColorSetRGB(66, 116, 201);
const SkColor kDefaultColorFrameInactive = SkColorSetRGB(161, 182, 228);
#endif

const gfx::Font& GetTitleFont() {
  static gfx::Font* title_font = NULL;
  if (!title_font) {
#if defined(USE_AURA)
    title_font = new gfx::Font(NativeWidgetAura::GetWindowTitleFont());
#elif defined(OS_WIN)
    title_font = new gfx::Font(NativeWidgetWin::GetWindowTitleFont());
#elif defined(OS_LINUX)
    // TODO(ben): need to resolve what font this is.
    title_font = new gfx::Font();
#endif
  }
  return *title_font;
}

}  // namespace

///////////////////////////////////////////////////////////////////////////////
// CustomFrameView, public:

NewCustomFrameView::NewCustomFrameView()
    : frame_(NULL),
      should_show_minmax_buttons_(false) {
}

NewCustomFrameView::~NewCustomFrameView() {
}

void NewCustomFrameView::Init(Widget* frame) {
  frame_ = frame;
}

///////////////////////////////////////////////////////////////////////////////
// CustomFrameView, NonClientFrameView implementation:

gfx::Rect NewCustomFrameView::GetBoundsForClientView() const {
  return client_view_bounds_;
}

gfx::Rect NewCustomFrameView::GetWindowBoundsForClientBounds(
    const gfx::Rect& client_bounds) const {
  //int top_height = NonClientTopBorderHeight();
  //int border_thickness = NonClientBorderThickness();
  return gfx::Rect(std::max(0, client_bounds.x() - client_area.left()),
                   std::max(0, client_bounds.y() - client_area.top()),
                   client_bounds.width() + client_area.width(),
                   client_bounds.height() + client_area.height());
}

int NewCustomFrameView::GetHTComponentForPadding(const gfx::Point& point,
  int top_resize_corner_height,
  int resize_corner_width,
  bool can_resize) {
    // Tricky: In XP, native behavior is to return HTTOPLEFT and HTTOPRIGHT for
    // a |resize_corner_size|-length strip of both the side and top borders, but
    // only to return HTBOTTOMLEFT/HTBOTTOMRIGHT along the bottom border + corner
    // (not the side border).  Vista goes further and doesn't return these on any
    // of the side borders.  We allow callers to match either behavior.
    int component;
    if (point.x() < content_insets.left()) {
      if (point.y() < top_resize_corner_height)
        component = HTTOPLEFT;
      else if (point.y() >= (height() - content_insets.bottom()))
        component = HTBOTTOMLEFT;
      else
        component = HTLEFT;
    } else if (point.x() >= (width() - content_insets.right())) {
      if (point.y() < top_resize_corner_height)
        component = HTTOPRIGHT;
      else if (point.y() >= (height() - content_insets.bottom()))
        component = HTBOTTOMRIGHT;
      else
        component = HTRIGHT;
    } else if (point.y() < content_insets.top()) {
      if (point.x() < resize_corner_width)
        component = HTTOPLEFT;
      else if (point.x() >= (width() - resize_corner_width))
        component = HTTOPRIGHT;
      else
        component = HTTOP;
    } else if (point.y() >= (height() - content_insets.bottom())) {
      if (point.x() < resize_corner_width)
        component = HTBOTTOMLEFT;
      else if (point.x() >= (width() - resize_corner_width))
        component = HTBOTTOMRIGHT;
      else
        component = HTBOTTOM;
    } else {
      return HTNOWHERE;
    }

    // If the window can't be resized, there are no resize boundaries, just
    // window borders.
    return can_resize ? component : HTBORDER;
}

int NewCustomFrameView::NonClientHitTest(const gfx::Point& point) {
  // Sanity check.
  if (!bounds().Contains(point))
    return HTNOWHERE;

  int frame_component = frame_->client_view()->NonClientHitTest(point);
  if (frame_component == HTCLIENT)
    return frame_component;

  View* pView = GetTooltipHandlerForPoint(point);
  if(pView != this && pView->GetAttribute(L"hittest") != L"none") {
    if(pView->GetName() == "sysclose")
      return HTCLOSE;

    if(pView->GetName() == "sysmax" || pView->GetName() == "sysrestore")
      return HTMAXBUTTON;

    if(pView->GetName() == "sysmin")
      return HTMINBUTTON;

    if(pView->GetName() == "sysicon")
      return HTSYSMENU;
  }
  int window_component = GetHTComponentForPadding(point,kResizeAreaCornerSize, kResizeAreaCornerSize,
      frame_->widget_delegate()->CanResize());

  // Fall back to the caption if no other component matches.
  return (window_component == HTNOWHERE) ? HTCAPTION : window_component;
}

void NewCustomFrameView::GetWindowMask(const gfx::Size& size,
                                    gfx::Path* window_mask) {
  DCHECK(window_mask);
  if (frame_->IsMaximized())
    return;
  return;
  //return;
  GetDefaultWindowMask(size, window_mask);
}

gfx::NativeRegion NewCustomFrameView::GetWindowNaviteHRGN(const gfx::Size& size) {
  if(frame_->IsMaximized())
    return NULL;

  if(Image_rgn_.isNull())
    return NULL;

  return gfx::GetHRGNFromImageSkia(Image_rgn_, size);
}

void NewCustomFrameView::ResetWindowControls() {
  NewButton* btn = NewButton::get_named_item(this, "sysrestore");
  if(btn)
    btn->SetState(CustomButton::STATE_NORMAL);

  btn = NewButton::get_named_item(this, "sysmax");
  if(btn)
    btn->SetState(CustomButton::STATE_NORMAL);

  btn = NewButton::get_named_item(this, "sysmin");
  if(btn)
    btn->SetState(CustomButton::STATE_NORMAL);
}

void NewCustomFrameView::UpdateWindowIcon() {
  //if (window_icon_)
  //  window_icon_->SchedulePaint();
}

void NewCustomFrameView::UpdateWindowTitle() {
  //SchedulePaintInRect(title_bounds_);
}

///////////////////////////////////////////////////////////////////////////////
// CustomFrameView, View overrides:

void NewCustomFrameView::OnPaint(gfx::Canvas* canvas) {
  if(!frame_bg_.get() && !image_alpha_.isNull() && !image_bg_.isNull()) {
    frame_bg_.reset(gfx::CopyAlphaChannel(image_bg_, image_alpha_));
  }
  
  if(frame_bg_.get()) {
    gfx::ImageSkia imageka = gfx::ImageSkia::CreateFrom1xBitmap(*frame_bg_);
    canvas->DrawImageByStretchPoint(imageka, 0, 0, bounds().width(), bounds().height(),
      10, imageka.width() - 10, 10, imageka.height() - 10, 0, 1);

    if(!image_shadow_.isNull()) {
      canvas->DrawImageByStretchPoint(image_shadow_, 0, 0, bounds().width(), bounds().height(),
        10, image_shadow_.width() - 10, 10, image_shadow_.height() - 10, 0, 1);
    }
  }
  else if(!image_bg_.isNull())
    canvas->DrawImageByStretchLine(image_bg_, 0, 0, bounds().width(), bounds().height());
}

void NewCustomFrameView::Layout() {
  View::Layout();
  LayoutClientView();
}

gfx::Size NewCustomFrameView::GetPreferredSize() {
  return frame_->non_client_view()->GetWindowBoundsForClientBounds(
      gfx::Rect(frame_->client_view()->GetPreferredSize())).size();
}

void NewCustomFrameView::SetAttribute(const string16& name, const string16& v) {
  if(name == L"clientarea") {
    std::vector<string16> padding = split_text(v, L" ");
    if(padding.size() == 4) {
      int left, top, bottom, right;
      if(base::StringToInt(padding[0], &left) &&
        base::StringToInt(padding[1], &top) &&
        base::StringToInt(padding[2], &right) &&
        base::StringToInt(padding[3], &bottom)) {
          client_area.Set(top, left, bottom, right);
      }
    }
  }
  else if(name == L"image-bg") {
    int icon = 0;
    if(base::StringToInt(v, &icon)) {
      const gfx::ImageSkia* icon_image = ResourceBundle::GetSharedInstance().GetImageNamed(
        icon).ToImageSkia();
      image_bg_ = *icon_image;
    }
  } else if(name == L"image-alpha") {
    int icon = 0;
    if(base::StringToInt(v, &icon)) {
      const gfx::ImageSkia* icon_image = ResourceBundle::GetSharedInstance().GetImageNamed(
        icon).ToImageSkia();
      image_alpha_ = *icon_image;
    }
  } else if(name == L"image-shadow") {
    int icon = 0;
    if(base::StringToInt(v, &icon)) {
      const gfx::ImageSkia* icon_image = ResourceBundle::GetSharedInstance().GetImageNamed(
        icon).ToImageSkia();
      image_shadow_ = *icon_image;
    }
  } else if(name == L"image-rgn") {
    int icon = 0;
    if(base::StringToInt(v, &icon)) {
      const gfx::ImageSkia* icon_image = ResourceBundle::GetSharedInstance().GetImageNamed(
        icon).ToImageSkia();
      Image_rgn_ = *icon_image;
    }
  }
  else View::SetAttribute(name, v); 
}

gfx::Size NewCustomFrameView::GetMinimumSize() {
  return frame_->non_client_view()->GetWindowBoundsForClientBounds(
      gfx::Rect(frame_->client_view()->GetMinimumSize())).size();
}

gfx::Size NewCustomFrameView::GetMaximumSize() {
  gfx::Size max_size = frame_->client_view()->GetMaximumSize();
  gfx::Size converted_size =
      frame_->non_client_view()->GetWindowBoundsForClientBounds(
          gfx::Rect(max_size)).size();
  return gfx::Size(max_size.width() == 0 ? 0 : converted_size.width(),
                   max_size.height() == 0 ? 0 : converted_size.height());
}

///////////////////////////////////////////////////////////////////////////////
// CustomFrameView, ButtonListener implementation:

void NewCustomFrameView::ButtonPressed(Button* sender, const ui::Event& event) {
  if(sender->GetName() == "sysclose")
    frame_->Close();
  else if(sender->GetName() == "sysmin")
    frame_->Minimize();
  else if(sender->GetName() == "sysmax")
    frame_->Maximize();
  else if(sender->GetName() == "sysrestore")
    frame_->Restore();
}

///////////////////////////////////////////////////////////////////////////////
// CustomFrameView, private:

int NewCustomFrameView::IconSize() const {
#if defined(OS_WIN)
  // This metric scales up if either the titlebar height or the titlebar font
  // size are increased.
  return GetSystemMetrics(SM_CYSMICON);
#else
  return std::max(GetTitleFont().GetHeight(), kIconMinimumSize);
#endif
}

bool NewCustomFrameView::ShouldShowClientEdge() const {
  return !frame_->IsMaximized();
}

SkColor NewCustomFrameView::GetFrameColor() const {
  return frame_->IsActive() ? kDefaultColorFrame : kDefaultColorFrameInactive;
}

const gfx::ImageSkia* NewCustomFrameView::GetFrameImage() const {
  return ui::ResourceBundle::GetSharedInstance().GetImageNamed(
      frame_->IsActive() ? IDR_FRAME : IDR_FRAME_INACTIVE).ToImageSkia();
}

void NewCustomFrameView::LayoutClientView() {
  client_view_bounds_.SetRect(client_area.left(), client_area.top(),
      std::max(0, width() - client_area.width()),
      std::max(0, height() - client_area.height()));
}
}  // namespace views
