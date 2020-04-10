// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/views/controls/meta_view.h"

#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"
#include "base/strings/string_number_conversions.h"
#include "third_party/skia/include/core/SkPaint.h"
#include "ui/base/accessibility/accessible_view_state.h"
#include "ui/gfx/canvas.h"
#include "ui/gfx/insets.h"

#include "ui/base/resource/resource_bundle.h"
#include "third_party/css/css_document.h"


namespace views {

	const char MetaImage::kViewClassName[] = "Image";

	MetaImage::MetaImage()
		: image_size_set_(false),
		interactive_(true){
			SetMeta(true);
	}

	MetaImage::~MetaImage() {
	}

	void MetaImage::SetImage(const gfx::ImageSkia& img) {
		gfx::Size pref_size(GetPreferredSize());
		image_ = img;
		if (pref_size != GetPreferredSize())
			PreferredSizeChanged();
		SchedulePaint();
	}

	void MetaImage::SetImage(const gfx::ImageSkia* image_skia) {
		if (image_skia) {
			SetImage(*image_skia);
		} else {
			gfx::ImageSkia t;
			SetImage(t);
		}
	}

	const gfx::ImageSkia& MetaImage::GetImage() {
		return image_;
	}

	gfx::Size MetaImage::GetPreferredSize() {
		gfx::Insets insets = GetInsets();
		if(image_.isNull()) {
			CSSIMAGEINFO* pinfo = GetActionInfo(GetAction());
			if(pinfo) {
				ui::ResourceBundle& rb = ui::ResourceBundle::GetSharedInstance();
				const gfx::ImageSkia* image = rb.GetImageNamed(pinfo->id).ToImageSkia();
				if(image->isNull())
					return image_size_;
				image_ = *image;
				image_size_.set_width(image_.width() / pinfo->state);
				image_size_.set_height(image_.height());
			}
		}

		return image_size_;
	}

	void MetaImage::OnPaint(gfx::Canvas* canvas) {
		View::OnPaint(canvas);

		if (image_.isNull())
			return;

		DrawImage(GetActionInfo(GetAction()), bounds(), canvas);
	}

	void MetaImage::GetAccessibleState(ui::AccessibleViewState* state) {
		state->role = ui::AccessibilityTypes::ROLE_GRAPHIC;
		state->name = tooltip_text_;
	}

	void MetaImage::SetTooltipText(const string16& tooltip) {
		tooltip_text_ = tooltip;
	}

	string16 MetaImage::GetTooltipText() const {
		return tooltip_text_;
	}

	bool MetaImage::GetTooltipText(const gfx::Point& p, string16* tooltip) const {
		if (tooltip_text_.empty())
			return false;

		*tooltip = GetTooltipText();
		return true;
	}

	bool MetaImage::HitTestRect(const gfx::Rect& rect) const {
		return interactive_ ? View::HitTestRect(rect) : false;
	}

	const char* MetaImage::GetClassName() const {
		return kViewClassName;
	}

	std::string MetaImage::GetAction() {
		std::string action = View::GetAction();

		std::string parent_action;
		std::string parent_state;
		if(parent()) {
			parent_action =  parent()->GetAction();
			parent_state = parent()->GetState();
		}

		std::string ret;
		if(!parent_action.empty())
			ret += parent_action;
		if(!action.empty()) {
			ret += ".";
			ret += action;
		}

		if(!parent_state.empty()) {
			ret += ":";
			ret += parent_state;
		}

		return ret;
	}

	MetaImage::CSSIMAGEINFO* MetaImage::GetActionInfo(const std::string& action) {
		CSSIMAGEINFO* pInfo = NULL;
		if(!action.empty()) {
			std::map<std::string, CSSIMAGEINFO>::iterator itr = image_infos_.find(action);
			if(itr != image_infos_.end()) {
				pInfo = (&(itr->second));
			} else {
				LU_CSSDocument& css = ui::ResourceBundle::GetShareCssInstance();
				std::map<string16, string16> images = css.select_map(UTF8ToWide(action));
				if(images.size() != 0) {
					CSSIMAGEINFO info;
          GetCSSInfo(&info, images);
					image_infos_[action] = info;
					pInfo = &(image_infos_[action]);
				}
			}
		}

		return pInfo;
	}

  bool MetaImage::GetCSSInfo(CSSIMAGEINFO* css_info, std::map<string16, string16>& images) {
    if(!css_info)
      return false;

    if(images.find(L"image-src") != images.end())
      base::StringToInt(images[L"image-src"], &css_info->id);

    base::StringToInt(images[L"image-state"], &css_info->state);
    if(css_info->state <= 0)
      css_info->state = 1;

    base::StringToInt(images[L"image-index"], &css_info->index);
    if(css_info->index < 0 || css_info->index < css_info->state)
      css_info->index = 0;

    string16 stretch = images[L"image-stretch"];
    if(stretch == L"nine") {
      css_info->strtch = STRETCH_NINE;
    } else if(stretch == L"fill") {
      css_info->strtch = STRETCH_FILL;
    } else if(stretch == L"tile"){
      css_info->strtch = STRETCH_TILE;
    } else 
      css_info->strtch = STRETCH_NONE;

    string16 align = images[L"image-align"];
    if(align == L"center") {
      css_info->align = ALIGN_CENTER;
    } else if(align == L"right") {
      css_info->align = ALIGN_RIGHT;
    } else
      css_info->align = ALIGN_LEFT;

    string16 valign = images[L"image-valign"];
    if(valign == L"center") {
      css_info->valign = VALIGN_CENTER;
    } else if(valign == L"bottom") {
      css_info->valign = VALIGN_BOTTOM;
    } else
      css_info->valign = VALIGN_TOP;

    return true;
  }

	void MetaImage::DrawImage(MetaImage::CSSIMAGEINFO* css_info, const gfx::Rect& rc, gfx::Canvas* canvas) {
		if(!css_info)
			return;

		ui::ResourceBundle& rb = ui::ResourceBundle::GetSharedInstance();
		const gfx::ImageSkia* image = rb.GetImageNamed(css_info->id).ToImageSkia();
		if(image->isNull())
			return;

		int width = image->width() / css_info->state;
		int height = image->height();

		int x = rc.x();
		int y = rc.y();

		if(css_info->strtch == STRETCH_NINE) {
			canvas->DrawImageByStretchLine(*image, rc.x(), rc.y(), rc.width(), rc.height(), css_info->index, css_info->state);
		} else if(css_info->strtch == STRETCH_FILL) {
			canvas->DrawImageInt(*image, css_info->index*width, 0, width, height, rc.x(), rc.y(), rc.width(), rc.height(), false);
		} else if(css_info->strtch == STRETCH_TILE){
			canvas->TileImageInt(*image, rc.x(), rc.y(), rc.width(), rc.height());
		} else {
			if(css_info->align == ALIGN_CENTER) {
				x = rc.x() + (rc.width() - width) / 2;
			} else if(css_info->align == ALIGN_RIGHT) {
				x = rc.x() + (rc.width() - width);
			}

			if(css_info->valign == VALIGN_CENTER) {
				y = rc.y() + (rc.height() - height) / 2;
			} else if(css_info->valign == VALIGN_BOTTOM) {
				y = rc.y() + (rc.height() - height);
			}

			canvas->DrawImageInt(*image, css_info->index*width, 0, width, height, x, y, width, height, false);
		}  
	}

	//////////////////////////////////////////////////////////////////////////
	// CMetaText

	const char MetaText::kViewClassName[] = "Text";

	MetaText::MetaText() {

	}

	MetaText::~MetaText() {

	}

	gfx::Size MetaText::GetPreferredSize() {
		gfx::Rect rc = GetTextBounds(bounds(), text_, GetTextInfo(GetAction()));
		return gfx::Size(rc.width(), rc.height());
	}

	void MetaText::OnPaint(gfx::Canvas* canvas) {
		CSSTEXTINFO* pInfo = GetTextInfo(GetAction());
		if(pInfo) {
			string16 paint_text;
			gfx::Rect text_bounds;
			int flags = 0;
			gfx::FontList font_list(pInfo->font);
			CalculateDrawStringParams(&paint_text, &text_bounds, &flags,
				bounds(), text_, pInfo);
			gfx::ShadowValues shadows;
			if(pInfo->halo) {
				/*std::string color = base::GetRGBAColor(halo);
				uint64 ch;
				SkColor halocolor;
				if(base::HexStringToUInt64(color, &ch)) {
					halocolor = (uint32)ch;
				}
				canvas->DrawStringRectWithHalo(paint_text, font_list, cb, halocolor, text_bounds, flags);*/
			} else {
				canvas->DrawStringRectWithShadows(paint_text, font_list,
					pInfo->text_color,
					pInfo->multi ? bounds() : text_bounds, 0, flags, shadows);
			}
		}
	}

	std::string MetaText::GetAction() {
		std::string action = View::GetAction();

		std::string parent_action;
		std::string parent_state;
		if(parent()) {
			parent_action =  parent()->GetAction();
			parent_state = parent()->GetState();
		}

		std::string ret;
		if(!parent_action.empty())
			ret += parent_action;
		if(!action.empty()) {
			ret += ".";
			ret += action;
		}

		if(!parent_state.empty()) {
			ret += ":";
			ret += parent_state;
		}

		return ret;
	}

	const char* MetaText::GetClassName() const {
		return kViewClassName;
	}

	void MetaText::SetAttribute(const string16& name, const string16& v) {
		if(name == L"text")
			text_ = v;
		else
			View::SetAttribute(name, v);
	}

	MetaText::CSSTEXTINFO* MetaText::GetTextInfo(const std::string& action) {
		CSSTEXTINFO* pInfo = NULL;
		if(!action.empty()) {
			std::map<std::string, CSSTEXTINFO>::iterator itr = text_infos_.find(action);
			if(itr != text_infos_.end()) {
				pInfo = (&(itr->second));
			} else {
				CSSTEXTINFO info;
				LU_CSSDocument& css = ui::ResourceBundle::GetShareCssInstance();
				std::map<string16, string16> testinfos = css.select_map(UTF8ToWide(action));
				if(testinfos.size() == 0)
					return pInfo;

				string16 color_value = testinfos[L"text-color"];
				if(!color_value.empty()) {
					std::string color = base::GetRGBAColor(color_value);
					uint64 c;
					if(base::HexStringToUInt64(color, &c)) {
						info.text_color = (uint32)c;
					}
				}

				ui::ResourceBundle& rb = ui::ResourceBundle::GetSharedInstance();
				info.font = rb.GetFont(ui::ResourceBundle::BaseFont);
				if(testinfos.find(L"text-fontstyle") != testinfos.end()) {
					int style = 3;
					base::StringToInt(testinfos[L"text-fontstyle"], &style);
					info.font = rb.GetFont(ui::ResourceBundle::FontStyle(style));
				} else {
					string16 font_name = testinfos[L"text-fontname"];
					string16 font_size = testinfos[L"text-fontsize"];

					if(!font_name.empty() && !font_size.empty()) {
						int fs = 12;
						base::StringToInt(font_size, &fs);
						info.font = gfx::Font(WideToASCII(font_name), fs); 
					}
				}

				info.halo = (testinfos[L"text-halo"] == L"true");
				info.multi = (testinfos[L"text-multiline"] == L"true");
				int elide = 0;
				base::StringToInt(testinfos[L"text-elide"], &elide);
				if(elide == 0)
					info.elide = gfx::ELIDE_AT_END;
				else if(elide == 1)
					info.elide = gfx::ELIDE_IN_MIDDLE;
				else 
					info.elide = gfx::TRUNCATE_AT_END;

				info.char_break = (testinfos[L"text-charbreak"] == L"true");
				string16 align = testinfos[L"text-align"];
				if(align == L"center")
					info.align = 1;
				else if(align == L"right")
					info.align = 2;

				text_infos_[action] = info;
				pInfo = &(text_infos_[action]);
			}
		}

		return pInfo;
	}

	int MetaText::ComputeDrawStringFlags(const CSSTEXTINFO* pInfo) {
		int flags = 0;
		flags |= gfx::Canvas::FORCE_LTR_DIRECTIONALITY;

		if(pInfo) {
			if(pInfo->align == 1)
				flags |= gfx::Canvas::TEXT_ALIGN_CENTER;
			else if(pInfo->align == 2)
				flags |= gfx::Canvas::TEXT_ALIGN_RIGHT;
			else
				flags |= gfx::Canvas::TEXT_ALIGN_LEFT;

			if (!pInfo->multi)
				return flags;

			flags |= gfx::Canvas::MULTI_LINE;

			if (pInfo->char_break)
				flags |= gfx::Canvas::CHARACTER_BREAK;
		}

		return flags;
	}

	gfx::Rect MetaText::GetTextBounds(const gfx::Rect& display_rc, const string16& text, const CSSTEXTINFO* pInfo) {
		if(!pInfo)
			return gfx::Rect();

		int w = pInfo->multi ?
			display_rc.width() : std::numeric_limits<int>::max();
		int h = pInfo->font.GetHeight();

		// For single-line strings, ignore the available width and calculate how
		// wide the text wants to be.
		int flags = ComputeDrawStringFlags(pInfo);
		if (!pInfo->multi)
			flags |= gfx::Canvas::NO_ELLIPSIS;
		w = display_rc.width();
		gfx::FontList fontlist(pInfo->font);
		gfx::Canvas::SizeStringInt(text, fontlist, &w, &h, 0, flags);
		gfx::Size text_size;
		text_size.SetSize(w, h);
		text_size.set_width(std::min(display_rc.width(), text_size.width()));

		gfx::Point text_origin(0, 0);
		if(pInfo->align == 1)
			text_origin.Offset((display_rc.width() + 1 - text_size.width()) / 2,
			0);
		else if(pInfo->align == 2)
			text_origin.set_x(display_rc.right() - text_size.width());

		text_origin.Offset(0,
			std::max(0, (display_rc.height() - text_size.height())) / 2);
		return gfx::Rect(text_origin, text_size);
	}


	void MetaText::CalculateDrawStringParams(string16* paint_text,
		gfx::Rect* text_bounds,
		int* flags, const gfx::Rect& display_rc, 
		const string16& text, const CSSTEXTINFO* pInfo)  {
			DCHECK(paint_text && text_bounds && flags);
			if(!pInfo)
				return ;

			gfx::FontList fontlist(pInfo->font);
			// TODO(msw): Use ElideRectangleText to support eliding multi-line text.  Once
			// this is done, we can set NO_ELLIPSIS unconditionally at the bottom.
			if (pInfo->multi || (pInfo->elide == gfx::ELIDE_AT_END)) {
				*paint_text = text;
			} else if (pInfo->elide == gfx::ELIDE_IN_MIDDLE) {
				*paint_text = gfx::ElideText(text, fontlist, display_rc.width(),
					gfx::ELIDE_IN_MIDDLE);
			} else if (pInfo->elide == gfx::TRUNCATE_AT_END) {
				*paint_text = gfx::ElideText(text, fontlist, display_rc.width(),
					gfx::ELIDE_AT_END);
			} else {
				*paint_text = gfx::ElideEmail(text, fontlist,
					display_rc.width());
			}

			*text_bounds = GetTextBounds(display_rc, text, pInfo);
			*flags = ComputeDrawStringFlags(pInfo);
			if (!pInfo->multi || (pInfo->elide == gfx::ELIDE_AT_END))
				*flags |= gfx::Canvas::NO_ELLIPSIS;
	}

}  // namespace views
