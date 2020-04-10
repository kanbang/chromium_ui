#pragma once
namespace minibase
{
	namespace Detours
	{
    void APIHookBegin();
    void APIAttach(LPCWSTR lpszModuleName,void* pOrig,void* pHook);
    void APIHookEnd();
	}
}