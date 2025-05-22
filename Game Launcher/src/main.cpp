#include "framework.h"
#include "gui/MainWindow.h"

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // 启用 DPI 感知，适用于所有 Windows 版本
    EnableDPIAwareness();

    MainWindow mainWindow;

    if (!mainWindow.Create(hInstance, nCmdShow)) {
        MessageBox(nullptr, _T("无法创建主窗口"), _T("错误"), MB_ICONERROR);
        return -1;
    }

    MSG msg;

    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}
