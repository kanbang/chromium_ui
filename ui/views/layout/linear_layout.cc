
#include "ui/views/view.h"
#include "ui/views/layout/linear_layout.h"
#include "base/strings/string_number_conversions.h"

namespace views
{
  LinearLayout::LinearLayout()
  {
    orientation_ = kVertical;
    inside_border_horizontal_spacing_ = inside_border_vertical_spacing_ = between_child_spacing_ = 5;
  }

  LinearLayout::LinearLayout(LinearLayout::Orientation orientation,
    int inside_border_horizontal_spacing,
    int inside_border_vertical_spacing,
    int between_child_spacing)
    : orientation_(orientation),
    inside_border_horizontal_spacing_(inside_border_horizontal_spacing),
    inside_border_vertical_spacing_(inside_border_vertical_spacing),
    between_child_spacing_(between_child_spacing) {}

  LinearLayout::~LinearLayout() {}

  void LinearLayout::ReadyLayout(View* host, std::vector<LinearLayoutProperty>& lp, gfx::Size& content, int &weight)
  {
    int w = 0;
    int h = 0;
    int nCount = 0;
    for(int i=0; i<host->child_count(); ++i)
    {
      LinearLayoutProperty item_property;
      View* child = host->child_at(i);
      View::VIEWLAYOUTINFO& layout_info =  child->GetLayoutInfo();
      layout_info.GetPropertyBool(LayoutProperty::layout_miss_visible, item_property.missvisible);
      if(child->visible() || item_property.missvisible)
      {
        //gfx::Rect bounds(x, y, childArea.width(), childArea.height());
        gfx::Size size(child->GetLayoutSize());
        item_property.w = size.width();
        item_property.h = size.height();

        std::wstring layout_gravity;
        layout_info.GetProperty(LayoutProperty::layout_gravity, layout_gravity);
        item_property.x = layout_info.layout_x;
        item_property.y = layout_info.layout_y;
        layout_info.GetPropertyInt(LayoutProperty::layout_weight, item_property.weight);

        if(item_property.w > 0) {
          if(orientation_ == kHorizontal) {
            if(item_property.x == 0) {
              w += item_property.w;
              nCount++;
            }
            else if(w < (item_property.x + item_property.w))
              w = item_property.x + item_property.w;
          } else {
            if(item_property.x == 0) {
              if(w < item_property.w && layout_info.layout_width != -1)
                w = item_property.w;
            } else if(w < (item_property.x + item_property.w))
              w = item_property.x + item_property.w;
          }
        } else if(item_property.weight > 0)
			nCount++;

        if(item_property.h > 0) {
          if(orientation_ == kVertical) {
            if(item_property.y == 0) {
              h += item_property.h;
              nCount ++;
            }
            else if(h < item_property.y + item_property.h)
              h = item_property.y + item_property.h;
          } else {
            if(item_property.y == 0) {
              if(h < item_property.h && layout_info.layout_height != -1)
                h = item_property.h;
            }
            else if(h < item_property.y + item_property.h)
              h = item_property.y + item_property.h;
          }
        } else if(item_property.weight > 0)
			nCount++;

        weight += item_property.weight;
        if(orientation_ == kHorizontal)
        {
          if(layout_gravity.find(L"right") != std::string::npos)
            item_property.align = kRight; // right
          else
            item_property.align = kLeft;

          if(layout_gravity.find(L"top") != std::string::npos) {
            item_property.valign = kTop;
          } else if(layout_gravity.find(L"bottom") != std::string::npos) {
            item_property.valign = kBottom;
          } else {
            item_property.valign = kVCenter;
          }
        }
        else
        {
          if(layout_gravity.find(L"bottom") != std::string::npos)
            item_property.valign = kBottom;
          else 
            item_property.valign = kTop;

          if(layout_gravity.find(L"left") != std::string::npos) {
            item_property.align = kLeft;
          } else if(layout_gravity.find(L"right") != std::string::npos) {
            item_property.align = kRight;
          } else
            item_property.align = kCenter;
        }
      }

      lp.push_back(item_property);
    }

    if(nCount > 0) {
      if(orientation_ == kHorizontal)
        w += (nCount - 1) * between_child_spacing_;
      else
        h += (nCount - 1) * between_child_spacing_;
    }

    content.set_width(w);
    content.set_height(h);
  }

  void LinearLayout::Layout(View* host)
  {
    gfx::Rect childArea(gfx::Rect(host->size()));
    childArea.Inset(host->GetInsets());
    childArea.Inset(inside_border_horizontal_spacing_,
      inside_border_vertical_spacing_);
    childArea.Inset(host->GetContentInsets());
    int x = childArea.x();
    int y = childArea.y();
    int rx = 0;
    int by = 0;

    std::vector<LinearLayoutProperty> lp;
    int weight = 0;
    gfx::Size contentSize;
    ReadyLayout(host, lp, contentSize, weight);
    //DCHECK(lp.size() == host->child_count());

    int every_weight = 0;
    int more_width = 0;
    if(weight > 0) {
      if(orientation_ == kHorizontal) {
        if(childArea.width() -  contentSize.width() > 0) {
          every_weight = (childArea.width() - contentSize.width()) / weight;
          more_width = (childArea.width() - contentSize.width()) % weight;
        }
      } else {
        if(childArea.height() -  contentSize.height() > 0) {
          every_weight = (childArea.height() - contentSize.height()) / weight;
          more_width = (childArea.height() - contentSize.height()) % weight;
        }
      }
    }

    for(int i=0; i<host->child_count(); ++i)
    {
      View* child = host->child_at(i);
      LinearLayoutProperty& item_property = lp[i];


      if(child->visible() || item_property.missvisible)
      {
        gfx::Rect bounds(x, y, childArea.width(), childArea.height());
        gfx::Size size(child->GetLayoutSize());

        if(orientation_ == kHorizontal)
        {
          if(item_property.weight > 0) {
            item_property.w += (item_property.weight * every_weight);
            item_property.w += more_width;
            more_width = 0;
          }

          if(item_property.w != -1)
            bounds.set_width(item_property.w);

          if(item_property.x != 0) {
            bounds.set_x(childArea.x() + item_property.x);
          }
          else if(item_property.align == kRight)
          {
            bounds.set_x(childArea.x() + childArea.width() - rx - bounds.width());
            rx += bounds.width() + between_child_spacing_;
          }
          else 
            x += bounds.width() + between_child_spacing_;

          // fill_parent
          if(item_property.h != -1) {
            bounds.set_height(item_property.h);
          }

          if(item_property.y == 0) {
            if(item_property.valign ==  kTop) {
              bounds.set_y(y);
            } else if(item_property.valign == kBottom) {
              bounds.set_y(y + childArea.height() - bounds.height());
            } else {
              bounds.set_y(y + (childArea.height() - bounds.height())/2);
            }
          } else {
            bounds.set_y(y + item_property.y);
          }
        }
        else
        {
          if(item_property.weight > 0) {
            item_property.h += (item_property.weight * every_weight);
            item_property.h += more_width;
            more_width = 0;
          }

          if(item_property.h != -1)
            bounds.set_height(item_property.h);

          if(item_property.y != 0) {
            bounds.set_y(childArea.y() + item_property.y);
          }
          else if(item_property.valign == kBottom)
          {
            bounds.set_y(childArea.y() + childArea.height() - by - bounds.height());
            by += bounds.height() + between_child_spacing_;
          }
          else 
            y += bounds.height() + between_child_spacing_;

          if(item_property.w != -1)
            bounds.set_width(item_property.w);

          if(item_property.x == 0) {
            if(item_property.align == kLeft) {
              bounds.set_x(x); 
            } else if(item_property.align == kRight) {
              bounds.set_x(x + childArea.width() - bounds.width());
            } else
              bounds.set_x(x + (childArea.width() - bounds.width()) / 2);
          } else {
            bounds.set_x(x + item_property.x);
          }
        }
        // Clamp child view bounds to |childArea|.
        bounds.Intersect(childArea);
        child->SetBoundsRect(bounds);
      }
    }
  }

  gfx::Size LinearLayout::GetPreferredSize(View* host)
  {
    std::vector<LinearLayoutProperty> lp;
    int weight = 0;
    gfx::Size content_size;
    ReadyLayout(host, lp, content_size, weight);

    gfx::Insets insets(host->GetInsets());
    return gfx::Size(
      content_size.width()+insets.width()+2*inside_border_horizontal_spacing_,
      content_size.height()+insets.height()+2*inside_border_vertical_spacing_);
  }

  void LinearLayout::SetAttribute(const string16& key, const string16& value) {
    if(LayoutProperty::layout_orientation == key) {
      if(value == L"horizontal") {
        orientation_ = kHorizontal;
      } else {
        orientation_ = kVertical;
      }
    } else if(LayoutProperty::layout_border_horizontal_space == key) {
      base::StringToInt(value, &inside_border_horizontal_spacing_);
    } else if(LayoutProperty::layout_border_vertical_space == key) {
      base::StringToInt(value, &inside_border_vertical_spacing_);
    } else if(LayoutProperty::layout_between_child_space == key) {
      base::StringToInt(value, &between_child_spacing_);
    }
  }

} //namespace view