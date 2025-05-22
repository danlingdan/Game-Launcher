//
// framework.h: 标准系统包含文件的集合和项目通用功能
//

#pragma once

//==============================================================================
// Windows 系统版本定义
//==============================================================================
// 开启对 Windows 8.1 及以上系统的支持
#ifndef WINVER
#define WINVER 0x0603        // Windows 8.1
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0603  // Windows 8.1
#endif

#include "../targetver.h"

//==============================================================================
// Windows 系统头文件
//==============================================================================
#define NOMINMAX  // 防止 Windows 头文件定义的 min/max 与 STL 冲突
#include <windows.h>
#pragma comment(lib, "Version.lib")
#include <CommCtrl.h>
#pragma comment(lib, "comctl32.lib")
#include <commdlg.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#include <shellapi.h>
#include <ShlObj.h>

//==============================================================================
// C/C++ 标准库头文件
//==============================================================================
// C 运行时头文件
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

// C++ 标准库
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <set>
#include <codecvt>
#include <locale>
#include <cwctype>
#include <thread>

// 第三方库
#include <nlohmann/json.hpp>

//==============================================================================
// DPI 相关定义和函数
//==============================================================================
// DPI 感知类型定义 (避免依赖较新的头文件)
#ifndef PROCESS_DPI_UNAWARE
#define PROCESS_DPI_UNAWARE 0          // DPI 无感知
#endif
#ifndef PROCESS_SYSTEM_DPI_AWARE
#define PROCESS_SYSTEM_DPI_AWARE 1     // 系统 DPI 感知
#endif
#ifndef PROCESS_PER_MONITOR_DPI_AWARE
#define PROCESS_PER_MONITOR_DPI_AWARE 2 // 每显示器 DPI 感知
#endif

// DPI 相关函数类型定义
typedef BOOL(WINAPI* FnSetProcessDPIAware)(VOID);
typedef HRESULT(WINAPI* FnSetProcessDpiAwareness)(int dpiAwareness);
typedef HRESULT(WINAPI* FnGetDpiForMonitor)(HMONITOR hmonitor, int dpiType, UINT* dpiX, UINT* dpiY);

/**
 * 启用 DPI 感知
 * 优先使用 Windows 8.1+ API，失败后回退到旧版 API
 * @return 是否成功启用 DPI 感知
 */
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

/**
 * 获取窗口所在显示器的 DPI
 * @param hwnd 窗口句柄
 * @param dpiX 用于返回水平 DPI
 * @param dpiY 用于返回垂直 DPI
 * @return 是否成功获取 DPI
 */
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