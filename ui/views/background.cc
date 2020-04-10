// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/views/background.h"

#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"
#include "base/strings/string_number_conversions.h"
#include "skia/ext/skia_utils_win.h"
#include "third_party/skia/include/core/SkPaint.h"
#include "ui/gfx/canvas.h"
#include "ui/gfx/color_utils.h"
#include "ui/views/painter.h"
#include "ui/views/view.h"
#include "ui/views/controls/meta_view.h"

#include "ui/base/resource/resource_bundle.h"
#include "third_party/css/css_document.h"

namespace views {

// SolidBackground is a trivial Background implementation that fills the
// background in a solid color.
class SolidBackground : public Background {
 public:
  explicit SolidBackground(SkColor color) {
    SetNativeControlColor(color);
  }

  virtual void Paint(gfx::Canvas* canvas, View* view) OVERRIDE {
    // Fill the background. Note that we don't constrain to the bounds as
    // canvas is already clipped for us.
    canvas->DrawColor(get_color());
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(SolidBackground);
};

class BackgroundPainter : public Background {
 public:
  BackgroundPainter(bool owns_painter, Painter* painter)
      : owns_painter_(owns_painter), painter_(painter) {
    DCHECK(painter);
  }

  virtual ~BackgroundPainter() {
    if (owns_painter_)
      delete painter_;
  }


  virtual void Paint(gfx::Canvas* canvas, View* view) OVERRIDE {
    Painter::PaintPainterAt(canvas, painter_, view->GetLocalBounds());
  }

 private:
  bool owns_painter_;
  Painter* painter_;

  DISALLOW_COPY_AND_ASSIGN(BackgroundPainter);
};

class ImageBackground : public Background {
public:
	ImageBackground(){
		set_image_info_ = false;
	}

	ImageBackground(int id, int state, int index) {
		css_info_.id = id;
		css_info_.state = state;
		css_info_.index = index;
		set_image_info_ = true;
	}

	virtual ~ImageBackground() {

	}

	virtual void Paint(gfx::Canvas* canvas, View* view) OVERRIDE {
    if(!set_image_info_) {
      LU_CSSDocument& css = ui::ResourceBundle::GetShareCssInstance();
      std::map<string16, string16> images = css.select_map(UTF8ToWide(view->GetAction()));
      MetaImage::GetCSSInfo(&css_info_, images);
      set_image_info_ = true;
    }

    if(css_info_.state == 4) {
      std::string status = view->GetState();
      if(status == "hot")
        css_info_.index = 1;
      else if(status == "checked")
        css_info_.index = 2;
      else if(status == "disable")
        css_info_.index = 3;
      else
        css_info_.index = 0;
    }

    MetaImage::DrawImage(&css_info_, view->GetLocalBounds(), canvas);
	}

protected:
	MetaImage::CSSIMAGEINFO css_info_;
	bool set_image_info_;
};

Background::Background()
    : color_(SK_ColorWHITE)
#if defined(OS_WIN)
    , native_control_brush_(NULL)
#endif
{
}

Background::~Background() {
#if defined(OS_WIN)
  DeleteObject(native_control_brush_);
#endif
}

void Background::SetNativeControlColor(SkColor color) {
  color_ = color;
#if defined(OS_WIN)
  DeleteObject(native_control_brush_);
  native_control_brush_ = NULL;
#endif
}

#if defined(OS_WIN)
HBRUSH Background::GetNativeControlBrush() const {
  if (!native_control_brush_)
    native_control_brush_ = CreateSolidBrush(skia::SkColorToCOLORREF(color_));
  return native_control_brush_;
}
#endif

//static
Background* Background::CreateSolidBackground(SkColor color) {
  return new SolidBackground(color);
}

//static
Background* Background::CreateStandardPanelBackground() {
  // TODO(beng): Should be in NativeTheme.
#if defined(USE_AURA)
  return CreateSolidBackground(SK_ColorWHITE);
#else
  return CreateVerticalGradientBackground(SkColorSetRGB(246, 250, 255),
                                          SkColorSetRGB(219, 235, 255));
#endif
}

Background* Background::CreateImageBackground() {
  return new ImageBackground();
}

Background* Background::CreateImageBackground(int id, int state, int index) {
  return new ImageBackground(id, state, index);
}

//static
Background* Background::CreateVerticalGradientBackground(SkColor color1,
                                                         SkColor color2) {
  Background* background = CreateBackgroundPainter(
      true, Painter::CreateVerticalGradient(color1, color2));
  background->SetNativeControlColor(
      color_utils::AlphaBlend(color1, color2, 128));

  return background;
}

//static
Background* Background::CreateVerticalMultiColorGradientBackground(
    SkColor* colors,
    SkScalar* pos,
    size_t count) {
  Background* background = CreateBackgroundPainter(
      true, Painter::CreateVerticalMultiColorGradient(colors, pos, count));
  background->SetNativeControlColor(
      color_utils::AlphaBlend(colors[0], colors[count-1], 128));

  return background;
}

//static
Background* Background::CreateBackgroundPainter(bool owns_painter,
                                                Painter* painter) {
  return new BackgroundPainter(owns_painter, painter);
}

}  // namespace views
