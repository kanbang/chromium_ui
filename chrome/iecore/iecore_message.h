#pragma once

// browserwnd ======> child_thread

#define BROWSER_TO_CHILDTHREAD_IPC_MESSAGE    (WM_USER + 0x100)
#define BROWSER_TO_CHILDTHREAD_NEWWINDOW      (WM_USER + 0x101)

// child_thread =====> child_thread
#define CHILDTHREAD_TO_BROWSER_IPC_MESSAGE    (WM_USER + 0x100)
#define CHILDTHREAD_TO_BROWSER_SET_PARENT     (WM_USER + 0x101)