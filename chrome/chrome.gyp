# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
{
  'variables': {
    'chromium_code': 1,
    'allocator_target': '../base/allocator/allocator.gyp:allocator',
  },
  'target_defaults': {
    'conditions': [
      ['use_aura==1', {
        'sources/': [ ['exclude', '_win\\.(h|cc)$'] ],
      }],
      ['OS!="linux" or chromeos==1', {
        'sources/': [ ['exclude', '_linux\\.(h|cc)$'] ],
      }],
    ],
  },
  'targets': [
    {
      'target_name': 'chrome',
      'type': 'executable',
      'dependencies': [
        '../base/base.gyp:base',
        '../base/base.gyp:base_prefs',
        '../skia/skia.gyp:skia',
        '../win8/win8.gyp:win8_util',
        '../ui/events/events.gyp:events',
        '../ui/gfx/gfx.gyp:gfx',
        '../ui/ui.gyp:ui',
        '../ui/ui.gyp:ui_resources',
        '../ui/views/views.gyp:views',
        '../ui/ui.gyp:shell_dialogs',
        '../ipc/ipc.gyp:ipc',
      ],
      'include_dirs': [
        '..',
      ],
      'sources': [
        'browser_main.cc',
        'theme_resources_id.h',
        'theme_strings_id.h',
        'theme_resources_browser.h',
        'theme_resources_browser.cc',
        'chrome_export.h',
        'app/chrome_exe.rc',
        'app/chrome_exe_version.rc',
        'app/chrome_exe_resource.h',
        '../content/public/app/content_main.h',
        '../content/public/app/browser_main_parts.h',
        '../content/public/app/content_browser_client.h',
        '../content/public/app/content_client.h',
        '../content/public/app/content_main_runner.h',
        '../content/public/app/content_main.h',
        '../content/public/app/content_main_delegate.h',
        '../content/public/app/startup_helper_win.h',
        '../content/app/content_main.cc',
        '../content/app/browser_main_parts.cc',
        '../content/app/content_browser_client.cc',
        '../content/app/content_client.cc',
        '../content/app/content_main_runner.cc',
        '../content/app/content_main.cc',
        '../content/app/content_main_delegate.cc',
        '../content/app/startup_helper_win.cc',
        'chrome_content/chrome_content_main_delegate.h',
        'chrome_content/chrome_content_main_delegate.cc',
        'chrome_content/chrome_content_main_delegate.h',
        'chrome_content/chrome_content_content_browser_client.h',
        'chrome_content/chrome_content_content_browser_client.cc',
        'chrome_content/chrome_content_browser_main_parts.h',
        'chrome_content/chrome_content_browser_main_parts.cc',
        'ui/browser.h',
        'ui/browser.cc',
		'ui/invalidate_type.h',
        'ui/browser_finder.h',
        'ui/browser_finder.cc',
        'ui/browser_iterator.h',
        'ui/browser_iterator.cc',
        'ui/browser_list.h',
        'ui/browser_list.cc',
        'ui/browser_tab_contents.h',
        'ui/browser_tab_contents.cc',
        'ui/browser_tab_strip_model_delegate.cc',
        'ui/browser_tab_strip_model_delegate.h',
        'ui/browser_tabrestore.cc',
        'ui/browser_tabrestore.h',
        'ui/browser_tabstrip.cc',
        'ui/browser_tabstrip.h',
        'ui/browser_content_setting_bubble_model_delegate.cc',
        'ui/browser_content_setting_bubble_model_delegate.h',
        'ui/browser_instant_controller.cc',
        'ui/browser_instant_controller.h',
        'ui/browser_tab_restore_service_delegate.cc',
        'ui/browser_tab_restore_service_delegate.h',
        'ui/browser_toolbar_model_delegate.cc',
        'ui/browser_toolbar_model_delegate.h',
        'ui/browser_win.cc',
        'ui/browser_window.h',
        'ui/browser_window_state.cc',
        'ui/browser_window_state.h',
        'ui/chrome_web_modal_dialog_manager_delegate.cc',
        'ui/chrome_web_modal_dialog_manager_delegate.h',
        'ui/host_desktop.cc',
        'ui/host_desktop.h',
        'ui/browser_list.h',
        'ui/browser_list.cc',
        'ui/browser_list_observer.h',
        'ui/status_bubble.h',
        'ui/bookmarks/bookmark_bar.h',
        'ui/bookmarks/bookmark_bar_constants.h',
        'ui/bookmarks/bookmark_bar_instructions_delegate.h',
        'ui/bookmarks/bookmark_browsertest.cc',
        'ui/bookmarks/bookmark_bubble_delegate.h',
        'ui/bookmarks/bookmark_bubble_sign_in_delegate.cc',
        'ui/bookmarks/bookmark_bubble_sign_in_delegate.h',
        'ui/bookmarks/bookmark_context_menu_controller.cc',
        'ui/bookmarks/bookmark_context_menu_controller.h',
        'ui/bookmarks/bookmark_drag_drop.cc',
        'ui/bookmarks/bookmark_drag_drop.h',
        'ui/bookmarks/bookmark_editor.cc',
        'ui/bookmarks/bookmark_editor.h',
        'ui/bookmarks/bookmark_prompt_controller.cc',
        'ui/bookmarks/bookmark_prompt_controller.h',
        'ui/bookmarks/bookmark_tab_helper.cc',
        'ui/bookmarks/bookmark_tab_helper.h',
        'ui/bookmarks/bookmark_tab_helper_delegate.cc',
        'ui/bookmarks/bookmark_tab_helper_delegate.h',
        'ui/bookmarks/bookmark_utils.cc',
        'ui/bookmarks/bookmark_utils.h',
        'ui/bookmarks/recently_used_folders_combo_model.cc',
        'ui/bookmarks/recently_used_folders_combo_model.h',
        'ui/find_bar/find_bar.h',
        'ui/find_bar/find_bar_controller.cc',
        'ui/find_bar/find_bar_controller.h',
        'ui/find_bar/find_bar_state.h',
        'ui/find_bar/find_bar_state_factory.cc',
        'ui/find_bar/find_bar_state_factory.h',
        'ui/find_bar/find_notification_details.h',
        'ui/find_bar/find_tab_helper.cc',
        'ui/find_bar/find_tab_helper.h',
        'ui/tab_contents/chrome_web_contents_view_delegate.h',
        'ui/tab_contents/core_tab_helper.cc',
        'ui/tab_contents/core_tab_helper.h',
        'ui/tab_contents/core_tab_helper_delegate.cc',
        'ui/tab_contents/core_tab_helper_delegate.h',
        'ui/tab_contents/tab_contents_iterator.cc',
        'ui/tab_contents/tab_contents_iterator.h',
        'ui/tabs/dock_info.cc',
        'ui/tabs/dock_info.h',
        'ui/tabs/hover_tab_selector.cc',
        'ui/tabs/hover_tab_selector.h',
        'ui/tabs/pinned_tab_codec.cc',
        'ui/tabs/pinned_tab_codec.h',
        'ui/tabs/pinned_tab_service.cc',
        'ui/tabs/pinned_tab_service.h',
        'ui/tabs/pinned_tab_service_factory.cc',
        'ui/tabs/pinned_tab_service_factory.h',
        'ui/tabs/tab_menu_model.cc',
        'ui/tabs/tab_menu_model.h',
        'ui/tabs/tab_resources.cc',
        'ui/tabs/tab_resources.h',
        'ui/tabs/tab_strip_layout_type.h',
        'ui/tabs/tab_strip_layout_type_prefs.cc',
        'ui/tabs/tab_strip_layout_type_prefs.h',
        'ui/tabs/tab_strip_model.cc',
        'ui/tabs/tab_strip_model.h',
        'ui/tabs/tab_strip_model_delegate.h',
        'ui/tabs/tab_strip_model_observer.cc',
        'ui/tabs/tab_strip_model_observer.h',
        'ui/tabs/tab_strip_model_order_controller.cc',
        'ui/tabs/tab_strip_model_order_controller.h',
        'ui/tabs/tab_strip_model_utils.cc',
        'ui/tabs/tab_strip_model_utils.h',
        'ui/tabs/tab_utils.cc',
        'ui/tabs/tab_utils.h',
        'ui/tabs/test_tab_strip_model_delegate.cc',
        'ui/tabs/test_tab_strip_model_delegate.h',
        'ui/toolbar/back_forward_menu_model.cc',
        'ui/toolbar/back_forward_menu_model.h',
        'ui/toolbar/bookmark_sub_menu_model.cc',
        'ui/toolbar/bookmark_sub_menu_model.h',
        'ui/toolbar/encoding_menu_controller.cc',
        'ui/toolbar/encoding_menu_controller.h',
        'ui/toolbar/recent_tabs_sub_menu_model.cc',
        'ui/toolbar/recent_tabs_sub_menu_model.h',
        'ui/toolbar/toolbar_model.h',
        'ui/toolbar/toolbar_model_delegate.h',
        'ui/toolbar/toolbar_model_impl.cc',
        'ui/toolbar/toolbar_model_impl.h',
        'ui/toolbar/toolbar_model_security_level_list.h',
        'ui/toolbar/wrench_icon_painter.cc',
        'ui/toolbar/wrench_icon_painter.h',
        'ui/toolbar/wrench_menu_model.cc',
        'ui/toolbar/wrench_menu_model.h',
        'ui/zoom/zoom_controller.cc',
        'ui/zoom/zoom_controller.h',
        'ui/zoom/zoom_observer.h',
        'ui/omnibox/alternate_nav_infobar_delegate.h',
        'ui/omnibox/alternate_nav_infobar_delegate.cc',
        'ui/omnibox/autocomplete_match.cc',
		'ui/omnibox/autocomplete_match.h',
		'ui/omnibox/autocomplete_match_type.cc',
		'ui/omnibox/autocomplete_match_type.h',
		'ui/omnibox/autocomplete_provider.cc',
		'ui/omnibox/autocomplete_provider.h',
		'ui/omnibox/autocomplete_provider_listener.h',
		'ui/omnibox/autocomplete_result.cc',
		'ui/omnibox/autocomplete_result.h',
		'ui/omnibox/autocomplete_input.h',
		'ui/omnibox/autocomplete_input.cc',
		'ui/omnibox/autocomplete_controller_delegate.h',
		'ui/omnibox/autocomplete_controller.h',
		'ui/omnibox/autocomplete_controller.cc',
        'ui/omnibox/location_bar_util.h',
        'ui/omnibox/location_bar_util.cc',
        'ui/omnibox/omnibox_controller.cc',
        'ui/omnibox/omnibox_controller.h',
        'ui/omnibox/omnibox_edit_controller.h',
        'ui/omnibox/omnibox_edit_controller.cc',
        'ui/omnibox/omnibox_edit_model.h',
        'ui/omnibox/omnibox_edit_model.cc',
        'ui/omnibox/omnibox_navigation_observer.h',
        'ui/omnibox/omnibox_navigation_observer.cc',
        'ui/omnibox/omnibox_popup_model.h',
        'ui/omnibox/omnibox_popup_model.cc',
        'ui/omnibox/omnibox_view.h',
        'ui/omnibox/omnibox_view.cc',
        'ui/omnibox/omnibox_popup_view.h',
        'ui/omnibox/omnibox_popup_model_observer.h',
		'ui/omnibox/location_bar.h',
        'ui/omnibox/omnibox_current_page_delegate_impl.h',
        'ui/omnibox/omnibox_current_page_delegate_impl.cc',
        'ui/omnibox/omnibox_current_page_delegate.h',
        'ui/omnibox/omnibox_focus_state.h',
        'ui/fullscreen/fullscreen_controller.h',
        'ui/fullscreen/fullscreen_controller.cc',
        'ui/fullscreen/fullscreen_exit_bubble.cc',
        'ui/fullscreen/fullscreen_exit_bubble.h',
        'ui/fullscreen/fullscreen_exit_bubble_type.cc',
        'ui/fullscreen/fullscreen_exit_bubble_type.h',
        'ui/views/frame/app_panel_browser_frame_view.h',
        'ui/views/frame/app_panel_browser_frame_view.cc',
        'ui/views/frame/browser_frame.h',
        'ui/views/frame/browser_frame.cc',
        'ui/views/frame/browser_frame_common_win.cc',
        'ui/views/frame/browser_frame_common_win.h',
        'ui/views/frame/browser_frame_win.cc',
        'ui/views/frame/browser_frame_win.h',
        'ui/views/frame/browser_non_client_frame_view.cc',
        'ui/views/frame/browser_non_client_frame_view.h',
        'ui/views/frame/browser_non_client_frame_view_factory_win.cc',
        'ui/views/frame/browser_root_view.cc',
        'ui/views/frame/browser_root_view.h',
        'ui/views/frame/browser_view.cc',
        'ui/views/frame/browser_view.h',
        'ui/views/frame/browser_view_layout.cc',
        'ui/views/frame/browser_view_layout.h',
        'ui/views/frame/browser_view_layout_delegate.h',
        'ui/views/frame/contents_container.cc',
        'ui/views/frame/contents_container.h',
        'ui/views/frame/glass_browser_frame_view.cc',
        'ui/views/frame/glass_browser_frame_view.h',
        'ui/views/frame/immersive_mode_controller.cc',
        'ui/views/frame/immersive_mode_controller.h',
        'ui/views/frame/immersive_mode_controller_factory.cc',
        'ui/views/frame/immersive_mode_controller_stub.cc',
        'ui/views/frame/immersive_mode_controller_stub.h',
        'ui/views/frame/minimize_button_metrics_win.cc',
        'ui/views/frame/minimize_button_metrics_win.h',
        'ui/views/frame/native_browser_frame.h',
        'ui/views/frame/native_browser_frame_factory.cc',
        'ui/views/frame/native_browser_frame_factory.h',
        'ui/views/frame/native_browser_frame_factory_win.cc',
        'ui/views/frame/opaque_browser_frame_view.cc',
        'ui/views/frame/opaque_browser_frame_view.h',
        'ui/views/frame/opaque_browser_frame_view_layout.cc',
        'ui/views/frame/opaque_browser_frame_view_layout.h',
        'ui/views/frame/opaque_browser_frame_view_layout_delegate.h',
        'ui/views/frame/opaque_browser_frame_view_platform_specific.cc',
        'ui/views/frame/opaque_browser_frame_view_platform_specific.h',
        'ui/views/frame/popup_non_client_frame_view.cc',
        'ui/views/frame/popup_non_client_frame_view.h',
        'ui/views/frame/scroll_end_effect_controller.h',
        'ui/views/frame/system_menu_insertion_delegate_win.cc',
        'ui/views/frame/system_menu_insertion_delegate_win.h',
        'ui/views/frame/system_menu_model_builder.cc',
        'ui/views/frame/system_menu_model_builder.h',
        'ui/views/frame/system_menu_model_delegate.cc',
        'ui/views/frame/system_menu_model_delegate.h',
        'ui/views/frame/taskbar_decorator.h',
        'ui/views/frame/taskbar_decorator_win.cc',
        'ui/views/frame/top_container_view.cc',
        'ui/views/frame/top_container_view.h',
        'ui/views/panels/panel_frame_view.cc',
        'ui/views/panels/panel_frame_view.h',
        'ui/views/panels/panel_stack_view.cc',
        'ui/views/panels/panel_stack_view.h',
        'ui/views/panels/panel_view.cc',
        'ui/views/panels/panel_view.h',
        'ui/views/panels/taskbar_window_thumbnailer_win.cc',
        'ui/views/panels/taskbar_window_thumbnailer_win.h',
        'ui/views/omnibox/omnibox_popup_contents_view.cc',
        'ui/views/omnibox/omnibox_popup_contents_view.h',
        'ui/views/omnibox/omnibox_result_view.cc',
        'ui/views/omnibox/omnibox_result_view.h',
        'ui/views/omnibox/omnibox_result_view_model.h',
        'ui/views/omnibox/omnibox_view_views.cc',
        'ui/views/omnibox/omnibox_view_views.h',
        'ui/views/omnibox/omnibox_view_win.cc',
        'ui/views/omnibox/omnibox_view_win.h',
        'ui/views/omnibox/omnibox_views.cc',
        'ui/views/omnibox/omnibox_views.h',
        'ui/views/omnibox/touch_omnibox_popup_contents_view.cc',
        'ui/views/omnibox/touch_omnibox_popup_contents_view.h',
        'ui/views/status_icons/status_icon_win.cc',
        'ui/views/status_icons/status_icon_win.h',
        'ui/views/status_icons/status_tray_win.cc',
        'ui/views/status_icons/status_tray_win.h',
        'ui/views/tab_contents/chrome_web_contents_view_delegate_views.cc',
        'ui/views/tab_contents/chrome_web_contents_view_delegate_views.h',
        'ui/views/tab_contents/web_drag_bookmark_handler_win.cc',
        'ui/views/tab_contents/web_drag_bookmark_handler_win.h',
        'ui/views/tabs/browser_tab_strip_controller.cc',
        'ui/views/tabs/browser_tab_strip_controller.h',
        'ui/views/tabs/dock_info_views.cc',
        'ui/views/tabs/dock_info_win.cc',
        'ui/views/tabs/dragged_tab_view.cc',
        'ui/views/tabs/dragged_tab_view.h',
        'ui/views/tabs/native_view_photobooth.h',
        'ui/views/tabs/native_view_photobooth_win.cc',
        'ui/views/tabs/native_view_photobooth_win.h',
        'ui/views/tabs/stacked_tab_strip_layout.cc',
        'ui/views/tabs/stacked_tab_strip_layout.h',
        'ui/views/tabs/tab.cc',
        'ui/views/tabs/tab.h',
        'ui/views/tabs/tab_controller.h',
        'ui/views/tabs/tab_drag_controller.cc',
        'ui/views/tabs/tab_drag_controller.h',
        'ui/views/tabs/tab_strip.cc',
        'ui/views/tabs/tab_strip.h',
        'ui/views/tabs/tab_renderer_data.h',
        'ui/views/tabs/tab_renderer_data.cc',
        'ui/views/tabs/tab_strip_controller.h',
        'ui/views/location_bar/content_setting_image_view.cc',
        'ui/views/location_bar/content_setting_image_view.h',
        'ui/views/location_bar/ev_bubble_view.cc',
        'ui/views/location_bar/ev_bubble_view.h',
        'ui/views/location_bar/generated_credit_card_view.cc',
        'ui/views/location_bar/generated_credit_card_view.h',
        'ui/views/location_bar/icon_label_bubble_view.cc',
        'ui/views/location_bar/icon_label_bubble_view.h',
        'ui/views/location_bar/keyword_hint_view.cc',
        'ui/views/location_bar/keyword_hint_view.h',
        'ui/views/location_bar/location_bar_decoration_view.cc',
        'ui/views/location_bar/location_bar_decoration_view.h',
        'ui/views/location_bar/location_bar_layout.cc',
        'ui/views/location_bar/location_bar_layout.h',
        'ui/views/location_bar/location_bar_view.cc',
        'ui/views/location_bar/location_bar_view.h',
        'ui/views/location_bar/location_icon_view.cc',
        'ui/views/location_bar/location_icon_view.h',
        'ui/views/location_bar/open_pdf_in_reader_view.cc',
        'ui/views/location_bar/open_pdf_in_reader_view.h',
        'ui/views/location_bar/page_action_image_view.cc',
        'ui/views/location_bar/page_action_image_view.h',
        'ui/views/location_bar/page_action_with_badge_view.cc',
        'ui/views/location_bar/page_action_with_badge_view.h',
        'ui/views/location_bar/page_info_helper.cc',
        'ui/views/location_bar/page_info_helper.h',
        'ui/views/location_bar/script_bubble_icon_view.cc',
        'ui/views/location_bar/script_bubble_icon_view.h',
        'ui/views/location_bar/selected_keyword_view.cc',
        'ui/views/location_bar/selected_keyword_view.h',
        'ui/views/location_bar/star_view.cc',
        'ui/views/location_bar/star_view.h',
        'ui/views/location_bar/zoom_bubble_view.cc',
        'ui/views/location_bar/zoom_bubble_view.h',
        'ui/views/location_bar/zoom_view.cc',
        'ui/views/location_bar/zoom_view.h',
        'ui/views/webview/webview.cc',
        'ui/views/webview/webview.h',
        'ui/views/bookmarks/bookmark_bar_view.cc',
        'ui/views/bookmarks/bookmark_bar_view.h',
        'ui/views/touch_uma/touch_uma.h',
        'ui/views/touch_uma/touch_uma.cc',
        'ui/views/theme_image_mapper.h',
        'ui/views/theme_image_mapper.cc',
        'ui/views/tab_icon_view.cc',
        'ui/views/tab_icon_view.h',
        'ui/views/accelerator_table.cc',
        'ui/views/accelerator_table.h',
        'ui/views/status_bubble_views.cc',
        'ui/views/status_bubble_views.h',
        'ui/views/detachable_toolbar_view.cc',
        'ui/views/detachable_toolbar_view.h',
        'ui/views/toolbar_view.cc',
        'ui/views/toolbar_view.h',
        'ui/views/wrench_toolbar_button.h',
        'ui/views/wrench_toolbar_button.cc',
        'ui/views/app_menu_button_win.h',
        'ui/views/app_menu_button_win.cc',
        'ui/views/home_button.h',
        'ui/views/home_button.cc',
        'ui/views/reload_button.h',
        'ui/views/reload_button.cc',
        'ui/views/wrench_menu.h',
        'ui/views/wrench_menu.cc',
        'ui/views/fullscreen_exit_bubble_views.h',
        'ui/views/fullscreen_exit_bubble_views.cc',
        'ui/views/avatar_label.h',
        'ui/views/avatar_label.cc',
        'ui/theme/theme_properties.h',
        'ui/theme/theme_properties.cc',
        'browser/platform_util.h',
        'browser/platform_util_win.cc',
        'browser/web_contents.cc',
        'browser/web_contents.h',
		'browser/navigation_entry.h',
		'browser/navigation_entry_impl.h',
		'browser/navigation_entry_impl.cc',
        'browser/web_contents_impl.cc',
        'browser/web_contents_impl.h',
        'browser/web_contents_delegate.cc',
        'browser/web_contents_delegate.h',
        'browser/web_contents_observer.cc',
        'browser/web_contents_observer.h',
        'browser/web_contents_user_data.h',
        'browser/web_contents_view.h',
        'browser/web_contents_view_delegate.h',
        'browser/browser_process.h',
        'browser/browser_process.cc',
        'browser/browser_process_impl.h',
        'browser/browser_process_impl.cc',
        'browser/browser_shutdown.cc',
        'browser/browser_shutdown.h',
        'browser/browser_util_win.cc',
        'browser/browser_util_win.h',
        'browser/fullscreen.h',
        'browser/fullscreen_win.cc',
        'browser/status_icons/desktop_notification_balloon.cc',
        'browser/status_icons/desktop_notification_balloon.h',
        'browser/status_icons/status_icon.cc',
        'browser/status_icons/status_icon.h',
        'browser/status_icons/status_icon_menu_model.cc',
        'browser/status_icons/status_icon_menu_model.h',
        'browser/status_icons/status_icon_observer.h',
        'browser/status_icons/status_tray.cc',
        'browser/status_icons/status_tray.h',
        'browser/browser_thread.h',
        'browser/browser_thread_delegate.h',
        'browser/browser_thread_impl.h',
        'browser/browser_thread_impl.cc',
        'browser/browser_process_sub_thread.h',
        'browser/browser_process_sub_thread.cc',
        'browser/profile.h',
        'browser/profile.cc',
        'browser/command_observer.h',
        'browser/command_updater.h',
        'browser/command_updater.cc',
        'browser/command_updater_delegate.h',
        'browser/browser_command_controller.h',
        'browser/browser_command_controller.cc',
        'browser/chrome_command_ids.h',
        'browser/browser_commands.h',
        'browser/browser_commands.cc',
        'browser/browser_navigator.h',
		'browser/browser_navigator.cc',
		'browser/navigation_controller_impl.h',
        'browser/navigation_controller_impl.cc',
        'browser/navigation_controller.h',
        'browser/navigation_controller.cc',
		'browser/navigation_type.h',
        'render/web_contents_view_win.h',
        'render/web_contents_view_win.cc',
        'render/web_contents_view_port.h',
        'render/web_contents_view_guest.h',
        'render/web_contents_view_guest.cc',
        'render/site_instance.h',
        'render/site_instance_impl.h',
        'render/site_instance_impl.cc',
        'render/browsing_instance.h',
        'render/browsing_instance.cc',
        'render/browser_context.h',
        'render/render_host/render_view_host.h',
        'render/render_host/render_view_host_impl.cc',
        'render/render_host/render_view_host_impl.h',    
        'render/render_host/render_widget_host.h',
        'render/render_host/render_widget_host_impl.cc',
        'render/render_host/render_widget_host_impl.h',
		'render/render_host/render_widget_host_iterator.h',
		'render/render_host/render_widget_host_view.h',
		'render/render_host/render_widget_host_view_base.cc',
		'render/render_host/render_widget_host_view_base.h',
		'render/render_host/render_widget_host_view_port.h',
		'render/render_host/render_widget_host_view_win.h',
		'render/render_host/render_widget_host_view_win.cc',
		'render/render_host/render_view_host_delegate_view.h',
		'render/render_host/render_view_host_manager.h',
		'render/render_host/render_view_host_manager.cc',
		'render/render_host/render_widget_host_delegate.h',
		'render/render_host/render_widget_host_delegate.cc',
        'common/child_process_data.h',
        'common/child_process_host.h',
        'common/child_process_host_delegate.cc',
        'common/child_process_host_delegate.h',
        'common/child_process_host_impl.cc',
        'common/child_process_host_impl.h',
        'common/child_process_launcher.cc',
        'common/child_process_launcher.h',
        'common/child_process_messages.h',
        'common/render_process_host.h',
        'common/render_process_host_factory.h',
        'common/render_process_host_impl.cc',
        'common/render_process_host_impl.h',
        'common/content_switches.cc',
        'common/content_switches.h',
		'common/context_menu_params.h',
        'common/context_menu_params.cc',
        'common/view_messages.h',
        'common/view_messages.cc',
        'common/render_view_host_delegate.h',
        'common/render_view_host_delegate.cc',
        'common/page_transition_types.h',
        'common/page_transition_types.cc',
        'common/page_transition_types_list.h',
      ],
      'conditions': [
        ['OS=="win"', {
          'link_settings': {
            'libraries': [
              '-limm32.lib',
              '-loleacc.lib',
            ]
          },
          'msvs_settings': {
            'VCManifestTool': {
              'AdditionalManifestFiles': [
                'chrome\\chrome.exe.manifest',
              ],
            },
            'VCLinkerTool': {
              'SubSystem': '2',  # Set /SUBSYSTEM:WINDOWS
            },
          },
          'dependencies': [
          ],
        }],
        ['OS=="win"', {
          'sources/': [
            # This is needed because the aura rule strips it from the default
            # sources list.
            ['include', '^../../content/app/startup_helper_win.cc'],
          ],
        }],
      ],
    },  # target_name: chrome
  ],
}
