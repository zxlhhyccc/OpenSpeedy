#ifndef CONFIG_H
#define CONFIG_H

#define OPENSPEEDY_VERSION "v1.7.2"

#define BRIDGE32_EXE "bridge32.exe"
#define BRIDGE64_EXE "bridge64.exe"

#define SPEEDPATCH32_DLL "speedpatch32.dll"
#define SPEEDPATCH64_DLL "speedpatch64.dll"

#define CONFIG_LANGUAGE "General/Language"

// 热键ID
enum HotkeyIds
{
    HOTKEY_INCREASE_SPEED = 1001,
    HOTKEY_DECREASE_SPEED = 1002,
    HOTKEY_RESET_SPEED = 1003,
    HOTKEY_SHIFT1 = 1011,
    HOTKEY_SHIFT2 = 1012,
    HOTKEY_SHIFT3 = 1013,
    HOTKEY_SHIFT4 = 1014,
    HOTKEY_SHIFT5 = 1015
};

#endif // CONFIG_H
