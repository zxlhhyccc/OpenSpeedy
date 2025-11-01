/*
 * OpenSpeedy - Open Source Game Speed Controller
 * Copyright (C) 2025 Game1024
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef SPEEDPATCH_H
#define SPEEDPATCH_H
#include <windows.h>
#include <string>

#if defined(SPEEDPATCH_LIBRARY)
#define SPEEDPATCH_API __declspec(dllexport)
#else
#define SPEEDPATCH_API __declspec(dllimport)
#endif

std::wstring
GetCurrentProcessName();

std::wstring
GetProcessFileMapName(DWORD processId);

extern "C"
{
SPEEDPATCH_API void Init();
SPEEDPATCH_API void Clean();
SPEEDPATCH_API BOOL GetStatus();
SPEEDPATCH_API void SetProcessStatus(DWORD processId,BOOL status);
SPEEDPATCH_API void ChangeSpeed(double factor_);
SPEEDPATCH_API LRESULT CALLBACK HookProc(int    nCode,
                                         WPARAM wParam,
                                         LPARAM lParam
                                         );
}

#endif // SPEEDPATCH_H
