// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/views/controls/button/new_button.h"

#include "base/strings/utf_string_conversions.h"
#include "ui/base/accessibility/accessible_view_state.h"
#include "ui/gfx/animation/throb_animation.h"
#include "ui/gfx/canvas.h"
#include "ui/gfx/image/image_skia_operations.h"
#include "ui/gfx/scoped_canvas.h"
#include "ui/views/widget/widget.h"
#include "ui/base/resource/resource_bundle.h"

namespace views {

static const int kDefaultWidth = 16;   // Default button width if no theme.
static const int kDefaultHeight = 14;  // Default button height if no theme.

const char NewButton::kViewClassName[] = "Button";

////////////////////////////////////////////////////////////////////////////////
// ImageButton, public:

NewButton::NewButton(ButtonListener* listener)
    : CustomButton(listener),
      h_alignment_(ALIGN_LEFT),
      v_alignment_(ALIGN_TOP),
      type_(TYPE_TEXT_BUTTON),
      preferred_size_(kDefaultWidth, kDefaultHeight),
      draw_image_mirrored_(false),
      icont_text_space_(2){
  // By default, we request that the gfx::Canvas passed to our View::OnPaint()
  // implementation is flipped horizontally so that the button's images are
  // mirrored when the UI directionality is right-to-left.
  EnableCanvasFlippingForRTLUI(true);
  color_normar_ = color_hover_ = color_check_ = SK_ColorBLACK;
  color_disable_ = SK_ColorGRAY;
}

NewButton::~NewButton() {
}

const gfx::ImageSkia& NewButton::GetImage(ButtonState state) const {
  return images_;//[state];
}

void NewButton::SetImage(const gfx::ImageSkia* image) {
  images_ = image ? *image : gfx::ImageSkia();
  PreferredSizeChanged();
}

void NewButton::SetBackground(const gfx::ImageSkia* mask) {
  if (mask == NULL) {
    background_image_ = gfx::ImageSkia();
    return;
  }

  background_image_ = *mask;
}

void NewButton::SetOverlayImage(const gfx::ImageSkia* image) {
  if (!image) {
    overlay_image_ = gfx::ImageSkia();
    return;
  }
  overlay_image_ = *image;
}

void NewButton::SetImageAlignment(HorizontalAlignment h_align,
                                    VerticalAlignment v_align) {
  h_alignment_ = h_align;
  v_alignment_ = v_align;
  SchedulePaint();
}

void NewButton::SetText(const string16& text) {
  if(text_ != text) {
    text_ = text;
    CalculateTextSize(&text_size_, 0);
  }
}

void NewButton::SetFont(gfx::Font font) {
  font_ = font;
  if(text_ != L"")
    CalculateTextSize(&text_size_, 0);
}

////////////////////////////////////////////////////////////////////////////////
// ImageButton, View overrides:

void NewButton::CalculateTextSize(gfx::Size* text_size, int max_width) {
  int h = font_.GetHeight();
  int w = 0;
  int flags = 0;
  flags |= gfx::Canvas::NO_ELLIPSIS;
  gfx::Canvas::SizeStringInt(text_, font_, &w, &h, 0, flags);
  text_size->SetSize(w, h);
}

gfx::Size NewButton::GetPreferredSize() {
  gfx::Size size = preferred_size_;

  if(!images_.isNull()) {
    if(type_ == TYPE_TEXT_BUTTON || type_ == TYPE_ICON_BUTTON) {
      size = gfx::Size(images_.width(), images_.height());
    }
    else
      size = gfx::Size(images_.width() / 4, images_.height());
  }

  if(type_ == TYPE_TEXT_BUTTON) {
    size.Enlarge(text_size_.width() + icont_text_space_, 0);

    if(size.height() < text_size_.height())
      size.set_height(text_size_.height());
  }

  gfx::Insets insets = content_insets;
  size.Enlarge(insets.width(), insets.height());
  return size;
}

const char* NewButton::GetClassName() const {
  return kViewClassName;
}

void NewButton::OnPaint(gfx::Canvas* canvas) {
  // Call the base class first to paint any background/borders.
  View::OnPaint(canvas);
	
  gfx::ImageSkia img = GetImageToPaint();

  int index = GetStateIndex();
  if (!img.isNull()) {
    gfx::ScopedCanvas scoped(canvas);

    if (!background_image_.isNull())
      canvas->DrawImageByStretchLine(background_image_, 0, 0, width(), height(), index, 4);
    
    if(type_ != TYPE_TEXT_BUTTON) {
      gfx::Point position = ComputeImagePaintPosition(img);
      if(type_ == TYPE_IMAGE_BUTTON || type_ == TYPE_MASK_IMAGE_NUTTON)
        canvas->DrawImageByStretchLine(images_, position.x(), position.y(), images_.width() / 4, images_.height(), index, 4);
      else
        canvas->DrawImageByStretchLine(images_, position.x(), position.y(), images_.height(), images_.height(), 0, 1);

      if (!overlay_image_.isNull())
        canvas->DrawImageInt(overlay_image_, position.x(), position.y());
    } else {
      int x = 0, y = 0;
      gfx::Rect rect = GetContentsBounds();
      rect.Inset(content_insets);

      int width = text_size_.width();
      if(!images_.isNull())
        width += (icont_text_space_ + images_.width());
      x = (rect.width() - width) / 2;
      if(!images_.isNull()) {
        y = (rect.height() - images_.height()) /2;
        canvas->DrawImageInt(images_, x + rect.x(), y + rect.y());
        x += (icont_text_space_ + images_.width());
      }
      
      y = (rect.height() - text_size_.height()) / 2;
      gfx::ShadowValues shadows;
      gfx::Rect text_bounds;
      text_bounds.SetRect(x + rect.x(), y + rect.y(), text_size_.width(), text_size_.height());
      SkColor text_color = color_normar_;
      if(index == 1)
        text_color = color_hover_;
      else if(index == 2)
        text_color = color_check_;
      else if(index == 3)
        text_color = color_disable_;

      canvas->DrawStringWithShadows(text_, font_, text_color, text_bounds,
        0, 0, shadows);
    }
  }
  OnPaintFocusBorder(canvas);
}

std::string NewButton::GetState() {
	if(state_ == Button::STATE_NORMAL)
		return "";
	
	if(state_ == Button::STATE_HOVERED)
		return "hot";

	if(state_ == Button::STATE_PRESSED)
		return "checked";

	if(state_ == Button::STATE_DISABLED)
		return "disabled";

	return "";
}

int NewButton::GetStateIndex() {
  if(state_ == Button::STATE_NORMAL)
    return 0;

  if(state_ == Button::STATE_HOVERED)
    return 1;

  if(state_ == Button::STATE_PRESSED)
    return 2;

  if(state_ == Button::STATE_DISABLED)
    return 3;

  return 0;
}

void NewButton::SetAttribute(const string16& name, const string16& v) {
  if(name == L"text")
    SetText(v);
  else if(name == L"font") {
    int fontindex  = 0;
    if(base::StringToInt(v, &fontindex)) {
      ui::ResourceBundle& rb = ui::ResourceBundle::GetSharedInstance();
      SetFont(rb.GetFont((ui::ResourceBundle::FontStyle)fontindex));
    }
  }
  else if(name == L"type") {
    if(v == L"text") {
      SetType(TYPE_TEXT_BUTTON);
    }
    else if(v == L"icon")
      SetType(TYPE_ICON_BUTTON);
    else if(v == L"image")
      SetType(TYPE_IMAGE_BUTTON);
    else 
      SetType(TYPE_MASK_IMAGE_NUTTON);
  }else if(name == L"color") {
    color_normar_ = (SkColor(base::GetColor(v)));
  }
  else if(name == L"color-hot")
    color_hover_ = (SkColor(base::GetColor(v)));
  else if(name == L"color-highlight")
    color_check_ = (SkColor(base::GetColor(v)));
  else if(name == L"color-disable")
    color_disable_= (SkColor(base::GetColor(v)));
  else if(name == L"icon-space") {
    int space  = 0;
    if(base::StringToInt(v, &space)) {
      icont_text_space_ = space;
    }
  }
  if(name == L"image") {
    int icon = 0;
    if(base::StringToInt(v, &icon)) {
      const gfx::ImageSkia* icon_image = ResourceBundle::GetSharedInstance().GetImageNamed(
        icon).ToImageSkia();
      SetImage(icon_image);
    }
  } else if(name == L"mask") {
    int icon = 0;
    if(base::StringToInt(v, &icon)) {
      const gfx::ImageSkia* icon_image = ResourceBundle::GetSharedInstance().GetImageNamed(
        icon).ToImageSkia();
      SetBackground(icon_image);
    }
  }
  else
    CustomButton::SetAttribute(name, v);
}

////////////////////////////////////////////////////////////////////////////////
// ImageButton, protected:

gfx::ImageSkia NewButton::GetImageToPaint() {
  gfx::ImageSkia img;
  return images_.isNull() ? img : images_;
}

////////////////////////////////////////////////////////////////////////////////
// ImageButton, private:

gfx::Point NewButton::ComputeImagePaintPosition(const gfx::ImageSkia& image) {
  int x = 0, y = 0;
  gfx::Rect rect = GetContentsBounds();
  rect.Inset(content_insets);

  int w = image.width();
  if(type_ == TYPE_IMAGE_BUTTON || type_ == TYPE_MASK_IMAGE_NUTTON)
    w = image.width() / 4;

  if (h_alignment_ == ALIGN_CENTER)
    x = (rect.width() - w) / 2;
  else if (h_alignment_ == ALIGN_RIGHT)
    x = rect.width() - w;

  if (v_alignment_ == ALIGN_MIDDLE)
    y = (rect.height() - image.height()) / 2;
  else if (v_alignment_ == ALIGN_BOTTOM)
    y = rect.height() - image.height();

  x += rect.x();
  y += rect.y();

  return gfx::Point(x, y);
}

NewButton *NewButton::get_named_item(View* parent, const std::string& name)
{
  NewButton *object = NULL;
  if (parent)
    object = static_cast<NewButton*>(parent->GetViewByName(name));

  return object;
}
}  // namespace views
