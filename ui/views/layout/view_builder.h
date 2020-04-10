#ifndef VIEW_LAYOUT_BUILDER_H_
#define VIEW_LAYOUT_BUILDER_H_
#pragma once
class TiXmlElement;

namespace views{
class View;
class ButtonListener;
}

#include <string>

class ViewBuilder
{
public:
	
	ViewBuilder();

	// 生产UI的元素
  virtual views::View * BuilderUi(const std::wstring& xmlContent, views::View * pParent = NULL);
	virtual views::View * BuilderUiUtf8(const char * lpszBuffer, views::View * pParent = NULL);
  virtual views::View * BuilderUiFromFile(const std::wstring& file_path, views::View * pParent = NULL);
  virtual views::View * GetControl(const std::wstring& tag){ return NULL; }; // 方便扩展调用

  void SetButtonListen(views::ButtonListener* btn_listen) { btn_listen_ = btn_listen; }

protected:
	virtual views::View* ParseElement(TiXmlElement * pEleMent,views::View *pParent);
  virtual void ParseElementLayout(TiXmlElement * pEleMent,views::View *host);
	virtual bool ParserControlXml(TiXmlElement *pEleMent, views::View * pRoot);

  views::ButtonListener* btn_listen_;
};
#endif //VIEW_LAYOUT_BUILDER_H_