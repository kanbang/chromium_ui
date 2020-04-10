// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/gfx/canvas.h"

#include <cmath>
#include <limits>

#include "base/logging.h"
#include "third_party/skia/include/core/SkBitmap.h"
#include "third_party/skia/include/effects/SkGradientShader.h"
#include "ui/gfx/canvas.h"
#include "ui/gfx/font_list.h"
#include "ui/gfx/rect.h"
#include "ui/gfx/size_conversions.h"
#include "ui/gfx/skia_util.h"
#include "ui/gfx/transform.h"

#if defined(OS_WIN)
#include "ui/gfx/canvas_skia_paint.h"
#endif

namespace gfx {

const SkColor kStretchLineColor = SkColorSetRGB(255, 0, 255);

Canvas::Canvas(const Size& size, float image_scale, bool is_opaque)
    : image_scale_(image_scale),
      canvas_(NULL) {
  Size pixel_size = ToCeiledSize(ScaleSize(size, image_scale));
  owned_canvas_ = skia::AdoptRef(skia::CreatePlatformCanvas(pixel_size.width(),
                                                            pixel_size.height(),
                                                            is_opaque));
  canvas_ = owned_canvas_.get();
#if defined(OS_WIN) || defined(OS_MACOSX)
  // skia::PlatformCanvas instances are initialized to 0 by Cairo on Linux, but
  // uninitialized on Win and Mac.
  if (!is_opaque)
    owned_canvas_->clear(SkColorSetARGB(0, 0, 0, 0));
#endif

  SkScalar scale_scalar = SkFloatToScalar(image_scale);
  canvas_->scale(scale_scalar, scale_scalar);
}

Canvas::Canvas(const ImageSkiaRep& image_rep, bool is_opaque)
    : image_scale_(image_rep.scale()),
      owned_canvas_(skia::AdoptRef(
          skia::CreatePlatformCanvas(image_rep.pixel_width(),
                                     image_rep.pixel_height(),
                                     is_opaque))),
      canvas_(owned_canvas_.get()) {
  SkScalar scale_scalar = SkFloatToScalar(image_scale_);
  canvas_->scale(scale_scalar, scale_scalar);
  DrawImageInt(ImageSkia(image_rep), 0, 0);
}

Canvas::Canvas()
    : image_scale_(1.0),
      owned_canvas_(skia::AdoptRef(skia::CreatePlatformCanvas(0, 0, false))),
      canvas_(owned_canvas_.get()) {
}

Canvas::~Canvas() {
}

// static
Canvas* Canvas::CreateCanvasWithoutScaling(SkCanvas* canvas,
                                           float image_scale) {
  return new Canvas(canvas, image_scale);
}

void Canvas::RecreateBackingCanvas(const Size& size,
                                   float image_scale,
                                   bool is_opaque) {
  image_scale_ = image_scale;
  Size pixel_size = ToFlooredSize(ScaleSize(size, image_scale));
  owned_canvas_ = skia::AdoptRef(skia::CreatePlatformCanvas(pixel_size.width(),
                                                            pixel_size.height(),
                                                            is_opaque));
  canvas_ = owned_canvas_.get();
  SkScalar scale_scalar = SkFloatToScalar(image_scale);
  canvas_->scale(scale_scalar, scale_scalar);
}

// static
void Canvas::SizeStringInt(const base::string16& text,
                           const FontList& font_list,
                           int* width,
                           int* height,
                           int line_height,
                           int flags) {
  /*float fractional_width = *width;
  float factional_height = *height;
  SizeStringFloat(text, font_list, &fractional_width,
                  &factional_height, line_height, flags);
  *width = std::ceil(fractional_width);
  *height = std::ceil(factional_height);*/
                             SizeStringInt(text, font_list.GetPrimaryFont(), width, height, line_height, flags);
}

// static
// void Canvas::SizeStringInt(const base::string16& text,
//                            const Font& font,
//                            int* width,
//                            int* height,
//                            int line_height,
//                            int flags) {
//   SizeStringInt(text, FontList(font), width, height, line_height, flags);
// }

// static
int Canvas::GetStringWidth(const base::string16& text,
                           const FontList& font_list) {
  int width = 0, height = 0;
  SizeStringInt(text, font_list, &width, &height, 0, NO_ELLIPSIS);
  return width;
}

// static
float Canvas::GetStringWidthF(const base::string16& text,
                              const FontList& font_list) {
  int width = 0, height = 0;
  SizeStringInt(text, font_list, &width, &height, 0, NO_ELLIPSIS);
  //SizeStringFloat(text, font_list, &width, &height, 0, NO_ELLIPSIS);
  return (float)width;
}

// static
int Canvas::GetStringWidth(const base::string16& text, const Font& font) {
  int width = 0, height = 0;
  SizeStringInt(text, FontList(font), &width, &height, 0, NO_ELLIPSIS);
  return width;
}

// static
int Canvas::DefaultCanvasTextAlignment() {
  return 0;
}

void Canvas::DrawStringRectWithHalo(const base::string16& text,
  const FontList& font_list,
  SkColor text_color,
  SkColor halo_color,
  const Rect& display_rect,
  int flags) {
    DrawStringWithHalo(text, font_list.GetPrimaryFont(), text_color, halo_color,
      display_rect.x(), display_rect.y(), display_rect.width(), display_rect.height(), flags);
}

// void Canvas::DrawStringWithHalo(const base::string16& text,
//                                 const Font& font,
//                                 SkColor text_color,
//                                 SkColor halo_color_in,
//                                 int x,
//                                 int y,
//                                 int w,
//                                 int h,
//                                 int flags) {
//   DrawStringRectWithHalo(text, FontList(font), text_color, halo_color_in,
//                          Rect(x, y, w, h), flags);
// }

ImageSkiaRep Canvas::ExtractImageRep() const {
  const SkBitmap& device_bitmap = canvas_->getDevice()->accessBitmap(false);

  // Make a bitmap to return, and a canvas to draw into it. We don't just want
  // to call extractSubset or the copy constructor, since we want an actual copy
  // of the bitmap.
  SkBitmap result;
  device_bitmap.copyTo(&result, SkBitmap::kARGB_8888_Config);

  return ImageSkiaRep(result, image_scale_);
}

void Canvas::DrawDashedRect(const Rect& rect, SkColor color) {
  // Create a 2D bitmap containing alternating on/off pixels - we do this
  // so that you never get two pixels of the same color around the edges
  // of the focus rect (this may mean that opposing edges of the rect may
  // have a dot pattern out of phase to each other).
  static SkColor last_color;
  static SkBitmap* dots = NULL;
  if (!dots || last_color != color) {
    int col_pixels = 32;
    int row_pixels = 32;

    delete dots;
    last_color = color;
    dots = new SkBitmap;
    dots->setConfig(SkBitmap::kARGB_8888_Config, col_pixels, row_pixels);
    dots->allocPixels();
    dots->eraseARGB(0, 0, 0, 0);

    uint32_t* dot = dots->getAddr32(0, 0);
    for (int i = 0; i < row_pixels; i++) {
      for (int u = 0; u < col_pixels; u++) {
        if ((u % 2 + i % 2) % 2 != 0) {
          dot[i * row_pixels + u] = color;
        }
      }
    }
  }

  // Make a shader for the bitmap with an origin of the box we'll draw. This
  // shader is refcounted and will have an initial refcount of 1.
  skia::RefPtr<SkShader> shader = skia::AdoptRef(
      SkShader::CreateBitmapShader(
          *dots, SkShader::kRepeat_TileMode, SkShader::kRepeat_TileMode));
  // Assign the shader to the paint & release our reference. The paint will
  // now own the shader and the shader will be destroyed when the paint goes
  // out of scope.
  SkPaint paint;
  paint.setShader(shader.get());

  DrawRect(Rect(rect.x(), rect.y(), rect.width(), 1), paint);
  DrawRect(Rect(rect.x(), rect.y() + rect.height() - 1, rect.width(), 1),
           paint);
  DrawRect(Rect(rect.x(), rect.y(), 1, rect.height()), paint);
  DrawRect(Rect(rect.x() + rect.width() - 1, rect.y(), 1, rect.height()),
           paint);
}

void Canvas::Save() {
  canvas_->save();
}

void Canvas::SaveLayerAlpha(uint8 alpha) {
  canvas_->saveLayerAlpha(NULL, alpha);
}


void Canvas::SaveLayerAlpha(uint8 alpha, const Rect& layer_bounds) {
  SkRect bounds(RectToSkRect(layer_bounds));
  canvas_->saveLayerAlpha(&bounds, alpha);
}

void Canvas::Restore() {
  canvas_->restore();
}

bool Canvas::ClipRect(const Rect& rect) {
  return canvas_->clipRect(RectToSkRect(rect));
}

bool Canvas::ClipPath(const SkPath& path) {
  return canvas_->clipPath(path);
}

bool Canvas::GetClipBounds(Rect* bounds) {
  SkRect out;
  bool has_non_empty_clip = canvas_->getClipBounds(&out);
  bounds->SetRect(out.left(), out.top(), out.width(), out.height());
  return has_non_empty_clip;
}

void Canvas::Translate(const Vector2d& offset) {
  canvas_->translate(SkIntToScalar(offset.x()), SkIntToScalar(offset.y()));
}

void Canvas::Scale(int x_scale, int y_scale) {
  canvas_->scale(SkIntToScalar(x_scale), SkIntToScalar(y_scale));
}

void Canvas::DrawColor(SkColor color) {
  DrawColor(color, SkXfermode::kSrcOver_Mode);
}

void Canvas::DrawColor(SkColor color, SkXfermode::Mode mode) {
  canvas_->drawColor(color, mode);
}

void Canvas::FillRect(const Rect& rect, SkColor color) {
  FillRect(rect, color, SkXfermode::kSrcOver_Mode);
}

void Canvas::FillRect(const Rect& rect,
                      SkColor color,
                      SkXfermode::Mode mode) {
  SkPaint paint;
  paint.setColor(color);
  paint.setStyle(SkPaint::kFill_Style);
  paint.setXfermodeMode(mode);
  DrawRect(rect, paint);
}

void Canvas::DrawRect(const Rect& rect, SkColor color) {
  DrawRect(rect, color, SkXfermode::kSrcOver_Mode);
}

void Canvas::DrawRect(const Rect& rect,
                      SkColor color,
                      SkXfermode::Mode mode) {
  SkPaint paint;
  paint.setColor(color);
  paint.setStyle(SkPaint::kStroke_Style);
  // Set a stroke width of 0, which will put us down the stroke rect path.  If
  // we set a stroke width of 1, for example, this will internally create a
  // path and fill it, which causes problems near the edge of the canvas.
  paint.setStrokeWidth(SkIntToScalar(0));
  paint.setXfermodeMode(mode);

  DrawRect(rect, paint);
}

void Canvas::DrawRect(const Rect& rect, const SkPaint& paint) {
  canvas_->drawIRect(RectToSkIRect(rect), paint);
}

void Canvas::DrawPoint(const Point& p1, const SkPaint& paint) {
  canvas_->drawPoint(SkIntToScalar(p1.x()), SkIntToScalar(p1.y()), paint);
}

void Canvas::DrawLine(const Point& p1, const Point& p2, SkColor color) {
  SkPaint paint;
  paint.setColor(color);
  paint.setStrokeWidth(SkIntToScalar(1));
  DrawLine(p1, p2, paint);
}

void Canvas::DrawLine(const Point& p1, const Point& p2, const SkPaint& paint) {
  canvas_->drawLine(SkIntToScalar(p1.x()), SkIntToScalar(p1.y()),
                    SkIntToScalar(p2.x()), SkIntToScalar(p2.y()), paint);
}

void Canvas::DrawCircle(const Point& center_point,
                        int radius,
                        const SkPaint& paint) {
  canvas_->drawCircle(SkIntToScalar(center_point.x()),
      SkIntToScalar(center_point.y()), SkIntToScalar(radius), paint);
}

void Canvas::DrawRoundRect(const Rect& rect,
                           int radius,
                           const SkPaint& paint) {
  canvas_->drawRoundRect(RectToSkRect(rect), SkIntToScalar(radius),
                         SkIntToScalar(radius), paint);
}

void Canvas::DrawPath(const SkPath& path, const SkPaint& paint) {
  canvas_->drawPath(path, paint);
}

void Canvas::DrawFocusRect(const Rect& rect) {
  DrawDashedRect(rect, SK_ColorGRAY);
}

void Canvas::DrawImageInt(const ImageSkia& image, int x, int y) {
  SkPaint paint;
  DrawImageInt(image, x, y, paint);
}

void Canvas::DrawImageInt(const ImageSkia& image, int x, int y, uint8 a) {
  SkPaint paint;
  paint.setAlpha(a);
  DrawImageInt(image, x, y, paint);
}

void Canvas::DrawImageInt(const ImageSkia& image,
                          int x,
                          int y,
                          const SkPaint& paint) {
  const ImageSkiaRep& image_rep = GetImageRepToPaint(image);
  if (image_rep.is_null())
    return;
  const SkBitmap& bitmap = image_rep.sk_bitmap();
  float bitmap_scale = image_rep.scale();

  canvas_->save();
  canvas_->scale(SkFloatToScalar(1.0f / bitmap_scale),
                 SkFloatToScalar(1.0f / bitmap_scale));
  canvas_->drawBitmap(bitmap,
                      SkFloatToScalar(x * bitmap_scale),
                      SkFloatToScalar(y * bitmap_scale),
                      &paint);
  canvas_->restore();
}

void Canvas::DrawImageInt(const ImageSkia& image,
                          int src_x,
                          int src_y,
                          int src_w,
                          int src_h,
                          int dest_x,
                          int dest_y,
                          int dest_w,
                          int dest_h,
                          bool filter) {
  SkPaint p;
  DrawImageInt(image, src_x, src_y, src_w, src_h, dest_x, dest_y,
               dest_w, dest_h, filter, p);
}

void Canvas::DrawImageInt(const ImageSkia& image,
                          int src_x,
                          int src_y,
                          int src_w,
                          int src_h,
                          int dest_x,
                          int dest_y,
                          int dest_w,
                          int dest_h,
                          bool filter,
                          const SkPaint& paint) {
  DLOG_ASSERT(src_x + src_w < std::numeric_limits<int16_t>::max() &&
              src_y + src_h < std::numeric_limits<int16_t>::max());
  if (src_w <= 0 || src_h <= 0) {
    NOTREACHED() << "Attempting to draw bitmap from an empty rect!";
    return;
  }

  if (!IntersectsClipRectInt(dest_x, dest_y, dest_w, dest_h))
    return;

  float user_scale_x = static_cast<float>(dest_w) / src_w;
  float user_scale_y = static_cast<float>(dest_h) / src_h;

  const ImageSkiaRep& image_rep = GetImageRepToPaint(image,
      user_scale_x, user_scale_y);
  if (image_rep.is_null())
    return;

  SkRect dest_rect = { SkIntToScalar(dest_x),
                       SkIntToScalar(dest_y),
                       SkIntToScalar(dest_x + dest_w),
                       SkIntToScalar(dest_y + dest_h) };

  if (src_w == dest_w && src_h == dest_h &&
      user_scale_x == 1.0f && user_scale_y == 1.0f &&
      image_rep.scale() == 1.0f) {
    // Workaround for apparent bug in Skia that causes image to occasionally
    // shift.
    SkIRect src_rect = { src_x, src_y, src_x + src_w, src_y + src_h };
    const SkBitmap& bitmap = image_rep.sk_bitmap();
    canvas_->drawBitmapRect(bitmap, &src_rect, dest_rect, &paint);
    return;
  }

  // Make a bitmap shader that contains the bitmap we want to draw. This is
  // basically what SkCanvas.drawBitmap does internally, but it gives us
  // more control over quality and will use the mipmap in the source image if
  // it has one, whereas drawBitmap won't.
  SkMatrix shader_scale;
  shader_scale.setScale(SkFloatToScalar(user_scale_x),
                        SkFloatToScalar(user_scale_y));
  shader_scale.preTranslate(SkIntToScalar(-src_x), SkIntToScalar(-src_y));
  shader_scale.postTranslate(SkIntToScalar(dest_x), SkIntToScalar(dest_y));

  skia::RefPtr<SkShader> shader = CreateImageRepShader(
      image_rep,
      SkShader::kRepeat_TileMode,
      shader_scale);

  // Set up our paint to use the shader & release our reference (now just owned
  // by the paint).
  SkPaint p(paint);
  p.setFilterBitmap(filter);
  p.setShader(shader.get());

  // The rect will be filled by the bitmap.
  canvas_->drawRect(dest_rect, p);
}

void Canvas::DrawImageInPath(const ImageSkia& image,
                             int x,
                             int y,
                             const SkPath& path,
                             const SkPaint& paint) {
  const ImageSkiaRep& image_rep = GetImageRepToPaint(image);
  if (image_rep.is_null())
    return;

  SkMatrix matrix;
  matrix.setTranslate(SkIntToScalar(x), SkIntToScalar(y));
  skia::RefPtr<SkShader> shader = CreateImageRepShader(
      image_rep,
      SkShader::kRepeat_TileMode,
      matrix);

  SkPaint p(paint);
  p.setShader(shader.get());
  canvas_->drawPath(path, p);
}

void Canvas::DrawImageByStretchLine(const gfx::ImageSkia& image,
  int dst_x, int dst_y, int dst_w, int dst_h, int s, int t) {
    // 查找拉伸点的方法为：从图片第一行开始，从左至右找到符合拉伸线颜色的位置=
    // 然后再纵向查找所有的点均为拉伸线的颜色，满足则是，不满足则继续查找=
    int x1 = 0;   // 横向第一个拉伸点x值=
    int x2 = 0;   // 横向第二个拉伸点x值=
    int y1 = 0;   // 纵向第一个拉伸点y值=
    int y2 = 0;   // 横向第二个拉伸点y值=

    if(image.isNull())
      return;

    if(t < 1 || s >= t || s < 0)
      return;

    int eve_width = image.width() / t;
    int startx =  s * eve_width;
    int endx = startx + eve_width;
    x1 = x2 = startx;

    // 横扫第一行=
    const SkBitmap* source = image.bitmap();
    SkAutoLockPixels locker(*source);
    for(int x = startx; x < endx; x++) {
      if(source->getColor(x, 0) == kStretchLineColor) {
        int y = 1;
        for(; y < image.height(); y++)
          if(source->getColor(x, y) != kStretchLineColor)
            break;

        if(y == image.height()) {
          if(x1 == startx) {
            x1 = x;
          } else {
            x2 = x;
            break;
          }
        }
      }
    }

    // 横扫第一列=
    for(int y = 0; y < image.height(); y++) {
      if(source->getColor(0, y) == kStretchLineColor) {
        int x = startx + 1;
        for(; x < endx; x++)
          if(source->getColor(x, y) != kStretchLineColor)
            break;

        if(x == endx) {
          if(y1 == 0) {
            y1 = y;
          } else {
            y2 = y;
            break;
          }
        }
      }
    }

    DrawImageByStretchPoint(image, dst_x, dst_y, dst_w, dst_h, x1, x2, y1, y2, s, t);
}

void Canvas::DrawImageByStretchPoint(const gfx::ImageSkia& image,
  int dst_x, int dst_y, int dst_w, int dst_h,
  int x1, int x2, int y1, int y2, int s, int t) {
    //DCHECK(x1 < x2 && y1 < y2);

    int eve_width = image.width() / t;
    int startx =  s * eve_width;
    int endx = startx + eve_width;
    
    if(x1 == startx || x2 == startx || y1 == 0 || y2 == 0)
      DrawImageInt(image, startx, 0, eve_width, image.height(), dst_x, dst_y, dst_w, dst_h, false);
    else {
      x1 -= startx;
      x2 -= startx;
      int tmp_dst_center_w = dst_w - (x1 + eve_width - (x2+1));
      int tmp_dst_right_x = dst_w - (eve_width - (x2+1));
      if(tmp_dst_center_w < 0) {
        // 目标太窄，不画中间，重新调整x2点=
        tmp_dst_center_w = 0;
        x2 = eve_width - (dst_w - x1) - 1;
        tmp_dst_right_x = dst_w - (eve_width - (x2+1));
      }
      // top left
      DrawImageInt(image, startx, 0, x1, y1, dst_x, dst_y, x1, y1, false);
      // top right
      DrawImageInt(image, startx + x2 + 1, 0, eve_width - (x2+1), y1,
        dst_x + tmp_dst_right_x, dst_y, eve_width - (x2+1), y1, false);
      // top center
      DrawImageInt(image, startx + x1 + 1, 0, x2 - (x1+1), y1,
        dst_x + x1, dst_y, tmp_dst_center_w, y1, false);

      // center left
      int tmp_src_h = y2 - (y1 + 1);
      int tmp_dst_h = dst_h - (y1 + image.height() - y2 - 1);
      if(tmp_dst_h > 0) {
        DrawImageInt(image, startx, y1 + 1, x1, tmp_src_h,
          dst_x, dst_y + y1, x1, tmp_dst_h, false);
        // center right
        DrawImageInt(image, startx + x2 + 1, y1 + 1, eve_width - (x2+1), tmp_src_h,
          dst_x + tmp_dst_right_x, dst_y + y1, eve_width - (x2+1), tmp_dst_h, false);
        // center center
        DrawImageInt(image, startx + x1 + 1, y1 + 1, x2 - (x1+1), tmp_src_h,
          dst_x + x1, dst_y + y1, tmp_dst_center_w, tmp_dst_h, false);
      }

      // bottom left
      tmp_src_h = image.height() - y2 - 1;
      if(tmp_src_h > dst_h - y1) {
        // 目标太矮，不画中间，重新调整y2点=
        tmp_src_h = std::min(tmp_src_h, std::max(dst_h - y1, 0));
        y2 = image.height() - tmp_src_h - 1;
      }
      tmp_dst_h = tmp_src_h;
      int tmp_dst_y = dst_h - tmp_dst_h;
      DrawImageInt(image, startx, y2 + 1, x1, tmp_src_h,
        dst_x, tmp_dst_y, x1, tmp_dst_h, false);
      // bottom right
      DrawImageInt(image, startx + x2 + 1, y2 + 1, eve_width - (x2+1), tmp_src_h,
        dst_x + tmp_dst_right_x, tmp_dst_y, eve_width - (x2+1), tmp_dst_h, false);
      // bottom center
      DrawImageInt(image, startx + x1 + 1, y2 + 1, x2 - (x1+1), tmp_src_h,
        dst_x + x1, tmp_dst_y, tmp_dst_center_w, tmp_dst_h, false);
    }
}

void Canvas::DrawStringRect(const base::string16& text,
                            const FontList& font_list,
                            SkColor color,
                            const Rect& display_rect) {
  DrawStringRectWithFlags(text, font_list, color, display_rect,
                          DefaultCanvasTextAlignment());
}

void Canvas::DrawStringRectWithFlags(const base::string16& text,
                                     const FontList& font_list,
                                     SkColor color,
                                     const Rect& display_rect,
                                     int flags) {
//   DrawStringRectWithShadows(text, font_list, color, display_rect, 0, flags,
//                             ShadowValues());
                                       DrawStringRectWithFlags(text, font_list.GetPrimaryFont(), color, display_rect, flags);
}

void Canvas::DrawStringInt(const base::string16& text,
                           const Font& font,
                           SkColor color,
                           int x,
                           int y,
                           int w,
                           int h) {
  DrawStringInt(text, font, color, x, y, w, h, DefaultCanvasTextAlignment());
}

void Canvas::DrawStringInt(const base::string16& text,
                           const Font& font,
                           SkColor color,
                           const Rect& display_rect) {
  DrawStringInt(text, font, color, display_rect.x(), display_rect.y(),
                display_rect.width(), display_rect.height());
}

void Canvas::DrawStringInt(const base::string16& text,
                           const Font& font,
                           SkColor color,
                           int x,
                           int y,
                           int w,
                           int h,
                           int flags) {
  DrawStringWithShadows(text, font, color, Rect(x, y, w, h), 0, flags,
                        ShadowValues());
}

void Canvas::DrawStringRectWithShadows(const base::string16& text,
  const FontList& font_list,
  SkColor color,
  const Rect& text_bounds,
  int line_height,
  int flags,
  const ShadowValues& shadows){
  DrawStringWithShadows(text, font_list.GetPrimaryFont(), color, text_bounds,
                            line_height, flags, shadows);
}

void Canvas::TileImageInt(const ImageSkia& image,
                          int x,
                          int y,
                          int w,
                          int h) {
  TileImageInt(image, 0, 0, x, y, w, h);
}

void Canvas::TileImageInt(const ImageSkia& image,
                          int src_x,
                          int src_y,
                          int dest_x,
                          int dest_y,
                          int w,
                          int h) {
  TileImageInt(image, src_x, src_y, 1.0f, 1.0f, dest_x, dest_y, w, h);
}

void Canvas::TileImageInt(const ImageSkia& image,
                          int src_x,
                          int src_y,
                          float tile_scale_x,
                          float tile_scale_y,
                          int dest_x,
                          int dest_y,
                          int w,
                          int h) {
  if (!IntersectsClipRectInt(dest_x, dest_y, w, h))
    return;

  const ImageSkiaRep& image_rep = GetImageRepToPaint(
      image, tile_scale_x, tile_scale_y);
  if (image_rep.is_null())
    return;

  SkMatrix shader_scale;
  shader_scale.setScale(SkFloatToScalar(tile_scale_x),
                        SkFloatToScalar(tile_scale_y));
  shader_scale.preTranslate(SkIntToScalar(-src_x), SkIntToScalar(-src_y));
  shader_scale.postTranslate(SkIntToScalar(dest_x), SkIntToScalar(dest_y));

  skia::RefPtr<SkShader> shader = CreateImageRepShader(
      image_rep,
      SkShader::kRepeat_TileMode,
      shader_scale);

  SkPaint paint;
  paint.setShader(shader.get());
  paint.setXfermodeMode(SkXfermode::kSrcOver_Mode);

  SkRect dest_rect = { SkIntToScalar(dest_x),
                       SkIntToScalar(dest_y),
                       SkIntToScalar(dest_x + w),
                       SkIntToScalar(dest_y + h) };
  canvas_->drawRect(dest_rect, paint);
}

NativeDrawingContext Canvas::BeginPlatformPaint() {
  return skia::BeginPlatformPaint(canvas_);
}

void Canvas::EndPlatformPaint() {
  skia::EndPlatformPaint(canvas_);
}

void Canvas::Transform(const gfx::Transform& transform) {
  canvas_->concat(transform.matrix());
}

Canvas::Canvas(SkCanvas* canvas, float image_scale)
    : image_scale_(image_scale),
      owned_canvas_(),
      canvas_(canvas) {
  DCHECK(canvas);
}

bool Canvas::IntersectsClipRectInt(int x, int y, int w, int h) {
  SkRect clip;
  return canvas_->getClipBounds(&clip) &&
      clip.intersect(SkIntToScalar(x), SkIntToScalar(y), SkIntToScalar(x + w),
                     SkIntToScalar(y + h));
}

bool Canvas::IntersectsClipRect(const Rect& rect) {
  return IntersectsClipRectInt(rect.x(), rect.y(),
                               rect.width(), rect.height());
}

const ImageSkiaRep& Canvas::GetImageRepToPaint(const ImageSkia& image) const {
  return GetImageRepToPaint(image, 1.0f, 1.0f);
}

const ImageSkiaRep& Canvas::GetImageRepToPaint(
    const ImageSkia& image,
    float user_additional_scale_x,
    float user_additional_scale_y) const {
  const ImageSkiaRep& image_rep = image.GetRepresentation(image_scale_);

  if (!image_rep.is_null()) {
    SkMatrix m = canvas_->getTotalMatrix();
    float scale_x = SkScalarToFloat(SkScalarAbs(m.getScaleX())) *
        user_additional_scale_x;
    float scale_y = SkScalarToFloat(SkScalarAbs(m.getScaleY())) *
        user_additional_scale_y;

    float bitmap_scale = image_rep.scale();
    if (scale_x < bitmap_scale || scale_y < bitmap_scale)
      const_cast<SkBitmap&>(image_rep.sk_bitmap()).buildMipMap();
  }

  return image_rep;
}

}  // namespace gfx
