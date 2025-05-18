// header.h: 标准系统包含文件的包含文件，
// 或特定于项目的包含文件
//

#pragma once

// 开启对 Windows 8.1 及以上系统的支持
#ifndef WINVER
#define WINVER 0x0603        // Windows 8.1
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0603  // Windows 8.1
#endif

#include "../targetver.h"
// Windows 头文件
#define NOMINMAX
#include <windows.h>
#pragma comment(lib, "Version.lib")
#include <CommCtrl.h>
#pragma comment(lib, "comctl32.lib")
#include <commdlg.h>

#include <nlohmann/json.hpp>
#include <codecvt>
#include <locale>

// C 运行时头文件
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <shlwapi.h>
#include <shellapi.h>
#pragma comment(lib, "shlwapi.lib")
#include <sstream>
#include <fstream>
#include <algorithm>
#include <set>
#include <ShlObj.h>
#include <cwctype>

// DPI 相关定义 - 避免依赖较新的头文件
#ifndef PROCESS_DPI_UNAWARE
#define PROCESS_DPI_UNAWARE 0
#endif
#ifndef PROCESS_SYSTEM_DPI_AWARE
#define PROCESS_SYSTEM_DPI_AWARE 1
#endif
#ifndef PROCESS_PER_MONITOR_DPI_AWARE
#define PROCESS_PER_MONITOR_DPI_AWARE 2
#endif

// 通用 DPI 感知函数原型
typedef BOOL(WINAPI* FnSetProcessDPIAware)(VOID);
typedef HRESULT(WINAPI* FnSetProcessDpiAwareness)(int dpiAwareness);
typedef HRESULT(WINAPI* FnGetDpiForMonitor)(HMONITOR hmonitor, int dpiType, UINT* dpiX, UINT* dpiY);

// 帮助函数 - 启用 DPI 感知
inline bool EnableDPIAwareness() {
    bool success = false;

    // 尝试使用 Windows 8.1 及更高版本的 API
    HMODULE hShcore = LoadLibraryW(L"Shcore.dll");
    if (hShcore) {
        FnSetProcessDpiAwareness fnSetProcessDpiAwareness =
            (FnSetProcessDpiAwareness)GetProcAddress(hShcore, "SetProcessDpiAwareness");

        if (fnSetProcessDpiAwareness) {
            // 尝试设置为每显示器 DPI 感知
            HRESULT hr = fnSetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
            success = SUCCEEDED(hr);
        }
        FreeLibrary(hShcore);
    }

    // 如果上述方法失败，尝试使用旧版 Windows 的方法
    if (!success) {
        HMODULE hUser32 = LoadLibraryW(L"user32.dll");
        if (hUser32) {
            FnSetProcessDPIAware fnSetProcessDPIAware =
                (FnSetProcessDPIAware)GetProcAddress(hUser32, "SetProcessDPIAware");

            if (fnSetProcessDPIAware) {
                success = fnSetProcessDPIAware() != 0;
            }
            FreeLibrary(hUser32);
        }
    }

    return success;
}

// 获取显示器 DPI 帮助函数
inline bool GetMonitorDpiForWindow(HWND hwnd, UINT& dpiX, UINT& dpiY) {
    dpiX = dpiY = 96; // 默认 DPI

    // 尝试使用 Windows 8.1+ API
    HMODULE hShcore = LoadLibraryW(L"Shcore.dll");
    if (hShcore) {
        FnGetDpiForMonitor fnGetDpiForMonitor =
            (FnGetDpiForMonitor)GetProcAddress(hShcore, "GetDpiForMonitor");

        if (fnGetDpiForMonitor) {
            HMONITOR hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
            if (hMonitor) {
                HRESULT hr = fnGetDpiForMonitor(hMonitor, 0 /*MDT_EFFECTIVE_DPI*/, &dpiX, &dpiY);
                if (SUCCEEDED(hr)) {
                    FreeLibrary(hShcore);
                    return true;
                }
            }
        }
        FreeLibrary(hShcore);
    }

    // 回退到旧版方法
    HDC hdc = GetDC(hwnd);
    if (hdc) {
        dpiX = GetDeviceCaps(hdc, LOGPIXELSX);
        dpiY = GetDeviceCaps(hdc, LOGPIXELSY);
        ReleaseDC(hwnd, hdc);
        return true;
    }

    return false;
}