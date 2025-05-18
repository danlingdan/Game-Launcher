#include "MainWindow.h"

#define WINDOW_CLASS_NAME _T("GameLauncherMainWindow")
#define WINDOW_TITLE _T("Game Launcher")
#define SIDEBAR_WIDTH 180

MainWindow::MainWindow() : m_hwnd(nullptr), m_sidebarHwnd(nullptr),
m_toolbarHwnd(nullptr), m_contentHwnd(nullptr), m_searchHwnd(nullptr),
m_pageInputHwnd(nullptr), m_hMenu(nullptr), m_hoverMenuItem(-1),
m_currentPage(0), m_totalPages(1), m_gamesPerPage(8) {
    // ��ʼ�������˵���
    m_menuItems = {
        { L"��Ϸ", {0}, ID_MENU_GAME, false },
        { L"����", {0}, ID_MENU_BOX, false },
        { L"����", {0}, ID_MENU_COMMUNITY, false },
        { L"����", {0}, ID_MENU_CONFIG, false },
        { L"����", {0}, ID_MENU_HELP, false }
    };

    // ��ʼ��������ͼ�갴ť
    m_toolbarButtons = {
        { L"[T]", {0}, 3001, false }, // ����ͼ��ͼ
        { L"[L]", {0}, 3002, false }, // �б���ͼ
        { L"[D]", {0}, 3003, false }, // ������ͼ
        { L"[G]", {0}, 3004, false }, // ������ͼ
        { L"[B]", {0}, 3005, false }, // ��������ͼ
        { L"[O]", {0}, 3006, false }, // ������ͼ
        { L"[F]", {0}, 3007, false }, // �ļ���ͼ
        { L"[+]", {0}, 3008, false }  // �����ͼ
    };

    // ��������
    memset(&m_titleFont, 0, sizeof(LOGFONT));
    m_titleFont.lfHeight = 22;
    m_titleFont.lfWeight = FW_BOLD;
    wcscpy_s(m_titleFont.lfFaceName, LF_FACESIZE, L"Arial");

    memset(&m_menuFont, 0, sizeof(LOGFONT));
    m_menuFont.lfHeight = 18;
    wcscpy_s(m_menuFont.lfFaceName, LF_FACESIZE, L"Microsoft YaHei"); 

    // ������Ϸ����
    m_gameCollection.LoadGames();
}

MainWindow::~MainWindow() {
    if (m_hTitleFont) DeleteObject(m_hTitleFont);
    if (m_hMenuFont) DeleteObject(m_hMenuFont);
}

bool MainWindow::Create(HINSTANCE hInstance, int nCmdShow) {
    // ��ʼ��ͨ�ÿؼ�
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&icex);

    // ��������
    m_hTitleFont = CreateFontIndirect(&m_titleFont);
    m_hMenuFont = CreateFontIndirect(&m_menuFont);

    // ע�ᴰ����
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = MainWindow::WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = WINDOW_CLASS_NAME;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GAMELAUNCHER));
    RegisterClass(&wc);

    // ע������������
    WNDCLASS wcSidebar = { 0 };
    wcSidebar.lpfnWndProc = MainWindow::SidebarWndProc;
    wcSidebar.hInstance = hInstance;
    wcSidebar.lpszClassName = L"SidebarWindowClass";
    wcSidebar.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClass(&wcSidebar);

    // ��Ϸ��
    WNDCLASS wcContent = { 0 };
    wcContent.lpfnWndProc = MainWindow::ContentWndProc;
    wcContent.hInstance = hInstance;
    wcContent.lpszClassName = L"GameContentArea";
    wcContent.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClass(&wcContent);

    // ��������
    m_hwnd = CreateWindowEx(0, WINDOW_CLASS_NAME, WINDOW_TITLE,
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 1000, 700,
        nullptr, nullptr, hInstance, this);
    if (!m_hwnd) return false;

    ShowWindow(m_hwnd, nCmdShow);
    UpdateWindow(m_hwnd);
    return true;
}

void MainWindow::OnCreate(HWND hwnd) {
    HINSTANCE hInstance = GetModuleHandle(nullptr);
    RECT rcClient;
    GetClientRect(hwnd, &rcClient);
    int clientWidth = rcClient.right - rcClient.left;
    int clientHeight = rcClient.bottom - rcClient.top;

    // ����������
    m_searchHwnd = CreateWindowEx(0, _T("EDIT"), _T(""),
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
        clientWidth - 300, TOPBAR_HEIGHT + 5, 200, 20,
        hwnd, (HMENU)ID_SEARCH_BOX, hInstance, NULL);

    // ���������
    m_sidebarHwnd = CreateWindowEx(0, L"SidebarWindowClass", _T(""),
        WS_CHILD | WS_VISIBLE | SS_NOTIFY | SS_SUNKEN,
        0, TOPBAR_HEIGHT + CATEGORY_HEIGHT, SIDEBAR_WIDTH,
        clientHeight - TOPBAR_HEIGHT - CATEGORY_HEIGHT - STATUSBAR_HEIGHT,
        hwnd, (HMENU)ID_SIDEBAR, hInstance, NULL);

    // ������Ϸ������
    m_contentHwnd = CreateWindowEx(0, L"GameContentArea", L"",
        WS_CHILD | WS_VISIBLE,
        SIDEBAR_WIDTH, TOPBAR_HEIGHT + CATEGORY_HEIGHT,
        clientWidth - SIDEBAR_WIDTH,
        clientHeight - TOPBAR_HEIGHT - CATEGORY_HEIGHT - STATUSBAR_HEIGHT,
        hwnd, (HMENU)ID_CONTENT_AREA, hInstance, this);

    // �����ײ���ҳ��ť
    int btnWidth = 80;
    int btnHeight = 25;
    int btnY = clientHeight - STATUSBAR_HEIGHT / 2 - btnHeight / 2;
    int startX = (clientWidth - 5 * btnWidth - 40) / 2; // 40 ��ҳ���������

    CreateWindowEx(0, _T("BUTTON"), _T("��ҳ"),
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        startX, btnY, btnWidth, btnHeight,
        hwnd, (HMENU)PAGING_BTN_FIRST, hInstance, NULL);

    CreateWindowEx(0, _T("BUTTON"), _T("��һҳ"),
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        startX + btnWidth, btnY, btnWidth, btnHeight,
        hwnd, (HMENU)PAGING_BTN_PREV, hInstance, NULL);

    m_pageInputHwnd = CreateWindowEx(0, _T("EDIT"), _T("1"),
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER | ES_CENTER,
        startX + 2 * btnWidth, btnY, 40, btnHeight,
        hwnd, (HMENU)PAGING_EDIT_PAGE, hInstance, NULL);

    CreateWindowEx(0, _T("BUTTON"), _T("��һҳ"),
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        startX + 2 * btnWidth + 40, btnY, btnWidth, btnHeight,
        hwnd, (HMENU)PAGING_BTN_NEXT, hInstance, NULL);

    CreateWindowEx(0, _T("BUTTON"), _T("βҳ"),
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        startX + 3 * btnWidth + 40, btnY, btnWidth, btnHeight,
        hwnd, (HMENU)PAGING_BTN_LAST, hInstance, NULL);

    CreateWindowEx(0, _T("BUTTON"), _T("��ת"),
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        startX + 4 * btnWidth + 40, btnY, btnWidth, btnHeight,
        hwnd, (HMENU)PAGING_BTN_GOTO, hInstance, NULL);

    // ��Ӳ�������ఴť
    AddSidebarButtons();

    // ����˵���λ��
    CalculateMenuItemPositions();

    // ��ʼ��ʾ������Ϸ
    RefreshGameDisplay();
}

void MainWindow::CalculateMenuItemPositions() {
    // ����ÿ�������˵����λ��
    const int logoWidth = 200;  // LOGO������
    const int menuItemWidth = 80;  // ÿ���˵�����
    const int menuItemHeight = TOPBAR_HEIGHT;

    int x = logoWidth;
    for (auto& item : m_menuItems) {
        item.rect = { x, 0, x + menuItemWidth, menuItemHeight };
        x += menuItemWidth;
    }

    // ���㹤������ťλ��
    const int toolbarX = SIDEBAR_WIDTH;
    const int toolButtonWidth = 30;
    const int toolButtonHeight = 30;
    const int toolbarY = TOPBAR_HEIGHT + (CATEGORY_HEIGHT - toolButtonHeight) / 2;

    x = toolbarX + 35;  // ����"��Ϸ����"�Ŀռ�
    for (auto& btn : m_toolbarButtons) {
        btn.rect = { x, toolbarY, x + toolButtonWidth, toolbarY + toolButtonHeight };
        x += toolButtonWidth + 2;
    }
}

void MainWindow::AddSidebarButtons() {
    OutputDebugString((L"AddSidebarButtons(): ��ǰ����=" + (m_currentCategory.empty() ? L"[��]" : m_currentCategory) + L"\n").c_str());

    HINSTANCE hInstance = GetModuleHandle(nullptr);
    int buttonHeight = 40;
    int yPos = 0;

    // ������з����б�
    m_sidebarCategories.clear();

    // ʹ��Ԥ����ķ����б����ٴ���Ϸ�����л�ȡ����
    for (size_t i = 0; i < PREDEFINED_GAME_CATEGORIES.size(); i++) {
        const std::wstring& category = PREDEFINED_GAME_CATEGORIES[i];
        m_sidebarCategories.push_back(category);

        HWND hButton = CreateWindowEx(0, L"BUTTON", category.c_str(),
            WS_CHILD | WS_VISIBLE | BS_FLAT | BS_LEFT,
            0, yPos, SIDEBAR_WIDTH, buttonHeight,
            m_sidebarHwnd, (HMENU)(ID_SIDEBAR + 1 + i), hInstance, NULL);

        // ������ǰ����
        if ((i == 0 && m_currentCategory.empty()) ||
            (i > 0 && category == m_currentCategory)) {
            std::wstring selectedText = L"�� " + category;
            SendMessage(hButton, WM_SETTEXT, 0, (LPARAM)selectedText.c_str());
            OutputDebugString((L"AddSidebarButtons(): ��Ǹ������� '" + category + L"'\n").c_str());
        }

        yPos += buttonHeight;
    }
}

void MainWindow::RefreshSidebar() {
    // ������в������ť
    HWND child = GetWindow(m_sidebarHwnd, GW_CHILD);
    while (child) {
        HWND nextChild = GetWindow(child, GW_HWNDNEXT);
        DestroyWindow(child);
        child = nextChild;
    }

    // ������Ӳ������ť
    AddSidebarButtons();
}

void MainWindow::OnDestroy() {
    PostQuitMessage(0);
}

void MainWindow::OnPaint() {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(m_hwnd, &ps);

    RECT rcClient;
    GetClientRect(m_hwnd, &rcClient);
    int width = rcClient.right - rcClient.left;
    int height = rcClient.bottom - rcClient.top;

    // �����ڴ�DC��λͼ����˫����
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBitmap = CreateCompatibleBitmap(hdc, width, height);
    HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

    // ���ƶ����˵������� (����ɫ)
    RECT rcTopBar = { 0, 0, width, TOPBAR_HEIGHT };
    HBRUSH hBrushTopBar = CreateSolidBrush(RGB(30, 35, 70));
    FillRect(memDC, &rcTopBar, hBrushTopBar);
    DeleteObject(hBrushTopBar);

    // ����LOGO
    SetBkMode(memDC, TRANSPARENT);
    SetTextColor(memDC, RGB(255, 200, 0));  // ���ɫLOGO
    HFONT oldFont = (HFONT)SelectObject(memDC, m_hTitleFont);
    TextOut(memDC, 20, (TOPBAR_HEIGHT - 22) / 2, L"DG", 2);

    SetTextColor(memDC, RGB(0, 150, 220));  // ��ɫPlayer
    TextOut(memDC, 60, (TOPBAR_HEIGHT - 22) / 2, L"player", 6);

    SetTextColor(memDC, RGB(180, 180, 180));  // ��ɫ�汾��
    HFONT tempFont = (HFONT)SelectObject(memDC, m_hMenuFont);
    TextOut(memDC, 140, (TOPBAR_HEIGHT - 18) / 2, L"Ver 0.1", wcslen(L"Ver 0.1"));
    SelectObject(memDC, oldFont);

    // ���Ʋ˵���
    SelectObject(memDC, m_hMenuFont);
    SetTextColor(memDC, RGB(220, 220, 220));  // ����ɫ�˵�����

    for (const auto& item : m_menuItems) {
        // ��������ͣ�ڲ˵����ϣ����Ƹ�������
        if (item.isHovered) {
            HBRUSH hBrushHover = CreateSolidBrush(RGB(50, 60, 100));
            FillRect(memDC, &item.rect, hBrushHover);
            DeleteObject(hBrushHover);
        }

        // ���Ʋ˵�����
        RECT textRect = item.rect;
        DrawText(memDC, item.text.c_str(), -1, &textRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }

    // ������Ϸ���๤�������� (ǳ����ɫ)
    RECT rcCategoryBar = { 0, TOPBAR_HEIGHT, width, TOPBAR_HEIGHT + CATEGORY_HEIGHT };
    HBRUSH hBrushCategoryBar = CreateSolidBrush(RGB(240, 240, 245));
    FillRect(memDC, &rcCategoryBar, hBrushCategoryBar);
    DeleteObject(hBrushCategoryBar);

    // ����"��Ϸ����"����
    SetTextColor(memDC, RGB(50, 50, 50));
    TextOut(memDC, 20, TOPBAR_HEIGHT + 10, L"��Ϸ����", 4);

    // ���ƹ�������ť
    for (const auto& btn : m_toolbarButtons) {
        // ��������ͣ�ڰ�ť�ϣ����Ƹ�������
        if (btn.isHovered) {
            HBRUSH hBrushBtnHover = CreateSolidBrush(RGB(200, 210, 230));
            FillRect(memDC, &btn.rect, hBrushBtnHover);
            DeleteObject(hBrushBtnHover);
        }
        else {
            // ������ͨ����
            HBRUSH hBrushBtn = CreateSolidBrush(RGB(230, 230, 235));
            FillRect(memDC, &btn.rect, hBrushBtn);
            DeleteObject(hBrushBtn);
        }

        // ���Ʊ߿�
        HPEN hPen = CreatePen(PS_SOLID, 1, RGB(180, 180, 180));
        HPEN hOldPen = (HPEN)SelectObject(memDC, hPen);
        Rectangle(memDC, btn.rect.left, btn.rect.top, btn.rect.right, btn.rect.bottom);
        SelectObject(memDC, hOldPen);
        DeleteObject(hPen);

        // ���Ƽ�ͼ��ͼ��
        int iconX = (btn.rect.left + btn.rect.right) / 2;
        int iconY = (btn.rect.top + btn.rect.bottom) / 2;
        int iconSize = 10;

        switch (btn.id) {
        case 3001: // ����ͼ��ͼ - ����С����
        {
            Rectangle(memDC, iconX - iconSize / 2, iconY - iconSize / 2,
                iconX + iconSize / 2, iconY + iconSize / 2);
        }
            break;
        case 3002: // �б���ͼ - ��������ˮƽ��
        {
            for (int i = -3; i <= 3; i += 3) {
                MoveToEx(memDC, iconX - iconSize / 2, iconY + i, NULL);
                LineTo(memDC, iconX + iconSize / 2, iconY + i);
            }
        }
            break;
        case 3003: // ������ͼ
        {
            Rectangle(memDC, iconX - iconSize / 2, iconY - iconSize / 2,
                iconX + iconSize / 2, iconY + iconSize / 2);
            MoveToEx(memDC, iconX - iconSize / 4, iconY - iconSize / 4, NULL);
            LineTo(memDC, iconX + iconSize / 4, iconY - iconSize / 4);
        }
            break;
        case 3004: // ������ͼ - ����2x2����
        {
            Rectangle(memDC, iconX - iconSize / 2, iconY - iconSize / 2,
                iconX, iconY);
            Rectangle(memDC, iconX, iconY - iconSize / 2,
                iconX + iconSize / 2, iconY);
            Rectangle(memDC, iconX - iconSize / 2, iconY,
                iconX, iconY + iconSize / 2);
            Rectangle(memDC, iconX, iconY,
                iconX + iconSize / 2, iconY + iconSize / 2);
        }
            break;
        case 3005: // ��������ͼ
        {
            Rectangle(memDC, iconX - iconSize / 2, iconY - iconSize / 2,
                iconX + iconSize / 2, iconY + iconSize / 2);
            MoveToEx(memDC, iconX, iconY - iconSize / 2, NULL);
            LineTo(memDC, iconX, iconY + iconSize / 2);
            MoveToEx(memDC, iconX - iconSize / 2, iconY, NULL);
            LineTo(memDC, iconX + iconSize / 2, iconY);
        }
            break;
        case 3006: // ������ͼ
        {
            Ellipse(memDC, iconX - iconSize / 2, iconY - iconSize / 2,
                iconX + iconSize / 2, iconY + iconSize / 2);
        }
            break;
        case 3007: // �ļ���ͼ
        {
            Rectangle(memDC, iconX - iconSize / 2, iconY - iconSize / 2,
                iconX + iconSize / 2, iconY - iconSize / 4);
            Rectangle(memDC, iconX - iconSize / 2, iconY - iconSize / 4,
                iconX + iconSize / 2, iconY + iconSize / 2);
        }
            break;
        case 3008: // �����ͼ
        {
            MoveToEx(memDC, iconX - iconSize / 2, iconY, NULL);
            LineTo(memDC, iconX + iconSize / 2, iconY);
            MoveToEx(memDC, iconX, iconY - iconSize / 2, NULL);
            LineTo(memDC, iconX, iconY + iconSize / 2);
        }
            break;
        }
    }

    // ����"Go"��ť
    RECT goButtonRect = { width - 70, TOPBAR_HEIGHT + 5, width - 10, TOPBAR_HEIGHT + CATEGORY_HEIGHT - 5 };
    HBRUSH hBrushGoBtn = CreateSolidBrush(RGB(50, 70, 100));
    FillRect(memDC, &goButtonRect, hBrushGoBtn);
    DeleteObject(hBrushGoBtn);

    SetTextColor(memDC, RGB(255, 255, 255));
    DrawText(memDC, L"Go", -1, &goButtonRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    // ���Ƶײ�״̬��
    RECT rcStatusBar = { 0, rcClient.bottom - STATUSBAR_HEIGHT, width, rcClient.bottom };
    DrawStatusBar(memDC, rcStatusBar);

    // ���ڴ�DC����һ���Ը��Ƶ���Ļ
    BitBlt(hdc, 0, 0, width, height, memDC, 0, 0, SRCCOPY);

    // ������Դ
    SelectObject(memDC, oldBitmap);
    DeleteObject(memBitmap);
    DeleteDC(memDC);

    // �ָ�Ĭ������
    SelectObject(hdc, oldFont);

    EndPaint(m_hwnd, &ps);
}

void MainWindow::OnSize(int width, int height) {
    if (m_sidebarHwnd && m_contentHwnd && m_searchHwnd) {
        // ���������
        MoveWindow(m_sidebarHwnd, 0, TOPBAR_HEIGHT + CATEGORY_HEIGHT, SIDEBAR_WIDTH,
            height - TOPBAR_HEIGHT - CATEGORY_HEIGHT - STATUSBAR_HEIGHT, TRUE);

        // ����������
        MoveWindow(m_searchHwnd, width - 300, TOPBAR_HEIGHT + 5, 200, 20, TRUE);

        // ����������
        MoveWindow(m_contentHwnd, SIDEBAR_WIDTH, TOPBAR_HEIGHT + CATEGORY_HEIGHT,
            width - SIDEBAR_WIDTH, height - TOPBAR_HEIGHT - CATEGORY_HEIGHT - STATUSBAR_HEIGHT, TRUE);

        // �����ײ���ҳ��ť
        int btnWidth = 80;
        int btnHeight = 25;
        int btnY = height - STATUSBAR_HEIGHT / 2 - btnHeight / 2;
        int startX = (width - 5 * btnWidth - 40) / 2; // 40 ��ҳ���������

        // ��λ��ҳ��ť
        MoveWindow(GetDlgItem(m_hwnd, PAGING_BTN_FIRST), startX, btnY, btnWidth, btnHeight, TRUE);
        MoveWindow(GetDlgItem(m_hwnd, PAGING_BTN_PREV), startX + btnWidth, btnY, btnWidth, btnHeight, TRUE);
        MoveWindow(m_pageInputHwnd, startX + 2 * btnWidth, btnY, 40, btnHeight, TRUE);
        MoveWindow(GetDlgItem(m_hwnd, PAGING_BTN_NEXT), startX + 2 * btnWidth + 40, btnY, btnWidth, btnHeight, TRUE);
        MoveWindow(GetDlgItem(m_hwnd, PAGING_BTN_LAST), startX + 3 * btnWidth + 40, btnY, btnWidth, btnHeight, TRUE);
        MoveWindow(GetDlgItem(m_hwnd, PAGING_BTN_GOTO), startX + 4 * btnWidth + 40, btnY, btnWidth, btnHeight, TRUE);

        // ���¼���˵���λ��
        CalculateMenuItemPositions();

        // ----------- �ؼ�������Ӧÿҳ��ʾ���� -----------
        // �������ߴ�
        int contentWidth = width - SIDEBAR_WIDTH;
        int contentHeight = height - TOPBAR_HEIGHT - CATEGORY_HEIGHT - STATUSBAR_HEIGHT;

        // ��Ƭ�ͼ��
        int cardWidth = 200, cardHeight = 280;
        int gapX = 20, gapY = 20;

        // ��������������
        int columns = std::max(1, (contentWidth - gapX) / (cardWidth + gapX));
        int rows = std::max(1, (contentHeight - gapY) / (cardHeight + gapY));

        m_gamesPerPage = columns * rows;

        // ǿ��ˢ����Ϸ��ʾ
        RefreshGameDisplay();

        // ǿ���ػ�
        InvalidateRect(m_hwnd, NULL, TRUE);
    }
}

void MainWindow::OnMouseMove(int x, int y) {
    bool needRedraw = false;

    // ���˵�����ͣ
    for (auto& item : m_menuItems) {
        bool wasHovered = item.isHovered;
        item.isHovered = PtInRect(&item.rect, { x, y });

        if (wasHovered != item.isHovered) {
            needRedraw = true;
        }
    }

    // ��鹤������ť��ͣ
    for (auto& btn : m_toolbarButtons) {
        bool wasHovered = btn.isHovered;
        btn.isHovered = PtInRect(&btn.rect, { x, y });

        if (wasHovered != btn.isHovered) {
            needRedraw = true;
        }
    }

    if (needRedraw) {
        // ���ػ涥������
        RECT rcTopArea = { 0, 0, 9999, TOPBAR_HEIGHT + CATEGORY_HEIGHT };
        InvalidateRect(m_hwnd, &rcTopArea, TRUE);
    }
}

void MainWindow::OnLButtonDown(int x, int y) {
    bool handled = false;

    // ���˵�����
    for (const auto& item : m_menuItems) {
        if (PtInRect(&item.rect, { x, y })) {
            // ����˵�����¼�
            if (item.id == ID_MENU_GAME) {
                OnAddGame();
                handled = true;
            }
            else if (item.id == ID_MENU_HELP) {
                // ���ð����Ի���
                ShowHelpDialog();
                handled = true;
            }
            else {
                MessageBox(m_hwnd, (L"����˲˵�: " + item.text).c_str(), L"�˵����", MB_OK);
            }
            break;
        }
    }

    if (!handled) {
        // ��鹤������ť���
        for (const auto& btn : m_toolbarButtons) {
            if (PtInRect(&btn.rect, { x, y })) {
                // �ر��������Ϸ��ť (ID 3008)
                if (btn.id == 3008) {
                    // ��ʾȷ�ϵ�����Ϣ
                    OutputDebugString(L"����������Ϸ��ť\n");
                    OnAddGame();
                    handled = true;
                }
                else {
                    // ����������ť����¼�
                    MessageBox(m_hwnd, (L"����˹��߰�ť: " + btn.text).c_str(), L"��ť���", MB_OK);
                }
                break;
            }
        }
    }

    if (!handled) {
        // ���"Go"��ť���
        RECT rcClient;
        GetClientRect(m_hwnd, &rcClient);
        RECT goButtonRect = { rcClient.right - 70, TOPBAR_HEIGHT + 5, rcClient.right - 10, TOPBAR_HEIGHT + CATEGORY_HEIGHT - 5 };

        if (PtInRect(&goButtonRect, { x, y })) {
            // ����"Go"��ť���
            TCHAR searchText[256] = { 0 };
            GetWindowText(m_searchHwnd, searchText, 256);
            if (searchText[0] != 0) {
                // ͨ������������Ϸ
                std::vector<Game*> results = m_gameCollection.FindGamesByName(searchText);
                // ��ʾ����������� 
                MessageBox(m_hwnd,
                    (std::wstring(L"�ҵ� ") + std::to_wstring(results.size()) + L" ����Ϸ").c_str(),
                    L"�������", MB_OK);
                // ˢ����Ϸ��ʾ
                RefreshGameDisplay(results);
            }
        }
    }
}

void MainWindow::OnDeleteGame(int gameIndex) {
    if (gameIndex < 0 || gameIndex >= static_cast<int>(m_gameCollection.GetGameCount())) {
        MessageBox(m_hwnd, L"��Ч����Ϸ����", L"����", MB_OK | MB_ICONERROR);
        return;
    }
    Game* game = m_gameCollection.GetGame(gameIndex);
    std::wstring message = L"ȷ��Ҫɾ����Ϸ \"" + game->GetName() + L"\" ��\n�˲������ɳ�����";
    if (MessageBox(m_hwnd, message.c_str(), L"ȷ��ɾ��", MB_YESNO | MB_ICONQUESTION) == IDYES) {
        if (m_gameCollection.RemoveGame(gameIndex)) {
            MessageBox(m_hwnd, L"��Ϸɾ���ɹ�", L"ɾ����Ϸ", MB_OK | MB_ICONINFORMATION);
            RefreshSidebar();
            // ����ǰ����ˢ��
            RefreshGameDisplay();
        }
        else {
            MessageBox(m_hwnd, L"ɾ����Ϸʧ��", L"����", MB_OK | MB_ICONERROR);
        }
    }
}

// ˢ����Ϸ��ʾ
void MainWindow::RefreshGameDisplay(const std::vector<Game*>& games) {
    // ���浱ǰҪ��ʾ����Ϸ�б�
    m_currentGames = games;

    // ������ҳ��
    m_totalPages = (games.size() + m_gamesPerPage - 1) / m_gamesPerPage;  // ����ȡ��
    if (m_totalPages == 0) m_totalPages = 1;  // ������1ҳ

    // ȷ����ǰҳ������Ч��Χ��
    if (m_currentPage >= m_totalPages) m_currentPage = m_totalPages - 1;
    if (m_currentPage < 0) m_currentPage = 0;

    // ��ӵ������
    WCHAR debugStr[100];
    swprintf_s(debugStr, L"��ǰҳ: %d, ��ҳ��: %d, ��Ϸ����: %d\n",
        m_currentPage + 1, m_totalPages, (int)games.size());
    OutputDebugString(debugStr);

    // ����ҳ�������
    if (m_pageInputHwnd) {
        TCHAR pageText[16];
        _stprintf_s(pageText, _T("%d"), m_currentPage + 1);
        SetWindowText(m_pageInputHwnd, pageText);
    }

    // ��¼��Ϸ��Ϣ���ڵ���
    std::wstring gameList;
    for (const auto& game : games) {
        if (!gameList.empty()) gameList += L"\n";
        gameList += game->GetName() + L" (" + game->GetCategory() + L")";
    }

    if (!gameList.empty()) {
        OutputDebugString((L"��Ϸ�б�:\n" + gameList + L"\n").c_str());
    }
    else {
        OutputDebugString(L"��Ϸ�б�Ϊ��\n");
    }

    // �ػ�������
    InvalidateRect(m_contentHwnd, NULL, TRUE);

    // �ػ�״̬������
    RECT rcClient;
    GetClientRect(m_hwnd, &rcClient);
    RECT rcStatusBar = { 0, rcClient.bottom - STATUSBAR_HEIGHT, rcClient.right, rcClient.bottom };
    InvalidateRect(m_hwnd, &rcStatusBar, TRUE);
}

// �����޲����汾��ˢ����Ϸ��ʾ
void MainWindow::RefreshGameDisplay() {
    OutputDebugString((L"RefreshGameDisplay(): ��ǰ����=" + (m_currentCategory.empty() ? L"[��]" : m_currentCategory) +
        L", ��ǰҳ=" + std::to_wstring(m_currentPage) + L"\n").c_str());

    if (m_currentCategory.empty() || m_currentCategory == L"ȫ����Ϸ") {
        // ��ʾȫ����Ϸ
        OutputDebugString(L"RefreshGameDisplay(): ������ʾȫ����Ϸ\n");
        std::vector<Game*> allGames;
        allGames.reserve(m_gameCollection.GetGameCount());

        for (size_t i = 0; i < m_gameCollection.GetGameCount(); i++) {
            allGames.push_back(m_gameCollection.GetGame(i));
        }
        RefreshGameDisplay(allGames);
    }
    else {
        // ��ʾ��ǰ�������Ϸ
        OutputDebugString((L"RefreshGameDisplay(): ������ʾ���� '" + m_currentCategory + L"' ����Ϸ\n").c_str());
        std::vector<Game*> games = m_gameCollection.FindGamesByCategory(m_currentCategory);
        RefreshGameDisplay(games);
    }
}

// �����Ϸ
void MainWindow::OnAddGame() {
    OutputDebugString(L"����ִ�� OnAddGame() ����\n");
    Game newGame;
    GameDialog dialog;
    try {
        if (dialog.ShowAddDialog(m_hwnd, newGame, m_gameCollection)) {
            OutputDebugString(L"�Ի��򷵻�OK�����������Ϸ\n");
            if (m_gameCollection.AddGame(newGame)) {
                MessageBox(m_hwnd, L"��Ϸ��ӳɹ�", L"�����Ϸ", MB_OK | MB_ICONINFORMATION);
                RefreshSidebar();
                // ����ǰ����ˢ��
                RefreshGameDisplay();
            }
            else {
                MessageBox(m_hwnd, L"������Ϸ��Ϣʧ��", L"����", MB_OK | MB_ICONERROR);
                OutputDebugString(L"m_gameCollection.AddGame ���� false\n");
            }
        }
        else {
            OutputDebugString(L"�û�ȡ������Ϸ��ӶԻ���\n");
        }
    }
    catch (const std::exception& e) {
        char buffer[1024];
        sprintf_s(buffer, "�����Ϸʱ�����쳣: %s", e.what());
        OutputDebugStringA(buffer);
        MessageBoxA(m_hwnd, buffer, "����", MB_OK | MB_ICONERROR);
    }
    catch (...) {
        OutputDebugString(L"�����Ϸʱ����δ֪�쳣\n");
        MessageBox(m_hwnd, L"�����Ϸʱ����δ֪�쳣", L"����", MB_OK | MB_ICONERROR);
    }
}

// ������Ϸ��ͼ
void MainWindow::DrawGameCard(HDC hdc, const Game* game, int x, int y, int width, int height, int gameIndex) {
    // ����������Ⱦģʽ
    SetBkMode(hdc, TRANSPARENT);

    // ��Ƭ����ɫ - ������ɫ
    HBRUSH hBrushBg = CreateSolidBrush(RGB(45, 53, 80));
    RECT cardRect = { x, y, x + width, y + height };
    FillRect(hdc, &cardRect, hBrushBg);
    DeleteObject(hBrushBg);

    // ͼ������ - �Ϸ�����֮һ����
    int iconHeight = height / 3;
    RECT iconRect = { x, y, x + width, y + iconHeight };

    // ����ͼ�걳��
    HBRUSH hBrushIconBg = CreateSolidBrush(RGB(30, 35, 50));
    FillRect(hdc, &iconRect, hBrushIconBg);
    DeleteObject(hBrushIconBg);

    // �����ʺ�������ʾ������
    HFONT hLabelFont = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH, L"΢���ź�");

    HFONT hValueFont = CreateFont(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH, L"΢���ź�");


    // ɾ����ť�����ʵ��Ӵ󣬱���̫����
    RECT deleteButtonRect = { x + width - 35, y + 15, x + width - 15, y + 35 };

    // 1. ����ɫ����
    HBRUSH hDeleteBrush = CreateSolidBrush(RGB(220, 60, 60));
    FillRect(hdc, &deleteButtonRect, hDeleteBrush);
    DeleteObject(hDeleteBrush);

    // 2. ֻ����ɫ�߿򣬲��������ɫ
    HPEN hWhitePen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
    HPEN hOldPen = (HPEN)SelectObject(hdc, hWhitePen);
    HBRUSH hNullBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hNullBrush);
    Rectangle(hdc, deleteButtonRect.left, deleteButtonRect.top, deleteButtonRect.right, deleteButtonRect.bottom);
    SelectObject(hdc, hOldBrush);
    SelectObject(hdc, hOldPen);
    DeleteObject(hWhitePen);

    // 3. ���ơ�X��
    HFONT hButtonFont = CreateFont(18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH, L"΢���ź�");
    HFONT hOldFont = (HFONT)SelectObject(hdc, hButtonFont);
    SetTextColor(hdc, RGB(255, 255, 255));
    SetBkMode(hdc, TRANSPARENT);
    DrawTextW(hdc, L"X", -1, &deleteButtonRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    SelectObject(hdc, hOldFont);
    DeleteObject(hButtonFont);

    // ����ɾ����ť��Ϣ
    DeleteButtonInfo deleteBtn;
    deleteBtn.rect = deleteButtonRect;
    deleteBtn.gameIndex = gameIndex;
    m_deleteButtons.push_back(deleteBtn);

    // ������ť����
    RECT launchButtonRect = { x + 15, y + 15, x + 75, y + 35 };

    // 1. �����ɫ����
    HBRUSH hLaunchBrush = CreateSolidBrush(RGB(60, 180, 60));
    FillRect(hdc, &launchButtonRect, hLaunchBrush);
    DeleteObject(hLaunchBrush);

    // 2. ����ɫ�߿�
    HPEN hGreenPen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
    hOldPen = (HPEN)SelectObject(hdc, hGreenPen);
    hOldBrush = (HBRUSH)SelectObject(hdc, hNullBrush);
    Rectangle(hdc, launchButtonRect.left, launchButtonRect.top, launchButtonRect.right, launchButtonRect.bottom);
    SelectObject(hdc, hOldBrush);
    SelectObject(hdc, hOldPen);
    DeleteObject(hGreenPen);

    // 3. ���ơ�����������
    hButtonFont = CreateFont(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH, L"΢���ź�");
    hOldFont = (HFONT)SelectObject(hdc, hButtonFont);
    SetTextColor(hdc, RGB(255, 255, 255));
    DrawTextW(hdc, L"����", -1, &launchButtonRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    SelectObject(hdc, hOldFont);
    DeleteObject(hButtonFont);

    // ����������ť��Ϣ
    DeleteButtonInfo launchBtn;
    launchBtn.rect = launchButtonRect;
    launchBtn.gameIndex = gameIndex + 100000; // �ô��ڵ�ǰ��Ϸ����ƫ������������ť
    m_deleteButtons.push_back(launchBtn);

    // ���Լ�����Ϸͼ��
    bool iconLoaded = false;
    HICON hIcon = NULL;

    if (!game->GetExecutablePath().empty()) {
        // ���ȳ��Դӿ�ִ���ļ���ȡͼ��
        HINSTANCE hInst = GetModuleHandle(NULL);
        hIcon = ExtractIcon(hInst, game->GetExecutablePath().c_str(), 0);

        // �����ִ���ļ�û��ͼ�꣬����ʹ��ָ����ͼ��·��
        if (hIcon == NULL || hIcon == (HICON)1) {
            if (!game->GetIconPath().empty()) {
                hIcon = (HICON)LoadImage(NULL, game->GetIconPath().c_str(), IMAGE_ICON,
                    0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE);
            }
        }
    }
    else if (!game->GetIconPath().empty()) {
        // ���û�п�ִ���ļ�·����ֱ�ӳ���ͼ��·��
        hIcon = (HICON)LoadImage(NULL, game->GetIconPath().c_str(), IMAGE_ICON,
            0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE);
    }

    if (hIcon && hIcon != (HICON)1) {
        // ����ͼ���������о��е�λ��
        int iconSize = std::min(64, std::min(width - 20, iconHeight - 20));
        int iconX = x + (width - iconSize) / 2;
        int iconY = y + (iconHeight - iconSize) / 2;

        // ����ͼ��
        DrawIconEx(hdc, iconX, iconY, hIcon, iconSize, iconSize, 0, NULL, DI_NORMAL);
        DestroyIcon(hIcon);
        iconLoaded = true;
    }

    // ���û��ͼ�꣬����"NOT FOUND"ռλ��
    if (!iconLoaded) {
        SetTextColor(hdc, RGB(220, 220, 220));
        DrawTextW(hdc, L"NOT FOUND", -1, &iconRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }

    // ��Ϣ���� - �в�
    RECT infoRect = { x, y + iconHeight, x + width, y + height - 60 };
    HBRUSH hBrushInfoBg = CreateSolidBrush(RGB(60, 70, 100));
    FillRect(hdc, &infoRect, hBrushInfoBg);
    DeleteObject(hBrushInfoBg);

    // ������Ϣ��ʾ - ʹ�ñ��ʽ����
    const int padding = 8;        // �߾�
    const int labelWidth = 65;    // ��ǩ���
    const int lineHeight = 20;    // �и�

    // ��Ϸ������
    int textY = y + iconHeight + padding;
    RECT labelRect = { x + padding, textY, x + padding + labelWidth, textY + lineHeight };
    RECT valueRect = { x + padding + labelWidth, textY, x + width - padding, textY + lineHeight };

    // ���Ʊ�ǩ
    SelectObject(hdc, hLabelFont);
    SetTextColor(hdc, RGB(200, 200, 200));
    DrawTextW(hdc, L"��Ϸ����", -1, &labelRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    // ����ֵ��ʹ�ô��壩
    SelectObject(hdc, hValueFont);
    SetTextColor(hdc, RGB(220, 220, 255));

    // �����Ϸ����̫�����ضϲ����ʡ�Ժ�
    std::wstring gameName = game->GetName();
    if (gameName.length() > 15) { // ����ʵ����������ضϳ���
        gameName = gameName.substr(0, 15) + L"...";
    }
    DrawTextW(hdc, gameName.c_str(), -1, &valueRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    // ���������
    textY += lineHeight + 5;
    labelRect = { x + padding, textY, x + padding + labelWidth, textY + lineHeight };
    valueRect = { x + padding + labelWidth, textY, x + width - padding, textY + lineHeight };

    SelectObject(hdc, hLabelFont);
    SetTextColor(hdc, RGB(200, 200, 200));
    DrawTextW(hdc, L"�������", -1, &labelRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    SelectObject(hdc, hValueFont);
    SetTextColor(hdc, RGB(220, 220, 255));
    DrawTextW(hdc, game->GetReleaseYear().c_str(), -1, &valueRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    // ����������
    textY += lineHeight + 5;
    labelRect = { x + padding, textY, x + padding + labelWidth, textY + lineHeight };
    valueRect = { x + padding + labelWidth, textY, x + width - padding, textY + lineHeight };

    SelectObject(hdc, hLabelFont);
    SetTextColor(hdc, RGB(200, 200, 200));
    DrawTextW(hdc, L"��������", -1, &labelRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    SelectObject(hdc, hValueFont);
    SetTextColor(hdc, RGB(220, 220, 255));

    // �������������̫�����ضϲ����ʡ�Ժ�
    std::wstring publisher = game->GetPublisher();
    if (publisher.length() > 15) {
        publisher = publisher.substr(0, 15) + L"...";
    }
    DrawTextW(hdc, publisher.c_str(), -1, &valueRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    // �ײ���Ϣ���� - ����
    // ��һ��: ���Ժͷ���
    RECT langRect = { x, y + height - 60, x + width / 2, y + height - 30 };
    HBRUSH hBrushLangBg = CreateSolidBrush(RGB(34, 96, 120)); // ����ɫ
    FillRect(hdc, &langRect, hBrushLangBg);
    DeleteObject(hBrushLangBg);

    RECT catRect = { x + width / 2, y + height - 60, x + width, y + height - 30 };
    HBRUSH hBrushCatBg = CreateSolidBrush(RGB(70, 100, 150)); // ��ɫ
    FillRect(hdc, &catRect, hBrushCatBg);
    DeleteObject(hBrushCatBg);

    // ���Ժͷ����ı� - ʹ�ñ�ǩ����
    SelectObject(hdc, hLabelFont);
    SetTextColor(hdc, RGB(240, 240, 240));
    DrawTextW(hdc, game->GetLanguage().c_str(), -1, &langRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    DrawTextW(hdc, game->GetCategory().c_str(), -1, &catRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    // �ڶ���: ƽ̨�ͱ��
    RECT platformRect = { x, y + height - 30, x + width / 2, y + height };
    HBRUSH hBrushPlatformBg = CreateSolidBrush(RGB(40, 40, 80)); // ����ɫ
    FillRect(hdc, &platformRect, hBrushPlatformBg);
    DeleteObject(hBrushPlatformBg);

    RECT numRect = { x + width / 2, y + height - 30, x + width, y + height };
    HBRUSH hBrushNumBg = CreateSolidBrush(RGB(40, 40, 80)); // ����ɫ
    FillRect(hdc, &numRect, hBrushNumBg);
    DeleteObject(hBrushNumBg);

    // ƽ̨�ͱ���ı�
    DrawTextW(hdc, game->GetPlatform().c_str(), -1, &platformRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    // ��ʾ��Ϸ���
    std::wstring indexStr = L"- " + std::to_wstring(gameIndex + 1) + L" -";
    DrawTextW(hdc, indexStr.c_str(), -1, &numRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    // �ָ�ԭ���岢������Դ
    SelectObject(hdc, hOldFont);
    DeleteObject(hLabelFont);
    DeleteObject(hValueFont);
    DeleteObject(hButtonFont);
}

// ����״̬��
void MainWindow::DrawStatusBar(HDC hdc, const RECT& rect) {
    // ����״̬��������ǳ��ɫ��
    HBRUSH hBrushStatusBar = CreateSolidBrush(RGB(235, 235, 235));
    FillRect(hdc, &rect, hBrushStatusBar);
    DeleteObject(hBrushStatusBar);

    // ���ƶ����߿���
    HPEN hPen = CreatePen(PS_SOLID, 1, RGB(200, 200, 200));
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
    MoveToEx(hdc, rect.left, rect.top, NULL);
    LineTo(hdc, rect.right, rect.top);
    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);

    // ������Ϸ������ҳ����Ϣ
    SetTextColor(hdc, RGB(50, 50, 50));
    SetBkMode(hdc, TRANSPARENT);

    // ������ʾ�ı���ʹ����ͼƬʾ����ͬ�ĸ�ʽ
    std::wstring statusText = L"���� " + std::to_wstring(m_currentGames.size()) +
        L" ����Ϸ��ÿҳ��ʾ " + std::to_wstring(m_gamesPerPage) +
        L" ��� " + std::to_wstring(m_totalPages) + L" ҳ";

    // �����ı�λ�ã��ڰ�ť�Ϸ�����
    const int leftMargin = 20; // ���߾�
    int textY = rect.top + (rect.bottom - rect.top - 20) / 2; // ��ֱ����
    RECT textRect = { rect.left + leftMargin, textY, rect.right, textY + 20 };
    DrawText(hdc, statusText.c_str(), -1, &textRect, DT_LEFT | DT_SINGLELINE);
}

void MainWindow::ShowHelpDialog() {
    // ����������Ϣ�ı�
    std::wstring helpText =
        L"��Ϸ������ - ʹ�ð���\n\n"
        L"����������\n"
        L"1. �����Ϸ����������˵���Ϸ�򹤾����е�+��ť\n"
        L"2. ɾ����Ϸ������Ϸ��Ƭ���Ͻǵ��X��ť\n"
        L"3. ������Ϸ������Ϸ��Ƭ���Ͻǵ��������ť\n"
        L"4. ������Ϸ�������Ͻ�����������ؼ��ʣ����Go��ť\n"
        L"5. �������������������ѡ����Ϸ����\n"
        L"6. ��ҳ�����ʹ�õײ���ҳ�뵼����ť����ʹ��������\n\n"
        L"��ʾ��\n"
        L"- �����Ϸʱ��ѡ���ִ���ļ����Զ���䲿����Ϸ��Ϣ\n"
        L"- ���κη����¶�����ͨ��ҳ����е���\n"
        L"- ֧�������ֿ��ٷ�ҳ\n\n"
        L"DGplayer ��Ϸ������ Ver 0.1\n"
        L"HuaJH��������ֹ������";

    // ��ʾ�����Ի���
    MessageBox(m_hwnd, helpText.c_str(), L"����", MB_OK | MB_ICONINFORMATION);
}

// ������
LRESULT CALLBACK MainWindow::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    MainWindow* pThis = nullptr;
    if (msg == WM_NCCREATE) {
        CREATESTRUCT* pcs = (CREATESTRUCT*)lParam;
        pThis = (MainWindow*)pcs->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
    }
    else {
        pThis = (MainWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }

    switch (msg) {
    case WM_ERASEBKGND:
        // ��ֹ���������Լ�����˸
        return 1;
    case WM_CREATE:
        if (pThis) pThis->OnCreate(hwnd);
        break;
    case WM_DESTROY:
        if (pThis) pThis->OnDestroy();
        break;
    case WM_PAINT:
        if (pThis) pThis->OnPaint();
        return 0;
    case WM_SIZE:
        if (pThis) {
            UINT width = LOWORD(lParam);
            UINT height = HIWORD(lParam);
            pThis->OnSize(width, height);
        }
        break;
    case WM_MOUSEMOVE:
        if (pThis) {
            pThis->OnMouseMove(LOWORD(lParam), HIWORD(lParam));
        }
        break;
    case WM_LBUTTONDOWN:
        if (pThis) {
            pThis->OnLButtonDown(LOWORD(lParam), HIWORD(lParam));
        }
        break;
    case WM_COMMAND:
        if (pThis) {
            int wmId = LOWORD(wParam);

            // 1. ��������ఴť
            if (wmId >= ID_SIDEBAR + 1 && wmId < ID_SIDEBAR + 1 + (int)pThis->m_sidebarCategories.size()) {
                int catIndex = wmId - (ID_SIDEBAR + 1);
                if (catIndex == 0) {
                    pThis->m_currentCategory.clear();
                }
                else {
                    pThis->m_currentCategory = pThis->m_sidebarCategories[catIndex];
                }
                pThis->m_currentPage = 0;
                pThis->RefreshGameDisplay();
                pThis->RefreshSidebar();
                break;
            }

            // 2. �����˵�
            switch (wmId) {
            case ID_MENU_GAME:
                pThis->OnAddGame();
                break;
            case ID_MENU_BOX:
            case ID_MENU_COMMUNITY:
            case ID_MENU_CONFIG:
                MessageBox(pThis->m_hwnd, L"�ù�����δʵ��", L"��ʾ", MB_OK | MB_ICONINFORMATION);
                break;
            case ID_MENU_HELP:
                pThis->ShowHelpDialog();
                break;

                // 3. ��ҳ��ť
            case PAGING_BTN_FIRST:
                pThis->m_currentPage = 0;
                pThis->RefreshGameDisplay();
                break;
            case PAGING_BTN_PREV:
                if (pThis->m_currentPage > 0) {
                    pThis->m_currentPage--;
                    pThis->RefreshGameDisplay();
                }
                break;
            case PAGING_BTN_NEXT:
                if (pThis->m_currentPage < pThis->m_totalPages - 1) {
                    pThis->m_currentPage++;
                    pThis->RefreshGameDisplay();
                }
                break;
            case PAGING_BTN_LAST:
                pThis->m_currentPage = pThis->m_totalPages - 1;
                pThis->RefreshGameDisplay();
                break;
            case PAGING_BTN_GOTO: {
                TCHAR pageText[16] = { 0 };
                GetWindowText(pThis->m_pageInputHwnd, pageText, 16);
                int page = _wtoi(pageText);
                if (page >= 1 && page <= pThis->m_totalPages) {
                    pThis->m_currentPage = page - 1;
                    pThis->RefreshGameDisplay();
                }
                else {
                    MessageBox(pThis->m_hwnd, L"��������Чҳ��", L"��ʾ", MB_OK | MB_ICONINFORMATION);
                    _stprintf_s(pageText, _T("%d"), pThis->m_currentPage + 1);
                    SetWindowText(pThis->m_pageInputHwnd, pageText);
                }
                break;
            }
            default:
                return DefWindowProc(hwnd, msg, wParam, lParam);
            }
        }
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

// ��Ϸ���ݴ���
LRESULT CALLBACK MainWindow::ContentWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    MainWindow* pThis = nullptr;
    if (msg == WM_NCCREATE) {
        CREATESTRUCT* pcs = (CREATESTRUCT*)lParam;
        pThis = (MainWindow*)pcs->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
    }
    else {
        pThis = (MainWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }

    switch (msg) {
    case WM_ERASEBKGND:
        // ��ֹ���������Լ�����˸
        return 1;
    case WM_MOUSEWHEEL:
    {
        if (!pThis) break;

        // ��ȡ�����������������ж�
        short zDelta = GET_WHEEL_DELTA_WPARAM(wParam);

        // ͬʱ���Ctrl���Ƿ��£�ʵ�ֿ��ٷ�ҳ
        bool isCtrlPressed = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
        int pagesToScroll = isCtrlPressed ? 3 : 1;

        if (zDelta > 0) {
            // ���Ϲ�������һҳ (֧�ֶ�ҳ���ٷ�ҳ)
            for (int i = 0; i < pagesToScroll && pThis->m_currentPage > 0; i++) {
                pThis->m_currentPage--;
            }
            pThis->RefreshGameDisplay();
        }
        else if (zDelta < 0) {
            // ���¹�������һҳ (֧�ֶ�ҳ���ٷ�ҳ)
            for (int i = 0; i < pagesToScroll && pThis->m_currentPage < pThis->m_totalPages - 1; i++) {
                pThis->m_currentPage++;
            }
            pThis->RefreshGameDisplay();
        }
        return 0; // ����0��ʾ��Ϣ�Ѵ���
    }

    case WM_KEYDOWN:
    {
        if (pThis) {
            switch (wParam) {
            case VK_PRIOR: // Page Up��
                if (pThis->m_currentPage > 0) {
                    pThis->m_currentPage--;
                    pThis->RefreshGameDisplay();
                }
                return 0;

            case VK_NEXT: // Page Down��
                if (pThis->m_currentPage < pThis->m_totalPages - 1) {
                    pThis->m_currentPage++;
                    pThis->RefreshGameDisplay();
                }
                return 0;

            case VK_HOME: // Home����ת����һҳ
                pThis->m_currentPage = 0;
                pThis->RefreshGameDisplay();
                return 0;

            case VK_END: // End����ת�����һҳ
                pThis->m_currentPage = pThis->m_totalPages - 1;
                pThis->RefreshGameDisplay();
                return 0;
            }
        }
    }
        break;
    case WM_PAINT:
    {
        if (pThis) {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            // ��ȡ��������С
            RECT rcClient;
            GetClientRect(hwnd, &rcClient);
            int width = rcClient.right - rcClient.left;
            int height = rcClient.bottom - rcClient.top;

            // �����ڴ�DC��λͼ����˫����
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP memBitmap = CreateCompatibleBitmap(hdc, width, height);
            HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

            // ����ɫ����
            HBRUSH hBrushBg = CreateSolidBrush(RGB(240, 240, 245));
            FillRect(memDC, &rcClient, hBrushBg);
            DeleteObject(hBrushBg);

            // ���֮ǰ��ɾ����ť��Ϣ
            pThis->m_deleteButtons.clear();

            // ��Ƭ�ͼ��
            int cardWidth = 200;
            int cardHeight = 280;
            int gapX = 20;
            int gapY = 20;
            int startX = 10;
            int startY = 10;

            // �ؼ���������������ȶ�̬��������
            int columns = std::max(1, (width - gapX) / (cardWidth + gapX));

            // ʹ�õ�ǰɸѡ�����Ϸ�б�
            const std::vector<Game*>& gamesToShow = pThis->m_currentGames;

            // ���㵱ǰҳ����ʼ�ͽ�������
            int startIndex = pThis->m_currentPage * pThis->m_gamesPerPage;
            int endIndex = std::min((int)gamesToShow.size(), startIndex + pThis->m_gamesPerPage);

            // ���Ƶ�ǰҳ����Ϸ
            for (int i = startIndex; i < endIndex; i++) {
                int idx = i - startIndex;
                int col = idx % columns;
                int row = idx / columns;
                int x = startX + col * (cardWidth + gapX);
                int y = startY + row * (cardHeight + gapY);

                pThis->DrawGameCard(memDC, gamesToShow[i], x, y, cardWidth, cardHeight, i);
            }

            // �����ǰҳû����Ϸ����ʾ��ʾ��Ϣ
            if (startIndex >= (int)gamesToShow.size() || gamesToShow.empty()) {
                RECT textRect = rcClient;
                SetTextColor(memDC, RGB(100, 100, 100));
                SetBkMode(memDC, TRANSPARENT);
                HFONT hFont = CreateFont(20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                    DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                    CLEARTYPE_QUALITY, DEFAULT_PITCH, L"΢���ź�");
                HFONT oldFont = (HFONT)SelectObject(memDC, hFont);

                DrawText(memDC, L"������Ϸ�����������˵�����Ϸ�򹤾����е� + �����Ϸ",
                    -1, &textRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

                SelectObject(memDC, oldFont);
                DeleteObject(hFont);
            }

            // ��ʾ��ҳ��ʾ������ж�ҳ��
            if (pThis->m_totalPages > 1) {
                RECT navHintRect = { 10, height - 30, width - 10, height - 10 };
                SetTextColor(memDC, RGB(80, 80, 80));
                SetBkMode(memDC, TRANSPARENT);
                HFONT hHintFont = CreateFont(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                    DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                    CLEARTYPE_QUALITY, DEFAULT_PITCH, L"΢���ź�");
                HFONT oldHintFont = (HFONT)SelectObject(memDC, hHintFont);

                std::wstring navHint = L"��ʾ����ʹ�������֡�Page Up/Down��Home/End ���з�ҳ";
                DrawText(memDC, navHint.c_str(), -1, &navHintRect, DT_RIGHT | DT_BOTTOM | DT_SINGLELINE);

                SelectObject(memDC, oldHintFont);
                DeleteObject(hHintFont);
            }

            // ��ɻ��ƺ󣬽��ڴ�DC���ݸ��Ƶ���Ļ
            BitBlt(hdc, 0, 0, width, height, memDC, 0, 0, SRCCOPY);

            // ������Դ
            SelectObject(memDC, oldBitmap);
            DeleteObject(memBitmap);
            DeleteDC(memDC);

            EndPaint(hwnd, &ps);
            return 0;
        }
    }
        break;
    case WM_LBUTTONDOWN:
    {
        if (pThis) {
            // ��ȡ�����λ��
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);

            // ����Ƿ�����ɾ����������ť
            for (const auto& btn : pThis->m_deleteButtons) {
                if (PtInRect(&btn.rect, { x, y })) {
                    if (btn.gameIndex >= 100000) {
                        // ������ť��gameIndexƫ��100000��
                        int realIndex = btn.gameIndex - 100000;
                        Game* game = pThis->m_currentGames[realIndex];
                        if (game && game->Launch()) {
                            MessageBox(pThis->m_hwnd, L"��Ϸ������", L"��ʾ", MB_OK | MB_ICONINFORMATION);
                        }
                        else {
                            MessageBox(pThis->m_hwnd, L"����ʧ�ܣ������ִ���ļ�·��", L"����ʧ��", MB_OK | MB_ICONERROR);
                        }
                    }
                    else {
                        // ɾ����ť
                        pThis->OnDeleteGame(btn.gameIndex);
                    }
                    return 0;
                }
            }
        }
    }
    break;
    case WM_RBUTTONDOWN:
    {
        if (pThis) {
            // ��ȡ�����λ��
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);

            // ����������Ϸ��Ƭ���򣬼����λ��
            int cardWidth = 200;
            int cardHeight = 280;
            int gapX = 20;
            int gapY = 20;
            int startX = 10;
            int startY = 10;
            int columns = 3;

            const std::vector<Game*>& gamesToShow = pThis->m_currentGames;
            int startIndex = pThis->m_currentPage * pThis->m_gamesPerPage;
            int endIndex = std::min((int)gamesToShow.size(), startIndex + pThis->m_gamesPerPage);

            for (int i = startIndex; i < endIndex; i++) {
                int col = (i - startIndex) % columns;
                int row = (i - startIndex) / columns;
                int cardX = startX + col * (cardWidth + gapX);
                int cardY = startY + row * (cardHeight + gapY);

                RECT cardRect = { cardX, cardY, cardX + cardWidth, cardY + cardHeight };

                if (PtInRect(&cardRect, { x, y })) {
                    // �����Ҽ��˵�
                    HMENU hPopMenu = CreatePopupMenu();
                    AppendMenu(hPopMenu, MF_STRING, 1, L"������Ϸ");
                    AppendMenu(hPopMenu, MF_STRING, 2, L"�༭��Ϸ��Ϣ");
                    AppendMenu(hPopMenu, MF_SEPARATOR, 0, NULL);
                    AppendMenu(hPopMenu, MF_STRING, 3, L"ɾ����Ϸ");

                    // ת�����굽��Ļ����
                    POINT pt = { x, y };
                    ClientToScreen(hwnd, &pt);

                    // ��ʾ�Ҽ��˵�����¼ѡ�е���Ϸ����
                    SetProp(hwnd, L"SelectedGameIndex", (HANDLE)(INT_PTR)i);
                    TrackPopupMenu(hPopMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON,
                        pt.x, pt.y, 0, hwnd, NULL);
                    DestroyMenu(hPopMenu);
                    return 0;
                }
            }
        }
    }
    break;
    case WM_COMMAND:
    {
        // �����Ҽ��˵�����
        if (pThis) {
            int gameIndex = (int)(INT_PTR)GetProp(hwnd, L"SelectedGameIndex");
            RemoveProp(hwnd, L"SelectedGameIndex");

            if (gameIndex >= 0 && gameIndex < (int)pThis->m_currentGames.size()) {
                Game* game = pThis->m_currentGames[gameIndex];

                switch (LOWORD(wParam)) {
                case 1: // ������Ϸ
                    if (game && game->Launch()) {
                        MessageBox(pThis->m_hwnd, L"��Ϸ������", L"��ʾ", MB_OK | MB_ICONINFORMATION);
                    }
                    else {
                        MessageBox(pThis->m_hwnd, L"����ʧ�ܣ������ִ���ļ�·��", L"����ʧ��", MB_OK | MB_ICONERROR);
                    }
                    break;

                case 2: // �༭��Ϸ��Ϣ
                    MessageBox(pThis->m_hwnd, L"�༭��Ϸ������δʵ��", L"��ʾ", MB_OK | MB_ICONINFORMATION);
                    break;

                case 3: // ɾ����Ϸ
                    pThis->OnDeleteGame(gameIndex);
                    break;
                }
            }
            return 0;
        }
    }
        break;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// ������Զ��崰�ڹ���
LRESULT CALLBACK MainWindow::SidebarWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    // ��ȡ������ָ��
    HWND hMainWnd = GetParent(hwnd);
    MainWindow* pThis = (MainWindow*)GetWindowLongPtr(hMainWnd, GWLP_USERDATA);

    switch (msg) {
    case WM_ERASEBKGND:
        // ��ֹ���������Լ�����˸
        return 1;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        // ��ȡ�������С
        RECT rcClient;
        GetClientRect(hwnd, &rcClient);
        int width = rcClient.right - rcClient.left;
        int height = rcClient.bottom - rcClient.top;

        // �����ڴ�DC��λͼ����˫����
        HDC memDC = CreateCompatibleDC(hdc);
        HBITMAP memBitmap = CreateCompatibleBitmap(hdc, width, height);
        HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

        // ����������� (����ɫ)
        HBRUSH hBrushBg = CreateSolidBrush(RGB(240, 245, 250));
        FillRect(memDC, &rcClient, hBrushBg);
        DeleteObject(hBrushBg);

        // ��ɻ��ƺ󣬽��ڴ�DC���ݸ��Ƶ���Ļ
        BitBlt(hdc, 0, 0, width, height, memDC, 0, 0, SRCCOPY);

        // ������Դ
        SelectObject(memDC, oldBitmap);
        DeleteObject(memBitmap);
        DeleteDC(memDC);

        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_COMMAND:
        // ת����������
        if (hMainWnd) {
            SendMessage(hMainWnd, WM_COMMAND, wParam, lParam);
            return 0;
        }
        break;
    }
    // Ĭ�ϴ���
    return DefWindowProc(hwnd, msg, wParam, lParam);
}