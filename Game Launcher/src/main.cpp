#include "framework.h"
#include "gui/MainWindow.h"

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // ���� DPI ��֪������������ Windows �汾
    EnableDPIAwareness();

    MainWindow mainWindow;

    if (!mainWindow.Create(hInstance, nCmdShow)) {
        MessageBox(nullptr, _T("�޷�����������"), _T("����"), MB_ICONERROR);
        return -1;
    }

    MSG msg;

    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}
