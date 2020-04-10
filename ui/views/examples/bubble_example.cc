// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/views/examples/bubble_example.h"

#include "base/strings/utf_string_conversions.h"
#include "ui/views/bubble/bubble_delegate.h"
#include "ui/views/controls/button/label_button.h"
#include "ui/views/controls/label.h"
#include "ui/views/layout/box_layout.h"
#include "ui/views/layout/linear_layout.h"
#include "ui/views/widget/widget.h"
#include "ui/views/layout/view_builder.h"
#include "ui/views/controls/combobox/combobox_listener.h"
#include "ui/views/controls/combobox/combobox.h"
#include "ui/views/window/new_custom_frame_view.h"

namespace views {
namespace examples {

namespace {

SkColor colors[] = { SK_ColorWHITE, SK_ColorGRAY, SK_ColorCYAN, 0xFFC1B1E1 };

BubbleBorder::Arrow arrows[] = {
    BubbleBorder::TOP_LEFT, BubbleBorder::TOP_CENTER,
    BubbleBorder::TOP_RIGHT, BubbleBorder::RIGHT_TOP,
    BubbleBorder::RIGHT_CENTER, BubbleBorder::RIGHT_BOTTOM,
    BubbleBorder::BOTTOM_RIGHT, BubbleBorder::BOTTOM_CENTER,
    BubbleBorder::BOTTOM_LEFT, BubbleBorder::LEFT_BOTTOM,
    BubbleBorder::LEFT_CENTER, BubbleBorder::LEFT_TOP };

string16 GetArrowName(BubbleBorder::Arrow arrow) {
  switch (arrow) {
    case BubbleBorder::TOP_LEFT:      return ASCIIToUTF16("TOP_LEFT");
    case BubbleBorder::TOP_RIGHT:     return ASCIIToUTF16("TOP_RIGHT");
    case BubbleBorder::BOTTOM_LEFT:   return ASCIIToUTF16("BOTTOM_LEFT");
    case BubbleBorder::BOTTOM_RIGHT:  return ASCIIToUTF16("BOTTOM_RIGHT");
    case BubbleBorder::LEFT_TOP:      return ASCIIToUTF16("LEFT_TOP");
    case BubbleBorder::RIGHT_TOP:     return ASCIIToUTF16("RIGHT_TOP");
    case BubbleBorder::LEFT_BOTTOM:   return ASCIIToUTF16("LEFT_BOTTOM");
    case BubbleBorder::RIGHT_BOTTOM:  return ASCIIToUTF16("RIGHT_BOTTOM");
    case BubbleBorder::TOP_CENTER:    return ASCIIToUTF16("TOP_CENTER");
    case BubbleBorder::BOTTOM_CENTER: return ASCIIToUTF16("BOTTOM_CENTER");
    case BubbleBorder::LEFT_CENTER:   return ASCIIToUTF16("LEFT_CENTER");
    case BubbleBorder::RIGHT_CENTER:  return ASCIIToUTF16("RIGHT_CENTER");
    case BubbleBorder::NONE:          return ASCIIToUTF16("NONE");
    case BubbleBorder::FLOAT:         return ASCIIToUTF16("FLOAT");
  }
  return ASCIIToUTF16("INVALID");
}

class ExampleBubble : public BubbleDelegateView {
 public:
   ExampleBubble(View* anchor, BubbleBorder::Arrow arrow)
       : BubbleDelegateView(anchor, arrow) {}

 protected:
  virtual void Init() OVERRIDE {
    SetLayoutManager(new BoxLayout(BoxLayout::kVertical, 50, 50, 0));
    AddChildView(new Label(GetArrowName(arrow())));
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(ExampleBubble);
};

class TestWidgetDelegate : public views::WidgetDelegateView, public views::ComboboxListener {
public:

  TestWidgetDelegate() {
    
  //  set_background(Background::CreateSolidBackground(SK_ColorGRAY));test_linear - ¸±±¾.xml
	ViewBuilder builder;
	builder.BuilderUiFromFile(L"C:\\work\\frame\\myapp\\src\\ui\\res\\skin\\xml\\test_linear.xml", this);

  //
  //builder.BuilderUiFromFile(L"D:\\dev\\c-frame\\myapp\\src\\ui\\res\\skin\\xml\\test_linear.xml", this);
  views::Combobox* pCombox = views::Combobox::get_named_item(this, "type");
  if(pCombox)
    pCombox->set_listener(this);

  AddExample(0);
	return;
	SetLayoutManager(new LinearLayout());
	{
		LabelButton* button = new LabelButton(NULL, ASCIIToUTF16("Close"));
		button->set_focusable(true);
		AddChildView(button);
	}

	{
		LabelButton* button = new LabelButton(NULL, ASCIIToUTF16("Close"));
		button->SetAttribute(L"layout_width", L"fill_parent");
		AddChildView(button);
	}

    Label* label = new Label(L"23111111111111");
    label->SetNewUIStyle(true);
    //label->set_id(1021);
    //label->SetMultiLine(true);
    //label->SetSize(gfx::Size(50, 100));
	//label->set_background(Background::CreateSolidBackground(255, 255, 255, 255));
    label->SetAttribute(L"layout_width", L"fill_parent");
	label->SetAttribute(L"layout_gravity", L"bottom;right");

	
    //button->SetBoundsRect(gfx::Rect(0, 0, 100, 40));
    AddChildView(label);

    //     View* ctn = widget->GetContentsView();
    //     ctn->set_id(888);
    //     //View* contents = new View();
    //     ctn->SetLayoutManager(new BoxLayout(BoxLayout::kHorizontal, 50, 50, 0));
    //     ctn->set_background(Background::CreateSolidBackground(SK_ColorGRAY));
    //     BuildButton(ctn, "Close", 4);
  }


  // Called by the Widget to create the NonClient Frame View for this widget.
  // Return NULL to use the default one.
  virtual NonClientFrameView* CreateNonClientFrameView(Widget* widget) {
    views::NewCustomFrameView* new_frame_view = new views::NewCustomFrameView;
    new_frame_view->SetAttribute(L"padding", L"21 13 21 30");
    new_frame_view->SetAttribute(L"clientarea", L"23 47 23 31");
    //new_frame_view->SetAttribute(L"image-bg", L"5828");
    new_frame_view->SetAttribute(L"image-bg", L"5838");
    new_frame_view->SetAttribute(L"image-alpha", L"5837");
    new_frame_view->SetAttribute(L"image-shadow", L"5836");
    //new_frame_view->SetAttribute(L"image-rgn", L"5837");

    ViewBuilder builder;
    builder.SetButtonListen(new_frame_view);
    builder.BuilderUiFromFile(L"C:\\work\\frame\\myapp\\src\\ui\\res\\skin\\xml\\frame.xml", new_frame_view);

    new_frame_view->Init(widget);
    return new_frame_view;
  }

  void AddExample(int index) {
    string16 paths[]  = {
      L"C:\\work\\frame\\myapp\\src\\ui\\res\\skin\\xml\\button.xml",
      L"C:\\work\\frame\\myapp\\src\\ui\\res\\skin\\xml\\label.xml",
      L"C:\\work\\frame\\myapp\\src\\ui\\res\\skin\\xml\\checkbox.xml",
      L"C:\\work\\frame\\myapp\\src\\ui\\res\\skin\\xml\\edit.xml",
      L"C:\\work\\frame\\myapp\\src\\ui\\res\\skin\\xml\\splitview.xml",
      L"C:\\work\\frame\\myapp\\src\\ui\\res\\skin\\xml\\other.xml",
    };

    View* pParent = GetViewByName("ctn");
    if(pParent) {
      pParent->RemoveAllChildViews(true);

      ViewBuilder builder;
      builder.BuilderUiFromFile(paths[index], pParent);

      pParent->RequestFocus();
      Layout();
    }
    
  }
  virtual void OnSelectedIndexChanged(Combobox* combobox) OVERRIDE {
    AddExample(combobox->selected_index());
  }
  virtual void Layout() 
  {
    views::View::Layout();
  }

  virtual bool CanResize() const {
    return true;
  }

  virtual bool CanMaximize() const {
    return false;
  }

  string16 GetWindowTitle() const {
    return L"Test Delegate";
  }

  virtual View* GetContentsView() OVERRIDE { return this; }
  Widget* widget_;
};

}  // namespace

BubbleExample::BubbleExample() : ExampleBase("Bubble") {}

BubbleExample::~BubbleExample() {}

void BubbleExample::CreateExampleView(View* container) {
  PrintStatus("Click with optional modifiers: [Ctrl] for set_arrow(NONE), "
     "[Alt] for set_arrow(FLOAT), or [Shift] to reverse the arrow iteration.");
  container->SetLayoutManager(new BoxLayout(BoxLayout::kHorizontal, 0, 0, 10));
  shadow_ = new LabelButton(this, ASCIIToUTF16("Shadow"));
  container->AddChildView(shadow_);
  no_shadow_ = new LabelButton(this, ASCIIToUTF16("No Shadow"));
  container->AddChildView(no_shadow_);
  big_shadow_ = new LabelButton(this, ASCIIToUTF16("Big Shadow"));
  container->AddChildView(big_shadow_);
  small_shadow_ = new LabelButton(this, ASCIIToUTF16("Small Shadow"));
  container->AddChildView(small_shadow_);
  align_to_edge_ = new LabelButton(this, ASCIIToUTF16("Align To Edge"));
  container->AddChildView(align_to_edge_);
  persistent_ = new LabelButton(this, ASCIIToUTF16("Persistent"));
  container->AddChildView(persistent_);
  fade_in_ = new LabelButton(this, ASCIIToUTF16("Fade In"));
  container->AddChildView(fade_in_);
  test_wiget_ = new LabelButton(this, ASCIIToUTF16("Test Widget"));
  container->AddChildView(test_wiget_);
}

void BubbleExample::BuildButton(View* container,
  const std::string& label,
  int tag) {
    LabelButton* button = new LabelButton(this, ASCIIToUTF16(label));
    button->set_focusable(true);
    button->set_tag(tag);
    button->set_id(991);
    container->AddChildView(button);
}

void BubbleExample::ShowWidget(View* sender, Widget::InitParams params) {
  // Setup shared Widget heirarchy and bounds parameters.
  params.parent = sender->GetWidget()->GetNativeView();
  params.bounds = gfx::Rect(sender->GetBoundsInScreen().CenterPoint(),
    gfx::Size(600, 400));
  params.opacity = Widget::InitParams::TRANSLUCENT_WINDOW;
  //params.opacity = Widget::InitParams::TRANSLUCENT_WINDOW;
  Widget* widget = new Widget();
  params.delegate = new TestWidgetDelegate();
  params.new_ui = true;
  
  widget->Init(params);


  // If the Widget has no contents by default, add a view with a 'Close' button.
//   if (widget->GetContentsView()) {
//     View* ctn = widget->GetContentsView();
//     ctn->set_id(888);
//     //View* contents = new View();
//     ctn->SetLayoutManager(new BoxLayout(BoxLayout::kHorizontal, 50, 50, 0));
//     ctn->set_background(Background::CreateSolidBackground(SK_ColorGRAY));
//     BuildButton(ctn, "Close", 4);
//     //widget->SetContentsView(contents);
//   }

  widget->Show();
}

void BubbleExample::ButtonPressed(Button* sender, const ui::Event& event) {
  if(sender == test_wiget_) {
    // show widget
    ShowWidget(sender, Widget::InitParams(Widget::InitParams::TYPE_WINDOW));
    return;
  }

  static int arrow_index = 0, color_index = 0;
  static const int count = arraysize(arrows);
  arrow_index = (arrow_index + count + (event.IsShiftDown() ? -1 : 1)) % count;
  BubbleBorder::Arrow arrow = arrows[arrow_index];
  if (event.IsControlDown())
    arrow = BubbleBorder::NONE;
  else if (event.IsAltDown())
    arrow = BubbleBorder::FLOAT;

  ExampleBubble* bubble = new ExampleBubble(sender, arrow);
  bubble->set_color(colors[(color_index++) % arraysize(colors)]);

  if (sender == shadow_)
    bubble->set_shadow(BubbleBorder::SHADOW);
  else if (sender == no_shadow_)
    bubble->set_shadow(BubbleBorder::NO_SHADOW);
  else if (sender == big_shadow_)
    bubble->set_shadow(BubbleBorder::BIG_SHADOW);
  else if (sender == small_shadow_)
    bubble->set_shadow(BubbleBorder::SMALL_SHADOW);

  if (sender == persistent_) {
    bubble->set_close_on_deactivate(false);
    bubble->set_move_with_anchor(true);
  }

  BubbleDelegateView::CreateBubble(bubble);
  if (sender == align_to_edge_)
    bubble->SetAlignment(BubbleBorder::ALIGN_EDGE_TO_ANCHOR_EDGE);

  if (sender == fade_in_)
    bubble->StartFade(true);
  else
    bubble->GetWidget()->Show();
}

}  // namespace examples
}  // namespace views
