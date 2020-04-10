#include <map>

#include "ui/base/resource/resource_bundle.h"
#include "chrome/theme_resources_browser.h"
#include "base/path_service.h"
#include "base/files/file_path.h"
#include "base/strings/sys_string_conversions.h"
#include "base/files/memory_mapped_file.h"
#include "third_party/skia/include/core/SkBitmap.h"
#include "chrome/ui/theme/theme_properties.h"
#include "grit/app_strings_id.h"
#include "chrome/theme_strings_id.h"

namespace themeresoure
{
  ThemeRes::ThemeRes() {
     init_theme_res();
     init_theme_string();
  }

  void ThemeRes::init_theme_string() {
    string_ids[IDS_APP_UNDO] = L"&Undo";
    string_ids[IDS_APP_CUT] = L"Cu&t";
    string_ids[IDS_APP_COPY] = L"&Copy";
    string_ids[IDS_APP_PASTE] = L"&Paste";
    string_ids[IDS_APP_DELETE] = L"&Delete";
    string_ids[IDS_APP_SELECT_ALL] = L"Select &all";
      
    string_ids[IDS_PRODUCT_NAME] = L"Chromium";
    string_ids[IDS_UI_FONT_FAMILY] = L"default";
    string_ids[IDS_UI_FONT_FAMILY_XP] = L"default";
    string_ids[IDS_UI_FONT_SIZE_SCALER] = L"100";
    string_ids[IDS_UI_FONT_SIZE_SCALER_XP] = L"100";
    string_ids[IDS_TOOLTIP_NEW_TAB] = L"New Tab";
    string_ids[IDS_ACCNAME_NEWTAB] = L"New Tab";
    string_ids[IDS_ACCNAME_MINIMIZE] = L"Minimize";
    string_ids[IDS_ACCNAME_MAXIMIZE] = L"Maximize";
    string_ids[IDS_ACCNAME_RESTORE] = L"Restore";
    string_ids[IDS_ACCNAME_CLOSE] = L"Close";
    string_ids[IDS_TOOLTIP_BACK] = L"Click to go back, hold to see history";
    string_ids[IDS_TOOLTIP_FORWARD] = L"Click to go forward, hold to see history";
    string_ids[IDS_TOOLTIP_HOME] = L"Open the home page";
    string_ids[IDS_TOOLTIP_RELOAD] = L"Reload this page";
    string_ids[IDS_ACCNAME_BACK] = L"Back";
    string_ids[IDS_ACCNAME_FORWARD] = L"Forward";
    string_ids[IDS_ACCNAME_HOME] = L"Home";
    string_ids[IDS_ACCNAME_RELOAD] = L"Reload";
    string_ids[IDS_ACCNAME_APP] = L"Chromium";
    string_ids[IDS_APPMENU_TOOLTIP] = L"Customize and control Chromium";
    string_ids[IDS_RELOAD_MENU_NORMAL_RELOAD_ITEM] = L"Normal Reload";
    string_ids[IDS_RELOAD_MENU_HARD_RELOAD_ITEM] = L"Hard Reload";
    string_ids[IDS_RELOAD_MENU_EMPTY_AND_HARD_RELOAD_ITEM] = L"Empty Cache and Hard Reload";
    string_ids[IDS_TOOLTIP_LOCATION_ICON] = L"View site information";
    string_ids[IDS_TOOLTIP_MIC_SEARCH] = L"Search by voice";
    string_ids[IDS_PDF_BUBBLE_OPEN_IN_READER_LINK] = L"Open in Adobe Reader";
    string_ids[IDS_TOOLTIP_SCRIPT_BUBBLE] = L" View extensions interacting with page";
    string_ids[IDS_TOOLTIP_STAR] = L"Bookmark this page";
    string_ids[IDS_TOOLTIP_STARRED] = L"Edit bookmark for this page";
    string_ids[IDS_ACCNAME_BOOKMARKS_CHEVRON] = L"Menu containing hidden bookmarks";
    string_ids[IDS_BOOKMARK_BAR_APPS_SHORTCUT_NAME] = L"Apps";
    string_ids[IDS_BOOKMARK_BAR_APPS_SHORTCUT_TOOLTIP] = L"Show apps";
    string_ids[IDS_TOOLTIP_RELOAD_WITH_MENU] = L"Reload this page, hold to see more options";
    string_ids[IDS_TOOLTIP_STOP] = L"Stop loading this page";

    string_ids[IDS_PASTE_AND_GO] = L"Pa&ste and go";
    string_ids[IDS_PASTE_AND_SEARCH] = L"Pa&ste and search";
    string_ids[IDS_SHOW_URL] = L"Show URL";
    string_ids[IDS_EDIT_SEARCH_ENGINES] = L"&Edit search engines...";

    string_ids[IDS_NEW_TAB] = L"New &tab";
    string_ids[IDS_NEW_WINDOW] = L"&New window";
    string_ids[IDS_NEW_INCOGNITO_WINDOW] = L"New &incognito window";
    string_ids[IDS_BOOKMARKS_MENU] = L"&Bookmarks";
    string_ids[IDS_RECENT_TABS_MENU] = L"Recent Tabs";
    string_ids[IDS_EDIT2] = L"Edit";
    string_ids[IDS_UNDO] = L"&Undo";
    string_ids[IDS_CUT] = L"Cut";
    string_ids[IDS_COPY] = L"Copy";  
    string_ids[IDS_PASTE] = L"Paste";    
    string_ids[IDS_DELETE] = L"&Delete";  
    string_ids[IDS_SELECT_ALL] = L"Select &all";  
     
    string_ids[IDS_FIND] = L"&Find...";
    string_ids[IDS_SAVE_PAGE] = L"Save Page &As...";
    string_ids[IDS_PRINT] = L"&Print...";
    string_ids[IDS_TOOLS_MENU] = L"Too&ls";
    string_ids[IDS_SHOW_HISTORY] = L"&History";
    string_ids[IDS_SHOW_DOWNLOADS] = L"&Downloads";
    string_ids[IDS_SETTINGS] = L"&Settings";
    string_ids[IDS_ABOUT] = L"about chrome";
    string_ids[IDS_HELP_PAGE] = L"H&elp";
    string_ids[IDS_MORE_TOOLS_MENU] = L"More too&ls";
    string_ids[IDS_EXIT] = L"E&xit";

    string_ids[IDS_ZOOM_PLUS] = L"&Larger";
    string_ids[IDS_ZOOM_PLUS2] = L"+";
    string_ids[IDS_ZOOM_MINUS] = L"&Smaller";
    string_ids[IDS_FULLSCREEN] = L"&Full Screen";
    string_ids[IDS_ZOOM_MENU] = L"&Zoom";
    string_ids[IDS_ZOOM_MENU2] = L"Zoom";
    string_ids[IDS_ZOOM_NORMAL] = L"Normal";
    string_ids[IDS_ZOOM_MINUS2] = L"-";
    string_ids[IDS_ZOOM_PERCENT] = L"$1%";
    
    string_ids[IDS_ACCNAME_ZOOM_PLUS2] = L"Make Text Larger";
    string_ids[IDS_ACCNAME_ZOOM_MINUS2] = L"Make Text Smaller";
    string_ids[IDS_ACCNAME_FULLSCREEN] = L"Full screen";
    
    /*
    <message name="IDS_ZOOM_NORMAL" desc="The text label of the Make Text Normal Size menu item">
    &amp;Normal
    </message>
    <message name="IDS_ZOOM_MINUS" desc="The text label of the Make Text Smaller menu item">
    &amp;Smaller
    </message>
    <message name="IDS_ZOOM_MINUS2" desc="The text label of the Make Text Smaller menu item in the merged menu">
    &#8722;
    </message>
    <message name="IDS_ZOOM_PERCENT" desc="Current pages zoom factor; shown in merged menu">
    <ph name="VALUE">$1<ex>100</ex></ph>%
    </message>
    */
  }

  void ThemeRes::init_theme_res() {
      res_ids[IDR_TAB_ALPHA_LEFT] = "common\\tab_alpha_left.png";
      res_ids[IDR_TAB_ALPHA_RIGHT] = "common\\tab_alpha_right.png";
      res_ids[IDR_TAB_ACTIVE_CENTER] = "common\\tab_active_center.png";
      res_ids[IDR_TAB_ACTIVE_LEFT] = "common\\tab_active_left.png";
      res_ids[IDR_TAB_ACTIVE_RIGHT] = "common\\tab_active_right.png";
      res_ids[IDR_TAB_INACTIVE_CENTER] = "common\\tab_inactive_center.png";
      res_ids[IDR_TAB_INACTIVE_LEFT] = "common\\tab_inactive_left.png";
      res_ids[IDR_TAB_INACTIVE_RIGHT] = "common\\tab_inactive_right.png";
      res_ids[IDR_THEME_TAB_BACKGROUND_V] = "theme_tab_background_glass.png";
      res_ids[IDR_THEME_TAB_BACKGROUND_INCOGNITO] = "theme_tab_background_incognito.png";
      res_ids[IDR_THEME_TAB_BACKGROUND] = "theme_tab_background.png";
      
      res_ids[IDR_NEWTAB_BUTTON] = "common\\newtab_normal.png";
      res_ids[IDR_NEWTAB_BUTTON_H] = "common\\newtab_hover.png";
      res_ids[IDR_NEWTAB_BUTTON_MASK] = "common\\newtab_button_mask.png";
      res_ids[IDR_NEWTAB_BUTTON_P] = "common\\newtab_pressed.png";
      
      res_ids[IDR_TAB_DROP_DOWN] = "tab_drop_down.png";
      res_ids[IDR_TAB_DROP_UP] = "tab_drop_up.png";
      res_ids[IDR_DOCK_HIGH] = "dock_tab_high.png";
      res_ids[IDR_DOCK_MAX] = "dock_tab_max.png";
      res_ids[IDR_DOCK_WIDE] = "dock_tab_wide.png";

      // wrench_toolbar_button
      res_ids[IDR_TOOLBAR_BEZEL_HOVER] = "common\\browser_toolbar_bezel_hover.png";
      res_ids[IDR_MENU_DROPARROW] = "common\\menu_droparrow.png";
      res_ids[IDR_HISTORY_FAVICON] = "common\\favicon_history.png";

      // toolbars
      res_ids[IDR_BACK] = "back.png";
      res_ids[IDR_BACK_D] = "back_disabled.png";
      res_ids[IDR_BACK_H] = "back_hover.png";
      res_ids[IDR_BACK_P] = "back_pressed.png";

      res_ids[IDR_FORWARD] = "forward.png";
      res_ids[IDR_FORWARD_D] = "forward_disabled.png";
      res_ids[IDR_FORWARD_H] = "forward_hover.png";
      res_ids[IDR_FORWARD_P] = "forward_pressed.png";

      res_ids[IDR_HOME] = "home.png";
      res_ids[IDR_HOME_H] = "home_hover.png";
      res_ids[IDR_HOME_P] = "home_pressed.png";

      res_ids[IDR_CONTENT_TOP_CENTER] = "content_top_center.png";

      // reloadbutton.cc
      res_ids[IDR_RELOAD] = "reload.png";
      res_ids[IDR_RELOAD_H] = "reload_hover.png";
      res_ids[IDR_RELOAD_P] = "reload_pressed.png";
      res_ids[IDR_RELOAD_DIMMED] = "reload_dimmed.png";
      res_ids[IDR_RELOAD_D] = "common\\browser_reload_disabled.png";
      res_ids[IDR_STOP] = "common\\browser_stop_normal.png";
      res_ids[IDR_STOP_D] = "common\\browser_stop_disabled.png";
      res_ids[IDR_STOP_H] = "common\\browser_stop_hover.png";
      res_ids[IDR_STOP_P] = "common\\browser_stop_pressed.png";

      //ev_bubble
      res_ids[IDR_OMNIBOX_HTTPS_VALID] = "common\\omnibox_https_valid.png";
      res_ids[IDR_OMNIBOX_EV_BUBBLE_BOTTOM] = "common\\omnibox_ev_bubble_bottom.png";
      res_ids[IDR_OMNIBOX_EV_BUBBLE_BOTTOM_LEFT] = "common\\omnibox_ev_bubble_bottom_left.png";
      res_ids[IDR_OMNIBOX_EV_BUBBLE_BOTTOM_RIGHT] = "common\\omnibox_ev_bubble_bottom_right.png";
      res_ids[IDR_OMNIBOX_EV_BUBBLE_CENTER] = "common\\omnibox_ev_bubble_center.png";
      res_ids[IDR_OMNIBOX_EV_BUBBLE_LEFT] = "common\\omnibox_ev_bubble_left.png";
      res_ids[IDR_OMNIBOX_EV_BUBBLE_RIGHT] = "common\\omnibox_ev_bubble_right.png";
      res_ids[IDR_OMNIBOX_EV_BUBBLE_TOP] = "common\\omnibox_ev_bubble_top.png";
      res_ids[IDR_OMNIBOX_EV_BUBBLE_TOP_LEFT] = "common\\omnibox_ev_bubble_top_left.png";
      res_ids[IDR_OMNIBOX_EV_BUBBLE_TOP_RIGHT] = "common\\omnibox_ev_bubble_top_right.png";

      // select keyword
      res_ids[IDR_KEYWORD_SEARCH_MAGNIFIER] = "keyword_search_magnifier.png";
      res_ids[IDR_OMNIBOX_SELECTED_KEYWORD_BUBBLE_BOTTOM] = "common\\omnibox_selected_keyword_bubble_bottom.png";
      res_ids[IDR_OMNIBOX_SELECTED_KEYWORD_BUBBLE_BOTTOM_LEFT] = "common\\omnibox_selected_keyword_bubble_bottom_left.png";
      res_ids[IDR_OMNIBOX_SELECTED_KEYWORD_BUBBLE_BOTTOM_RIGHT] = "common\\omnibox_selected_keyword_bubble_bottom_right.png";
      res_ids[IDR_OMNIBOX_SELECTED_KEYWORD_BUBBLE_CENTER] = "common\\omnibox_selected_keyword_bubble_center.png";
      res_ids[IDR_OMNIBOX_SELECTED_KEYWORD_BUBBLE_LEFT] = "common\\omnibox_selected_keyword_bubble_left.png";
      res_ids[IDR_OMNIBOX_SELECTED_KEYWORD_BUBBLE_RIGHT] = "common\\omnibox_selected_keyword_bubble_right.png";
      res_ids[IDR_OMNIBOX_SELECTED_KEYWORD_BUBBLE_TOP] = "common\\omnibox_selected_keyword_bubble_top.png";
      res_ids[IDR_OMNIBOX_SELECTED_KEYWORD_BUBBLE_TOP_LEFT] = "common\\omnibox_selected_keyword_bubble_top_left.png";
      res_ids[IDR_OMNIBOX_SELECTED_KEYWORD_BUBBLE_TOP_RIGHT] = "common\\omnibox_selected_keyword_bubble_top_right.png";

      // keyword_hint_view
      res_ids[IDR_OMNIBOX_KEYWORD_HINT_TAB] = "common\\omnibox_keyword_hint_tab.png";

      // location_bar_view.cc
     res_ids[IDR_OMNIBOX_BORDER_BOTTOM] = "common\\omnibox_border_bottom.png";
     res_ids[IDR_OMNIBOX_BORDER_BOTTOM_LEFT] = "common\\omnibox_border_bottom_left.png";
     res_ids[IDR_OMNIBOX_BORDER_BOTTOM_RIGHT] = "common\\omnibox_border_bottom_right.png";
     res_ids[IDR_OMNIBOX_BORDER_CENTER] = "common\\omnibox_border_center.png";
     res_ids[IDR_OMNIBOX_BORDER_LEFT] = "common\\omnibox_border_left.png";
     res_ids[IDR_OMNIBOX_BORDER_RIGHT] = "common\\omnibox_border_right.png";
     res_ids[IDR_OMNIBOX_BORDER_TOP] = "common\\omnibox_border_top.png";
     res_ids[IDR_OMNIBOX_BORDER_TOP_LEFT] = "common\\omnibox_border_top_left.png";
     res_ids[IDR_OMNIBOX_BORDER_TOP_RIGHT] = "common\\omnibox_border_top_right.png";
     res_ids[IDR_OMNIBOX_POPUP_BORDER_BOTTOM] = "common\\omnibox_popup_border_bottom.png";
     res_ids[IDR_OMNIBOX_POPUP_BORDER_BOTTOM_LEFT] = "common\\omnibox_popup_border_bottom_left.png";
     res_ids[IDR_OMNIBOX_POPUP_BORDER_BOTTOM_RIGHT] = "common\\omnibox_popup_border_bottom_right.png";
     res_ids[IDR_OMNIBOX_POPUP_BORDER_CENTER] = "common\\omnibox_popup_border_center.png";
     res_ids[IDR_OMNIBOX_POPUP_BORDER_LEFT] = "common\\omnibox_popup_border_left.png";
     res_ids[IDR_OMNIBOX_POPUP_BORDER_RIGHT] = "common\\omnibox_popup_border_right.png";
     res_ids[IDR_OMNIBOX_POPUP_BORDER_TOP] = "common\\omnibox_popup_border_top.png";
     res_ids[IDR_OMNIBOX_POPUP_BORDER_TOP_LEFT] = "common\\omnibox_popup_border_top_left.png";
     res_ids[IDR_OMNIBOX_POPUP_BORDER_TOP_RIGHT] = "common\\omnibox_popup_border_top_right.png";
     res_ids[IDR_OMNIBOX_MIC_SEARCH] = "common\\omnibox_mic_search.png";
     res_ids[IDR_OMNIBOX_TTS] = "common\\omnibox_tts.png";
     res_ids[IDR_OMNIBOX_TTS_SELECTED] = "common\\omnibox_mic_search.png";
     res_ids[IDR_OMNIBOX_STAR_SELECTED] = "common\\omnibox_star_selected.png";
     res_ids[IDR_OMNIBOX_SEARCH_SELECTED] = "common\\omnibox_search_selected.png";
     res_ids[IDR_OMNIBOX_EXTENSION_APP_SELECTED] = "common\\omnibox_extension_app_selected.png";
     res_ids[IDR_OMNIBOX_HTTP_SELECTED] = "common\\omnibox_http_selected.png";
     res_ids[IDR_OMNIBOX_HTTP] = "common\\omnibox_http.png";
     
     /*
     <structure type="chrome_scaled_image" name="IDR_OMNIBOX_POPUP_BORDER_BOTTOM" file="common/omnibox_popup_border_bottom.png" />
     <structure type="chrome_scaled_image" name="IDR_OMNIBOX_POPUP_BORDER_BOTTOM_LEFT" file="common/omnibox_popup_border_bottom_left.png" />
     <structure type="chrome_scaled_image" name="IDR_OMNIBOX_POPUP_BORDER_BOTTOM_RIGHT" file="common/omnibox_popup_border_bottom_right.png" />
     <structure type="chrome_scaled_image" name="IDR_OMNIBOX_POPUP_BORDER_CENTER" file="common/omnibox_popup_border_center.png" />
     <structure type="chrome_scaled_image" name="IDR_OMNIBOX_POPUP_BORDER_LEFT" file="common/omnibox_popup_border_left.png" />
     <structure type="chrome_scaled_image" name="IDR_OMNIBOX_POPUP_BORDER_RIGHT" file="common/omnibox_popup_border_right.png" />
     <structure type="chrome_scaled_image" name="IDR_OMNIBOX_POPUP_BORDER_TOP" file="common/omnibox_popup_border_top.png" />
     <structure type="chrome_scaled_image" name="IDR_OMNIBOX_POPUP_BORDER_TOP_LEFT" file="common/omnibox_popup_border_top_left.png" />
     <structure type="chrome_scaled_image" name="IDR_OMNIBOX_POPUP_BORDER_TOP_RIGHT" file="common/omnibox_popup_border_top_right.png" />
     <structure type="chrome_scaled_image" name="IDR_OMNIBOX_SEARCH" file="common/omnibox_search.png" />
     <structure type="chrome_scaled_image" name="IDR_OMNIBOX_SEARCH_DARK" file="common/omnibox_search_dark.png" />
     <structure type="chrome_scaled_image" name="IDR_OMNIBOX_SEARCH_SECURED" file="common/omnibox_search_secured.png" />
     <structure type="chrome_scaled_image" name="IDR_OMNIBOX_SEARCH_SELECTED" file="common/omnibox_search_selected.png" />
     <structure type="chrome_scaled_image" name="IDR_OMNIBOX_SELECTED_KEYWORD_BUBBLE_BOTTOM" file="common/omnibox_selected_keyword_bubble_bottom.png" />
     <structure type="chrome_scaled_image" name="IDR_OMNIBOX_SELECTED_KEYWORD_BUBBLE_BOTTOM_LEFT" file="common/omnibox_selected_keyword_bubble_bottom_left.png" />
     <structure type="chrome_scaled_image" name="IDR_OMNIBOX_SELECTED_KEYWORD_BUBBLE_BOTTOM_RIGHT" file="common/omnibox_selected_keyword_bubble_bottom_right.png" />
     <structure type="chrome_scaled_image" name="IDR_OMNIBOX_SELECTED_KEYWORD_BUBBLE_CENTER" file="common/omnibox_selected_keyword_bubble_center.png" />
     <structure type="chrome_scaled_image" name="IDR_OMNIBOX_SELECTED_KEYWORD_BUBBLE_LEFT" file="common/omnibox_selected_keyword_bubble_left.png" />
     <structure type="chrome_scaled_image" name="IDR_OMNIBOX_SELECTED_KEYWORD_BUBBLE_RIGHT" file="common/omnibox_selected_keyword_bubble_right.png" />
     <structure type="chrome_scaled_image" name="IDR_OMNIBOX_SELECTED_KEYWORD_BUBBLE_TOP" file="common/omnibox_selected_keyword_bubble_top.png" />
     <structure type="chrome_scaled_image" name="IDR_OMNIBOX_SELECTED_KEYWORD_BUBBLE_TOP_LEFT" file="common/omnibox_selected_keyword_bubble_top_left.png" />
     <structure type="chrome_scaled_image" name="IDR_OMNIBOX_SELECTED_KEYWORD_BUBBLE_TOP_RIGHT" file="common/omnibox_selected_keyword_bubble_top_right.png" />
     <structure type="chrome_scaled_image" name="IDR_OMNIBOX_STAR" file="common/omnibox_star.png" />
     <structure type="chrome_scaled_image" name="IDR_OMNIBOX_STAR_DARK" file="common/omnibox_star_dark.png" />
     <structure type="chrome_scaled_image" name="IDR_OMNIBOX_STAR_SELECTED" file="common/omnibox_star_selected.png" />
     <structure type="chrome_scaled_image" name="IDR_OMNIBOX_TTS" file="common/omnibox_tts.png" />
     <structure type="chrome_scaled_image" name="IDR_OMNIBOX_TTS_DARK" file="common/omnibox_tts_dark.png" />
     <structure type="chrome_scaled_image" name="IDR_OMNIBOX_TTS_SELECTED" file="common/omnibox_tts_selected.png" />
     */

     //open_pdf_in_reader_view
     res_ids[IDR_OMNIBOX_PDF_ICON] = "pdf.png";

     //star_view
     res_ids[IDR_STAR_LIT] = "star_lit.png";
     res_ids[IDR_STAR] = "star.png";
     
     // http
     res_ids[IDR_LOCATION_BAR_HTTP] = "common\\location_bar_http.png";

     // browser_view
     res_ids[IDR_OTR_ICON_FULLSCREEN] = "common\\otr_icon.png";
     res_ids[IDR_OTR_ICON] = "common\\otr_icon.png";
     res_ids[IDR_GUEST_ICON] = "common\\guest_icon.png";

     // glass_browser_frame_view.cc
     res_ids[IDR_THEME_TOOLBAR] = "theme_toolbar_default.png";
     res_ids[IDR_CONTENT_TOP_CENTER] = "content_top_center.png";
     res_ids[IDR_CONTENT_TOP_LEFT_CORNER] = "content_top_left_corner.png";
     res_ids[IDR_CONTENT_TOP_LEFT_CORNER_MASK] = "content_top_left_corner_mask.png";
     res_ids[IDR_CONTENT_TOP_RIGHT_CORNER] = "content_top_right_corner.png";
     res_ids[IDR_CONTENT_TOP_RIGHT_CORNER_MASK] = "content_top_right_corner_mask.png";
     res_ids[IDR_CONTENT_RIGHT_SIDE] = "content_right_side.png";
     res_ids[IDR_CONTENT_LEFT_SIDE] = "content_left_side.png";
     res_ids[IDR_CONTENT_BOTTOM_CENTER] = "content_bottom_center.png";
     res_ids[IDR_CONTENT_BOTTOM_LEFT_CORNER] = "content_bottom_left_corner.png";
     res_ids[IDR_CONTENT_BOTTOM_RIGHT_CORNER] = "content_bottom_right_corner.png";

     // whrench_icon_painter.cc
     res_ids[IDR_TOOLBAR_BEZEL_HOVER] = "common\\browser_toolbar_bezel_hover.png";
     res_ids[IDR_TOOLBAR_BEZEL_PRESSED] = "common\\browser_toolbar_bezel_pressed.png";
     res_ids[IDR_TOOLS_BAR] = "common\\browser_tools_bar.png";
     res_ids[IDR_TOOLS_BAR_LOW] = "common\\browser_tools_bar_low.png";
     res_ids[IDR_TOOLS_BAR_MEDIUM] = "common\\browser_tools_bar_medium.png";
     res_ids[IDR_TOOLS_BAR_HIGH] = "common\\browser_tools_bar_high.png";

     // textbutton
     res_ids[IDR_TEXTBUTTON_HOVER_BOTTOM] = "common\\textbutton_hover_bottom.png";
     res_ids[IDR_TEXTBUTTON_HOVER_BOTTOM_LEFT] = "common\\textbutton_hover_bottom_left.png";
     res_ids[IDR_TEXTBUTTON_HOVER_BOTTOM_RIGHT] = "common\\textbutton_hover_bottom_right.png";
     res_ids[IDR_TEXTBUTTON_HOVER_CENTER] = "common\\textbutton_hover_center.png";
     res_ids[IDR_TEXTBUTTON_HOVER_LEFT] = "common\\textbutton_hover_left.png";
     res_ids[IDR_TEXTBUTTON_HOVER_RIGHT] = "common\\textbutton_hover_right.png";
     res_ids[IDR_TEXTBUTTON_HOVER_TOP] = "common\\textbutton_hover_top.png";
     res_ids[IDR_TEXTBUTTON_HOVER_TOP_LEFT] = "common\\textbutton_hover_top_left.png";
     res_ids[IDR_TEXTBUTTON_HOVER_TOP_RIGHT] = "common\\textbutton_hover_top_right.png";
     res_ids[IDR_TEXTBUTTON_PRESSED_BOTTOM] = "common\\textbutton_pressed_bottom.png";
     res_ids[IDR_TEXTBUTTON_PRESSED_BOTTOM_LEFT] = "common\\textbutton_pressed_bottom_left.png";
     res_ids[IDR_TEXTBUTTON_PRESSED_BOTTOM_RIGHT] = "common\\textbutton_pressed_bottom_right.png";
     res_ids[IDR_TEXTBUTTON_PRESSED_CENTER] = "common\\textbutton_pressed_center.png";
     res_ids[IDR_TEXTBUTTON_PRESSED_LEFT] = "common\\textbutton_pressed_left.png";
     res_ids[IDR_TEXTBUTTON_PRESSED_RIGHT] = "common\\textbutton_pressed_right.png";
     res_ids[IDR_TEXTBUTTON_PRESSED_TOP] = "common\\textbutton_pressed_top.png";
     res_ids[IDR_TEXTBUTTON_PRESSED_TOP_LEFT] = "common\\textbutton_pressed_top_left.png";
     res_ids[IDR_TEXTBUTTON_PRESSED_TOP_RIGHT] = "common\\textbutton_pressed_top_right.png";

     res_ids[IDR_BUBBLE_B] = "bubble_bottom.png";
     res_ids[IDR_BUBBLE_BL] = "bubble_bottom_left.png";
     res_ids[IDR_BUBBLE_BR] = "bubble_bottom_right.png";
     res_ids[IDR_BUBBLE_B_ARROW] = "bubble_pointer_bottom.png";
     res_ids[IDR_BUBBLE_L] = "bubble_left.png";
     res_ids[IDR_BUBBLE_L_ARROW] = "bubble_pointer_left.png";
     res_ids[IDR_BUBBLE_R] = "bubble_right.png";
     res_ids[IDR_BUBBLE_R_ARROW] = "bubble_pointer_right.png";
     res_ids[IDR_BUBBLE_T] = "bubble_top.png";
     res_ids[IDR_BUBBLE_TL] = "bubble_top_left.png";
     res_ids[IDR_BUBBLE_TR] = "bubble_top_right.png";
     res_ids[IDR_BUBBLE_T_ARROW] = "bubble_pointer_top.png";
     res_ids[IDR_BUBBLE_SHADOW_B] = "bubble_shadow_bottom.png";
     res_ids[IDR_BUBBLE_SHADOW_BL] = "bubble_shadow_bottom_left.png";
     res_ids[IDR_BUBBLE_SHADOW_BR] = "bubble_shadow_bottom_right.png";
     res_ids[IDR_BUBBLE_SHADOW_L] = "bubble_shadow_left.png";
     res_ids[IDR_BUBBLE_SHADOW_R] = "bubble_shadow_right.png";
     res_ids[IDR_BUBBLE_SHADOW_T] = "bubble_shadow_top.png";
     res_ids[IDR_BUBBLE_SHADOW_TL] = "bubble_shadow_top_left.png";
     res_ids[IDR_BUBBLE_SHADOW_TR] = "bubble_shadow_top_right.png";
     res_ids[IDR_BUTTON_DISABLED] = "common\\button_inactive.png";
     res_ids[IDR_BUTTON_FOCUSED_HOVER] = "common\\button_focused_hover.png";
     res_ids[IDR_BUTTON_FOCUSED_NORMAL] = "common\\button_focused.png";
     res_ids[IDR_BUTTON_FOCUSED_PRESSED] = "common\\button_focused_pressed.png";
     res_ids[IDR_BUTTON_HOVER] = "common\\button_hover.png";
     res_ids[IDR_BUTTON_NORMAL] = "common\\button.png";
     res_ids[IDR_BUTTON_PRESSED] = "common\\button_pressed.png";
     res_ids[IDR_BLUE_BUTTON_DISABLED] = "common\\blue_button_inactive.png";
     res_ids[IDR_BLUE_BUTTON_FOCUSED_HOVER] = "common\\blue_button_focused_hover.png";
     res_ids[IDR_BLUE_BUTTON_FOCUSED_NORMAL] = "common\\blue_button_focused.png";
     res_ids[IDR_BLUE_BUTTON_FOCUSED_PRESSED] = "common\\blue_button_focused_pressed.png";
     res_ids[IDR_BLUE_BUTTON_HOVER] = "common\\blue_button_hover.png";
     res_ids[IDR_BLUE_BUTTON_NORMAL] = "common\\blue_button.png";
     res_ids[IDR_BLUE_BUTTON_PRESSED] = "common\\blue_button_pressed.png";
     res_ids[IDR_CHECKBOX] = "common\\checkbox.png";
     res_ids[IDR_CHECKBOX_CHECKED] = "common\\checkbox_checked.png";
     res_ids[IDR_CHECKBOX_CHECKED_DISABLED] = "common\\checkbox_checked_inactive.png";
     res_ids[IDR_CHECKBOX_CHECKED_HOVER] = "common\\checkbox_checked_hover.png";
     res_ids[IDR_CHECKBOX_CHECKED_PRESSED] = "common\\checkbox_checked_pressed.png";
     res_ids[IDR_CHECKBOX_DISABLED] = "common\\checkbox_inactive.png";
     res_ids[IDR_CHECKBOX_FOCUSED] = "common\\checkbox_focused.png";
     res_ids[IDR_CHECKBOX_FOCUSED_CHECKED] = "common\\checkbox_focused_checked.png";
     res_ids[IDR_CHECKBOX_FOCUSED_CHECKED_HOVER] = "common\\checkbox_focused_checked_hover.png";
     res_ids[IDR_CHECKBOX_FOCUSED_CHECKED_PRESSED] = "common\\checkbox_focused_checked_pressed.png";
     res_ids[IDR_CHECKBOX_FOCUSED_HOVER] = "common\\checkbox_focused_hover.png";
     res_ids[IDR_CHECKBOX_FOCUSED_PRESSED] = "common\\checkbox_focused_pressed.png";
     res_ids[IDR_CHECKBOX_HOVER] = "common\\checkbox_hover.png";
     res_ids[IDR_CHECKBOX_PRESSED] = "common\\checkbox_pressed.png";

     res_ids[IDR_MENU_CHECK] = "cros\\menu_check.png";
     res_ids[IDR_MENU_CHECK_CHECKED] = "common\\menu_check.png";
     res_ids[IDR_MENU_CHECK_CHECKED_DARK_BACKGROUND] = "common\\menu_check_white.png";
     res_ids[IDR_MENU_RADIO_EMPTY] = "common\\menu_radio_empty.png";
     res_ids[IDR_MENU_RADIO_SELECTED] = "common\\menu_radio_selected.png";
     res_ids[IDR_SLIDER_ACTIVE_LEFT] = "slider_left_active.png";
     res_ids[IDR_SLIDER_ACTIVE_RIGHT] = "slider_right_active.png";
     res_ids[IDR_SLIDER_ACTIVE_CENTER] = "slider_center_active.png";
     res_ids[IDR_SLIDER_DISABLED_LEFT] = "slider_left_disabled.png";
     res_ids[IDR_SLIDER_DISABLED_RIGHT] = "slider_right_disabled.png";
     res_ids[IDR_SLIDER_DISABLED_CENTER] = "slider_center_disabled.png";
     res_ids[IDR_SLIDER_PRESSED_LEFT] = "slider_left_pressed.png";
     res_ids[IDR_SLIDER_PRESSED_RIGHT] = "slider_right_pressed.png";
     res_ids[IDR_SLIDER_PRESSED_CENTER] = "slider_center_pressed.png";
     res_ids[IDR_SLIDER_ACTIVE_THUMB] = "slider_thumb.png";
     res_ids[IDR_SLIDER_DISABLED_THUMB] = "slider_thumb_disabled.png";

     // tabs
     res_ids[IDR_CLOSE_1] = "common\\close_1.png";
     res_ids[IDR_CLOSE_1_H] = "common\\close_1_hover.png";
     res_ids[IDR_CLOSE_1_MASK] = "common\\close_1_mask.png";
     res_ids[IDR_CLOSE_1_P] = "common\\close_1_pressed.png";

     res_ids[IDR_THEME_FRAME] = "theme_frame.png";
     res_ids[IDR_THEME_FRAME_INACTIVE] = "theme_frame_inactive.png";
     res_ids[IDR_THEME_FRAME_INCOGNITO] = "theme_frame_incognito.png";
     res_ids[IDR_THEME_FRAME_INCOGNITO_INACTIVE] = "theme_frame_incognito_inactive.png";

     // bookmarkbar view
     res_ids[IDR_BOOKMARK_BAR_CHEVRONS] = "common\\chevron.png";
     res_ids[IDR_BOOKMARK_BAR_FOLDER] = "common\\bookmark_bar_folder.png";
     res_ids[IDR_BOOKMARK_BAR_APPS_SHORTCUT] = "common\\apps_bookmark_bar_icon.png";
     res_ids[IDR_THROBBER] = "throbber.png";

     res_ids[IDR_MINIMIZE] = "minimize.png";
     res_ids[IDR_MINIMIZE_H] = "minimize_hover.png";
     res_ids[IDR_MINIMIZE_P] = "minimize_pressed.png";
     res_ids[IDR_MINIMIZE_BUTTON_MASK] = "common\\minimize_button_mask.png";
     res_ids[IDR_MAXIMIZE_BUTTON_MASK] = "common\\maximize_button_mask.png";
     res_ids[IDR_MAXIMIZE] = "maximize.png";
     res_ids[IDR_MAXIMIZE_H] = "maximize_hover.png";
     res_ids[IDR_MAXIMIZE_P] = "maximize_pressed.png";
     res_ids[IDR_RESTORE] = "restore.png";
     res_ids[IDR_RESTORE_H] = "restore_hover.png";
     res_ids[IDR_RESTORE_P] = "restore_pressed.png";
     res_ids[IDR_CLOSE] = "close.png";
     res_ids[IDR_CLOSE_2] = "close_2.png";
     res_ids[IDR_CLOSE_2_H] = "close_2_hover.png";
     res_ids[IDR_CLOSE_2_MASK] = "close_2_mask.png";
     res_ids[IDR_CLOSE_2_P] = "close_2_pressed.png";
     res_ids[IDR_CLOSE_DIALOG] = "close_dialog.png";
     res_ids[IDR_CLOSE_DIALOG_H] = "close_dialog_hover.png";
     res_ids[IDR_CLOSE_DIALOG_P] = "close_dialog_pressed.png";
     res_ids[IDR_CLOSE_H] = "close_hover.png";
     res_ids[IDR_CLOSE_P] = "close_pressed.png";
     res_ids[IDR_RESTORE_BUTTON_MASK] = "common\\restore_button_mask.png";
     res_ids[IDR_CLOSE_BUTTON_MASK] = "common\\close_button_mask.png";
     res_ids[IDR_THEME_WINDOW_CONTROL_BACKGROUND] = "notused.png";
     res_ids[IDR_APP_TOP_CENTER] = "app_top_center.png";
     res_ids[IDR_APP_TOP_LEFT] = "app_top_left.png";
     res_ids[IDR_APP_TOP_RIGHT] = "app_top_right.png";

     res_ids[IDR_THEME_FRAME_WIN] = "theme_frame.png";
     res_ids[IDR_THEME_FRAME_INACTIVE_WIN] = "theme_frame_inactive.png";
     res_ids[IDR_THEME_FRAME_INCOGNITO_WIN] = "theme_frame_incognito.png";
     res_ids[IDR_THEME_FRAME_INCOGNITO_INACTIVE_WIN] = "theme_frame_incognito_inactive.png";
     res_ids[IDR_THEME_FRAME_OVERLAY] = "notused.png";
     res_ids[IDR_THEME_FRAME_OVERLAY_INACTIVE] = "notused.png";
     res_ids[IDR_FULLSCREEN_MENU_BUTTON] = "fullscreen_menu_button.png";

     res_ids[IDR_WINDOW_BOTTOM_CENTER] = "common\\window_bottom_center.png";
     res_ids[IDR_WINDOW_BOTTOM_LEFT_CORNER] = "common\\window_bottom_left_corner.png";
     res_ids[IDR_WINDOW_BOTTOM_RIGHT_CORNER] = "common\\window_bottom_right_corner.png";
     res_ids[IDR_WINDOW_LEFT_SIDE] = "common\\window_left_side.png";
     res_ids[IDR_WINDOW_RIGHT_SIDE] = "common\\window_right_side.png";
     res_ids[IDR_WINDOW_TOP_CENTER] = "common\\window_top_center.png";
     res_ids[IDR_WINDOW_TOP_LEFT_CORNER] = "common\\window_top_left_corner.png";
     res_ids[IDR_WINDOW_TOP_RIGHT_CORNER] = "common\\window_top_right_corner.png";
     res_ids[IDR_WINDOW_BUBBLE_SHADOW_BIG_BOTTOM] = "common\\window_bubble_shadow_big_bottom.png";
     res_ids[IDR_WINDOW_BUBBLE_SHADOW_BIG_BOTTOM_LEFT] = "common\\window_bubble_shadow_big_bottom_left.png";
     res_ids[IDR_WINDOW_BUBBLE_SHADOW_BIG_BOTTOM_RIGHT] = "common\\window_bubble_shadow_big_bottom_right.png";
     res_ids[IDR_WINDOW_BUBBLE_SHADOW_BIG_LEFT] = "common\\window_bubble_shadow_big_left.png";
     res_ids[IDR_WINDOW_BUBBLE_SHADOW_BIG_RIGHT] = "common\\window_bubble_shadow_big_right.png";
     res_ids[IDR_WINDOW_BUBBLE_SHADOW_BIG_TOP] = "common\\window_bubble_shadow_big_top.png";
     res_ids[IDR_WINDOW_BUBBLE_SHADOW_BIG_TOP_LEFT] = "common\\window_bubble_shadow_big_top_left.png";
     res_ids[IDR_WINDOW_BUBBLE_SHADOW_BIG_TOP_RIGHT] = "common\\window_bubble_shadow_big_top_right.png";
     res_ids[IDR_WINDOW_BUBBLE_SHADOW_SPIKE_BIG_BOTTOM] = "common\\window_bubble_shadow_spike_big_bottom.png";
     res_ids[IDR_WINDOW_BUBBLE_SHADOW_SPIKE_BIG_LEFT] = "common\\window_bubble_shadow_spike_big_left.png";
     res_ids[IDR_WINDOW_BUBBLE_SHADOW_SPIKE_BIG_RIGHT] = "common\\window_bubble_shadow_spike_big_right.png";
     res_ids[IDR_WINDOW_BUBBLE_SHADOW_SPIKE_BIG_TOP] = "common\\window_bubble_shadow_spike_big_top.png";
     res_ids[IDR_WINDOW_BUBBLE_SHADOW_SMALL_BOTTOM] = "common\\window_bubble_shadow_small_bottom.png";
     res_ids[IDR_WINDOW_BUBBLE_SHADOW_SMALL_BOTTOM_LEFT] = "common\\window_bubble_shadow_small_bottom_left.png";
     res_ids[IDR_WINDOW_BUBBLE_SHADOW_SMALL_BOTTOM_RIGHT] = "common\\window_bubble_shadow_small_bottom_right.png";
     res_ids[IDR_WINDOW_BUBBLE_SHADOW_SMALL_LEFT] = "common\\window_bubble_shadow_small_left.png";
     res_ids[IDR_WINDOW_BUBBLE_SHADOW_SMALL_RIGHT] = "common\\window_bubble_shadow_small_right.png";
     res_ids[IDR_WINDOW_BUBBLE_SHADOW_SMALL_TOP] = "common\\window_bubble_shadow_small_top.png";
     res_ids[IDR_WINDOW_BUBBLE_SHADOW_SMALL_TOP_LEFT] = "common\\window_bubble_shadow_small_top_left.png";
     res_ids[IDR_WINDOW_BUBBLE_SHADOW_SMALL_TOP_RIGHT] = "common\\window_bubble_shadow_small_top_right.png";
     res_ids[IDR_WINDOW_BUBBLE_SHADOW_SPIKE_SMALL_BOTTOM] = "common\\window_bubble_shadow_spike_small_bottom.png";
     res_ids[IDR_WINDOW_BUBBLE_SHADOW_SPIKE_SMALL_LEFT] = "common\\window_bubble_shadow_spike_small_left.png";
     res_ids[IDR_WINDOW_BUBBLE_SHADOW_SPIKE_SMALL_RIGHT] = "common\\window_bubble_shadow_spike_small_right.png";
     res_ids[IDR_WINDOW_BUBBLE_SHADOW_SPIKE_SMALL_TOP] = "common\\window_bubble_shadow_spike_small_top.png";
  }

	base::FilePath ThemeRes::GetPathForResourcePack(const base::FilePath& pack_path,
	  ui::ScaleFactor scale_factor) {
      return base::FilePath();
	}

  base::FilePath ThemeRes::GetPathForLocalePack(const base::FilePath& pack_path,
	  const std::string& locale) {
      return base::FilePath();
  }

  gfx::Image ThemeRes::GetImageNamed(int resource_id) {
    std::map<int ,std::string>::iterator itr =  res_ids.find(resource_id);
    if(itr != res_ids.end()) {
      base::FilePath path;
      PathService::Get(base::DIR_SOURCE_ROOT, &path);
      //common_theme
      path = path.Append(L"chrome\\theme\\default_100_percent");
      path = path.Append(base::SysMultiByteToWide(itr->second, CP_ACP));
      base::MemoryMappedFile memoryfile;
      memoryfile.Initialize(path);
      if(memoryfile.IsValid()) {
        SkBitmap bitmap;
        bool fell_back_to_1x = false;
        if (ResourceBundle::DecodePNG(memoryfile.data(), memoryfile.length(), &bitmap, &fell_back_to_1x)) {
          return gfx::Image::CreateFrom1xBitmap(bitmap);;
        }
      } else {
        base::FilePath path_commone;
        PathService::Get(base::DIR_SOURCE_ROOT, &path_commone);
        //common_theme
        path_commone = path_commone.Append(L"chrome\\common_theme\\default_100_percent");
        path_commone = path_commone.Append(base::SysMultiByteToWide(itr->second, CP_ACP));
        base::MemoryMappedFile memoryfile;
        memoryfile.Initialize(path_commone);
        if(memoryfile.IsValid()) {
          SkBitmap bitmap;
          bool fell_back_to_1x = false;
          if (ResourceBundle::DecodePNG(memoryfile.data(), memoryfile.length(), &bitmap, &fell_back_to_1x)) {
            if(fell_back_to_1x)
              int i = 0;
            return gfx::Image::CreateFrom1xBitmap(bitmap);
          }
        }
      }
    }

    return gfx::Image();
  }

	gfx::Image ThemeRes::GetNativeImageNamed(int resource_id, ui::ResourceBundle::ImageRTL rtl) {
    return gfx::Image();
  }

  SkColor BrowserThemeProvider::GetColor(int id) const {
    return ThemeProperties::GetDefaultColor(id);
  }

  bool ThemeRes::GetLocalizedString(int message_id, string16* value) {
    std::map<int ,base::string16>::iterator itr =  string_ids.find(message_id);
    if(itr != string_ids.end()) {
      *value = itr->second;
      return true;
    }

    return false;
  }
}