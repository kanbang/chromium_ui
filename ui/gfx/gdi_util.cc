// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/gfx/gdi_util.h"

#include "base/logging.h"
#include "base/memory/scoped_ptr.h"
#include "third_party/skia/include/core/SkBitmap.h"
#include "base/win/scoped_gdi_object.h"
#include "skia/ext/bitmap_platform_device.h"
#include "skia/ext/skia_utils_win.h"
#include "third_party/skia/include/core/SkShader.h"
#include "ui/gfx/color_utils.h"
#include "ui/gfx/font.h"
#include "ui/gfx/rect.h"
#include "ui/gfx/shadow_value.h"
#include "ui/gfx/canvas.h"

#include <WinDef.h>

namespace gfx {

void CreateBitmapHeader(int width, int height, BITMAPINFOHEADER* hdr) {
  CreateBitmapHeaderWithColorDepth(width, height, 32, hdr);
}

void CreateBitmapHeaderWithColorDepth(int width, int height, int color_depth,
                                      BITMAPINFOHEADER* hdr) {
  // These values are shared with gfx::PlatformDevice
  hdr->biSize = sizeof(BITMAPINFOHEADER);
  hdr->biWidth = width;
  hdr->biHeight = -height;  // minus means top-down bitmap
  hdr->biPlanes = 1;
  hdr->biBitCount = color_depth;
  hdr->biCompression = BI_RGB;  // no compression
  hdr->biSizeImage = 0;
  hdr->biXPelsPerMeter = 1;
  hdr->biYPelsPerMeter = 1;
  hdr->biClrUsed = 0;
  hdr->biClrImportant = 0;
}

void CreateBitmapV4Header(int width, int height, BITMAPV4HEADER* hdr) {
  // Because bmp v4 header is just an extension, we just create a v3 header and
  // copy the bits over to the v4 header.
  BITMAPINFOHEADER header_v3;
  CreateBitmapHeader(width, height, &header_v3);
  memset(hdr, 0, sizeof(BITMAPV4HEADER));
  memcpy(hdr, &header_v3, sizeof(BITMAPINFOHEADER));

  // Correct the size of the header and fill in the mask values.
  hdr->bV4Size = sizeof(BITMAPV4HEADER);
  hdr->bV4RedMask   = 0x00ff0000;
  hdr->bV4GreenMask = 0x0000ff00;
  hdr->bV4BlueMask  = 0x000000ff;
  hdr->bV4AlphaMask = 0xff000000;
}

// Creates a monochrome bitmap header.
void CreateMonochromeBitmapHeader(int width,
                                  int height,
                                  BITMAPINFOHEADER* hdr) {
  hdr->biSize = sizeof(BITMAPINFOHEADER);
  hdr->biWidth = width;
  hdr->biHeight = -height;
  hdr->biPlanes = 1;
  hdr->biBitCount = 1;
  hdr->biCompression = BI_RGB;
  hdr->biSizeImage = 0;
  hdr->biXPelsPerMeter = 1;
  hdr->biYPelsPerMeter = 1;
  hdr->biClrUsed = 0;
  hdr->biClrImportant = 0;
}

void SubtractRectanglesFromRegion(HRGN hrgn,
                                  const std::vector<gfx::Rect>& cutouts) {
  if (cutouts.size()) {
    HRGN cutout = ::CreateRectRgn(0, 0, 0, 0);
    for (size_t i = 0; i < cutouts.size(); i++) {
      ::SetRectRgn(cutout,
                   cutouts[i].x(),
                   cutouts[i].y(),
                   cutouts[i].right(),
                   cutouts[i].bottom());
      ::CombineRgn(hrgn, hrgn, cutout, RGN_DIFF);
    }
    ::DeleteObject(cutout);
  }
}

HRGN ConvertPathToHRGN(const gfx::Path& path) {
#if defined(USE_AURA)
  int point_count = path.getPoints(NULL, 0);
  scoped_ptr<SkPoint[]> points(new SkPoint[point_count]);
  path.getPoints(points.get(), point_count);
  scoped_ptr<POINT[]> windows_points(new POINT[point_count]);
  for (int i = 0; i < point_count; ++i) {
    windows_points[i].x = SkScalarRound(points[i].fX);
    windows_points[i].y = SkScalarRound(points[i].fY);
  }

  return ::CreatePolygonRgn(windows_points.get(), point_count, ALTERNATE);
#elif defined(OS_WIN)
  return path.CreateNativeRegion();
#endif
}

HRGN GetHRGNFromImageSkia(const ImageSkia& Image, Size sz) {
 
  if(Image.isNull())
    return NULL;

  if(Image.width() < 20 || Image.height() < 20)
    return NULL;

  if(sz.width() < 20 || sz.height() < 20)
    return NULL;

  const SkBitmap* ct_dest = Image.bitmap();
  scoped_ptr<SkBitmap> dest;
  dest.reset(new SkBitmap);
  ct_dest->deepCopyTo(dest.get(), SkBitmap::kARGB_8888_Config);
  if(dest->config() != SkBitmap::kARGB_8888_Config)
    return NULL;

  const int rgn_width = 10;
  HRGN hRgnStart = NULL;
  hRgnStart = CreateRectRgn(0, 0, 0, 0);
  uint32_t transCr = *dest->getAddr32(0, 0);
  bool bfirst = false;
  int nOffsetX = 0;
  int nOffsetY = 0;
  // left - top
  int y;
  for ( y = 0; y < rgn_width; y++)
  {
    HRGN rgnTemp = NULL;
    int x = 0;
    while( x < rgn_width  && (*dest->getAddr32(x, y)) == transCr ) 
        x++;

    int iLeftx = x;
    int xBroder = rgn_width;
    if( iLeftx > rgn_width )
      continue;
    
    rgnTemp = CreateRectRgn(iLeftx + nOffsetX,   y + nOffsetY,   xBroder + nOffsetX,   y + 1 + nOffsetY); 
    CombineRgn(hRgnStart, hRgnStart, rgnTemp, RGN_OR); 
    ::DeleteObject(rgnTemp);
    rgnTemp = NULL;
  }

  // right - top
  for ( y = 0; y < rgn_width; y++)
  {
    nOffsetX = sz.width() - Image.width() ;
    HRGN rgnTemp = NULL;
    int x = Image.width() - 1;
    while( x >= Image.width() - rgn_width  && (*dest->getAddr32(x, y)) == transCr ) 
      x--;

    int iLeftx = Image.width() - rgn_width;
    int xBroder = x;
    if( iLeftx > xBroder)
      continue;

    rgnTemp = CreateRectRgn(iLeftx + nOffsetX,   y + nOffsetY,   xBroder + nOffsetX + 1,   y + 1 + nOffsetY); 
    CombineRgn(hRgnStart, hRgnStart, rgnTemp, RGN_OR); 
    ::DeleteObject(rgnTemp);
    rgnTemp = NULL;
  }

  // right - bottom
  for ( y = Image.height() - rgn_width; y < Image.height(); y++ )
  {
    nOffsetX = sz.width() - Image.width() ;
    nOffsetY = sz.height() - Image.height();
    HRGN rgnTemp = NULL;
    int x = Image.width() - 1;
    while( x >= Image.width() - rgn_width  && (*dest->getAddr32(x, y)) == transCr ) 
      x--;

    int iLeftx = Image.width() - rgn_width;
    int xBroder = x;
    if( iLeftx > xBroder)
      continue;

    rgnTemp = CreateRectRgn(iLeftx + nOffsetX,   y + nOffsetY,   xBroder + nOffsetX + 1,   y + 1 + nOffsetY); 
    CombineRgn(hRgnStart, hRgnStart, rgnTemp, RGN_OR); 
    ::DeleteObject(rgnTemp);
    rgnTemp = NULL;
  }

  // left - bottom
  for ( y = Image.height() - rgn_width; y < Image.height(); y++ )
  {
    nOffsetX = 0;
    nOffsetY = sz.height() - Image.height();

    HRGN rgnTemp = NULL;
    int x = 0;
    while( x < rgn_width  && (*dest->getAddr32(x, y)) == transCr ) 
      x++;

    int iLeftx = x;
    int xBroder = rgn_width;
    if( iLeftx > rgn_width )
      continue;

    rgnTemp = CreateRectRgn(iLeftx + nOffsetX,   y + nOffsetY,   xBroder + nOffsetX,   y + 1 + nOffsetY); 
    CombineRgn(hRgnStart, hRgnStart, rgnTemp, RGN_OR); 
    ::DeleteObject(rgnTemp);
    rgnTemp = NULL;
  }

  // 合并上3个矩形
  // left
  HRGN hLeft = CreateRectRgn(5, rgn_width, 10, sz.height() - rgn_width + 1);
  if( hLeft ) {
    CombineRgn(hRgnStart, hRgnStart, hLeft, RGN_OR);
    ::DeleteObject(hLeft);
  }

  HRGN hCenter = CreateRectRgn(rgn_width, 5, sz.width() - rgn_width + 1, sz.height() - 5);
  if( hCenter ) {
    CombineRgn(hRgnStart, hRgnStart, hCenter, RGN_OR);
    ::DeleteObject(hCenter);
  }

  HRGN hRight = CreateRectRgn(sz.width() - rgn_width, rgn_width, sz.width() - 5, sz.height() - rgn_width + 1);
  if( hRight ) {
    CombineRgn(hRgnStart, hRgnStart, hRight, RGN_OR);
    ::DeleteObject(hRight);
  }
  return hRgnStart;
}

double CalculatePageScale(HDC dc, int page_width, int page_height) {
  int dc_width = GetDeviceCaps(dc, HORZRES);
  int dc_height = GetDeviceCaps(dc, VERTRES);

  // If page fits DC - no scaling needed.
  if (dc_width >= page_width && dc_height >= page_height)
    return 1.0;

  double x_factor =
      static_cast<double>(dc_width) / static_cast<double>(page_width);
  double y_factor =
      static_cast<double>(dc_height) / static_cast<double>(page_height);
  return std::min(x_factor, y_factor);
}

// Apply scaling to the DC.
bool ScaleDC(HDC dc, double scale_factor) {
  SetGraphicsMode(dc, GM_ADVANCED);
  XFORM xform = {0};
  xform.eM11 = xform.eM22 = scale_factor;
  return !!ModifyWorldTransform(dc, &xform, MWT_LEFTMULTIPLY);
}

void StretchDIBits(HDC hdc, int dest_x, int dest_y, int dest_w, int dest_h,
                   int src_x, int src_y, int src_w, int src_h, void* pixels,
                   const BITMAPINFO* bitmap_info) {
  // When blitting a rectangle that touches the bottom, left corner of the
  // bitmap, StretchDIBits looks at it top-down!  For more details, see
  // http://wiki.allegro.cc/index.php?title=StretchDIBits.
  int rv;
  int bitmap_h = -bitmap_info->bmiHeader.biHeight;
  int bottom_up_src_y = bitmap_h - src_y - src_h;
  if (bottom_up_src_y == 0 && src_x == 0 && src_h != bitmap_h) {
    rv = ::StretchDIBits(hdc,
                         dest_x, dest_h + dest_y - 1, dest_w, -dest_h,
                         src_x, bitmap_h - src_y + 1, src_w, -src_h,
                         pixels, bitmap_info, DIB_RGB_COLORS, SRCCOPY);
  } else {
    rv = ::StretchDIBits(hdc,
                         dest_x, dest_y, dest_w, dest_h,
                         src_x, bottom_up_src_y, src_w, src_h,
                         pixels, bitmap_info, DIB_RGB_COLORS, SRCCOPY);
  }
  DCHECK(rv != GDI_ERROR);
}

HBITMAP CreateHBITMAPFromSkBitmap(const SkBitmap& sk_bitmap) {
  HBITMAP bitmap = NULL;
  if (!sk_bitmap.isNull() && !sk_bitmap.empty()) {
    HDC screen_dc = GetDC(NULL);
    if (screen_dc) {
      BITMAPINFOHEADER header;
      gfx::CreateBitmapHeader(sk_bitmap.width(), sk_bitmap.height(), &header);
      void* bits = NULL;
      bitmap = CreateDIBSection(screen_dc,
        reinterpret_cast<BITMAPINFO*>(&header),
        DIB_RGB_COLORS, &bits, NULL, 0);
      if (bitmap && bits) {
        SkAutoLockPixels lock(sk_bitmap);
        memcpy(bits, sk_bitmap.getPixels(), 
          sk_bitmap.height() * sk_bitmap.rowBytes());
      }
      ReleaseDC(NULL, screen_dc);
    }
  }
  return bitmap;
}

SkBitmap* CopyAlphaChannel(const ImageSkia& ImageDest, const ImageSkia& imageSrc) {
  const SkBitmap* dest = ImageDest.bitmap();
  if(dest->config() != SkBitmap::kARGB_8888_Config)
    return NULL;

  Canvas canvas(Size(ImageDest.width(), ImageDest.height()), 1.0, false);
  canvas.DrawImageByStretchPoint(imageSrc, 0, 0, ImageDest.width(), ImageDest.height(), 10,
    imageSrc.width() - 10, 10, imageSrc.height() - 10, 0, 1);
  const SkBitmap& bmpSrc = canvas.sk_canvas()->getTopDevice()->accessBitmap(false);
  if(bmpSrc.config() != SkBitmap::kARGB_8888_Config)
    return NULL;

  SkBitmap* newBmp = new SkBitmap;
  dest->deepCopyTo(newBmp, SkBitmap::kARGB_8888_Config);

  SkAutoLockPixels lock(*newBmp);
  // At this point the bitmap has black text on white.
  // The intensity of black tells us the alpha value of the text.
  for (int y = 0; y < newBmp->height(); y++) {
    for (int x = 0; x < newBmp->width(); x++) {
      // Gets the color directly. DrawText doesn't premultiply alpha so
      // using SkBitmap::getColor() won't work here.
      SkColor color = *newBmp->getAddr32(x, y);
      SkColor colorsrc = *bmpSrc.getAddr32(x, y);
      // Calculate the alpha using the luminance. Since this is black text
      // on a white background the luminosity must be inverted.
      BYTE alpha = SkColorGetA(colorsrc);//0xFF - color_utils::GetLuminanceForColor(colorsrc);
      BYTE text_color_r = SkColorGetR(color);
      BYTE text_color_g = SkColorGetG(color);
      BYTE text_color_b = SkColorGetB(color);
      *newBmp->getAddr32(x, y) = SkPreMultiplyColor(
        SkColorSetARGB(alpha, text_color_r, text_color_g, text_color_b));
    }
  }
  newBmp->setIsOpaque(false);
  return newBmp;
}

}  // namespace gfx
