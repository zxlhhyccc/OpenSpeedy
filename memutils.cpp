#include "memutils.h"

MemUtils::MemUtils() {}

bool MemUtils::init()
{
    if (PdhOpenQuery(NULL, NULL, &hQuery) != ERROR_SUCCESS) return false;

    if (PdhAddEnglishCounter(hQuery, L"\\Memory\\Available Bytes", NULL,
                             &hCounter) != ERROR_SUCCESS)
    {
        PdhCloseQuery(hQuery);
        return false;
    }

    PdhCollectQueryData(hQuery);
    initialized = true;
    return true;
}

double MemUtils::getTotal()
{
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);
    return memInfo.ullTotalPhys / (1024.0 * 1024.0 * 1024.0);
}

double MemUtils::getUsage()
{
    if (!initialized) return -1;

    if (PdhCollectQueryData(hQuery) != ERROR_SUCCESS) return -1;

    PDH_FMT_COUNTERVALUE available;
    if (PdhGetFormattedCounterValue(hCounter, PDH_FMT_DOUBLE, NULL,
                                    &available) != ERROR_SUCCESS)
        return -1;

    double totalMemory = getTotal() * 1024 * 1024 * 1024;
    double usageMemory = totalMemory - available.doubleValue;

    return usageMemory / (1024.0 * 1024.0 * 1024.0);
}
