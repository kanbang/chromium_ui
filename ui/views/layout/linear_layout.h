
#ifndef __view_linear_layout_h__
#define __view_linear_layout_h__
#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "ui/gfx/insets.h"
#include "ui/views/layout/layout_manager.h"

#include <vector>

namespace views
{

    // A Layout manager that arranges child views vertically or horizontally in a
    // side-by-side fashion with spacing around and between the child views. The
    // child views are always sized according to their preferred size. If the
    // host's bounds provide insufficient space, child views will be clamped.
    // Excess space will not be distributed.
    class VIEWS_EXPORT LinearLayout : public LayoutManager
    {
    public:
        enum Orientation
        {
            kHorizontal,
            kVertical,
        };

        enum ItemAlign
        {
          kLeft,
          kRight,
          kCenter,
          kTop,
          kVCenter,
          kBottom,
        };

        // LinearLayout Property
        struct LinearLayoutProperty
        {
          int x;
          int y;
          int w;
          int h;
          int weight;
          ItemAlign align;
          ItemAlign valign;
          bool missvisible;

          LinearLayoutProperty()
          {
            weight = x = y = w = h = 0;
            align = kLeft;
            valign = kTop;
            missvisible = false;
          }
        };

        // Use |inside_border_horizontal_spacing| and
        // |inside_border_vertical_spacing| to add additional space between the child
        // view area and the host view border. |between_child_spacing| controls the
        // space in between child views.
        LinearLayout();
        LinearLayout(Orientation orientation,
            int inside_border_horizontal_spacing,
            int inside_border_vertical_spacing,
            int between_child_spacing);
        virtual ~LinearLayout();

        // Overridden from view::LayoutManager:
        virtual void Layout(View* host);
        virtual gfx::Size GetPreferredSize(View* host);
		    virtual void SetAttribute(const string16& key, const string16& value);

    private:
        Orientation orientation_;

        // Spacing between child views and host view border.
        int inside_border_horizontal_spacing_;
        int inside_border_vertical_spacing_;

        // Spacing to put in between child views.
        int between_child_spacing_;

        // by readlayout calc realtive property
        void ReadyLayout(View* host, std::vector<LinearLayoutProperty>& lp, gfx::Size& content, int &weight);

        DISALLOW_COPY_AND_ASSIGN(LinearLayout);
    };

} //namespace view

#endif //__view_linear_layout_h__