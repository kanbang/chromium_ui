// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CSS_CSS_EXPORT_H_
#define CSS_CSS_EXPORT_H_

// Defines CSS_EXPORT so that functionality implemented by the UI module can be
// exported to consumers.

#if defined(COMPONENT_BUILD)
#if defined(WIN32)

#if defined(CSS_IMPLEMENTATION)
#define CSS_EXPORT __declspec(dllexport)
#else
#define CSS_EXPORT __declspec(dllimport)
#endif  // defined(UI_IMPLEMENTATION)

#else  // defined(WIN32)
#if defined(CSS_IMPLEMENTATION)
#define CSS_EXPORT __attribute__((visibility("default")))
#else
#define CSS_EXPORT
#endif
#endif

#else  // defined(COMPONENT_BUILD)
#define CSS_EXPORT
#endif

#endif  // CSS_CSS_EXPORT_H_
