
#ifndef __view_relative_layout_h__
#define __view_relative_layout_h__
#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/strings/string16.h"
#include "ui/gfx/insets.h"
#include "ui/gfx/size.h"
#include "ui/views/layout/layout_manager.h"

#include <vector>

namespace views
{

    // A Layout manager that arranges child views vertically or horizontally in a
    // side-by-side fashion with spacing around and between the child views. The
    // child views are always sized according to their preferred size. If the
    // host's bounds provide insufficient space, child views will be clamped.
    // Excess space will not be distributed.
    class VIEWS_EXPORT RelativeLayout : public LayoutManager
    {
    public:
      struct RELATIVELAYOUT
      {
        View * pControl;
        gfx::Size sz;
        string16 layout_below;
        string16 layout_above;
        string16 layout_toLeftOf;
        string16 layout_toRightOf;
        string16 layout_alignTop;
        string16 layout_alignLeft;
        string16 layout_alignBottom;
        string16 layout_alignRight;
        string16 layout_marginBottom;
        string16 layout_marginLeft;
        string16 layout_marginRight;
        string16 layouy_marginTop;
        int nMarginBottom;
        int nMarginLeft;
        int nMarginRight;
        int nMarginTop;
        bool bLayout;

        RELATIVELAYOUT()
        {
          pControl = NULL;
          nMarginTop = nMarginBottom = nMarginLeft = nMarginRight = 0;
          bLayout = false;
        }
      };

        // Use |inside_border_horizontal_spacing| and
        // |inside_border_vertical_spacing| to add additional space between the child
        // view area and the host view border. |between_child_spacing| controls the
        // space in between child views.
        RelativeLayout();
        virtual ~RelativeLayout();

        // Overridden from view::LayoutManager:
        virtual void Layout(View* host);
        virtual gfx::Size GetPreferredSize(View* host);
		    virtual void SetAttribute(const string16& key, const string16& value);

    private:

        // by readlayout calc realtive property
        void ReadyLayout(View* host, std::map<std::string, RELATIVELAYOUT> & mapRelative, gfx::Size& content);
        View* GetItemByName(View* host, const std::string & name);

        DISALLOW_COPY_AND_ASSIGN(RelativeLayout);
    };

} //namespace view

#endif //__view_relative_layout_h__