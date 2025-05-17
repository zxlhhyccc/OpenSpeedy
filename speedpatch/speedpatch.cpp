#include "speedpatch.h"
#include <atomic>
#include <shared_mutex>
#include <windows.h>
#include <mmsystem.h>
#include "Minhook.h"

#pragma comment(lib, "winmm.lib")
#pragma data_seg("shared")
static std::atomic<double> factor = 1.0;
#pragma data_seg()
#pragma comment(linker, "/section:shared,RWS")


static std::shared_mutex mutex;
typedef UINT_PTR(*SETTIMER)(HWND,UINT_PTR,UINT,TIMERPROC);
typedef DWORD(*TIMEGETTIME)(VOID);
typedef DWORD(*GETTICKCOUNT)(VOID);
typedef ULONGLONG(*GETTICKCOUNT64)(VOID);
typedef BOOL(*QUERYPERFOMANCECOUNTER)(LARGE_INTEGER*);
typedef VOID(*GETSYSTEMTIMEASFILETIME)(LPFILETIME);
typedef VOID(*GETSYSTEMTIMEPRECISEASFILETIME)(LPFILETIME);


static DWORD firstTimeGetTime = 0;
static DWORD firstGetTickCount = 0;
static ULONGLONG firstGetTickCount64 = 0;
static LARGE_INTEGER firstQueryPerformanceCounter = { 0 };
static FILETIME firstGetSystemTimeAsFileTime = {0};
static FILETIME firstGetSystemTimePreciseAsFileTime = {0};

static SETTIMER pfnKernelSetTimer = NULL;
static SETTIMER pfnDetourSetTimer = NULL;

static TIMEGETTIME pfnKernelTimeGetTime = NULL;
static TIMEGETTIME pfnDetourTimeGetTime = NULL;

static GETTICKCOUNT pfnKernelGetTickCount = NULL;
static GETTICKCOUNT pfnDetourGetTickCount = NULL;

static GETTICKCOUNT64 pfnKernelGetTickCount64 = NULL;
static GETTICKCOUNT64 pfnDetourGetTickCount64 = NULL;

static QUERYPERFOMANCECOUNTER pfnKernelQueryPerformanceCounter = NULL;
static QUERYPERFOMANCECOUNTER pfnDetourQueryPerformanceCounter = NULL;

static GETSYSTEMTIMEASFILETIME pfnKernelGetSystemTimeAsFileTime = NULL;
static GETSYSTEMTIMEASFILETIME pfnDetourGetSystemTimeAsFileTime = NULL;

static GETSYSTEMTIMEPRECISEASFILETIME pfnKernelGetSystemTimePreciseAsFileTime = NULL;
static GETSYSTEMTIMEPRECISEASFILETIME pfnDetourGetSystemTimePreciseAsFileTime = NULL;

UINT_PTR DetourSetTimer(HWND hWnd, UINT_PTR nIDEvent, UINT uElapse, TIMERPROC lpTimerFunc)
{
    std::shared_lock<std::shared_mutex> lock(mutex);
    return pfnKernelSetTimer(hWnd, nIDEvent, uElapse/factor.load(), lpTimerFunc);
}

DWORD DetourTimeGetTime(VOID)
{
    std::shared_lock<std::shared_mutex> lock(mutex);
    DWORD delta = factor.load() * (pfnKernelTimeGetTime() - firstTimeGetTime);
    return firstTimeGetTime + delta;
}

DWORD DetourGetTickCount(VOID)
{
    std::shared_lock<std::shared_mutex> lock(mutex);
    DWORD delta = factor.load() * (pfnKernelGetTickCount() - firstGetTickCount);
    return firstGetTickCount + delta;
}

ULONGLONG DetourGetTickCount64(VOID)
{
    std::shared_lock<std::shared_mutex> lock(mutex);
    ULONGLONG delta = factor.load() * (pfnKernelGetTickCount64() - firstGetTickCount64);
    return firstGetTickCount64 + delta;
}

BOOL DetourQueryPerformanceCounter(LARGE_INTEGER* lpPerformanceCount)
{
    std::shared_lock<std::shared_mutex> lock(mutex);
    if (lpPerformanceCount == NULL)
    {
        return FALSE;
    }
    else
    {
        LARGE_INTEGER now = { 0 };
        BOOL rtncode = pfnKernelQueryPerformanceCounter(&now);
        LONGLONG delta = factor.load() * (now.QuadPart - firstQueryPerformanceCounter.QuadPart);
        lpPerformanceCount->QuadPart = firstQueryPerformanceCounter.QuadPart + delta;
        return rtncode;
    }
}

VOID DetourGetSystemTimeAsFileTime(LPFILETIME lpSystemTimeAsFileTime)
{
    std::shared_lock<std::shared_mutex> lock(mutex);
    if (lpSystemTimeAsFileTime == NULL)
    {
        return pfnKernelGetSystemTimeAsFileTime(lpSystemTimeAsFileTime);
    }
    else
    {
        ULARGE_INTEGER firstDateTime = {0};
        firstDateTime.LowPart = firstGetSystemTimeAsFileTime.dwLowDateTime;
        firstDateTime.HighPart = firstGetSystemTimeAsFileTime.dwHighDateTime;
        FILETIME now = {0};
        pfnKernelGetSystemTimeAsFileTime(&now);
        ULARGE_INTEGER nowDateTime = {0};
        nowDateTime.LowPart = now.dwLowDateTime;
        nowDateTime.HighPart = now.dwHighDateTime;
        ULARGE_INTEGER rtnDateTime = {0};

        ULONGLONG delta = factor.load() * (nowDateTime.QuadPart - firstDateTime.QuadPart);
        rtnDateTime.QuadPart = firstDateTime.QuadPart + delta;
        lpSystemTimeAsFileTime->dwLowDateTime = rtnDateTime.LowPart;
        lpSystemTimeAsFileTime->dwHighDateTime = rtnDateTime.HighPart;
    }
}

VOID DetourGetSystemTimePreciseAsFileTime(LPFILETIME lpSystemTimeAsFileTime)
{
    std::shared_lock<std::shared_mutex> lock(mutex);
    if (lpSystemTimeAsFileTime == NULL)
    {
        return pfnKernelGetSystemTimePreciseAsFileTime(lpSystemTimeAsFileTime);
    }
    else
    {
        ULARGE_INTEGER firstDateTime = {0};
        firstDateTime.LowPart = firstGetSystemTimeAsFileTime.dwLowDateTime;
        firstDateTime.HighPart = firstGetSystemTimeAsFileTime.dwHighDateTime;
        FILETIME now = {0};
        pfnKernelGetSystemTimePreciseAsFileTime(&now);
        ULARGE_INTEGER nowDateTime = {0};
        nowDateTime.LowPart = now.dwLowDateTime;
        nowDateTime.HighPart = now.dwHighDateTime;
        ULARGE_INTEGER rtnDateTime = {0};

        ULONGLONG delta = factor.load() * (nowDateTime.QuadPart - firstDateTime.QuadPart);
        rtnDateTime.QuadPart = firstDateTime.QuadPart + delta;
        lpSystemTimeAsFileTime->dwLowDateTime = rtnDateTime.LowPart;
        lpSystemTimeAsFileTime->dwHighDateTime = rtnDateTime.HighPart;
    }
}

VOID MH_HOOK(LPVOID pTarget, LPVOID pDetour, LPVOID* ppOriginal) {
    MH_CreateHook(pTarget, pDetour, ppOriginal);
    MH_EnableHook(pTarget);
}

VOID MH_UNHOOK(LPVOID pTarget) {
    MH_RemoveHook(pTarget);
}

// Export to change speed factor for hacker process
SPEEDPATCH_API void SetSpeedFactor(double factor_) {
    factor.store(factor_);
}

BOOL APIENTRY DllMain(HMODULE hModule,
                      DWORD  ul_reason_for_call,
                      LPVOID lpReserved
                      )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        if (MH_Initialize() != MH_OK) {
            return FALSE;
        }
        firstTimeGetTime = timeGetTime();
        firstGetTickCount = GetTickCount();
        firstGetTickCount64 = GetTickCount64();
        QueryPerformanceCounter(&firstQueryPerformanceCounter);
        GetSystemTimeAsFileTime(&firstGetSystemTimeAsFileTime);
        GetSystemTimePreciseAsFileTime(&firstGetSystemTimePreciseAsFileTime);

        MH_HOOK(&SetTimer, &DetourSetTimer, reinterpret_cast<LPVOID*>(&pfnKernelSetTimer));
        MH_HOOK(&timeGetTime, &DetourTimeGetTime, reinterpret_cast<LPVOID*>(&pfnKernelTimeGetTime));
        MH_HOOK(&GetTickCount, &DetourGetTickCount, reinterpret_cast<LPVOID*>(&pfnKernelGetTickCount));
        MH_HOOK(&GetTickCount64, &DetourGetTickCount64, reinterpret_cast<LPVOID*>(&pfnKernelGetTickCount64));
        MH_HOOK(&QueryPerformanceCounter, &DetourQueryPerformanceCounter, reinterpret_cast<LPVOID*>(&pfnKernelQueryPerformanceCounter));
        MH_HOOK(&GetSystemTimeAsFileTime, &DetourGetSystemTimeAsFileTime, reinterpret_cast<LPVOID*>(&pfnKernelGetSystemTimeAsFileTime));
        MH_HOOK(&GetSystemTimePreciseAsFileTime, &DetourGetSystemTimePreciseAsFileTime, reinterpret_cast<LPVOID*>(&pfnKernelGetSystemTimePreciseAsFileTime));
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
    {
        std::unique_lock<std::shared_mutex> lock(mutex);
        MH_DisableHook(MH_ALL_HOOKS);
    }

        {
            std::unique_lock<std::shared_mutex> lock(mutex);
            MH_UNHOOK(pfnKernelTimeGetTime);
            MH_UNHOOK(pfnKernelGetTickCount);
            MH_UNHOOK(pfnKernelGetTickCount64);
            MH_UNHOOK(pfnKernelQueryPerformanceCounter);
            MH_UNHOOK(pfnKernelGetSystemTimeAsFileTime);
            MH_UNHOOK(pfnKernelGetSystemTimePreciseAsFileTime);
        }

        // Wait for All threads to finish detour api
        Sleep(1000);
        {
            std::unique_lock<std::shared_mutex> lock(mutex);
            if (MH_Uninitialize() != MH_OK)
            {
                MessageBoxW(NULL, L"DLL卸载失败", L"DLL", MB_OK);
                return FALSE;
            }
        }
        break;
    }
    return TRUE;
}
