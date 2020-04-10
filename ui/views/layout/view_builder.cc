#include "view_builder.h"
#include "third_party/tinyxml/tinyxml.h"
#include "third_party/tinyxml/tinystr.h"
#include "base/strings/sys_string_conversions.h"
#include "base/strings/string_piece.h"
#include "base/strings/string_util.h"

#include "ui/views/view.h"
#include "ui/views/controls/link.h"
#include "ui/views/controls/label.h"
#include "ui/views/controls/button/text_button.h"
#include "ui/views/controls/button/checkbox.h"
#include "ui/views/controls/button/radio_button.h"
#include "ui/views/controls/button/image_button.h"
#include "ui/views/controls/button/label_button.h"
#include "ui/views/controls/button/blue_button.h"
#include "ui/views/controls/separator.h"
#include "ui/views/controls/throbber.h"
#include "ui/views/controls/image_view.h"
#include "ui/views/controls/progress_bar.h"
#include "ui/views/controls/scroll_view.h"
#include "ui/views/controls/single_split_view.h"
#include "ui/views/controls/button/new_button.h"
#include "ui/views/controls/combobox/combobox.h"
#include "ui/views/controls/textfield/textfield.h"
#include "ui/views/controls/slider.h"
#include "ui/views/layout/layout_manager.h"
#include "ui/views/layout/box_layout.h"
#include "ui/views/layout/linear_layout.h"
#include "ui/views/layout/grid_layout.h"
#include "ui/views/layout/relative_layout.h"
#include "ui/base/models/combobox_model.h"

ViewBuilder::ViewBuilder() : btn_listen_(NULL)
{
}

views::View * ViewBuilder::BuilderUiFromFile(const std::wstring& file_path, views::View * pParent /*= NULL*/)
{
  if(file_path.length() == 0)
    return NULL;

  TiXmlDocument xmlDocument;
  if (xmlDocument.LoadFile(base::SysWideToMultiByte(file_path, CP_ACP).c_str(), TIXML_ENCODING_UTF8)) {
    TiXmlElement * pRoot = xmlDocument.RootElement();
    if (pRoot){
      const char * pszValue = pRoot->Value();
      if( pszValue ) {
        views::View * pControl = NULL;
        while( pRoot ) {
          pControl = ParseElement(pRoot, pParent);
          pRoot = pRoot->NextSiblingElement();
        }
        return pControl;
      }
    }
  }
  return NULL;
}

bool ViewBuilder::ParserControlXml(TiXmlElement *pEleMent, views::View * pRoot)
{
  // 加入各个节点
  TiXmlElement * pChild = pEleMent->FirstChildElement();
  while( pChild ) {
    ParseElement(pChild, pRoot);
    pChild = pChild->NextSiblingElement();
  }
  return false;
}

void ViewBuilder::ParseElementLayout(TiXmlElement * pEleMent,views::View *host)
{
  views::LayoutManager* pLayoutManage = NULL;

  const char* pLayotName = pEleMent->Attribute("name");
  if(pLayotName) {
    if(base::strcasecmp(pLayotName, "boxlayout") == 0)
      pLayoutManage = new views::BoxLayout(views::BoxLayout::kVertical, 5, 5, 5);
    else if(base::strcasecmp(pLayotName, "linearlayout") == 0) {
      pLayoutManage = new views::LinearLayout();
    } else if(base::strcasecmp(pLayotName, "gridlayout") == 0) {
      pLayoutManage = new views::GridLayout(host);
    } else if(base::strcasecmp(pLayotName, "relativelayout") == 0) {
		pLayoutManage = new views::RelativeLayout();
	}
  }

  if(pLayoutManage) {
    TiXmlAttribute * pAttri = pEleMent->FirstAttribute();
    while( pAttri ) {
      const char * pVaule = pAttri->Value();
      const char * pName = pAttri->Name();
      if( pVaule && pName ) {
        pLayoutManage->SetAttribute(base::SysUTF8ToWide(pName), base::SysUTF8ToWide(pVaule));
      }
      pAttri = pAttri->Next();
    }

    host->SetLayoutManager(pLayoutManage);
  }
}

views::View* ViewBuilder::ParseElement(TiXmlElement * pEleMent, views::View *pParent)
{
  bool add = true;
  const char * pVaule = pEleMent->Value();
  views::View * pControl = NULL;
  if( pVaule ) {
    // 解析插件
    std::wstring strTagName = base::SysUTF8ToWide(base::StringPiece(pVaule));
    pControl = GetControl(strTagName.c_str());
    if( NULL == pControl ) {
      int nLength = strlen(pVaule);
      switch(nLength) 
      {
      case 4:
        {
          if(base::strcasecmp(pVaule, "view") == 0) pControl = new views::View();
          else if(base::strcasecmp(pVaule, "link") == 0) pControl = new views::Link();
          else if(base::strcasecmp(pVaule, "edit") == 0) pControl = new views::Textfield();
          break;
        }
      case 5:
        {
          if(base::strcasecmp(pVaule, "label") == 0) pControl = new views::Label();
          break;
        }

      case 6:
        {
          if(base::strcasecmp(pVaule, "layout") == 0) {
            ParseElementLayout(pEleMent, pParent);
            return NULL;
          }

          if(base::strcasecmp(pVaule, "button") == 0) pControl = new views::NewButton(btn_listen_);
          if(base::strcasecmp(pVaule, "slider") == 0) {
            views::Slider::Orientation ori = views::Slider::HORIZONTAL;
            TiXmlAttribute * pAttri = pEleMent->FirstAttribute();
            while( pAttri ) {
              const char * pVaule = pAttri->Value();
              const char * pName = pAttri->Name();
              if( pVaule && pName ) {
                if(base::strcasecmp(pName, "orientation") == 0) {
                  if(base::strcasecmp(pVaule, "vertical") == 0)
                    ori = views::Slider::VERTICAL;
                }
              }
              pAttri = pAttri->Next();
            }

            pControl = new views::Slider(NULL, ori);
            if(pParent)
              pParent->AddChildView(pControl);
            return pControl;
          }
          
          break;
        }
      case 7:
        {
          
        }
        break;
      case 8:
        {
          if(base::strcasecmp(pVaule, "checkbox") == 0) pControl = new views::Checkbox(L"");
          if(base::strcasecmp(pVaule, "progress") == 0) pControl = new views::ProgressBar();
          if(base::strcasecmp(pVaule, "throbber") == 0) pControl = new views::Throbber(50, true);
          if(base::strcasecmp(pVaule, "combobox") == 0) {
            views::Combobox* pCombox = new views::Combobox();
            if(!pCombox)
              return NULL;

            ui::SimpleComboBoxModel* model = pCombox->GetSimpleModel();
          
            // 解析子元素
            TiXmlElement * pChild = pEleMent->FirstChildElement();
            while( pChild) {
              const char * pChildVaule = pChild->Value();
              if(pChildVaule) {
                if(base::strcasecmp(pChildVaule, "item") == 0) {
                  TiXmlAttribute * pAttri = pChild->FirstAttribute();
                  while( pAttri ) {
                    const char * pVaule = pAttri->Value();
                    const char * pName = pAttri->Name();
                    if( pVaule && pName ) {
                      if(base::strcasecmp(pName, "text") == 0)
                        model->AddItem(base::SysUTF8ToWide(pVaule));

                      if(base::strcasecmp(pName, "default") == 0 && base::strcasecmp(pVaule, "true") == 0)
                        model->SetDefaultIndex(model->GetItemCount() - 1);
                    }
                    pAttri = pAttri->Next();
                  }
                }
              }

              pChild = pChild->NextSiblingElement();
            }

            pCombox->SetModel(model);

            // 设置属性
            TiXmlAttribute * pAttri = pEleMent->FirstAttribute();
            while( pAttri ) {
              const char * pVaule = pAttri->Value();
              const char * pName = pAttri->Name();
              if( pVaule && pName ) {
                pCombox->SetAttribute(base::SysUTF8ToWide(pName), base::SysUTF8ToWide(pVaule));
              }
              pAttri = pAttri->Next();
            }

            if(pParent) 
              pParent->AddChildView(pCombox);

            return pCombox;
          }
          break;
        }
      case 9:
        {
          if(base::strcasecmp(pVaule, "Separator") == 0) pControl = new views::Separator(views::Separator::VERTICAL);
          if(base::strcasecmp(pVaule, "imageview") == 0) pControl = new views::ImageView();
          break;
        }
      case 10:
        {
          if(base::strcasecmp(pVaule, "bluebutton") == 0)
            pControl = new views::BlueButton(NULL, L"");
          if(base::strcasecmp(pVaule, "textbutton") == 0)
            pControl = new views::TextButton(NULL, L"");
          if(base::strcasecmp(pVaule, "scrollview") == 0) pControl = new views::ScrollView();
          
          break;
        } 
      case 11:
        {
		  if(base::strcasecmp(pVaule, "labelbutton") == 0) pControl = new views::LabelButton(btn_listen_, L"");
          if(base::strcasecmp(pVaule, "radiobutton") == 0) pControl = new views::RadioButton(L"", 0);
          if(base::strcasecmp(pVaule, "imagebutton") == 0) pControl = new views::ImageButton(btn_listen_);
          if(base::strcasecmp(pVaule, "contentview") == 0) {
            pControl = new views::View();
            views::ScrollView *pScroollview = static_cast<views::ScrollView*>(pParent);
            if(pScroollview)
              pScroollview->SetContents(pControl);
            add = false;
          }
          break;
        }
      case 15:
        {
          if(base::strcasecmp(pVaule, "SingleSplitView") == 0) {
            pControl = new views::SingleSplitView();
          }
          break;
        }
      case 16:
        {
          if(base::strcasecmp(pVaule, "SmoothedThrobber") == 0) pControl = new views::SmoothedThrobber(0);
          //if(base::strcasecmp(pVaule, "NativeTextButton") == 0) pControl = new views::NativeTextButton(NULL);
          break;
        }
      case 17:
        {
          if(base::strcasecmp(pVaule, "CheckmarkThrobber") == 0) pControl = new views::CheckmarkThrobber();
          if(base::strcasecmp(pVaule, "ToggleImageButton") == 0) pControl = new views::ToggleImageButton(btn_listen_);
          break;
        }
        // end 11ToggleImageButton
      }
    }

    // 解析属性
    if( pControl ) {
      TiXmlAttribute * pAttri = pEleMent->FirstAttribute();
      while( pAttri ) {
        const char * pVaule = pAttri->Value();
        const char * pName = pAttri->Name();
        if( pVaule && pName ) {
          pControl->SetAttribute(base::SysUTF8ToWide(pName), base::SysUTF8ToWide(pVaule));
        }
        pAttri = pAttri->Next();
      }

      // 解析子元素
      TiXmlElement * pChild = pEleMent->FirstChildElement();
      while( pChild) {
        const char * pChildVaule = pChild->Value();
        if(pChildVaule) {
          if(base::strcasecmp(pChildVaule, "layout") == 0) {
            ParseElementLayout(pChild, pControl);
          }
        }
        ParseElement(pChild, pControl);
        pChild = pChild->NextSiblingElement();
      }

      if(pParent && add) 
        pParent->AddChildView(pControl);

      return pControl;
    }
  }

  return pControl;
}

views::View * ViewBuilder::BuilderUi(const std::wstring& xmlContent, views::View * pPrarent/*=null*/)
{
  if( xmlContent.length() > 0 ) {
    TiXmlDocument xmlDocument;
    std::string strBuf = base::SysWideToMultiByte(xmlContent, CP_UTF8);
    xmlDocument.Parse(strBuf.c_str(), 0, TIXML_ENCODING_UTF8);
    TiXmlElement * pRoot = xmlDocument.RootElement();
    if (pRoot){
      const char * pszValue = pRoot->Value();
      if( pszValue ) {
        views::View * pControl = NULL;
        while( pRoot ) {
          pControl = ParseElement(pRoot, pPrarent);
          pRoot = pRoot->NextSiblingElement();
        }

        return pControl;	
      }
    }
  }
  return NULL;
}

views::View * ViewBuilder::BuilderUiUtf8(const char * lpszBuffer, views::View * pParent/*=null*/)
{
  if( lpszBuffer ) {
    TiXmlDocument xmlDocument;
    xmlDocument.Parse(lpszBuffer, 0, TIXML_ENCODING_UTF8);
    TiXmlElement * pRoot = xmlDocument.RootElement();
    if (pRoot){
      const char * pszValue = pRoot->Value();
      if( pszValue ) {

        views::View * pControl = NULL;
        while( pRoot ) {
          pControl = ParseElement(pRoot, pParent);
          pRoot = pRoot->NextSiblingElement();
        }

        return pControl;
      }
    }
  }

  return NULL;
}
