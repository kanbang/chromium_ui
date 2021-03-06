
/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifdef SK_BUILD_FOR_WIN32
#pragma warning(push)
#pragma warning(disable : 4530)
#endif

#include <poppler-document.h>
#include <poppler-image.h>
#include <poppler-page.h>
#include <poppler-page-renderer.h>

#include "SkPDFRasterizer.h"
#include "SkColorPriv.h"

bool SkPopplerRasterizePDF(SkStream* pdf, SkBitmap* output) {
  size_t size = pdf->getLength();
  SkAutoFree buffer(sk_malloc_throw(size));
  pdf->read(buffer.get(), size);

  SkAutoTDelete<poppler::document> doc(
      poppler::document::load_from_raw_data((const char*)buffer.get(), size));
  if (!doc.get() || doc->is_locked()) {
    return false;
  }

  SkAutoTDelete<poppler::page> page(doc->create_page(0));
  poppler::page_renderer renderer;
  poppler::image image = renderer.render_page(page.get());

  if (!image.is_valid() || image.format() != poppler::image::format_argb32) {
    return false;
  }

  size_t width = image.width(), height = image.height();
  size_t rowSize = image.bytes_per_row();
  char *imgData = image.data();

  SkBitmap bitmap;
  bitmap.setConfig(SkBitmap::kARGB_8888_Config, width, height);
  if (!bitmap.allocPixels()) {
    return false;
  }
  bitmap.eraseColor(SK_ColorWHITE);
  SkPMColor* bitmapPixels = (SkPMColor*)bitmap.getPixels();

  // do pixel-by-pixel copy to deal with RGBA ordering conversions
  for (size_t y = 0; y < height; y++) {
    char *rowData = imgData;
    for (size_t x = 0; x < width; x++) {
      uint8_t a = rowData[3];
      uint8_t r = rowData[2];
      uint8_t g = rowData[1];
      uint8_t b = rowData[0];

      *bitmapPixels = SkPreMultiplyARGB(a, r, g, b);

      bitmapPixels++;
      rowData += 4;
    }
    imgData += rowSize;
  }

  output->swap(bitmap);

  return true;
}
