#include "mml.h"
#include <fstream>
#include <iomanip>
#include <iostream>

#ifdef _WIN64
#define CURRENT_ARCH IMAGE_FILE_MACHINE_AMD64
#include "shellcode_array64.h"
#else
#define CURRENT_ARCH IMAGE_FILE_MACHINE_I386
#include "shellcode_array32.h"
#endif

bool mml::inject(HANDLE hProc,
                 BYTE* pSrcData,
                 SIZE_T FileSize,
                 bool ClearHeader,
                 bool ClearNonNeededSections,
                 bool AdjustProtections,
                 bool SEHExceptionSupport,
                 DWORD fdwReason,
                 LPVOID lpReserved)
{
    IMAGE_NT_HEADERS* pOldNtHeader = nullptr;
    IMAGE_OPTIONAL_HEADER* pOldOptHeader = nullptr;
    IMAGE_FILE_HEADER* pOldFileHeader = nullptr;
    BYTE* pTargetBase = nullptr;

    if (reinterpret_cast<IMAGE_DOS_HEADER*>(pSrcData)->e_magic != 0x5A4D)
    {  //"MZ"
        qDebug() << "Invalid file";
        return false;
    }

    pOldNtHeader = reinterpret_cast<IMAGE_NT_HEADERS*>(
        pSrcData + reinterpret_cast<IMAGE_DOS_HEADER*>(pSrcData)->e_lfanew);
    pOldOptHeader = &pOldNtHeader->OptionalHeader;
    pOldFileHeader = &pOldNtHeader->FileHeader;

    if (pOldFileHeader->Machine != CURRENT_ARCH)
    {
        qDebug() << "Invalid platform";
        return false;
    }

    pTargetBase = reinterpret_cast<BYTE*>(
        VirtualAllocEx(hProc, nullptr, pOldOptHeader->SizeOfImage,
                       MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
    if (!pTargetBase)
    {
        qDebug() << "Target process memory allocation failed (ex) 0x" << Qt::hex
                 << GetLastError();
        return false;
    }

    DWORD oldp = 0;
    VirtualProtectEx(hProc, pTargetBase, pOldOptHeader->SizeOfImage,
                     PAGE_EXECUTE_READWRITE, &oldp);

    MANUAL_MAPPING_DATA data{0};
    data.pLoadLibraryA = LoadLibraryA;
    data.pGetProcAddress = GetProcAddress;
#ifdef _WIN64
    data.pRtlAddFunctionTable = (f_RtlAddFunctionTable)RtlAddFunctionTable;
#else
    SEHExceptionSupport = false;
#endif
    data.pbase = pTargetBase;
    data.fdwReasonParam = fdwReason;
    data.reservedParam = lpReserved;
    data.SEHSupport = SEHExceptionSupport;

    // File header
    if (!WriteProcessMemory(hProc, pTargetBase, pSrcData, 0x1000, nullptr))
    {  // only first 0x1000 bytes for the header
        qDebug() << "Can't write file header 0x" << Qt::hex << GetLastError();
        VirtualFreeEx(hProc, pTargetBase, 0, MEM_RELEASE);
        return false;
    }

    IMAGE_SECTION_HEADER* pSectionHeader = IMAGE_FIRST_SECTION(pOldNtHeader);
    for (UINT i = 0; i != pOldFileHeader->NumberOfSections;
         ++i, ++pSectionHeader)
    {
        if (pSectionHeader->SizeOfRawData)
        {
            if (!WriteProcessMemory(
                    hProc, pTargetBase + pSectionHeader->VirtualAddress,
                    pSrcData + pSectionHeader->PointerToRawData,
                    pSectionHeader->SizeOfRawData, nullptr))
            {
                qDebug() << "Can't map sections: 0x" << Qt::hex
                         << GetLastError();
                VirtualFreeEx(hProc, pTargetBase, 0, MEM_RELEASE);
                return false;
            }
        }
    }

    // Mapping params
    BYTE* MappingDataAlloc = reinterpret_cast<BYTE*>(
        VirtualAllocEx(hProc, nullptr, sizeof(MANUAL_MAPPING_DATA),
                       MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
    if (!MappingDataAlloc)
    {
        qDebug() << "Target process mapping allocation failed (ex) 0x%"
                 << Qt::hex << GetLastError();
        VirtualFreeEx(hProc, pTargetBase, 0, MEM_RELEASE);
        return false;
    }

    if (!WriteProcessMemory(hProc, MappingDataAlloc, &data,
                            sizeof(MANUAL_MAPPING_DATA), nullptr))
    {
        qDebug() << "Can't write mapping 0x" << Qt::hex << GetLastError();
        VirtualFreeEx(hProc, pTargetBase, 0, MEM_RELEASE);
        VirtualFreeEx(hProc, MappingDataAlloc, 0, MEM_RELEASE);
        return false;
    }

    // Shell code
    void* pShellcode =
        VirtualAllocEx(hProc, nullptr, 0x1000, MEM_COMMIT | MEM_RESERVE,
                       PAGE_EXECUTE_READWRITE);
    if (!pShellcode)
    {
        qDebug() << "Memory shellcode allocation failed (ex) 0x" << Qt::hex
                 << GetLastError();
        VirtualFreeEx(hProc, pTargetBase, 0, MEM_RELEASE);
        VirtualFreeEx(hProc, MappingDataAlloc, 0, MEM_RELEASE);
        return false;
    }

    if (!WriteProcessMemory(hProc, pShellcode,
                            reinterpret_cast<PVOID>(g_Shellcode), 0x1000,
                            nullptr))
    {
        qDebug() << "Can't write shellcode 0x" << GetLastError();
        VirtualFreeEx(hProc, pTargetBase, 0, MEM_RELEASE);
        VirtualFreeEx(hProc, MappingDataAlloc, 0, MEM_RELEASE);
        VirtualFreeEx(hProc, pShellcode, 0, MEM_RELEASE);
        return false;
    }

    qDebug() << "Mapped DLL at " << Qt::hex << pTargetBase;
    qDebug() << "Mapping info at " << Qt::hex << MappingDataAlloc;
    qDebug() << "Shell code at " << Qt::hex << pShellcode;
    qDebug() << "Data allocated";
    qDebug() << "Local Shellcode Function pointer at " << Qt::hex
             << (UINT_PTR)Shellcode;
    qDebug() << "Remote Shellcode Function pointer at " << Qt::hex
             << (UINT_PTR)pShellcode;

    HANDLE hThread = CreateRemoteThread(hProc, nullptr, 0,
                                        (LPTHREAD_START_ROUTINE)pShellcode,
                                        MappingDataAlloc, 0, nullptr);
    if (!hThread)
    {
        qDebug() << "Thread creation failed 0x" << Qt::hex << GetLastError();
        VirtualFreeEx(hProc, pTargetBase, 0, MEM_RELEASE);
        VirtualFreeEx(hProc, MappingDataAlloc, 0, MEM_RELEASE);
        VirtualFreeEx(hProc, pShellcode, 0, MEM_RELEASE);
        return false;
    }

    WaitForSingleObject(hThread, INFINITE);

    CloseHandle(hThread);

    qDebug() << "Thread created at: " << Qt::hex << pShellcode
             << " waiting for return...\n";

    HINSTANCE hCheck = NULL;
    while (!hCheck)
    {
        DWORD exitcode = 0;
        GetExitCodeProcess(hProc, &exitcode);
        if (exitcode != STILL_ACTIVE)
        {
            qDebug() << "Process crashed, exit code: " << exitcode;
            return false;
        }

        MANUAL_MAPPING_DATA data_checked{0};
        ReadProcessMemory(hProc, MappingDataAlloc, &data_checked,
                          sizeof(data_checked), nullptr);
        hCheck = data_checked.hMod;

        if (hCheck == (HINSTANCE)0x404040)
        {
            qDebug() << "Wrong mapping ptr";
            VirtualFreeEx(hProc, pTargetBase, 0, MEM_RELEASE);
            VirtualFreeEx(hProc, MappingDataAlloc, 0, MEM_RELEASE);
            VirtualFreeEx(hProc, pShellcode, 0, MEM_RELEASE);
            return false;
        }
        else if (hCheck == (HINSTANCE)0x505050)
        {
            qDebug() << "WARNING: Exception support failed!";
        }

        Sleep(10);
    }

    BYTE* emptyBuffer = (BYTE*)malloc(1024 * 1024 * 20);
    if (emptyBuffer == nullptr)
    {
        qDebug() << "Unable to allocate memory";
        return false;
    }
    memset(emptyBuffer, 0, 1024 * 1024 * 20);

    // CLEAR PE HEAD
    if (ClearHeader)
    {
        if (!WriteProcessMemory(hProc, pTargetBase, emptyBuffer, 0x1000,
                                nullptr))
        {
            qDebug() << "WARNING!: Can't clear HEADER";
        }
    }
    // END CLEAR PE HEAD
    if (ClearNonNeededSections)
    {
        pSectionHeader = IMAGE_FIRST_SECTION(pOldNtHeader);
        for (UINT i = 0; i != pOldFileHeader->NumberOfSections;
             ++i, ++pSectionHeader)
        {
            if (pSectionHeader->Misc.VirtualSize)
            {
                if ((SEHExceptionSupport ? 0
                                         : strcmp((char*)pSectionHeader->Name,
                                                  ".pdata") == 0) ||
                    strcmp((char*)pSectionHeader->Name, ".rsrc") == 0 ||
                    strcmp((char*)pSectionHeader->Name, ".reloc") == 0)
                {
                    qDebug()
                        << "Processing " << pSectionHeader->Name << "removal";
                    if (!WriteProcessMemory(
                            hProc, pTargetBase + pSectionHeader->VirtualAddress,
                            emptyBuffer, pSectionHeader->Misc.VirtualSize,
                            nullptr))
                    {
                        qDebug()
                            << "Can't clear section " << pSectionHeader->Name
                            << ":0x" << GetLastError();
                    }
                }
            }
        }
    }

    if (AdjustProtections)
    {
        pSectionHeader = IMAGE_FIRST_SECTION(pOldNtHeader);
        for (UINT i = 0; i != pOldFileHeader->NumberOfSections;
             ++i, ++pSectionHeader)
        {
            if (pSectionHeader->Misc.VirtualSize)
            {
                DWORD old = 0;
                DWORD newP = PAGE_READONLY;

                if ((pSectionHeader->Characteristics & IMAGE_SCN_MEM_WRITE) > 0)
                {
                    newP = PAGE_READWRITE;
                }
                else if ((pSectionHeader->Characteristics &
                          IMAGE_SCN_MEM_EXECUTE) > 0)
                {
                    newP = PAGE_EXECUTE_READ;
                }
                if (VirtualProtectEx(
                        hProc, pTargetBase + pSectionHeader->VirtualAddress,
                        pSectionHeader->Misc.VirtualSize, newP, &old))
                {
                    qDebug() << "section " << pSectionHeader->Name << " set as "
                             << Qt::hex << newP;
                }
                else
                {
                    qDebug() << "FAIL: section " << pSectionHeader->Name
                             << " not set as " << Qt::hex << newP;
                }
            }
        }
        DWORD old = 0;
        VirtualProtectEx(hProc, pTargetBase,
                         IMAGE_FIRST_SECTION(pOldNtHeader)->VirtualAddress,
                         PAGE_READONLY, &old);
    }

    if (!WriteProcessMemory(hProc, pShellcode, emptyBuffer, 0x1000, nullptr))
    {
        qDebug() << "WARNING: Can't clear shellcode";
    }
    if (!VirtualFreeEx(hProc, pShellcode, 0, MEM_RELEASE))
    {
        qDebug() << "WARNING: can't release shell code memory";
    }
    if (!VirtualFreeEx(hProc, MappingDataAlloc, 0, MEM_RELEASE))
    {
        qDebug() << "WARNING: can't release mapping data memory";
    }

    return true;
}

#ifdef _DEBUG
#pragma push_macro("_DEBUG")
#undef _DEBUG
#define SHELLCODE_RELEASE_MODE
#endif

#define RELOC_FLAG32(RelInfo) ((RelInfo >> 0x0C) == IMAGE_REL_BASED_HIGHLOW)
#define RELOC_FLAG64(RelInfo) ((RelInfo >> 0x0C) == IMAGE_REL_BASED_DIR64)

#ifdef _WIN64
#define RELOC_FLAG RELOC_FLAG64
#else
#define RELOC_FLAG RELOC_FLAG32
#endif

#pragma runtime_checks("", off)
#pragma optimize("", off)
void WINAPI Shellcode(MANUAL_MAPPING_DATA* pData)
{
    if (!pData)
    {
        pData->hMod = (HINSTANCE)0x404040;
        return;
    }

    BYTE* pBase = pData->pbase;
    auto* pOpt =
        &reinterpret_cast<IMAGE_NT_HEADERS*>(
             pBase +
             reinterpret_cast<IMAGE_DOS_HEADER*>((uintptr_t)pBase)->e_lfanew)
             ->OptionalHeader;

    auto _LoadLibraryA = pData->pLoadLibraryA;
    auto _GetProcAddress = pData->pGetProcAddress;
#ifdef _WIN64
    auto _RtlAddFunctionTable = pData->pRtlAddFunctionTable;
#endif
    auto _DllMain =
        reinterpret_cast<f_DLL_ENTRY_POINT>(pBase + pOpt->AddressOfEntryPoint);

    BYTE* LocationDelta = pBase - pOpt->ImageBase;
    if (LocationDelta)
    {
        if (pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size)
        {
            auto* pRelocData = reinterpret_cast<IMAGE_BASE_RELOCATION*>(
                pBase + pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC]
                            .VirtualAddress);
            const auto* pRelocEnd = reinterpret_cast<IMAGE_BASE_RELOCATION*>(
                reinterpret_cast<uintptr_t>(pRelocData) +
                pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size);
            while (pRelocData < pRelocEnd && pRelocData->SizeOfBlock)
            {
                UINT AmountOfEntries =
                    (pRelocData->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) /
                    sizeof(WORD);
                WORD* pRelativeInfo = reinterpret_cast<WORD*>(pRelocData + 1);

                for (UINT i = 0; i != AmountOfEntries; ++i, ++pRelativeInfo)
                {
                    if (RELOC_FLAG(*pRelativeInfo))
                    {
                        UINT_PTR* pPatch = reinterpret_cast<UINT_PTR*>(
                            pBase + pRelocData->VirtualAddress +
                            ((*pRelativeInfo) & 0xFFF));
                        *pPatch += reinterpret_cast<UINT_PTR>(LocationDelta);
                    }
                }
                pRelocData = reinterpret_cast<IMAGE_BASE_RELOCATION*>(
                    reinterpret_cast<BYTE*>(pRelocData) +
                    pRelocData->SizeOfBlock);
            }
        }
    }

    if (pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size)
    {
        auto* pImportDescr = reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(
            pBase +
            pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
        while (pImportDescr->Name)
        {
            char* szMod = reinterpret_cast<char*>(pBase + pImportDescr->Name);
            HINSTANCE hDll = _LoadLibraryA(szMod);

            ULONG_PTR* pThunkRef = reinterpret_cast<ULONG_PTR*>(
                pBase + pImportDescr->OriginalFirstThunk);
            ULONG_PTR* pFuncRef =
                reinterpret_cast<ULONG_PTR*>(pBase + pImportDescr->FirstThunk);

            if (!pThunkRef) pThunkRef = pFuncRef;

            for (; *pThunkRef; ++pThunkRef, ++pFuncRef)
            {
                if (IMAGE_SNAP_BY_ORDINAL(*pThunkRef))
                {
                    *pFuncRef = (ULONG_PTR)_GetProcAddress(
                        hDll, reinterpret_cast<char*>(*pThunkRef & 0xFFFF));
                }
                else
                {
                    auto* pImport = reinterpret_cast<IMAGE_IMPORT_BY_NAME*>(
                        pBase + (*pThunkRef));
                    *pFuncRef = (ULONG_PTR)_GetProcAddress(hDll, pImport->Name);
                }
            }
            ++pImportDescr;
        }
    }

    if (pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].Size)
    {
        auto* pTLS = reinterpret_cast<IMAGE_TLS_DIRECTORY*>(
            pBase +
            pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress);
        auto* pCallback =
            reinterpret_cast<PIMAGE_TLS_CALLBACK*>(pTLS->AddressOfCallBacks);
        for (; pCallback && *pCallback; ++pCallback)
            (*pCallback)(pBase, DLL_PROCESS_ATTACH, nullptr);
    }

    bool ExceptionSupportFailed = false;

#ifdef _WIN64

    if (pData->SEHSupport)
    {
        auto excep = pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION];
        if (excep.Size)
        {
            if (!_RtlAddFunctionTable(
                    reinterpret_cast<IMAGE_RUNTIME_FUNCTION_ENTRY*>(
                        pBase + excep.VirtualAddress),
                    excep.Size / sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY),
                    (DWORD64)pBase))
            {
                ExceptionSupportFailed = true;
            }
        }
    }

#endif
    _DllMain(pBase, pData->fdwReasonParam, pData->reservedParam);

    if (ExceptionSupportFailed)
        pData->hMod = reinterpret_cast<HINSTANCE>(0x505050);
    else
        pData->hMod = reinterpret_cast<HINSTANCE>(pBase);
}

#pragma runtime_checks("", off)
#pragma optimize("", off)

#pragma runtime_checks("", off)
#pragma optimize("", off)
void WINAPI DumpShellcode()
{
    BYTE* start = (BYTE*)Shellcode;
    SIZE_T size = 0x1000;

    qDebug() << "Shellcode start: 0x" << Qt::hex << (UINT_PTR)start;
    qDebug() << "Shellcode size: " << Qt::dec << size << " bytes";

#ifdef _WIN64
    // 生成C++数组格式
    std::ofstream file("shellcode_array64.h");
    file << "// Auto-generated shellcode array\n";
    file << "unsigned char g_Shellcode[] = {\n    ";

    for (SIZE_T i = 0; i < size; i++)
    {
        if (i % 16 == 0 && i != 0)
        {
            file << "\n    ";
        }
        file << "0x" << std::hex << std::setfill('0') << std::setw(2)
             << (unsigned int)start[i];
        if (i < size - 1) file << ", ";
    }

    file << "\n};\n";
    file << "SIZE_T g_ShellcodeSize = " << std::dec << size << ";\n";
    file.close();
#else
    // 生成C++数组格式
    std::ofstream file("shellcode_array32.h");
    file << "// Auto-generated shellcode array\n";
    file << "unsigned char g_Shellcode[] = {\n    ";

    for (SIZE_T i = 0; i < size; i++)
    {
        if (i % 16 == 0 && i != 0)
        {
            file << "\n    ";
        }
        file << "0x" << std::hex << std::setfill('0') << std::setw(2)
             << (unsigned int)start[i];
        if (i < size - 1) file << ", ";
    }

    file << "\n};\n";
    file << "SIZE_T g_ShellcodeSize = " << std::dec << size << ";\n";
    file.close();
#endif
}
