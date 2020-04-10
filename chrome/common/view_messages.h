// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// IPC messages for page rendering.
// Multiply-included message file, hence no include guard.
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
#include "ui/gfx/size.h"
#include "ui/shell_dialogs/selected_file_info.h"

#undef IPC_MESSAGE_EXPORT
#define IPC_MESSAGE_EXPORT CONTENT_EXPORT

#define IPC_MESSAGE_START ViewMsgStart

IPC_STRUCT_BEGIN(ViewHostMsg_CreateWindow_Params)
  // Routing ID of the view initiating the open.
  IPC_STRUCT_MEMBER(int, opener_id)
IPC_STRUCT_END()

IPC_STRUCT_BEGIN(ViewMsg_New_Params)
  // The ID of the view to be created.
  IPC_STRUCT_MEMBER(int32, view_id)

  // The ID of the main frame hosted in the view.
  IPC_STRUCT_MEMBER(int32, main_frame_routing_id)

  // The ID of the rendering surface.
  IPC_STRUCT_MEMBER(int32, surface_id)

  // The session storage namespace ID this view should use.
  IPC_STRUCT_MEMBER(int64, session_storage_namespace_id)

   // The name of the frame associated with this view (or empty if none).
  IPC_STRUCT_MEMBER(string16, frame_name)

  // The route ID of the opener RenderView if we need to set one
  // (MSG_ROUTING_NONE otherwise).
  IPC_STRUCT_MEMBER(int, opener_route_id)

  // Whether the RenderView should initially be swapped out.
  IPC_STRUCT_MEMBER(bool, swapped_out)

  // Whether the RenderView should initially be hidden.
  IPC_STRUCT_MEMBER(bool, hidden)

  // The initial page ID to use for this view, which must be larger than any
  // existing navigation that might be loaded in the view.  Page IDs are unique
  // to a view and are only updated by the renderer after this initial value.
  IPC_STRUCT_MEMBER(int32, next_page_id)
  IPC_STRUCT_MEMBER(HWND, hwnd)
  IPC_STRUCT_MEMBER(std::string, url)

IPC_STRUCT_END()

IPC_STRUCT_BEGIN(ViewMsg_Resize_Params)
  IPC_STRUCT_MEMBER(int, w)
  IPC_STRUCT_MEMBER(int, h)
  IPC_STRUCT_MEMBER(bool, is_fullscreen)
IPC_STRUCT_END()


IPC_STRUCT_BEGIN(ViewMsg_Navigate_Params)
IPC_STRUCT_MEMBER(std::string, url)
IPC_STRUCT_MEMBER(std::string, reffer)
IPC_STRUCT_END()

// Tells the renderer to create a new view.
// This message is slightly different, the view it takes (via
// ViewMsg_New_Params) is the view to create, the message itself is sent as a
// non-view control message.
IPC_MESSAGE_CONTROL1(ViewMsg_New,
  ViewMsg_New_Params)

IPC_MESSAGE_ROUTED1(ViewMsg_CreateNewWindow,
  int)

 IPC_MESSAGE_ROUTED2(ViewMsg_URLCHANGE,
  int,
  string16)

 IPC_MESSAGE_ROUTED2(ViewMsg_TITLECHANGE,
  int,
  string16)

 IPC_MESSAGE_ROUTED1(ViewMsg_Resize,
  ViewMsg_Resize_Params /* params */)

 IPC_MESSAGE_ROUTED1(ViewMsg_Navigate,
  ViewMsg_Navigate_Params /* params */)

 IPC_MESSAGE_ROUTED0(ViewHostMsg_Focus)

 IPC_MESSAGE_ROUTED1(InputMsg_SetFocus,
 bool /* enable */)

 // Activate/deactivate the RenderView (i.e., set its controls' tint
 // accordingly, etc.).
 IPC_MESSAGE_ROUTED1(ViewMsg_SetActive,
 bool /* active */)

 IPC_MESSAGE_ROUTED2(ViewHostMsg_StatusChange,
   int/**/,
   int/* state*/)