// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/profile.h"

#include "base/bind.h"
#include "base/command_line.h"
#include "base/environment.h"
#include "base/file_util.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "base/threading/thread.h"

#include "chrome/common/content_switches.h"


#if defined(OS_WIN)
#include "base/base_paths_win.h"
#elif defined(OS_LINUX)
#include "base/nix/xdg_util.h"
#elif defined(OS_MACOSX)
#include "base/base_paths_mac.h"
#endif

Profile::Profile(bool off_the_record)
    : off_the_record_(off_the_record),
      ignore_certificate_errors_(false){
  InitWhileIOAllowed();
}

Profile::~Profile() {

}

void Profile::InitWhileIOAllowed() {
  
}

base::FilePath Profile::GetPath() const {
  return path_;
}

bool Profile::IsOffTheRecord() const {
  return off_the_record_;
}

