#include "speedpatch.h"
#include <atomic>
#include <shared_mutex>
#include <windows.h>
#include "Minhook.h"

#pragma data_seg("shared")
static std::atomic<double> factor = 1.0;
#pragma data_seg()
#pragma comment(linker, "/section:shared,RWS")

static std::shared_mutex mutex;
typedef DWORD(*GETTICKCOUNT)(VOID);
typedef ULONGLONG(*GETTICKCOUNT64)(VOID);
typedef BOOL(*QUERYPERFOMANCECOUNTER)(LARGE_INTEGER*);


static DWORD firstGetTickCount = 0;
static ULONGLONG firstGetTickCount64 = 0;
static LARGE_INTEGER firstQueryPerformanceCounter = { 0 };

static GETTICKCOUNT pfnKernelGetTickCount = NULL;
static GETTICKCOUNT pfnDetourGetTickCount = NULL;

static GETTICKCOUNT64 pfnKernelGetTickCount64 = NULL;
static GETTICKCOUNT64 pfnDetourGetTickCount64 = NULL;

static QUERYPERFOMANCECOUNTER pfnKernelQueryPerformanceCounter = NULL;
static QUERYPERFOMANCECOUNTER pfnDetourQueryPerformanceCounter = NULL;

DWORD DetourGetTickCount(VOID) {
    std::shared_lock<std::shared_mutex> lock(mutex);
    DWORD delta = factor.load() * (pfnKernelGetTickCount() - firstGetTickCount);
    return firstGetTickCount + delta;
}

ULONGLONG DetourGetTickCount64(VOID) {
    std::shared_lock<std::shared_mutex> lock(mutex);
    ULONGLONG delta = factor.load() * (pfnKernelGetTickCount64() - firstGetTickCount64);
    return firstGetTickCount64 + delta;
}

BOOL DetourQueryPerformanceCounter(LARGE_INTEGER* lpPerformanceCount) {
    std::shared_lock<std::shared_mutex> lock(mutex);
    if (lpPerformanceCount == NULL) {
        return FALSE;
    }
    else {
        LARGE_INTEGER now = { 0 };
        BOOL rtncode = pfnKernelQueryPerformanceCounter(&now);
        LONGLONG delta = factor.load() * (now.QuadPart - firstQueryPerformanceCounter.QuadPart);
        lpPerformanceCount->QuadPart = firstQueryPerformanceCounter.QuadPart + delta;
        return rtncode;
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
        firstGetTickCount = GetTickCount();
        firstGetTickCount64 = GetTickCount64();
        QueryPerformanceCounter(&firstQueryPerformanceCounter);

        MH_HOOK(&GetTickCount, &DetourGetTickCount, reinterpret_cast<LPVOID*>(&pfnKernelGetTickCount));
        MH_HOOK(&GetTickCount64, &DetourGetTickCount64, reinterpret_cast<LPVOID*>(&pfnKernelGetTickCount64));
        MH_HOOK(&QueryPerformanceCounter, &DetourQueryPerformanceCounter, reinterpret_cast<LPVOID*>(&pfnKernelQueryPerformanceCounter));
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
            MH_UNHOOK(pfnKernelGetTickCount);
            MH_UNHOOK(pfnKernelGetTickCount64);
            MH_UNHOOK(pfnKernelQueryPerformanceCounter);
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
