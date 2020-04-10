// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// IPC messages for page rendering.
// Multiply-included message file, hence no include guard.
#include <windowsx.h>
#include "base/memory/shared_memory.h"
#include "base/process/process.h"
#include "base/strings/string16.h"
#include "chrome/chrome_export.h"
#include "chrome/browser/gurl.h"
#include "ipc/ipc_channel_handle.h"
#include "ipc/ipc_message_macros.h"
#include "ipc/ipc_platform_file.h"
#include "ui/base/ime/text_input_mode.h"
#include "ui/base/ime/text_input_type.h"
#include "ui/base/ui_base_types.h"
#include "ui/gfx/point.h"
#include "ui/gfx/range/range.h"
#include "ui/gfx/rect.h"
#include "ui/gfx/rect_f.h"
#include "ui/gfx/vector2d.h"
#include "ui/gfx/vector2d_f.h"
#include "ui/shell_dialogs/selected_file_info.h"

#undef IPC_MESSAGE_EXPORT
#define IPC_MESSAGE_EXPORT CONTENT_EXPORT

#define IPC_MESSAGE_START ChromeMsgStart

IPC_STRUCT_BEGIN(Browser_Ready_Params)
  IPC_STRUCT_MEMBER(HWND, hwnd)
  IPC_STRUCT_MEMBER(int, roateid)
  IPC_STRUCT_MEMBER(LPARAM, param)
IPC_STRUCT_END()

IPC_STRUCT_BEGIN(Browser_NewWindow_Params)
IPC_STRUCT_MEMBER(HANDLE, hEvent)
IPC_STRUCT_MEMBER(HANDLE, hNewEvent)
IPC_STRUCT_MEMBER(LPVOID, pStream)
IPC_STRUCT_MEMBER(DWORD, dwSize)
IPC_STRUCT_MEMBER(string16, url)
IPC_STRUCT_MEMBER(bool, popwindow)
IPC_STRUCT_MEMBER(int, browser_id)
IPC_STRUCT_END()
// Tells the renderer to create a new view.
// This message is slightly different, the view it takes (via
// ViewMsg_New_Params) is the view to create, the message itself is sent as a
// non-view control message.
IPC_MESSAGE_CONTROL1(Browser_Ready,
  Browser_Ready_Params)

IPC_MESSAGE_ROUTED1(Browser_NewWindowReady,
  Browser_NewWindow_Params)