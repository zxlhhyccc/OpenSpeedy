#ifndef SPEEDPATCH_H
#define SPEEDPATCH_H


#if defined(SPEEDPATCH_LIBRARY)
#define SPEEDPATCH_API __declspec(dllexport)
#else
#define SPEEDPATCH_API __declspec(dllimport)
#endif

extern "C" {
SPEEDPATCH_API void SetSpeedFactor(double factor_);
}

#endif // SPEEDPATCH_H
