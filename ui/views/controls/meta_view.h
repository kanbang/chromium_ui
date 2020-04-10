// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_CONTROLS_META_VIEW_H_
#define UI_VIEWS_CONTROLS_META_VIEW_H_

#include "ui/gfx/image/image_skia.h"
#include "ui/views/view.h"
#include "ui/gfx/font.h"
#include "ui/gfx/text_elider.h"

namespace gfx {
class Canvas;
}

namespace views {

/////////////////////////////////////////////////////////////////////////////
//
// ImageView class.
//
// An ImageView can display an image from an ImageSkia. If a size is provided,
// the ImageView will resize the provided image to fit if it is too big or will
// center the image if smaller. Otherwise, the preferred size matches the
// provided image size.
//
/////////////////////////////////////////////////////////////////////////////
class VIEWS_EXPORT MetaImage : public View {
 public:
  static const char kViewClassName[];

  MetaImage();
  virtual ~MetaImage();

  enum
  {
	  ALIGN_LEFT,
	  ALIGN_CENTER,
	  ALIGN_RIGHT
  };

  enum
  {
	  VALIGN_TOP,
	  VALIGN_CENTER,
	  VALIGN_BOTTOM,
  };

  enum
  {
	  STRETCH_NONE,
	  STRETCH_FILL,
	  STRETCH_NINE,
	  STRETCH_TILE,
	  STRETCH_STRETCH,
  };

  struct CSSIMAGEINFO{
	  int id;
	  int state;
	  int index;
	  int align;
	  int valign;
	  int strtch;
	  int alpha;
    string16 imageid;

    CSSIMAGEINFO() {
      id = state = index = align = valign = strtch = alpha = 0;
    }
  };

  // Set the image that should be displayed.
  void SetImage(const gfx::ImageSkia& img);

  // Set the image that should be displayed from a pointer. Reset the image
  // if the pointer is NULL. The pointer contents is copied in the receiver's
  // image.
  void SetImage(const gfx::ImageSkia* image_skia);

  // Returns the image currently displayed or NULL of none is currently set.
  // The returned image is still owned by the ImageView.
  const gfx::ImageSkia& GetImage();

  // Set / Get the tooltip text.
  void SetTooltipText(const string16& tooltip);
  string16 GetTooltipText() const;

  void set_interactive(bool interactive) { interactive_ = interactive; }

  // Overriden from View:
  virtual gfx::Size GetPreferredSize() OVERRIDE;
  virtual void OnPaint(gfx::Canvas* canvas) OVERRIDE;
  virtual void GetAccessibleState(ui::AccessibleViewState* state) OVERRIDE;
  virtual bool GetTooltipText(const gfx::Point& p,
                              string16* tooltip) const OVERRIDE;
  virtual bool HitTestRect(const gfx::Rect& rect) const OVERRIDE;
  virtual std::string GetAction() OVERRIDE;
  virtual const char* GetClassName() const OVERRIDE;
  static void DrawImage(CSSIMAGEINFO* css_info, const gfx::Rect& rc, gfx::Canvas* canvas); 
  static bool GetCSSInfo(CSSIMAGEINFO* css_info, std::map<string16, string16>& infos);

 private:

 CSSIMAGEINFO* GetActionInfo(const std::string& action);

  // Whether the image size is set.
  bool image_size_set_;

  // The actual image size.
  gfx::Size image_size_;

  // The underlying image.
  gfx::ImageSkia image_;

  std::map<std::string, CSSIMAGEINFO> image_infos_;

  // The current tooltip text.
  string16 tooltip_text_;

  // A flag controlling hit test handling for interactivity.
  bool interactive_;

  DISALLOW_COPY_AND_ASSIGN(MetaImage);
};

//////////////////////////////////////////////////////////////////////////
// MetaText
class VIEWS_EXPORT MetaText : public View {
public:
	static const char kViewClassName[];

	/*
	  /*
  text-text:≤‚ ‘
  text-color:#FF
  text-fontname:"Œ¢»Ì—≈∫⁄"
  text-fontsize:14
  text-fontstyle:
  text-halo:true/false
  text-align:left center right
  text-valign:top center bottom
  text-endellipse:---
  */

	struct CSSTEXTINFO
	{
		CSSTEXTINFO() {
			halo = false;
			align = 0;
			valign = 0;
			char_break = false;
			multi = false;
		}

		SkColor text_color;
		gfx::Font font;
		gfx::ElideBehavior  elide;
		bool halo;
		int align;
		int valign;
		bool char_break;
		bool multi;
    string16 text_id;
	};

	MetaText();
	virtual ~MetaText();

	// Overriden from View:
	virtual gfx::Size GetPreferredSize() OVERRIDE;
	virtual void OnPaint(gfx::Canvas* canvas) OVERRIDE;
	virtual std::string GetAction() OVERRIDE;
	virtual const char* GetClassName() const OVERRIDE;
	virtual void SetAttribute(const string16& name, const string16& v) OVERRIDE;

private:
	int ComputeDrawStringFlags(const CSSTEXTINFO* pInfo);
	gfx::Rect GetTextBounds(const gfx::Rect& display_rc, const string16& text, const CSSTEXTINFO* pInfo);
	void CalculateDrawStringParams(string16* paint_text,
		gfx::Rect* text_bounds,
		int* flags, const gfx::Rect& display_rc, 
		const string16& text, const CSSTEXTINFO* pInfo);

	CSSTEXTINFO* GetTextInfo(const std::string& action);
	string16 text_;
	gfx::Size text_size_;

	std::map<std::string, CSSTEXTINFO> text_infos_;
};

}  // namespace views

#endif  // UI_VIEWS_CONTROLS_META_VIEW_H_
