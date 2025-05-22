#include "GameDialog.h"

// ��������
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

    if (!pThis) {
        return FALSE;
    }

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

        case IDC_EXECUTABLE_PATH:
            // ����Ǳ༭�ı����֪ͨ
            if (HIWORD(wParam) == EN_CHANGE) {
                // ������������Ӷ����߼�����������/���ð�ť
            }
            return TRUE;
        }
        break;
    }

    return FALSE;
}

void GameDialog::InitDialog(HWND hDlg) {
    // ���öԻ������
    SetWindowText(hDlg, m_bEditing ? L"�༭��Ϸ" : L"�����Ϸ");

    // ��ʼ�����ؼ�����
    SetDlgItemText(hDlg, IDC_GAME_NAME, m_pGame->GetName().c_str());
    SetDlgItemText(hDlg, IDC_RELEASE_YEAR, m_pGame->GetReleaseYear().c_str());
    SetDlgItemText(hDlg, IDC_PUBLISHER, m_pGame->GetPublisher().c_str());
    SetDlgItemText(hDlg, IDC_LANGUAGE, m_pGame->GetLanguage().c_str());
    SetDlgItemText(hDlg, IDC_CATEGORY, m_pGame->GetCategory().c_str());
    SetDlgItemText(hDlg, IDC_PLATFORM, m_pGame->GetPlatform().c_str());
    SetDlgItemText(hDlg, IDC_EXECUTABLE_PATH, m_pGame->GetExecutablePath().c_str());
    SetDlgItemText(hDlg, IDC_ICON_PATH, m_pGame->GetIconPath().c_str());
    SetDlgItemText(hDlg, IDC_LAUNCH_PARAMS, m_pGame->GetLaunchParams().c_str());

    // ʹ�ù̶������ʼ��������
    InitCategoryComboBox(hDlg);

    // ���õ�ǰ���ࣨ����У�
    std::wstring currentCategory = m_pGame->GetCategory();
    if (!currentCategory.empty()) {
        // ����ƥ���ѡ��
        int index = SendMessage(GetDlgItem(hDlg, IDC_CATEGORY), CB_FINDSTRINGEXACT, -1, (LPARAM)currentCategory.c_str());
        if (index != CB_ERR) {
            SendMessage(GetDlgItem(hDlg, IDC_CATEGORY), CB_SETCURSEL, index, 0);
        }
        else if (SendMessage(GetDlgItem(hDlg, IDC_CATEGORY), CB_GETCOUNT, 0, 0) > 0) {
            // ����Ҳ���ƥ���ѡ���һ������
            SendMessage(GetDlgItem(hDlg, IDC_CATEGORY), CB_SETCURSEL, 0, 0);
        }
    }
    else if (SendMessage(GetDlgItem(hDlg, IDC_CATEGORY), CB_GETCOUNT, 0, 0) > 0) {
        // �����ǰ�޷��൫����������Ŀ��ѡ�е�һ��
        SendMessage(GetDlgItem(hDlg, IDC_CATEGORY), CB_SETCURSEL, 0, 0);
    }
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

    GetDlgItemText(hDlg, IDC_LAUNCH_PARAMS, buffer, 1024);
    m_pGame->SetLaunchParams(buffer);
}

void GameDialog::InitCategoryComboBox(HWND hDlg) {
    // ��ȡ��������
    HWND hCategoryCombo = GetDlgItem(hDlg, IDC_CATEGORY);

    // ���������Ŀ
    SendMessage(hCategoryCombo, CB_RESETCONTENT, 0, 0);

    // ����Ϸ���ϻ�ȡ���з���
    std::vector<std::wstring> categories = m_pCollection->GetAllCategories();

    // ���"δ����"ѡ��
    SendMessage(hCategoryCombo, CB_ADDSTRING, 0, (LPARAM)L"δ����");

    // ������з��ൽ������
    for (const auto& category : categories) {
        if (!category.empty() && category != L"δ����") {
            SendMessage(hCategoryCombo, CB_ADDSTRING, 0, (LPARAM)category.c_str());
        }
    }
}

void GameDialog::BrowseExecutable(HWND hDlg) {
    WCHAR fileName[MAX_PATH] = { 0 };

    OPENFILENAME ofn = { 0 };
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hDlg;
    ofn.lpstrFilter = L"��ִ���ļ� (*.exe)\0*.exe\0�����ļ� (*.*)\0*.*\0";
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    if (GetOpenFileName(&ofn)) {
        SetDlgItemText(hDlg, IDC_EXECUTABLE_PATH, fileName);

        // �Զ���Ⲣ�����Ϸ��Ϣ
        std::wstring exePath = fileName;
        std::wstring productName = GetExeFileInfoString(exePath, L"ProductName");
        std::wstring companyName = GetExeFileInfoString(exePath, L"CompanyName");
        std::wstring fileDescription = GetExeFileInfoString(exePath, L"FileDescription");

        // ֻ�ڶ�Ӧ�ؼ�Ϊ��ʱ��䣬���⸲���û�����
        WCHAR buffer[1024];
        if (GetDlgItemText(hDlg, IDC_GAME_NAME, buffer, 1024) == 0) {
            if (!productName.empty())
                SetDlgItemText(hDlg, IDC_GAME_NAME, productName.c_str());
            else if (!fileDescription.empty())
                SetDlgItemText(hDlg, IDC_GAME_NAME, fileDescription.c_str());
        }

        if (GetDlgItemText(hDlg, IDC_PUBLISHER, buffer, 1024) == 0 && !companyName.empty())
            SetDlgItemText(hDlg, IDC_PUBLISHER, companyName.c_str());

        // ���Դӿ�ִ���ļ��Զ���ȡͼ��
        if (GetDlgItemText(hDlg, IDC_ICON_PATH, buffer, 1024) == 0) {
            // ���������Ӵ�EXE��ȡͼ��Ĺ���
            // ExtractIconFromExecutable(exePath, iconPath)
        }
    }
}

void GameDialog::BrowseIcon(HWND hDlg) {
    WCHAR fileName[MAX_PATH] = { 0 };

    OPENFILENAME ofn = { 0 };
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hDlg;
    ofn.lpstrFilter = L"ͼ���ļ� (*.ico;*.bmp;*.png;*.jpg)\0*.ico;*.bmp;*.png;*.jpg\0�����ļ� (*.*)\0*.*\0";
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    if (GetOpenFileName(&ofn)) {
        SetDlgItemText(hDlg, IDC_ICON_PATH, fileName);
    }
}

bool GameDialog::ValidateDialog(HWND hDlg) {
    WCHAR buffer[1024];

    // ��֤��Ϸ����
    GetDlgItemText(hDlg, IDC_GAME_NAME, buffer, 1024);
    if (wcslen(buffer) == 0) {
        MessageBox(hDlg, L"��������Ϸ����", L"��֤ʧ��", MB_OK | MB_ICONERROR);
        SetFocus(GetDlgItem(hDlg, IDC_GAME_NAME));
        return false;
    }

    // ��֤��ִ���ļ�·��
    GetDlgItemText(hDlg, IDC_EXECUTABLE_PATH, buffer, 1024);
    if (wcslen(buffer) == 0) {
        MessageBox(hDlg, L"��ѡ����Ϸ��ִ���ļ�", L"��֤ʧ��", MB_OK | MB_ICONERROR);
        SetFocus(GetDlgItem(hDlg, IDC_EXECUTABLE_PATH));
        return false;
    }

    // ����ļ��Ƿ����
    if (GetFileAttributes(buffer) == INVALID_FILE_ATTRIBUTES) {
        MessageBox(hDlg, L"ָ���Ŀ�ִ���ļ�������", L"��֤ʧ��", MB_OK | MB_ICONERROR);
        SetFocus(GetDlgItem(hDlg, IDC_EXECUTABLE_PATH));
        return false;
    }

    return true;
}

// ������������ȡ exe �ļ��İ汾��Ϣ�ֶ�
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