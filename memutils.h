#ifndef MEMUTILS_H
#define MEMUTILS_H
#include <windows.h>
#include <pdh.h>

class MemUtils
{
   private:
    PDH_HQUERY hQuery = NULL;
    PDH_HCOUNTER hCounter = NULL;
    bool initialized = false;

   public:
    MemUtils();

    bool init();

    double getTotal();

    double getUsage();
};

#endif  // MEMUTILS_H
