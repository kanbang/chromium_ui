// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_LAYOUT_LAYOUT_MANAGER_H_
#define UI_VIEWS_LAYOUT_LAYOUT_MANAGER_H_
#include "base/strings/string16.h"
#include "ui/views/views_export.h"


// 相对布局
#define DSUI_PROPERTY_RELATIVE_LAYOUT_CENTERHRIZONTAL				L"layout_centerHorizontal"
#define DSUI_PROPERTY_RELATIVE_LAYOUT_CENTERVERTICAL				L"layout_centerVertical"
#define DSUI_PROPERTY_RELATIVE_LAYOUT_CENTERINPARENT				L"layout_centerInparent"
#define DSUI_PROPERTY_RELATIVE_LAYOUT_ALIGNPARENTBOTTOM				L"layout_alignParentBottom"
#define DSUI_PROPERTY_RELATIVE_LAYOUT_ALIGNPARENTLEFT				L"layout_alignParentLeft"
#define DSUI_PROPERTY_RELATIVE_LAYOUT_ALIGNPARENTRIGHT				L"layout_alignParentRight"
#define DSUI_PROPERTY_RELATIVE_LAYOUT_ALIGNPARENTTOP				L"layout_alignParentTop"
#define DSUI_PROPERTY_RELATIVE_LAYOUT_ALIGNWITHPARENTIFMISSING		L"layout_alignWithParentIfMissing"

#define DSUI_PROPERTY_RELATIVE_LAYOUT_BELOW							L"layout_below"
#define DSUI_PROPERTY_RELATIVE_LAYOUT_ABOVE							L"layout_above"
#define DSUI_PROPERTY_RELATIVE_LAYOUT_TOLEFTOF						L"layout_toLeftOf"
#define DSUI_PROPERTY_RELATIVE_LAYOUT_TORIGHTOF						L"layout_toRightOf"
#define DSUI_PROPERTY_RELATIVE_LAYOUT_ALIGNTOP						L"layout_alignTop"
#define DSUI_PROPERTY_RELATIVE_LAYOUT_ALIGNLEFT						L"layout_alignLeft"
#define DSUI_PROPERTY_RELATIVE_LAYOUT_ALIGNBOTTOM					L"layout_alignBottom"
#define DSUI_PROPERTY_RELATIVE_LAYOUT_ALIGNRIGHT					L"layout_alignRight"

#define DSUI_PROPERTY_RELATIVE_LAYOUT_MARGINBOTTOM					L"layout_marginBottom"
#define DSUI_PROPERTY_RELATIVE_LAYOUT_MARGINLEFT					L"layout_marginLeft"
#define DSUI_PROPERTY_RELATIVE_LAYOUT_MARGINRIGHT					L"layout_marginRight"
#define DSUI_PROPERTY_RELATIVE_LAYOUT_MARGINTOP						L"layout_marginTop"

#define LAYOUT_GRAVITY_LEFT											0x00000001
#define LAYOUT_GRAVITY_RIGHT										0x00000002
#define LAYOUT_GRAVITY_TOP											0x00000004
#define LAYOUT_GRAVITY_BOTTOM										0x00000008
#define LAYOUT_GRAVITY_CENTER										0x00000010
#define LAYOUT_GRAVITY_VCENTER										0x00000020
#define LAYOUT_WIDTH_WRAP											0x00000040
#define LAYOUT_HEIGHT_WRAP											0x00000080
#define LAYOUT_HEIGHT_FILL											0x00000100
#define LAYOUT_WIDTH_FILL											0x00000200

namespace gfx {
class Size;
}

namespace views {

class View;

namespace LayoutProperty
{
  const string16 layout_width = L"layout_width";
  const string16 layout_height = L"layout_height";
  const string16 layout_gravity = L"layout_gravity";
  const string16 layout_weight = L"layout_weight";
  const string16 layout_x = L"layout_x";
  const string16 layout_y = L"layout_y";
  const string16 layout_miss_visible = L"layout_miss_visible";

  const string16 layout_space = L"layout_space";
  const string16 layout_orientation = L"layout_orientation";
  const string16 layout_border_horizontal_space = L"layout_horizontal_space";
  const string16 layout_border_vertical_space = L"layout_vertical_space";
  const string16 layout_between_child_space = L"layout_between_child_space";

};

/////////////////////////////////////////////////////////////////////////////
//
// LayoutManager interface
//
//   The LayoutManager interface provides methods to handle the sizing of
//   the children of a View according to implementation-specific heuristics.
//
/////////////////////////////////////////////////////////////////////////////
class VIEWS_EXPORT LayoutManager {
 public:
  virtual ~LayoutManager();

  // Notification that this LayoutManager has been installed on a particular
  // host.
  virtual void Installed(View* host);

  // Notification that this LayoutManager has been uninstalled on a particular
  // host.
  virtual void Uninstalled(View* host);

  // Lay out the children of |host| according to implementation-specific
  // heuristics. The graphics used during painting is provided to allow for
  // string sizing.
  virtual void Layout(View* host) = 0;

  // Return the preferred size which is the size required to give each
  // children their respective preferred size.
  virtual gfx::Size GetPreferredSize(View* host) = 0;
  virtual void SetAttribute(const string16& key, const string16& value){};

  // Returns the preferred height for the specified width. The default
  // implementation returns the value from GetPreferredSize.
  virtual int GetPreferredHeightForWidth(View* host, int width);

  // Notification that a view has been added.
  virtual void ViewAdded(View* host, View* view);

  // Notification that a view has been removed.
  virtual void ViewRemoved(View* host, View* view);
};

}  // namespace views

#endif  // UI_VIEWS_LAYOUT_LAYOUT_MANAGER_H_
