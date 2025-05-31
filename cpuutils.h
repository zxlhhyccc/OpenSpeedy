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
