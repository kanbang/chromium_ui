// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_CONTROLS_BUTTON_NEW_BUTTON_H_
#define UI_VIEWS_CONTROLS_BUTTON_NEW_BUTTON_H_

#include "base/gtest_prod_util.h"
#include "base/memory/scoped_ptr.h"
#include "ui/base/layout.h"
#include "ui/gfx/image/image_skia.h"
#include "ui/gfx/font.h"
#include "ui/views/controls/button/custom_button.h"

namespace views {

// An image button.

// Note that this type of button is not focusable by default and will not be
// part of the focus chain.  Call set_focusable(true) to make it part of the
// focus chain.

class VIEWS_EXPORT NewButton : public CustomButton {
 public:
  static const char kViewClassName[];

  enum HorizontalAlignment {
    ALIGN_LEFT = 0,
    ALIGN_CENTER,
    ALIGN_RIGHT
  };

  enum VerticalAlignment {
    ALIGN_TOP = 0,
    ALIGN_MIDDLE,
    ALIGN_BOTTOM
  };

  enum NEWBUTTON_TYPE
  {
    TYPE_TEXT_BUTTON,
    TYPE_IMAGE_BUTTON,
    TYPE_ICON_BUTTON,
    TYPE_MASK_IMAGE_NUTTON,
  };

  static NewButton *get_named_item(View* parent, const std::string& name);

  explicit NewButton(ButtonListener* listener);
  virtual ~NewButton();

  // Returns the image for a given |state|.
  virtual const gfx::ImageSkia& GetImage(ButtonState state) const;

  // Set the image the button should use for the provided state.
  virtual void SetImage(const gfx::ImageSkia* image);

  // Set the background details.
  void SetBackground(const gfx::ImageSkia* mask);

  // Set an |image| to draw on top of the normal / hot / pushed image.
  // Pass NULL for no image.
  void SetOverlayImage(const gfx::ImageSkia* image);

  // Sets how the image is laid out within the button's bounds.
  void SetImageAlignment(HorizontalAlignment h_align,
                         VerticalAlignment v_align);

  void SetType(NEWBUTTON_TYPE type) { type_ = type; }
  void SetText(const string16& text);
  void SetFont(gfx::Font font);

  // Overridden from View:
  virtual gfx::Size GetPreferredSize() OVERRIDE;
  virtual const char* GetClassName() const OVERRIDE;
  virtual void OnPaint(gfx::Canvas* canvas) OVERRIDE;
  virtual std::string GetState() OVERRIDE;
  virtual void SetAttribute(const string16& name, const string16& v);

  // Sets preferred size, so it could be correctly positioned in layout even if
  // it is NULL.
  void SetPreferredSize(const gfx::Size& preferred_size) {
    preferred_size_ = preferred_size;
  }

  // Whether we should draw our images resources horizontally flipped.
  void SetDrawImageMirrored(bool mirrored) {
    draw_image_mirrored_ = mirrored;
  }

 protected:
  // Returns the image to paint. This is invoked from paint and returns a value
  // from images.
  virtual gfx::ImageSkia GetImageToPaint();

  // Updates button background for |scale_factor|.
  void UpdateButtonBackground(ui::ScaleFactor scale_factor);

  int GetStateIndex();

  // The images used to render the different states of this button.
  gfx::ImageSkia images_;
  
  gfx::ImageSkia background_image_;

  // Image to draw on top of normal / hot / pushed image.  Usually empty.
  gfx::ImageSkia overlay_image_;

 private:

   void CalculateTextSize(gfx::Size* text_size, int max_width);

  // Returns the correct position of the image for painting.
  gfx::Point ComputeImagePaintPosition(const gfx::ImageSkia& image);

  // Image alignment.
  HorizontalAlignment h_alignment_;
  VerticalAlignment v_alignment_;
  gfx::Size preferred_size_;
  NEWBUTTON_TYPE type_;

  // Whether we draw our resources horizontally flipped. This can happen in the
  // linux titlebar, where image resources were designed to be flipped so a
  // small curved corner in the close button designed to fit into the frame
  // resources.
  bool draw_image_mirrored_;
  string16 text_;
  SkColor color_normar_;
  SkColor color_hover_;
  SkColor color_check_;
  SkColor color_disable_;
  gfx::Font font_;
  gfx::Size text_size_;
  int icont_text_space_;

  DISALLOW_COPY_AND_ASSIGN(NewButton);
};

}  // namespace views

#endif  // UI_VIEWS_CONTROLS_BUTTON_IMAGE_BUTTON_H_
