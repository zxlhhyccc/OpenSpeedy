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
#include "cpuutils.h"

CpuUtils::CpuUtils()
{
}

bool CpuUtils::init()
{
    // 创建查询
    if (PdhOpenQuery(NULL, NULL, &hQuery) != ERROR_SUCCESS) return false;

    if (PdhAddEnglishCounter(hQuery,
                             L"\\Processor(_Total)\\% Processor Time",
                             NULL,
                             &hCounter) != ERROR_SUCCESS)
    {
        PdhCloseQuery(hQuery);
        return false;
    }

    // 第一次采样
    PdhCollectQueryData(hQuery);
    initialized = true;
    return true;
}

double CpuUtils::getUsage()
{
    if (!initialized) return -1;

    // 收集数据
    if (PdhCollectQueryData(hQuery) != ERROR_SUCCESS) return -1;

    PDH_FMT_COUNTERVALUE value;
    if (PdhGetFormattedCounterValue(hCounter, PDH_FMT_DOUBLE, NULL, &value) !=
        ERROR_SUCCESS)
        return -1;

    return value.doubleValue;
}

QString CpuUtils::getModel()
{
    int cpuInfo[4];
    char cpuBrand[48 + 1] = {0};

    // 获取CPU品牌字符串（需要调用3次CPUID）
    __cpuid(cpuInfo, 0x80000002);
    memcpy(cpuBrand, cpuInfo, sizeof(cpuInfo));

    __cpuid(cpuInfo, 0x80000003);
    memcpy(cpuBrand + 16, cpuInfo, sizeof(cpuInfo));

    __cpuid(cpuInfo, 0x80000004);
    memcpy(cpuBrand + 32, cpuInfo, sizeof(cpuInfo));

    QString result(cpuBrand);

    return result.trimmed();
}

CpuUtils::~CpuUtils()
{
    if (hCounter) PdhRemoveCounter(hCounter);
    if (hQuery) PdhCloseQuery(hQuery);
}
