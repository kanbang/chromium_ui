
#include "ui/views/view.h"
#include "ui/views/layout/relative_layout.h"
#include "base/strings/string_number_conversions.h"

namespace views
{
  RelativeLayout::RelativeLayout()
  {
  }

  RelativeLayout::~RelativeLayout() {}

  void RelativeLayout::ReadyLayout(View* host, std::map<std::string, RELATIVELAYOUT> & mapRelative, gfx::Size& content)
  {
    mapRelative.clear();
    gfx::Rect childArea(gfx::Rect(host->size()));
    childArea.Inset(host->GetInsets());
    childArea.Inset(host->GetContentInsets());

    int nFW = childArea.width();
    int nFH = childArea.height();
    for (int i = 0; i < host->child_count(); i++)
    {
      View* pControl = host->child_at(i);
      View::VIEWLAYOUTINFO& layout_info = pControl->GetLayoutInfo();
      RELATIVELAYOUT relativeLayout;
      relativeLayout.pControl = pControl;
      DWORD dwStyle = layout_info.GetLayoutStyle();
      relativeLayout.sz = pControl->GetLayoutSize();

      /*SIZE szMinisize = pControl->GetProperty().GetPropertySize(DSUI_PROPERTY_LAYOUT_MINISIZE);
      if( relativeLayout.sz.cx < szMinisize.cx ) relativeLayout.sz.cx = szMinisize.cx;
      if( relativeLayout.sz.cy < szMinisize.cy ) relativeLayout.sz.cy = szMinisize.cy;*/

      layout_info.GetProperty(DSUI_PROPERTY_RELATIVE_LAYOUT_ABOVE, relativeLayout.layout_above);
      layout_info.GetProperty(DSUI_PROPERTY_RELATIVE_LAYOUT_BELOW, relativeLayout.layout_below);
      layout_info.GetProperty(DSUI_PROPERTY_RELATIVE_LAYOUT_TOLEFTOF, relativeLayout.layout_toLeftOf);
      layout_info.GetProperty(DSUI_PROPERTY_RELATIVE_LAYOUT_TORIGHTOF, relativeLayout.layout_toRightOf);
      layout_info.GetProperty(DSUI_PROPERTY_RELATIVE_LAYOUT_ALIGNTOP, relativeLayout.layout_alignTop);
      layout_info.GetProperty(DSUI_PROPERTY_RELATIVE_LAYOUT_ALIGNLEFT, relativeLayout.layout_alignLeft);
      layout_info.GetProperty(DSUI_PROPERTY_RELATIVE_LAYOUT_ALIGNBOTTOM, relativeLayout.layout_alignBottom);
      layout_info.GetProperty(DSUI_PROPERTY_RELATIVE_LAYOUT_ALIGNRIGHT, relativeLayout.layout_alignRight);

      relativeLayout.layout_marginBottom = layout_info.GetPropertyIdAndInt(DSUI_PROPERTY_RELATIVE_LAYOUT_MARGINBOTTOM, relativeLayout.nMarginBottom);
      relativeLayout.layout_marginLeft = layout_info.GetPropertyIdAndInt(DSUI_PROPERTY_RELATIVE_LAYOUT_MARGINLEFT, relativeLayout.nMarginLeft);
      relativeLayout.layout_marginRight = layout_info.GetPropertyIdAndInt(DSUI_PROPERTY_RELATIVE_LAYOUT_MARGINRIGHT, relativeLayout.nMarginRight);
      relativeLayout.layouy_marginTop = layout_info.GetPropertyIdAndInt(DSUI_PROPERTY_RELATIVE_LAYOUT_MARGINTOP, relativeLayout.nMarginTop);

      mapRelative[pControl->GetName()] = relativeLayout;
    }
  }

  View* RelativeLayout::GetItemByName(View* host, const std::string & name) {
    if(!host)
      return NULL;

    for (int i = 0; i < host->child_count(); i++) {
      View* pControl = host->child_at(i);
      if(pControl && pControl->GetName() == name)
        return pControl;
    }

    return NULL;
  }

  void RelativeLayout::Layout(View* host)
  {
    gfx::Rect childArea(gfx::Rect(host->size()));
    childArea.Inset(host->GetInsets());
    childArea.Inset(host->GetContentInsets());

    // 还未布局的列表
    std::map<std::string, RELATIVELAYOUT> m_mapAllChildControl;

    // 已经布局的列表
    std::set<View*> LayoutControl;
    gfx::Size content_size;
    ReadyLayout(host, m_mapAllChildControl, content_size);
    if( m_mapAllChildControl.size() > 0 ) {
      int nFW = childArea.width();
      int nFH = childArea.height();

      while( m_mapAllChildControl.size() > 0 ) {
        std::map<std::string, RELATIVELAYOUT>::iterator itr = m_mapAllChildControl.begin();
        std::vector<std::string> arrErase;
        for ( ; itr != m_mapAllChildControl.end(); itr++ )
        {
          RELATIVELAYOUT & layout = itr->second;

          int nStartX = -1;//rcInner.left;
          int nStartY = -1;//rcInner.top;

          int nEndX = -1;
          int nEndY = -1;

          bool bParentMissing = true;
          layout.pControl->GetLayoutInfo().GetPropertyBool(DSUI_PROPERTY_RELATIVE_LAYOUT_ALIGNWITHPARENTIFMISSING, bParentMissing);
          //////////////////////////////////////////////////////////////////////////
          // 检查依赖的是否已经布局
          if( layout.layout_below != L"" ) {
            View * pItem = GetItemByName(host, WideToASCII(layout.layout_below));
            if( pItem ) {
              std::set<View*>::iterator itr = LayoutControl.find(pItem);
              if( itr == LayoutControl.end() )
                continue;
              else
                nStartY = pItem->y() + pItem->height();
            }
          }

          if( layout.layout_above != L"" ) {
            View * pItem = GetItemByName(host, WideToASCII(layout.layout_above));
            if( pItem ) {
              std::set<View*>::iterator itr = LayoutControl.find(pItem);
              if( itr == LayoutControl.end() )
                continue;
              else
                nEndY = pItem->y();
            }
          }

          if( layout.layout_toLeftOf != L"" ) {
            View * pItem = GetItemByName(host, WideToASCII(layout.layout_toLeftOf));
            if( pItem ) {
              std::set<View*>::iterator itr = LayoutControl.find(pItem);
              if( itr == LayoutControl.end() )
                continue;
              else
                nEndX = pItem->x();
            }
          }

          if( layout.layout_toRightOf != L"" ) {
            View * pItem = GetItemByName(host, WideToASCII(layout.layout_toRightOf));
            if( pItem ) {
              std::set<View*>::iterator itr = LayoutControl.find(pItem);
              if( itr == LayoutControl.end() )
                continue;
              else
                nStartX = pItem->x() + pItem->width();
            }
          }

          if( layout.layout_alignTop != L"" ) {
            View * pItem = GetItemByName(host, WideToASCII(layout.layout_alignTop));
            if( pItem ) {
              std::set<View*>::iterator itr = LayoutControl.find(pItem);
              if( itr == LayoutControl.end() )
                continue;
              else nStartY = pItem->y();
            } else if( bParentMissing ) {
              nStartY = childArea.y();
            }
          }

          if( layout.layout_alignLeft != L"" ) {
            View * pItem = GetItemByName(host, WideToASCII(layout.layout_alignLeft));
            if( pItem ) {
              std::set<View*>::iterator itr = LayoutControl.find(pItem);
              if( itr == LayoutControl.end() )
                continue;
              else nStartX = pItem->x();
            } else if( bParentMissing ) {
              nStartX = childArea.x();
            }
          }

          if( layout.layout_alignBottom != L"" ) {
            View * pItem = GetItemByName(host, WideToASCII(layout.layout_alignBottom));
            if( pItem ) {
              std::set<View*>::iterator itr = LayoutControl.find(pItem);
              if( itr == LayoutControl.end() )
                continue;
              else nEndY = pItem->y() + pItem->height();
            } else if( bParentMissing ) {
              nEndY = childArea.y() + childArea.width() - layout.sz.height();
            }
          }

          if( layout.layout_alignRight != L"" ) {
            View * pItem = GetItemByName(host, WideToASCII(layout.layout_alignRight));
            if( pItem ) {
              std::set<View*>::iterator itr = LayoutControl.find(pItem);
              if( itr == LayoutControl.end() )
                continue;
              else nEndX = pItem->x() + pItem->width();
            } else if( bParentMissing ) {
              nEndX = childArea.x() + childArea.width();
            }
          }

          if( layout.layout_marginBottom != L"" ) {
            View * pItem = GetItemByName(host, WideToASCII(layout.layout_marginBottom));
            if( pItem ) {
              std::set<View*>::iterator itr = LayoutControl.find(pItem);
              if( itr == LayoutControl.end() )
                continue;
              else nStartY = pItem->y() + pItem->height() + layout.nMarginBottom;
            } else if( bParentMissing ) {
              assert(0);
              nEndY = childArea.y() + childArea.height() - layout.nMarginBottom;
            }
          } else if(layout.nMarginBottom > 0 ) {
            nEndY = childArea.y() + childArea.height() - layout.nMarginBottom;
          }

          if( layout.layout_marginLeft != L"" ) {
            View * pItem = GetItemByName(host, WideToASCII(layout.layout_marginLeft));
            if( pItem ) {
              std::set<View*>::iterator itr = LayoutControl.find(pItem);
              if( itr == LayoutControl.end() )
                continue;
              else nEndX = pItem->x() - layout.nMarginLeft;
            } else if( bParentMissing ) {
              assert(0);
              nStartX = childArea.x() + layout.nMarginLeft;
            }
          } else if( layout.nMarginLeft > 0 ) {
            nStartX = childArea.x() + layout.nMarginLeft;
          }

          if( layout.layout_marginRight != L"" ) {
            View * pItem = GetItemByName(host, WideToASCII(layout.layout_marginRight));
            if( pItem ) {
              std::set<View*>::iterator itr = LayoutControl.find(pItem);
              if( itr == LayoutControl.end() )
                continue;
              else nStartX = pItem->x() + pItem->width() + layout.nMarginRight;
            } else if( bParentMissing ) {
              assert(0);
              nEndX = childArea.x() + childArea.width() - layout.nMarginRight;
            }
          } else if( layout.nMarginRight > 0 ) {
            nEndX = childArea.x() + childArea.width() - layout.nMarginRight;
          }

          if( layout.layouy_marginTop != L"" ) {
            View * pItem = GetItemByName(host, WideToASCII(layout.layouy_marginTop));
            if( pItem ) {
              std::set<View*>::iterator itr = LayoutControl.find(pItem);
              if( itr == LayoutControl.end() )
                continue;
              else nEndY = pItem->y() - layout.nMarginTop;
            } else if( bParentMissing ) {
              assert(0);
              nStartY = childArea.y() + layout.nMarginTop;
            }
          } else if( layout.nMarginTop > 0 ) {
            nStartY = childArea.y() + layout.nMarginTop;
          }

          bool bCenter = false;
          layout.pControl->GetLayoutInfo().GetPropertyBool(DSUI_PROPERTY_RELATIVE_LAYOUT_CENTERHRIZONTAL, bCenter);
          bool bVCenter = false;
          layout.pControl->GetLayoutInfo().GetPropertyBool(DSUI_PROPERTY_RELATIVE_LAYOUT_CENTERVERTICAL, bVCenter);

          bool center_parent = false;
          layout.pControl->GetLayoutInfo().GetPropertyBool(DSUI_PROPERTY_RELATIVE_LAYOUT_CENTERINPARENT, center_parent);

          if( center_parent ) { 
            nStartX = childArea.x() + (nFW - layout.sz.width()) / 2;
            nStartY = childArea.y() + (nFH - layout.sz.height()) / 2;
          } else {
            bool bool_value = false;
            if(layout.pControl->GetLayoutInfo().GetPropertyBool(DSUI_PROPERTY_RELATIVE_LAYOUT_ALIGNPARENTTOP, bool_value) && bool_value) {
              nStartY = childArea.y();
              if(layout.pControl->GetLayoutInfo().GetPropertyBool(DSUI_PROPERTY_RELATIVE_LAYOUT_ALIGNPARENTBOTTOM, bool_value) && bool_value )
                nEndY = childArea.y() + childArea.height();
            }
            else if(layout.pControl->GetLayoutInfo().GetPropertyBool(DSUI_PROPERTY_RELATIVE_LAYOUT_ALIGNPARENTBOTTOM, bool_value) && bool_value ) 
              nStartY = childArea.y() + childArea.height() - layout.sz.height();

            if(layout.pControl->GetLayoutInfo().GetPropertyBool(DSUI_PROPERTY_RELATIVE_LAYOUT_ALIGNPARENTLEFT, bool_value) && bool_value) {
              nStartX = childArea.x();
              if(layout.pControl->GetLayoutInfo().GetPropertyBool(DSUI_PROPERTY_RELATIVE_LAYOUT_ALIGNPARENTRIGHT, bool_value) && bool_value)
                nEndX = childArea.x() + childArea.width();
            }
            else if(layout.pControl->GetLayoutInfo().GetPropertyBool(DSUI_PROPERTY_RELATIVE_LAYOUT_ALIGNPARENTRIGHT, bool_value) && bool_value)
              nStartX = childArea.x() + childArea.width() - layout.sz.width();
          }

          if( bCenter ) nStartX = childArea.x() + (nFW - layout.sz.width()) / 2;
          if( bVCenter ) nStartY = childArea.y() + (nFH - layout.sz.width()) / 2;

          int nWidth = layout.sz.width();
          int nHeight = layout.sz.height();

          if(nStartX != -1 && nEndX != -1 ) {
            if(nEndX > nStartX)
              nWidth = nEndX - nStartX;
          } else {
            if(nEndX != -1) 
              nStartX = nEndX - nWidth;
            else if(nStartX == -1)
              nStartX = childArea.x();
          }

          if(nStartY != -1 && nEndY != -1 ) {
            if(nEndY > nStartY)
              nHeight = nEndY - nStartY;
          } else {
            if(nEndY != -1) 
              nStartY = nEndY - nHeight;
            else if(nStartY == -1)
              nStartY = childArea.y();
          }

          gfx::Rect rcItemPos(nStartX, nStartY, nWidth, nHeight);
          layout.pControl->SetBoundsRect(rcItemPos);
          layout.bLayout = true;
          LayoutControl.insert(layout.pControl);
          arrErase.push_back(layout.pControl->GetName());
        } // for

        // 删除已经布局的;
        if( arrErase.size() == 0 ) break;
        for( size_t e = 0; e < arrErase.size(); e++ ) {
          std::map<std::string, RELATIVELAYOUT>::iterator itFind = m_mapAllChildControl.find(arrErase[e]);
          if( itFind != m_mapAllChildControl.end() ) m_mapAllChildControl.erase(itFind);
        }
      }	
    }
  }

  gfx::Size RelativeLayout::GetPreferredSize(View* host)
  {
    return gfx::Size();
  }

  void RelativeLayout::SetAttribute(const string16& key, const string16& value) {
  }

} //namespace view