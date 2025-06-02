#ifndef SHAREDDOUBLE_H
#define SHAREDDOUBLE_H

#include <windows.h>
#include <string>

class SharedDouble
{
   private:
    HANDLE hFile;
    double *data;

   public:
    SharedDouble(std::wstring name, double value);

    ~SharedDouble();

    double get();

    void set(double value);
};

#endif  // SHAREDDOUBLE_H
