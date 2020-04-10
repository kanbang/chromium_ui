# Copyright (c) 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'targets': [
    {
      'target_name': 'shell_dialogs',
      'type': '<(component)',
      'dependencies': [
        '../base/base.gyp:base',
        '../skia/skia.gyp:skia',
        'ui',
      ],
      'defines': [
        'SHELL_DIALOGS_IMPLEMENTATION',
      ],
      'sources': [
        'shell_dialogs/android/shell_dialogs_jni_registrar.cc',
        'shell_dialogs/android/shell_dialogs_jni_registrar.h',
        'shell_dialogs/base_shell_dialog.cc',
        'shell_dialogs/base_shell_dialog.h',
        'shell_dialogs/base_shell_dialog_win.cc',
        'shell_dialogs/base_shell_dialog_win.h',
        'shell_dialogs/linux_shell_dialog.cc',
        'shell_dialogs/linux_shell_dialog.h',
        'shell_dialogs/select_file_dialog.cc',
        'shell_dialogs/select_file_dialog.h',
        'shell_dialogs/select_file_dialog_android.cc',
        'shell_dialogs/select_file_dialog_android.h',
        'shell_dialogs/select_file_dialog_factory.cc',
        'shell_dialogs/select_file_dialog_factory.h',
        'shell_dialogs/select_file_dialog_mac.h',
        'shell_dialogs/select_file_dialog_mac.mm',
        'shell_dialogs/select_file_dialog_win.cc',
        'shell_dialogs/select_file_dialog_win.h',
        'shell_dialogs/select_file_policy.cc',
        'shell_dialogs/select_file_policy.h',
        'shell_dialogs/selected_file_info.cc',
        'shell_dialogs/selected_file_info.h',
      ],
      'include_dirs': [
        '../',
      ],
      'conditions': [
        ['use_aura==1', {
          'dependencies': [
            'aura/aura.gyp:aura',
          ],
          'sources/': [
            ['exclude', 'shell_dialogs/select_file_dialog_mac.mm'],
           ],
        }],
        ['OS=="android"', {
          'dependencies': [
            'ui_jni_headers',
          ],
          'include_dirs': [
            '<(SHARED_INTERMEDIATE_DIR)/ui',
          ],
          'link_settings': {
            'libraries': [
              '-ljnigraphics',
            ],
          },
        }],
        ['OS=="android" and android_webview_build==0', {
          'dependencies': [
            'ui_java',
          ],
        }],
      ],
    },  # target_name: shell_dialogs
  ],
}
