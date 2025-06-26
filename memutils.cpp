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
#include "memutils.h"

MemUtils::MemUtils()
{
}

bool MemUtils::init()
{
    if (PdhOpenQuery(NULL, NULL, &hQuery) != ERROR_SUCCESS) return false;

    if (PdhAddEnglishCounter(hQuery,
                             L"\\Memory\\Available Bytes",
                             NULL,
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
    if (PdhGetFormattedCounterValue(hCounter,
                                    PDH_FMT_DOUBLE,
                                    NULL,
                                    &available) != ERROR_SUCCESS)
        return -1;

    double totalMemory = getTotal() * 1024 * 1024 * 1024;
    double usageMemory = totalMemory - available.doubleValue;

    return usageMemory / (1024.0 * 1024.0 * 1024.0);
}
