#include "shareddouble.h"

SharedDouble::SharedDouble(std::wstring name, double value)
{
    hFile = CreateFileMappingW(INVALID_HANDLE_VALUE,  // 使用页面文件
                               nullptr,               // 默认安全
                               PAGE_READWRITE,        // 读写权限
                               0,                     // 高32位
                               sizeof(double),        // 只需要double大小
                               name.c_str()           // 共享内存名称
    );

    data = static_cast<double*>(
        MapViewOfFile(hFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(double)));

    if (*data == 0.0) *data = value;
}

SharedDouble::~SharedDouble()
{
    UnmapViewOfFile(data);
    CloseHandle(hFile);
}

double SharedDouble::get() { return *data; }

void SharedDouble::set(double value) { *data = value; }
