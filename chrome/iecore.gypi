# Copyright (c) 2010 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
      
{
'variables': {
  },

  'targets': [
    {
      'target_name': 'chromeie',
      'type': 'static_library',    	
      'msvs_settings': {
      'VCCLCompilerTool': {
        'WarningLevel': '3',
      },
    },
    'dependencies': [
    '../base/base.gyp:base',
    '../skia/skia.gyp:skia',
    '../ipc/ipc.gyp:ipc',
    ],
    'include_dirs': [
      '..',
    ],
    'sources': [
      'iecore/iecore.cc',
      'iecore/iecore.h',
      'iecore/iecore_message.h',
      'iecore/child/child_process.h',
      'iecore/child/child_process.cc',
      'iecore/child/child_thread.h',
      'iecore/child/child_thread.cc',
      'iecore/child/message_router.h',
      'iecore/child/message_router.cc',
      'iecore/child/render_view.cc',
      'iecore/child/render_view.h',
      'iecore/child/render_widget.cc',
      'iecore/child/render_widget.h',
      'iecore/child/browser_messages.h',
      'iecore/child/browser_messages.cc',
      
      'iecore/trident/preheader.h',
      'iecore/trident/stdafx.h',
      
      'iecore/trident/Dom/CallJsFunc.cpp',
      'iecore/trident/Dom/CallJsFunc.h',
      'iecore/trident/Dom/HTMLOperation.h',
      'iecore/trident/Dom/HTMLOperation.cpp',

      'iecore/trident/patch/CompatibilityView.cpp',
      'iecore/trident/patch/CompatibilityView.h',
      'iecore/trident/patch/CompatibilityViewRule.cpp',
      'iecore/trident/patch/CompatibilityViewRule.h',
      'iecore/trident/patch/des.cc',
      'iecore/trident/patch/des.h',
      'iecore/trident/patch/ExceptionHandler.cpp',
      'iecore/trident/patch/ExceptionHandler.h',
      'iecore/trident/patch/HookApi.cpp',
      'iecore/trident/patch/HookApi.h',
      'iecore/trident/patch/HtmlDocumentHookApi.cpp',
      'iecore/trident/patch/HtmlDocumentHookApi.h',
      'iecore/trident/patch/F12DevTool.h',
      'iecore/trident/patch/HtmlWindowHookApi.cpp',
      'iecore/trident/patch/HtmlWindowHookApi.h',
      'iecore/trident/patch/PatchIEMode.cpp',
      'iecore/trident/patch/PatchIEMode.h',
      'iecore/trident/patch/SyncCookieHelper.cpp',
      'iecore/trident/patch/SyncCookieHelper.h',
      'iecore/trident/patch/SystemInfo.cpp',
      'iecore/trident/patch/SystemInfo.h',
      'iecore/trident/patch/UserAgentHelper.cpp',
      'iecore/trident/patch/UserAgentHelper.h',

      'iecore/trident/WebBrowserControl/BrowserEvent.cpp',
      'iecore/trident/WebBrowserControl/BrowserEvent.h',
      'iecore/trident/WebBrowserControl/CDocHostUI.cpp',
      'iecore/trident/WebBrowserControl/CDocHostUI.h',
      'iecore/trident/WebBrowserControl/DownloadManager.cpp',
      'iecore/trident/WebBrowserControl/DownloadManager.h',
      'iecore/trident/WebBrowserControl/EventObserver.h',
      'iecore/trident/WebBrowserControl/ExternalObj.cpp',
      'iecore/trident/WebBrowserControl/ExternalObj.h',
      'iecore/trident/WebBrowserControl/IDownloadManager.h',
      'iecore/trident/WebBrowserControl/NewWindowManager.cpp',
      'iecore/trident/WebBrowserControl/NewWindowManager.h',
      'iecore/trident/WebBrowserControl/SecurityManager.cpp',
      'iecore/trident/WebBrowserControl/SecurityManager.h',
      'iecore/trident/WebBrowserControl/WebBrowser.cpp',
      'iecore/trident/WebBrowserControl/WebBrowser.h',

      'iecore/trident/WebBrowserHost/Browserdef.h',
      'iecore/trident/WebBrowserHost/IEBrowserWnd.cpp',
      'iecore/trident/WebBrowserHost/IEBrowserWnd.h',
      'iecore/trident/WebBrowserHost/XWnd.cpp',
      'iecore/trident/WebBrowserHost/XWnd.h',

      'iecore/third_party/comhook.h',
      'iecore/third_party/comhook.cpp',
      'iecore/third_party/detourshook.h',
    ],
    'link_settings': {
    },
    'conditions': [
      # Windows-specific rules.
      ['OS=="win"', {
        'include_dirs': [
          '<(DEPTH)/third_party/wtl/include',
        ],
        'msvs_settings': {
          'VCCLCompilerTool': {
            'ExceptionHandling': '1',
          },
        },
        'conditions': [
          ['win_use_allocator_shim==1', {
            'dependencies': [
            ],
            'export_dependent_settings': [
              
            ],
          }],
        ],
      }],
    ],
  },],
}
