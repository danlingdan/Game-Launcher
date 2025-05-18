#include "GameDialog.h"

//函数声明
std::wstring GetExeFileInfoString(const std::wstring& filePath, const std::wstring& key);

GameDialog::GameDialog() : m_pGame(nullptr), m_bEditing(false) {
}

GameDialog::~GameDialog() {
}

bool GameDialog::ShowAddDialog(HWND hParent, Game& game, GameCollection& collection) {
    m_pGame = &game;
    m_bEditing = false;
    m_pCollection = &collection;
    return DialogBoxParam(
        GetModuleHandle(NULL),
        MAKEINTRESOURCE(IDD_DIALOG_GAME),
        hParent,
        DialogProc,
        (LPARAM)this
    ) == IDOK;
}

bool GameDialog::ShowEditDialog(HWND hParent, Game& game, GameCollection& collection) {
    m_pGame = &game;
    m_bEditing = true;
    m_pCollection = &collection;
    return DialogBoxParam(
        GetModuleHandle(NULL),
        MAKEINTRESOURCE(IDD_DIALOG_GAME),
        hParent,
        DialogProc,
        (LPARAM)this
    ) == IDOK;
}

INT_PTR CALLBACK GameDialog::DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    GameDialog* pThis = (GameDialog*)GetWindowLongPtr(hDlg, GWLP_USERDATA);
    if (message == WM_INITDIALOG) {
        pThis = (GameDialog*)lParam;
        SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)pThis);
        pThis->InitDialog(hDlg);
        return TRUE;
    }
    if (pThis) {
        switch (message) {
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
            case IDOK:
                if (pThis->ValidateDialog(hDlg)) {
                    pThis->GetDialogData(hDlg);
                    EndDialog(hDlg, IDOK);
                }
                return TRUE;
            case IDCANCEL:
                EndDialog(hDlg, IDCANCEL);
                return TRUE;
            case IDC_BROWSE_EXECUTABLE:
                pThis->BrowseExecutable(hDlg);
                return TRUE;
            case IDC_BROWSE_ICON:
                pThis->BrowseIcon(hDlg);
                return TRUE;
            }
            break;
        }
    }
    return FALSE;
}

INT_PTR GameDialog::HandleMessage(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_INIT_DIALOG:
        InitDialog(hDlg);
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
            if (ValidateDialog(hDlg)) {
                GetDialogData(hDlg);
                EndDialog(hDlg, IDOK);
            }
            return TRUE;

        case IDCANCEL:
            EndDialog(hDlg, IDCANCEL);
            return TRUE;

        case IDC_BROWSE_EXECUTABLE:
            BrowseExecutable(hDlg);
            return TRUE;

        case IDC_BROWSE_ICON:
            BrowseIcon(hDlg);
            return TRUE;
        }
        break;
    }

    return FALSE;
}

void GameDialog::InitDialog(HWND hDlg) {
    // 设置对话框标题
    SetWindowText(hDlg, m_bEditing ? L"编辑游戏" : L"添加游戏");

    // 初始化各控件内容
    SetDlgItemText(hDlg, IDC_GAME_NAME, m_pGame->GetName().c_str());
    SetDlgItemText(hDlg, IDC_RELEASE_YEAR, m_pGame->GetReleaseYear().c_str());
    SetDlgItemText(hDlg, IDC_PUBLISHER, m_pGame->GetPublisher().c_str());
    SetDlgItemText(hDlg, IDC_LANGUAGE, m_pGame->GetLanguage().c_str());
    SetDlgItemText(hDlg, IDC_CATEGORY, m_pGame->GetCategory().c_str());
    SetDlgItemText(hDlg, IDC_PLATFORM, m_pGame->GetPlatform().c_str());
    SetDlgItemText(hDlg, IDC_EXECUTABLE_PATH, m_pGame->GetExecutablePath().c_str());
    SetDlgItemText(hDlg, IDC_ICON_PATH, m_pGame->GetIconPath().c_str());

    // 使用固定分类初始化下拉框
    InitCategoryComboBox(hDlg);

    // 设置当前分类（如果有）
    std::wstring currentCategory = m_pGame->GetCategory();
    if (!currentCategory.empty()) {
        // 查找匹配项并选中
        int index = SendMessage(GetDlgItem(hDlg, IDC_CATEGORY), CB_FINDSTRINGEXACT, -1, (LPARAM)currentCategory.c_str());
        if (index != CB_ERR) {
            SendMessage(GetDlgItem(hDlg, IDC_CATEGORY), CB_SETCURSEL, index, 0);
        }
        else if (SendMessage(GetDlgItem(hDlg, IDC_CATEGORY), CB_GETCOUNT, 0, 0) > 0) {
            // 如果找不到匹配项，选择第一个分类
            SendMessage(GetDlgItem(hDlg, IDC_CATEGORY), CB_SETCURSEL, 0, 0);
        }
    }
    else if (SendMessage(GetDlgItem(hDlg, IDC_CATEGORY), CB_GETCOUNT, 0, 0) > 0) {
        // 如果当前无分类但下拉框有项目，选中第一项
        SendMessage(GetDlgItem(hDlg, IDC_CATEGORY), CB_SETCURSEL, 0, 0);
    }
}
void GameDialog::InitCategoryComboBox(HWND hDlg) {
    HWND hCategory = GetDlgItem(hDlg, IDC_CATEGORY);

    // 清空原有内容
    SendMessage(hCategory, CB_RESETCONTENT, 0, 0);

    // 添加预定义分类（跳过"全部游戏"选项，因为它只是一个筛选选项）
    for (size_t i = 1; i < PREDEFINED_GAME_CATEGORIES.size(); ++i) {
        SendMessage(hCategory, CB_ADDSTRING, 0, (LPARAM)PREDEFINED_GAME_CATEGORIES[i].c_str());
    }

    // 默认选中第一个实际分类
    SendMessage(hCategory, CB_SETCURSEL, 0, 0);
}

void GameDialog::GetDialogData(HWND hDlg) {
    WCHAR buffer[1024];

    GetDlgItemText(hDlg, IDC_GAME_NAME, buffer, 1024);
    m_pGame->SetName(buffer);

    GetDlgItemText(hDlg, IDC_RELEASE_YEAR, buffer, 1024);
    m_pGame->SetReleaseYear(buffer);

    GetDlgItemText(hDlg, IDC_PUBLISHER, buffer, 1024);
    m_pGame->SetPublisher(buffer);

    GetDlgItemText(hDlg, IDC_LANGUAGE, buffer, 1024);
    m_pGame->SetLanguage(buffer);

    GetDlgItemText(hDlg, IDC_PLATFORM, buffer, 1024);
    m_pGame->SetPlatform(buffer);

    GetDlgItemText(hDlg, IDC_EXECUTABLE_PATH, buffer, 1024);
    m_pGame->SetExecutablePath(buffer);

    GetDlgItemText(hDlg, IDC_ICON_PATH, buffer, 1024);
    m_pGame->SetIconPath(buffer);

    GetDlgItemText(hDlg, IDC_CATEGORY, buffer, 1024);
    m_pGame->SetCategory(buffer);
}

void GameDialog::BrowseExecutable(HWND hDlg) {
    WCHAR fileName[MAX_PATH] = { 0 };

    OPENFILENAME ofn = { 0 };
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hDlg;
    ofn.lpstrFilter = L"可执行文件 (*.exe)\0*.exe\0所有文件 (*.*)\0*.*\0";
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    if (GetOpenFileName(&ofn)) {
        SetDlgItemText(hDlg, IDC_EXECUTABLE_PATH, fileName);

        // 自动检测并填充游戏信息
        std::wstring exePath = fileName;
        std::wstring productName = GetExeFileInfoString(exePath, L"ProductName");
        std::wstring companyName = GetExeFileInfoString(exePath, L"CompanyName");
        std::wstring fileDescription = GetExeFileInfoString(exePath, L"FileDescription");

        // 只在对应控件为空时填充，避免覆盖用户输入
        WCHAR buffer[1024];
        if (GetDlgItemText(hDlg, IDC_GAME_NAME, buffer, 1024) == 0 && !productName.empty())
            SetDlgItemText(hDlg, IDC_GAME_NAME, productName.c_str());
        else if (GetDlgItemText(hDlg, IDC_GAME_NAME, buffer, 1024) == 0 && !fileDescription.empty())
            SetDlgItemText(hDlg, IDC_GAME_NAME, fileDescription.c_str());

        if (GetDlgItemText(hDlg, IDC_PUBLISHER, buffer, 1024) == 0 && !companyName.empty())
            SetDlgItemText(hDlg, IDC_PUBLISHER, companyName.c_str());
    }
}

void GameDialog::BrowseIcon(HWND hDlg) {
    WCHAR fileName[MAX_PATH] = { 0 };

    OPENFILENAME ofn = { 0 };
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hDlg;
    ofn.lpstrFilter = L"图像文件 (*.ico;*.bmp;*.png;*.jpg)\0*.ico;*.bmp;*.png;*.jpg\0所有文件 (*.*)\0*.*\0";
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    if (GetOpenFileName(&ofn)) {
        SetDlgItemText(hDlg, IDC_ICON_PATH, fileName);
    }
}

bool GameDialog::ValidateDialog(HWND hDlg) {
    WCHAR buffer[1024];

    // 验证游戏名称
    GetDlgItemText(hDlg, IDC_GAME_NAME, buffer, 1024);
    if (wcslen(buffer) == 0) {
        MessageBox(hDlg, L"请输入游戏名称", L"验证失败", MB_OK | MB_ICONERROR);
        SetFocus(GetDlgItem(hDlg, IDC_GAME_NAME));
        return false;
    }

    return true;
}

// 辅助函数：获取 exe 文件的版本信息字段
std::wstring GetExeFileInfoString(const std::wstring& filePath, const std::wstring& key) {
    DWORD dummy;
    DWORD size = GetFileVersionInfoSizeW(filePath.c_str(), &dummy);
    if (size == 0) return L"";
    std::vector<BYTE> data(size);
    if (!GetFileVersionInfoW(filePath.c_str(), 0, size, data.data())) return L"";

    struct LANGANDCODEPAGE {
        WORD wLanguage;
        WORD wCodePage;
    } *lpTranslate;

    UINT cbTranslate = 0;
    if (!VerQueryValueW(data.data(), L"\\VarFileInfo\\Translation", (LPVOID*)&lpTranslate, &cbTranslate) || cbTranslate == 0)
        return L"";

    WCHAR subBlock[256];
    swprintf_s(subBlock, 256, L"\\StringFileInfo\\%04x%04x\\%s", lpTranslate[0].wLanguage, lpTranslate[0].wCodePage, key.c_str());

    LPVOID lpBuffer = nullptr;
    UINT sizeBuffer = 0;
    if (VerQueryValueW(data.data(), subBlock, &lpBuffer, &sizeBuffer) && lpBuffer)
        return std::wstring((WCHAR*)lpBuffer);

    return L"";
}