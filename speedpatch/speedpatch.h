#ifndef SPEEDPATCH_H
#define SPEEDPATCH_H
#include <windows.h>

#if defined(SPEEDPATCH_LIBRARY)
#define SPEEDPATCH_API __declspec(dllexport)
#else
#define SPEEDPATCH_API __declspec(dllimport)
#endif

extern "C"
{
    SPEEDPATCH_API void ChangeSpeed(double factor_);
    SPEEDPATCH_API LRESULT CALLBACK HookProc(int nCode,
                                             WPARAM wParam,
                                             LPARAM lParam);
}

#endif  // SPEEDPATCH_H
