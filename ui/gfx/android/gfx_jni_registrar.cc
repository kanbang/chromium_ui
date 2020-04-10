// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/gfx/android/gfx_jni_registrar.h"

#include "base/android/jni_android.h"
#include "base/android/jni_registrar.h"
#include "ui/gfx/android/device_display_info.h"
#include "ui/gfx/android/java_bitmap.h"

namespace gfx {
namespace android {

static base::android::RegistrationMethod kGfxRegisteredMethods[] = {
  { "DeviceDisplayInfo", gfx::DeviceDisplayInfo::RegisterDeviceDisplayInfo },
  { "JavaBitmap", gfx::JavaBitmap::RegisterJavaBitmap },
};

bool RegisterJni(JNIEnv* env) {
  return RegisterNativeMethods(env, kGfxRegisteredMethods,
                               arraysize(kGfxRegisteredMethods));
}

}  // namespace android
}  // namespace gfx
