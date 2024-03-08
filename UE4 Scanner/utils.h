#pragma once
#include "common.h"
#include <Psapi.h>

namespace MEMORY
{
    BOOL WINAPI ReadProcessMemoryCallback(_In_ HANDLE hProcess, _In_ LPCVOID lpBaseAddress, LPVOID lpBuffer, _In_ SIZE_T nSize, _Out_opt_ SIZE_T* lpNumberOfBytesRead)
    {
        BOOL bRet = ReadProcessMemory(hProcess, lpBaseAddress, lpBuffer, nSize, lpNumberOfBytesRead);

        return bRet;
    }

    HANDLE Process(const char* ProcessName)
    {
        HANDLE hPID = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
        PROCESSENTRY32 ProcEntry;
        ProcEntry.dwSize = sizeof(ProcEntry);
        do
            if (!strcmp(ProcEntry.szExeFile, ProcessName))
            {
                PID = ProcEntry.th32ProcessID;
                CloseHandle(hPID);
                return hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PID);
            }
        while (Process32Next(hPID, &ProcEntry));
    }

    bool DataCompare(BYTE* data, BYTE* sign, char* mask)
    {
        for (; *mask; mask++, sign++, data++)
        {
            if (*mask == 'x' && *data != *sign)
            {
                return false;
            }
        }
        return true;
    }

    uintptr_t FindSignature(uintptr_t base, uintptr_t size, BYTE* sign, char* mask)
    {
        MEMORY_BASIC_INFORMATION mbi = { 0 };
        uintptr_t offset = 0;
        while (offset < size)
        {
            VirtualQueryEx(hProcess, (LPCVOID)(base + offset), &mbi, sizeof(MEMORY_BASIC_INFORMATION));
            if (mbi.State != MEM_FREE)
            {
                BYTE* buffer = new BYTE[mbi.RegionSize];
                ReadProcessMemory(hProcess, mbi.BaseAddress, buffer, mbi.RegionSize, NULL);
                for (int i = 0; i < mbi.RegionSize; i++)
                {
                    if (DataCompare(buffer + i, sign, mask))
                    {
                        delete[] buffer;
                        return (uintptr_t)mbi.BaseAddress + i;
                    }
                }

                delete[] buffer;
            }
            offset += mbi.RegionSize;
        }
        return 0;
    }

    uintptr_t Module(const char* ModuleName)
    {
        HANDLE hModule = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, PID);
        MODULEENTRY32 mEntry;
        mEntry.dwSize = sizeof(mEntry);

        do
            if (!strcmp(mEntry.szModule, ModuleName))
            {
                CloseHandle(hModule);
                return (uintptr_t)mEntry.modBaseAddr;
            }
        while (Module32Next(hModule, &mEntry));

        return 0;
    }

    uintptr_t ModuleSize(const char* ModuleName)
    {
        HANDLE hModule = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, PID);
        MODULEENTRY32 mEntry;
        mEntry.dwSize = sizeof(mEntry);

        do
            if (!strcmp(mEntry.szModule, ModuleName))
            {
                CloseHandle(hModule);
                return (uintptr_t)mEntry.modBaseSize;
            }
        while (Module32Next(hModule, &mEntry));

        return 0;
    }

    HMODULE GetModuleBaseAddress(HANDLE handle) {
        HMODULE hMods[1024];
        DWORD   cbNeeded;

        if (EnumProcessModules(handle, hMods, sizeof(hMods), &cbNeeded)) {
            return hMods[0];
        }
        return NULL;
    }

    DWORD GetModuleSize(char* pModuleName)
    {
        HMODULE hModule = GetModuleHandle(pModuleName);

        if (!hModule)
            return 0;

        PIMAGE_DOS_HEADER       pImage_Dos_Header = PIMAGE_DOS_HEADER(hModule);
        PIMAGE_NT_HEADERS       pImage_PE_Header = PIMAGE_NT_HEADERS(long(hModule) + pImage_Dos_Header->e_lfanew);
        PIMAGE_OPTIONAL_HEADER pImage_Optional_Header = &pImage_PE_Header->OptionalHeader;

        return pImage_Optional_Header->SizeOfCode;
    }

    template <class T>
    T Read(LPVOID ptr) {
        T out;
        ReadProcessMemoryCallback(hProcess, ptr, &out, sizeof(T), NULL);
        return out;
    }

    template <class T>
    void ReadTo(LPVOID ptr, T* out, int len) {
        ReadProcessMemoryCallback(hProcess, ptr, out, len, NULL);
    }

    ULONG_PTR ReadInt(LPVOID ptr) {
        return Read<ULONG_PTR>(ptr);
    }


}

namespace UTILS
{
    HWND GetSoTWindowProcessId(__out LPDWORD lpdwProcessId)
    {
        HWND hWnd = FindWindowA(NULL, "Sea of Thieves");
        if (hWnd != NULL)
        {
            if (!GetWindowThreadProcessId(hWnd, lpdwProcessId))
                return NULL;
        }
        return hWnd;
    }
}