#pragma once
#include "chrome/iecore/trident/stdafx.h"
struct ComHookInfo
{
    int         nOffset;
    DWORD       NewFunc;
    DWORD*      OldFunc;
};

BOOL HookComInterface(DWORD ** p, int offset, DWORD dwNewVal, DWORD * pOldVal);

BOOL HookComInterfaceSafe(DWORD ** p, int offset, DWORD dwNewVal, DWORD * pOldVal);
