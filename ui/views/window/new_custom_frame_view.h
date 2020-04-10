// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_WINDOW_NEW_CUSTOM_FRAME_VIEW_H_
#define UI_VIEWS_WINDOW_NEW_CUSTOM_FRAME_VIEW_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "ui/gfx/image/image_skia.h"
#include "ui/views/controls/button/button.h"
#include "ui/views/window/non_client_view.h"

namespace gfx {
class ImageSkia;
}

namespace views {

class FrameBackground;
class ImageButton;
class Widget;

///////////////////////////////////////////////////////////////////////////////
//
// CustomFrameView
//
//  A view that provides the non client frame for Windows. This means
//  rendering the non-standard window caption, border, and controls.
//
////////////////////////////////////////////////////////////////////////////////
class NewCustomFrameView : public NonClientFrameView,
                        public ButtonListener {
 public:
  NewCustomFrameView();
  virtual ~NewCustomFrameView();

  void Init(Widget* frame);

  // Overridden from NonClientFrameView:
  virtual gfx::Rect GetBoundsForClientView() const OVERRIDE;
  virtual gfx::Rect GetWindowBoundsForClientBounds(
      const gfx::Rect& client_bounds) const OVERRIDE;
  virtual int NonClientHitTest(const gfx::Point& point) OVERRIDE;
  virtual void GetWindowMask(const gfx::Size& size,
                             gfx::Path* window_mask) OVERRIDE;
  virtual gfx::NativeRegion GetWindowNaviteHRGN(const gfx::Size& size) OVERRIDE;
  virtual void ResetWindowControls() OVERRIDE;
  virtual void UpdateWindowIcon() OVERRIDE;
  virtual void UpdateWindowTitle() OVERRIDE;

  // Overridden from View:
  virtual void OnPaint(gfx::Canvas* canvas) OVERRIDE;
  virtual void Layout() OVERRIDE;
  virtual gfx::Size GetPreferredSize() OVERRIDE;
  virtual gfx::Size GetMinimumSize() OVERRIDE;
  virtual gfx::Size GetMaximumSize() OVERRIDE;

  // Overridden from ButtonListener:
  virtual void ButtonPressed(Button* sender, const ui::Event& event) OVERRIDE;
  virtual void SetAttribute(const string16& name, const string16& v) OVERRIDE;

 private:
 
  // Returns the size of the titlebar icon.  This is used even when the icon is
  // not shown, e.g. to set the titlebar height.
  int IconSize() const;

  // Returns true if the client edge should be drawn. This is true if
  // the window is not maximized.
  bool ShouldShowClientEdge() const;


  int GetHTComponentForPadding(const gfx::Point& point,
    int top_resize_corner_height,
    int resize_corner_width,
    bool can_resize);

  // Compute aspects of the frame needed to paint the frame background.
  SkColor GetFrameColor() const;
  const gfx::ImageSkia* GetFrameImage() const;

  void LayoutClientView();

  // The bounds of the client view, in this view's coordinates.
  gfx::Rect client_view_bounds_;

  // The layout rect of the title, if visible.
  //gfx::Rect title_bounds_;

  //int br_,bt_,bl_, bb_;

  // Not owned.
  Widget* frame_;

  gfx::Insets client_area;

  // Should minimize and maximize buttons be shown? This is true when the
  // window can be maximized.
  bool should_show_minmax_buttons_;

  scoped_ptr<SkBitmap> frame_bg_;
  gfx::ImageSkia image_bg_;
  gfx::ImageSkia image_alpha_;
  gfx::ImageSkia image_shadow_;
  gfx::ImageSkia Image_rgn_;

  DISALLOW_COPY_AND_ASSIGN(NewCustomFrameView);
};

}  // namespace views

#endif  // UI_VIEWS_WINDOW_NEW_CUSTOM_FRAME_VIEW_H_
