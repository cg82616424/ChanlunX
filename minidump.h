#pragma once
#include <windows.h>
#include <DbgHelp.h>
#include <cstdlib>
#include <cmath>
#pragma comment(lib, "dbghelp.lib")
#pragma warning(disable:4996) //全部关掉
#pragma warning(once:4996) //仅显示一个
using namespace std;
/*
#ifndef _M_IX86
#error "The following code only works for x86!"
#endif
*/

inline BOOL IsDataSectionNeeded(const WCHAR* pModuleName)
{
    if (pModuleName == 0)
    {
        return FALSE;
    }

    WCHAR szFileName[_MAX_FNAME] = L"";
    _wsplitpath(pModuleName, NULL, NULL, szFileName, NULL);

    if (wcsicmp(szFileName, L"ntdll") == 0)
        return TRUE;

    return FALSE;
}
inline BOOL CALLBACK MiniDumpCallback(PVOID                            pParam,
    const PMINIDUMP_CALLBACK_INPUT   pInput,
    PMINIDUMP_CALLBACK_OUTPUT        pOutput)
{
    if (pInput == 0 || pOutput == 0)
        return FALSE;

    switch (pInput->CallbackType)
    {
    case ModuleCallback:
        if (pOutput->ModuleWriteFlags & ModuleWriteDataSeg)
            if (!IsDataSectionNeeded(pInput->Module.FullPath))
                pOutput->ModuleWriteFlags &= (~ModuleWriteDataSeg);
    case IncludeModuleCallback:
    case IncludeThreadCallback:
    case ThreadCallback:
    case ThreadExCallback:
        return TRUE;
    default:;
    }

    return FALSE;
}

inline void CreateMiniDump(PEXCEPTION_POINTERS pep, LPCTSTR strFileName)
{
    HANDLE hFile = CreateFile(strFileName, GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if ((hFile != NULL) && (hFile != INVALID_HANDLE_VALUE))
    {
        MINIDUMP_EXCEPTION_INFORMATION mdei;
        mdei.ThreadId = GetCurrentThreadId();
        mdei.ExceptionPointers = pep;
        mdei.ClientPointers = NULL;

        MINIDUMP_CALLBACK_INFORMATION mci;
        mci.CallbackRoutine = (MINIDUMP_CALLBACK_ROUTINE)MiniDumpCallback;
        mci.CallbackParam = 0;

        ::MiniDumpWriteDump(::GetCurrentProcess(), ::GetCurrentProcessId(), hFile, MiniDumpNormal, (pep != 0) ? &mdei : 0, NULL, &mci);

        CloseHandle(hFile);
    }
}

LONG __stdcall MyUnhandledExceptionFilter(PEXCEPTION_POINTERS pExceptionInfo)
{
    CreateMiniDump(pExceptionInfo, "orderBookMatchEngine.dmp");

    // EXCEPTION_EXECUTE_HANDLER equ 1 表示我已经处理了异常, 可以优雅地结束了
    // EXCEPTION_CONTINUE_SEARCH equ 0 表示我不处理, 其他人来吧, 于是windows调用默认的处理程序显示一个错误框, 并结束
    // EXCEPTION_CONTINUE_EXECUTION equ - 1 表示错误已经被修复, 请从异常发生处继续执行
    return EXCEPTION_EXECUTE_HANDLER;
}

// 此函数一旦成功调用，之后对 SetUnhandledExceptionFilter 的调用将无效
void DisableSetUnhandledExceptionFilter()
{
    void* addr = (void*)GetProcAddress(LoadLibrary("kernel32.dll"),
        "SetUnhandledExceptionFilter");

    if (addr)
    {
        unsigned char code[16];
        int size = 0;

        code[size++] = 0x33;
        code[size++] = 0xC0;
        code[size++] = 0xC2;
        code[size++] = 0x04;
        code[size++] = 0x00;

        DWORD dwOldFlag, dwTempFlag;
        VirtualProtect(addr, size, PAGE_READWRITE, &dwOldFlag);
        WriteProcessMemory(GetCurrentProcess(), addr, code, size, NULL);
        VirtualProtect(addr, size, dwOldFlag, &dwTempFlag);
    }
}
LPTOP_LEVEL_EXCEPTION_FILTER WINAPI MyDummySetUnhandledExceptionFilter(LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter)
{
    return NULL;
}

BOOL PreventSetUnhandledExceptionFilter()
{
    HMODULE hKernel32 = LoadLibrary("kernel32.dll");
    if (hKernel32 == NULL) return FALSE;
    void* pOrgEntry = GetProcAddress(hKernel32, "SetUnhandledExceptionFilter");
    if (pOrgEntry == NULL) return FALSE;
    unsigned char newJump[100];
    DWORD dwOrgEntryAddr = (DWORD)pOrgEntry;
    dwOrgEntryAddr += 5; // add 5 for 5 op-codes for jmp far
    void* pNewFunc = &MyDummySetUnhandledExceptionFilter;
    DWORD dwNewEntryAddr = (DWORD)pNewFunc;
    DWORD dwRelativeAddr = dwNewEntryAddr - dwOrgEntryAddr;

    newJump[0] = 0xE9;  // JMP absolute
    memcpy(&newJump[1], &dwRelativeAddr, sizeof(pNewFunc));
    SIZE_T bytesWritten;
    BOOL bRet = WriteProcessMemory(GetCurrentProcess(),
        pOrgEntry, newJump, sizeof(pNewFunc) + 1, &bytesWritten);
    return bRet;
}

void InitMinDump()
{
    //注册异常处理函数
    SetUnhandledExceptionFilter(MyUnhandledExceptionFilter);

    // PreventSetUnhandledExceptionFilter();
    //使SetUnhandledExceptionFilter
    DisableSetUnhandledExceptionFilter();
}
