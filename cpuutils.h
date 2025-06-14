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
#ifndef CPUUTILS_H
#define CPUUTILS_H

#include <windows.h>
#include <QString>
#include <pdh.h>

class CpuUtils
{
   private:
    PDH_HQUERY hQuery = NULL;
    PDH_HCOUNTER hCounter = NULL;
    bool initialized = false;

   public:
    CpuUtils();

    ~CpuUtils();

    bool init();

    double getUsage();

    QString getModel();
};

#endif  // CPUUTILS_H
