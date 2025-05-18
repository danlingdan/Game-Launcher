#include "MainWindow.h"

#define WINDOW_CLASS_NAME _T("GameLauncherMainWindow")
#define WINDOW_TITLE _T("Game Launcher")
#define SIDEBAR_WIDTH 180

MainWindow::MainWindow() : m_hwnd(nullptr), m_sidebarHwnd(nullptr),
m_toolbarHwnd(nullptr), m_contentHwnd(nullptr), m_searchHwnd(nullptr),
m_pageInputHwnd(nullptr), m_hMenu(nullptr), m_hoverMenuItem(-1),
m_currentPage(0), m_totalPages(1), m_gamesPerPage(8),
m_themeComboHwnd(nullptr), m_dataDirEditHwnd(nullptr), m_browseBtnHwnd(nullptr), m_saveSettingsBtnHwnd(nullptr) {
    // 初始化顶部菜单项
    m_menuItems = {
        { L"游戏", {0}, ID_MENU_GAME, false },
        { L"宝箱", {0}, ID_MENU_BOX, false },
        { L"社区", {0}, ID_MENU_COMMUNITY, false },
        { L"配置", {0}, ID_MENU_CONFIG, false },
        { L"帮助", {0}, ID_MENU_HELP, false }
    };

    // 初始化工具栏图标按钮
    m_toolbarButtons = {
        { L"[T]", {0}, 3001, false }, // 缩略图视图
        { L"[L]", {0}, 3002, false }, // 列表视图
        { L"[D]", {0}, 3003, false }, // 详情视图
        { L"[G]", {0}, 3004, false }, // 网格视图
        { L"[B]", {0}, 3005, false }, // 大网格视图
        { L"[O]", {0}, 3006, false }, // 其他视图
        { L"[F]", {0}, 3007, false }, // 文件视图
        { L"[+]", {0}, 3008, false }  // 添加视图
    };

    // 创建字体
    memset(&m_titleFont, 0, sizeof(LOGFONT));
    m_titleFont.lfHeight = 22;
    m_titleFont.lfWeight = FW_BOLD;
    wcscpy_s(m_titleFont.lfFaceName, LF_FACESIZE, L"Arial");

    memset(&m_menuFont, 0, sizeof(LOGFONT));
    m_menuFont.lfHeight = 18;
    wcscpy_s(m_menuFont.lfFaceName, LF_FACESIZE, L"Microsoft YaHei");

    // 加载设置
    InitializeSettings();

    // 加载游戏集合
    m_gameCollection.LoadGames();
}

MainWindow::~MainWindow() {
    if (m_hTitleFont) DeleteObject(m_hTitleFont);
    if (m_hMenuFont) DeleteObject(m_hMenuFont);
}

void MainWindow::InitializeSettings() {
    bool loadResult = m_settings.Load();
    if (!loadResult) {
        m_settings = Setting();
        // 使用默认主题
    }
    AppTheme currentTheme = m_settings.GetTheme();
    ThemeManager::GetInstance().Initialize(currentTheme);
    ThemeManager::GetInstance().SwitchTheme(currentTheme);
}

bool MainWindow::Create(HINSTANCE hInstance, int nCmdShow) {
    // 初始化通用控件
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&icex);

    // 创建字体
    m_hTitleFont = CreateFontIndirect(&m_titleFont);
    m_hMenuFont = CreateFontIndirect(&m_menuFont);

    // 注册窗口类
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = MainWindow::WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = WINDOW_CLASS_NAME;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GAMELAUNCHER));
    RegisterClass(&wc);

    // 注册侧边栏窗口类
    WNDCLASS wcSidebar = { 0 };
    wcSidebar.lpfnWndProc = MainWindow::SidebarWndProc;
    wcSidebar.hInstance = hInstance;
    wcSidebar.lpszClassName = L"SidebarWindowClass";
    wcSidebar.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClass(&wcSidebar);

    // 游戏区
    WNDCLASS wcContent = { 0 };
    wcContent.lpfnWndProc = MainWindow::ContentWndProc;
    wcContent.hInstance = hInstance;
    wcContent.lpszClassName = L"GameContentArea";
    wcContent.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClass(&wcContent);

    // 创建窗口
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

    // 创建搜索框
    m_searchHwnd = CreateWindowEx(0, _T("EDIT"), _T(""),
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
        clientWidth - 300, TOPBAR_HEIGHT + 5, 200, 20,
        hwnd, (HMENU)ID_SEARCH_BOX, hInstance, NULL);

    // 创建侧边栏
    m_sidebarHwnd = CreateWindowEx(0, L"SidebarWindowClass", _T(""),
        WS_CHILD | WS_VISIBLE | SS_NOTIFY | SS_SUNKEN,
        0, TOPBAR_HEIGHT + CATEGORY_HEIGHT, SIDEBAR_WIDTH,
        clientHeight - TOPBAR_HEIGHT - CATEGORY_HEIGHT - STATUSBAR_HEIGHT,
        hwnd, (HMENU)ID_SIDEBAR, hInstance, NULL);

    // 创建游戏内容区
    m_contentHwnd = CreateWindowEx(0, L"GameContentArea", L"",
        WS_CHILD | WS_VISIBLE,
        SIDEBAR_WIDTH, TOPBAR_HEIGHT + CATEGORY_HEIGHT,
        clientWidth - SIDEBAR_WIDTH,
        clientHeight - TOPBAR_HEIGHT - CATEGORY_HEIGHT - STATUSBAR_HEIGHT,
        hwnd, (HMENU)ID_CONTENT_AREA, hInstance, this);

    // 创建底部分页按钮
    int btnWidth = 80;
    int btnHeight = 25;
    int btnY = clientHeight - STATUSBAR_HEIGHT / 2 - btnHeight / 2;
    int startX = (clientWidth - 5 * btnWidth - 40) / 2; // 40 是页码输入框宽度

    CreateWindowEx(0, _T("BUTTON"), _T("首页"),
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        startX, btnY, btnWidth, btnHeight,
        hwnd, (HMENU)PAGING_BTN_FIRST, hInstance, NULL);

    CreateWindowEx(0, _T("BUTTON"), _T("上一页"),
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        startX + btnWidth, btnY, btnWidth, btnHeight,
        hwnd, (HMENU)PAGING_BTN_PREV, hInstance, NULL);

    m_pageInputHwnd = CreateWindowEx(0, _T("EDIT"), _T("1"),
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER | ES_CENTER,
        startX + 2 * btnWidth, btnY, 40, btnHeight,
        hwnd, (HMENU)PAGING_EDIT_PAGE, hInstance, NULL);

    CreateWindowEx(0, _T("BUTTON"), _T("下一页"),
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        startX + 2 * btnWidth + 40, btnY, btnWidth, btnHeight,
        hwnd, (HMENU)PAGING_BTN_NEXT, hInstance, NULL);

    CreateWindowEx(0, _T("BUTTON"), _T("尾页"),
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        startX + 3 * btnWidth + 40, btnY, btnWidth, btnHeight,
        hwnd, (HMENU)PAGING_BTN_LAST, hInstance, NULL);

    CreateWindowEx(0, _T("BUTTON"), _T("跳转"),
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        startX + 4 * btnWidth + 40, btnY, btnWidth, btnHeight,
        hwnd, (HMENU)PAGING_BTN_GOTO, hInstance, NULL);

    // 添加侧边栏分类按钮
    AddSidebarButtons();

    // 计算菜单项位置
    CalculateMenuItemPositions();

    // 初始显示所有游戏
    RefreshGameDisplay();

    // 添加主题变更监听器
    ThemeManager::GetInstance().AddThemeChangedListener(
        [this](AppTheme theme) {
            // 强制整个窗口重绘
            InvalidateRect(m_hwnd, NULL, TRUE);

            // 重绘所有子窗口
            EnumChildWindows(m_hwnd, [](HWND hwnd, LPARAM lParam) -> BOOL {
                InvalidateRect(hwnd, NULL, TRUE);
                return TRUE;
                }, 0);

            // 更新窗口
            UpdateWindow(m_hwnd);
        }
    );
}

void MainWindow::CalculateMenuItemPositions() {
    // 计算每个顶部菜单项的位置
    const int logoWidth = 200;  // LOGO区域宽度
    const int menuItemWidth = 80;  // 每个菜单项宽度
    const int menuItemHeight = TOPBAR_HEIGHT;

    int x = logoWidth;
    for (auto& item : m_menuItems) {
        item.rect = { x, 0, x + menuItemWidth, menuItemHeight };
        x += menuItemWidth;
    }

    // 计算工具栏按钮位置
    const int toolbarX = SIDEBAR_WIDTH;
    const int toolButtonWidth = 30;
    const int toolButtonHeight = 30;
    const int toolbarY = TOPBAR_HEIGHT + (CATEGORY_HEIGHT - toolButtonHeight) / 2;

    x = toolbarX + 35;  // 留出"游戏分类"的空间
    for (auto& btn : m_toolbarButtons) {
        btn.rect = { x, toolbarY, x + toolButtonWidth, toolbarY + toolButtonHeight };
        x += toolButtonWidth + 2;
    }
}

void MainWindow::AddSidebarButtons() {
    OutputDebugString((L"AddSidebarButtons(): 当前分类=" + (m_currentCategory.empty() ? L"[空]" : m_currentCategory) + L"\n").c_str());

    HINSTANCE hInstance = GetModuleHandle(nullptr);
    int buttonHeight = 40;
    int yPos = 0;

    // 清空现有分类列表
    m_sidebarCategories.clear();

    // 使用预定义的分类列表，不再从游戏集合中获取分类
    for (size_t i = 0; i < PREDEFINED_GAME_CATEGORIES.size(); i++) {
        const std::wstring& category = PREDEFINED_GAME_CATEGORIES[i];
        m_sidebarCategories.push_back(category);

        HWND hButton = CreateWindowEx(0, L"BUTTON", category.c_str(),
            WS_CHILD | WS_VISIBLE | BS_FLAT | BS_LEFT,
            0, yPos, SIDEBAR_WIDTH, buttonHeight,
            m_sidebarHwnd, (HMENU)(ID_SIDEBAR + 1 + i), hInstance, NULL);

        // 高亮当前分类
        if ((i == 0 && m_currentCategory.empty()) ||
            (i > 0 && category == m_currentCategory)) {
            std::wstring selectedText = L"● " + category;
            SendMessage(hButton, WM_SETTEXT, 0, (LPARAM)selectedText.c_str());
            OutputDebugString((L"AddSidebarButtons(): 标记高亮分类 '" + category + L"'\n").c_str());
        }

        yPos += buttonHeight;
    }
}

void MainWindow::RefreshSidebar() {
    // 清空现有侧边栏按钮
    HWND child = GetWindow(m_sidebarHwnd, GW_CHILD);
    while (child) {
        HWND nextChild = GetWindow(child, GW_HWNDNEXT);
        DestroyWindow(child);
        child = nextChild;
    }

    // 重新添加侧边栏按钮
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

    // 创建内存DC和位图用于双缓冲
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBitmap = CreateCompatibleBitmap(hdc, width, height);
    HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

    // 获取当前主题颜色
    const ThemeColors& colors = ThemeManager::GetInstance().GetColors();

    // 绘制顶部菜单栏背景
    RECT rcTopBar = { 0, 0, width, TOPBAR_HEIGHT };
    HBRUSH hBrushTopBar = CreateSolidBrush(colors.topBarBackground);
    FillRect(memDC, &rcTopBar, hBrushTopBar);
    DeleteObject(hBrushTopBar);

    // 绘制LOGO
    SetBkMode(memDC, TRANSPARENT);
    SetTextColor(memDC, RGB(255, 200, 0));  // 金黄色LOGO保持不变
    HFONT oldFont = (HFONT)SelectObject(memDC, m_hTitleFont);
    TextOut(memDC, 20, (TOPBAR_HEIGHT - 22) / 2, L"DG", 2);

    SetTextColor(memDC, RGB(0, 150, 220));  // 蓝色Player保持不变
    TextOut(memDC, 60, (TOPBAR_HEIGHT - 22) / 2, L"player", 6);

    SetTextColor(memDC, RGB(180, 180, 180));  // 灰色版本号保持不变
    HFONT tempFont = (HFONT)SelectObject(memDC, m_hMenuFont);
    TextOut(memDC, 140, (TOPBAR_HEIGHT - 18) / 2, L"Ver 0.2", wcslen(L"Ver 0.2"));
    SelectObject(memDC, oldFont);

    // 绘制菜单项
    SelectObject(memDC, m_hMenuFont);
    SetTextColor(memDC, colors.menuText);

    for (const auto& item : m_menuItems) {
        // 如果鼠标悬停在菜单项上，绘制高亮背景
        if (item.isHovered) {
            HBRUSH hBrushHover = CreateSolidBrush(colors.buttonHoverBackground);
            FillRect(memDC, &item.rect, hBrushHover);
            DeleteObject(hBrushHover);
        }

        // 绘制菜单文字
        RECT textRect = item.rect;
        DrawText(memDC, item.text.c_str(), -1, &textRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }

    // 绘制游戏分类工具栏背景
    RECT rcCategoryBar = { 0, TOPBAR_HEIGHT, width, TOPBAR_HEIGHT + CATEGORY_HEIGHT };
    HBRUSH hBrushCategoryBar = CreateSolidBrush(colors.categoryBarBackground);
    FillRect(memDC, &rcCategoryBar, hBrushCategoryBar);
    DeleteObject(hBrushCategoryBar);

    // 绘制"游戏分类"文字
    SetTextColor(memDC, colors.primaryText);
    TextOut(memDC, 20, TOPBAR_HEIGHT + 10, L"游戏分类", 4);

    // 绘制工具栏按钮
    for (const auto& btn : m_toolbarButtons) {
        // 如果鼠标悬停在按钮上，绘制高亮背景
        if (btn.isHovered) {
            HBRUSH hBrushBtnHover = CreateSolidBrush(colors.buttonHoverBackground);
            FillRect(memDC, &btn.rect, hBrushBtnHover);
            DeleteObject(hBrushBtnHover);
        }
        else {
            // 绘制普通背景
            HBRUSH hBrushBtn = CreateSolidBrush(colors.buttonBackground);
            FillRect(memDC, &btn.rect, hBrushBtn);
            DeleteObject(hBrushBtn);
        }

        // 绘制边框
        HPEN hPen = CreatePen(PS_SOLID, 1, RGB(180, 180, 180));
        HPEN hOldPen = (HPEN)SelectObject(memDC, hPen);
        Rectangle(memDC, btn.rect.left, btn.rect.top, btn.rect.right, btn.rect.bottom);
        SelectObject(memDC, hOldPen);
        DeleteObject(hPen);

        // 按钮图标绘制代码保持不变
        int iconX = (btn.rect.left + btn.rect.right) / 2;
        int iconY = (btn.rect.top + btn.rect.bottom) / 2;
        int iconSize = 10;

        // 使用主题颜色绘制图标
        SetTextColor(memDC, colors.buttonText);

        switch (btn.id) {
        case 3001: // 缩略图视图 - 绘制小方块
        {
            Rectangle(memDC, iconX - iconSize / 2, iconY - iconSize / 2,
                iconX + iconSize / 2, iconY + iconSize / 2);
        }
            break;
        case 3002: // 列表视图 - 绘制三条水平线
        {
            for (int i = -3; i <= 3; i += 3) {
                MoveToEx(memDC, iconX - iconSize / 2, iconY + i, NULL);
                LineTo(memDC, iconX + iconSize / 2, iconY + i);
            }
        }
            break;
        case 3003: // 详情视图
        {
            Rectangle(memDC, iconX - iconSize / 2, iconY - iconSize / 2,
                iconX + iconSize / 2, iconY + iconSize / 2);
            MoveToEx(memDC, iconX - iconSize / 4, iconY - iconSize / 4, NULL);
            LineTo(memDC, iconX + iconSize / 4, iconY - iconSize / 4);
        }
            break;
        case 3004: // 网格视图 - 绘制2x2网格
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
        case 3005: // 大网格视图
        {
            Rectangle(memDC, iconX - iconSize / 2, iconY - iconSize / 2,
                iconX + iconSize / 2, iconY + iconSize / 2);
            MoveToEx(memDC, iconX, iconY - iconSize / 2, NULL);
            LineTo(memDC, iconX, iconY + iconSize / 2);
            MoveToEx(memDC, iconX - iconSize / 2, iconY, NULL);
            LineTo(memDC, iconX + iconSize / 2, iconY);
        }
            break;
        case 3006: // 其他视图
        {
            Ellipse(memDC, iconX - iconSize / 2, iconY - iconSize / 2,
                iconX + iconSize / 2, iconY + iconSize / 2);
        }
            break;
        case 3007: // 文件视图
        {
            Rectangle(memDC, iconX - iconSize / 2, iconY - iconSize / 2,
                iconX + iconSize / 2, iconY - iconSize / 4);
            Rectangle(memDC, iconX - iconSize / 2, iconY - iconSize / 4,
                iconX + iconSize / 2, iconY + iconSize / 2);
        }
            break;
        case 3008: // 添加视图
        {
            MoveToEx(memDC, iconX - iconSize / 2, iconY, NULL);
            LineTo(memDC, iconX + iconSize / 2, iconY);
            MoveToEx(memDC, iconX, iconY - iconSize / 2, NULL);
            LineTo(memDC, iconX, iconY + iconSize / 2);
        }
            break;
        }
    }

    // 绘制"Go"按钮
    RECT goButtonRect = { width - 70, TOPBAR_HEIGHT + 5, width - 10, TOPBAR_HEIGHT + CATEGORY_HEIGHT - 5 };
    HBRUSH hBrushGoBtn = CreateSolidBrush(colors.topBarBackground); // 使用主题颜色
    FillRect(memDC, &goButtonRect, hBrushGoBtn);
    DeleteObject(hBrushGoBtn);

    SetTextColor(memDC, RGB(255, 255, 255)); // Go按钮文字保持白色
    DrawText(memDC, L"Go", -1, &goButtonRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    // 绘制底部状态栏
    RECT rcStatusBar = { 0, rcClient.bottom - STATUSBAR_HEIGHT, width, rcClient.bottom };
    DrawStatusBar(memDC, rcStatusBar);

    // 将内存DC内容一次性复制到屏幕
    BitBlt(hdc, 0, 0, width, height, memDC, 0, 0, SRCCOPY);

    // 清理资源
    SelectObject(memDC, oldBitmap);
    DeleteObject(memBitmap);
    DeleteDC(memDC);

    // 恢复默认字体
    SelectObject(hdc, oldFont);

    EndPaint(m_hwnd, &ps);
}

void MainWindow::OnSize(int width, int height) {
    if (m_sidebarHwnd && m_contentHwnd && m_searchHwnd) {
        // 调整侧边栏
        MoveWindow(m_sidebarHwnd, 0, TOPBAR_HEIGHT + CATEGORY_HEIGHT, SIDEBAR_WIDTH,
            height - TOPBAR_HEIGHT - CATEGORY_HEIGHT - STATUSBAR_HEIGHT, TRUE);

        // 调整搜索框
        MoveWindow(m_searchHwnd, width - 300, TOPBAR_HEIGHT + 5, 200, 20, TRUE);

        // 调整内容区
        MoveWindow(m_contentHwnd, SIDEBAR_WIDTH, TOPBAR_HEIGHT + CATEGORY_HEIGHT,
            width - SIDEBAR_WIDTH, height - TOPBAR_HEIGHT - CATEGORY_HEIGHT - STATUSBAR_HEIGHT, TRUE);

        // 调整底部分页按钮
        int btnWidth = 80;
        int btnHeight = 25;
        int btnY = height - STATUSBAR_HEIGHT / 2 - btnHeight / 2;
        int startX = (width - 5 * btnWidth - 40) / 2; // 40 是页码输入框宽度

        // 定位分页按钮
        MoveWindow(GetDlgItem(m_hwnd, PAGING_BTN_FIRST), startX, btnY, btnWidth, btnHeight, TRUE);
        MoveWindow(GetDlgItem(m_hwnd, PAGING_BTN_PREV), startX + btnWidth, btnY, btnWidth, btnHeight, TRUE);
        MoveWindow(m_pageInputHwnd, startX + 2 * btnWidth, btnY, 40, btnHeight, TRUE);
        MoveWindow(GetDlgItem(m_hwnd, PAGING_BTN_NEXT), startX + 2 * btnWidth + 40, btnY, btnWidth, btnHeight, TRUE);
        MoveWindow(GetDlgItem(m_hwnd, PAGING_BTN_LAST), startX + 3 * btnWidth + 40, btnY, btnWidth, btnHeight, TRUE);
        MoveWindow(GetDlgItem(m_hwnd, PAGING_BTN_GOTO), startX + 4 * btnWidth + 40, btnY, btnWidth, btnHeight, TRUE);

        // 重新计算菜单项位置
        CalculateMenuItemPositions();

        // --------- 增强的自适应每页游戏数量计算 ---------

        // 记录调整前的第一个可见游戏索引，用于保持滚动位置
        int oldStartIndex = m_currentPage * m_gamesPerPage;
        int oldGamesPerPage = m_gamesPerPage;

        // 内容区尺寸
        int contentWidth = width - SIDEBAR_WIDTH - 20; // 减去右侧边距
        int contentHeight = height - TOPBAR_HEIGHT - CATEGORY_HEIGHT - STATUSBAR_HEIGHT - 30; // 减去底部提示区域高度

        // 获取DPI比例以适应高分屏
        UINT dpiX = 96, dpiY = 96;
        GetMonitorDpiForWindow(m_hwnd, dpiX, dpiY);
        float dpiScale = dpiX / 96.0f;

        // 卡片尺寸和间距 - 可选择性缩放以适应不同分辨率
        int cardWidth = static_cast<int>(180 * dpiScale);     // 从200改为180
        int cardHeight = static_cast<int>(250 * dpiScale);    // 从280改为250
        int gapX = static_cast<int>(15 * dpiScale);          // 可选择减小间距
        int gapY = static_cast<int>(15 * dpiScale);

        // 确保卡片至少有最小尺寸
        cardWidth = std::max(140, std::min(220, cardWidth));  // 调整最小最大限制
        cardHeight = std::max(200, std::min(300, cardHeight));

        // 根据内容区域精确计算能够容纳的列数和行数
        int availableWidth = contentWidth - gapX; // 考虑左右边距
        int columns = std::max(1, availableWidth / (cardWidth + gapX));

        // 再次检查计算出的宽度是否适合内容区域
        int actualWidth = columns * (cardWidth + gapX) + gapX;
        if (actualWidth > contentWidth && columns > 1) {
            columns--;
        }

        int availableHeight = contentHeight - gapY; // 考虑上下边距
        int rows = std::max(1, availableHeight / (cardHeight + gapY));

        // 再次检查计算出的高度是否适合内容区域
        int actualHeight = rows * (cardHeight + gapY) + gapY;
        if (actualHeight > contentHeight && rows > 1) {
            rows--;
        }

        // 更新每页游戏数量
        int newGamesPerPage = columns * rows;

        // 日志输出计算结果
        WCHAR debugStr[256];
        swprintf_s(debugStr, L"自适应计算: 内容区 %dx%d, 卡片 %dx%d, 列数=%d, 行数=%d, 每页=%d 款游戏\n",
            contentWidth, contentHeight, cardWidth, cardHeight, columns, rows, newGamesPerPage);
        OutputDebugString(debugStr);

        // 如果计算结果有变化才更新
        if (newGamesPerPage != m_gamesPerPage) {
            m_gamesPerPage = newGamesPerPage;

            // 计算新的当前页，尽量保持原来查看的游戏可见
            if (m_currentGames.size() > 0) {
                // 计算新的页码，使得之前的第一个游戏尽可能保持可见
                m_currentPage = oldStartIndex / m_gamesPerPage;

                // 确保当前页码在有效范围内
                int totalPages = (m_currentGames.size() + m_gamesPerPage - 1) / m_gamesPerPage;
                if (totalPages == 0) totalPages = 1;
                if (m_currentPage >= totalPages) m_currentPage = totalPages - 1;
                if (m_currentPage < 0) m_currentPage = 0;

                // 记录页码变更日志
                swprintf_s(debugStr, L"页码调整: 从 %d/%d 变为 %d/%d\n",
                    oldStartIndex / oldGamesPerPage + 1,
                    (m_currentGames.size() + oldGamesPerPage - 1) / oldGamesPerPage,
                    m_currentPage + 1,
                    (m_currentGames.size() + m_gamesPerPage - 1) / m_gamesPerPage);
                OutputDebugString(debugStr);
            }

            // 强制刷新游戏显示
            RefreshGameDisplay();
        }

        // 强制重绘
        InvalidateRect(m_hwnd, NULL, TRUE);
    }
}

void MainWindow::OnMouseMove(int x, int y) {
    bool needRedraw = false;

    // 检查菜单项悬停
    for (auto& item : m_menuItems) {
        bool wasHovered = item.isHovered;
        item.isHovered = PtInRect(&item.rect, { x, y });

        if (wasHovered != item.isHovered) {
            needRedraw = true;
        }
    }

    // 检查工具栏按钮悬停
    for (auto& btn : m_toolbarButtons) {
        bool wasHovered = btn.isHovered;
        btn.isHovered = PtInRect(&btn.rect, { x, y });

        if (wasHovered != btn.isHovered) {
            needRedraw = true;
        }
    }

    if (needRedraw) {
        // 仅重绘顶部区域
        RECT rcTopArea = { 0, 0, 9999, TOPBAR_HEIGHT + CATEGORY_HEIGHT };
        InvalidateRect(m_hwnd, &rcTopArea, TRUE);
    }
}

void MainWindow::OnLButtonDown(int x, int y) {
    bool handled = false;

    // 检查菜单项点击
    for (const auto& item : m_menuItems) {
        if (PtInRect(&item.rect, { x, y })) {
            if (item.id == ID_MENU_GAME) {
                ShowGameListPage(); // 切回游戏列表
                handled = true;
            }
            else if (item.id == ID_MENU_CONFIG) {
                ShowSettingsPage(); // 显示设置
                handled = true;
            }
            else if (item.id == ID_MENU_HELP) {
                ShowHelpDialog();
                handled = true;
            }
            else {
                MessageBox(m_hwnd, (L"点击了菜单: " + item.text).c_str(), L"菜单点击", MB_OK);
            }
            break;
        }
    }

    if (!handled) {
        // 检查工具栏按钮点击
        for (const auto& btn : m_toolbarButtons) {
            if (PtInRect(&btn.rect, { x, y })) {
                // 特别处理添加游戏按钮 (ID 3008)
                if (btn.id == 3008) {
                    // 显示确认调试信息
                    OutputDebugString(L"点击了添加游戏按钮\n");
                    OnAddGame();
                    handled = true;
                }
                else {
                    // 处理其他按钮点击事件
                    MessageBox(m_hwnd, (L"点击了工具按钮: " + btn.text).c_str(), L"按钮点击", MB_OK);
                }
                break;
            }
        }
    }

    if (!handled) {
        // 检查"Go"按钮点击
        RECT rcClient;
        GetClientRect(m_hwnd, &rcClient);
        RECT goButtonRect = { rcClient.right - 70, TOPBAR_HEIGHT + 5, rcClient.right - 10, TOPBAR_HEIGHT + CATEGORY_HEIGHT - 5 };

        if (PtInRect(&goButtonRect, { x, y })) {
            // 处理"Go"按钮点击
            TCHAR searchText[256] = { 0 };
            GetWindowText(m_searchHwnd, searchText, 256);
            if (searchText[0] != 0) {
                // 通过名称搜索游戏
                std::vector<Game*> results = m_gameCollection.FindGamesByName(searchText);
                // 显示搜索结果数量 
                MessageBox(m_hwnd,
                    (std::wstring(L"找到 ") + std::to_wstring(results.size()) + L" 个游戏").c_str(),
                    L"搜索结果", MB_OK);
                // 刷新游戏显示
                RefreshGameDisplay(results);
            }
        }
    }
}

void MainWindow::OnDeleteGame(int gameIndex) {
    if (gameIndex < 0 || gameIndex >= static_cast<int>(m_gameCollection.GetGameCount())) {
        MessageBox(m_hwnd, L"无效的游戏索引", L"错误", MB_OK | MB_ICONERROR);
        return;
    }
    Game* game = m_gameCollection.GetGame(gameIndex);
    std::wstring message = L"确定要删除游戏 \"" + game->GetName() + L"\" 吗？\n此操作不可撤销。";
    if (MessageBox(m_hwnd, message.c_str(), L"确认删除", MB_YESNO | MB_ICONQUESTION) == IDYES) {
        if (m_gameCollection.RemoveGame(gameIndex)) {
            MessageBox(m_hwnd, L"游戏删除成功", L"删除游戏", MB_OK | MB_ICONINFORMATION);
            RefreshSidebar();
            // 按当前分类刷新
            RefreshGameDisplay();
        }
        else {
            MessageBox(m_hwnd, L"删除游戏失败", L"错误", MB_OK | MB_ICONERROR);
        }
    }
}

void MainWindow::ShowSettingsPage() {
    m_currentPageType = ContentPage::Settings;

    // 删除之前的控件
    if (m_themeComboHwnd) DestroyWindow(m_themeComboHwnd);
    if (m_dataDirEditHwnd) DestroyWindow(m_dataDirEditHwnd);
    if (m_browseBtnHwnd) DestroyWindow(m_browseBtnHwnd);
    if (m_saveSettingsBtnHwnd) DestroyWindow(m_saveSettingsBtnHwnd);

    // 获取内容区域尺寸
    RECT rcContent;
    GetClientRect(m_contentHwnd, &rcContent);
    int width = rcContent.right - rcContent.left;

    // 创建控件
    HINSTANCE hInst = GetModuleHandle(NULL);

    // 标题和说明用绘制方式显示，在DrawSettingsPage中处理

    // 主题选择控件
    m_themeLabelHwnd = CreateWindowEx(0, L"STATIC", L"应用主题：",
        WS_CHILD | WS_VISIBLE,
        100, 100, 100, 24,
        m_contentHwnd, (HMENU)4001, hInst, NULL);

    m_themeComboHwnd = CreateWindowEx(0, L"COMBOBOX", L"",
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
        210, 100, 150, 200,
        m_contentHwnd, (HMENU)4002, hInst, NULL);

    // 添加主题选项
    SendMessage(m_themeComboHwnd, CB_ADDSTRING, 0, (LPARAM)L"浅色主题");
    SendMessage(m_themeComboHwnd, CB_ADDSTRING, 0, (LPARAM)L"深色主题");
    // 选择当前主题
    SendMessage(m_themeComboHwnd, CB_SETCURSEL, m_settings.GetTheme() == AppTheme::Light ? 0 : 1, 0);

    // 数据目录控件
    m_dataDirLabelHwnd = CreateWindowEx(0, L"STATIC", L"数据目录：",
        WS_CHILD | WS_VISIBLE,
        100, 150, 100, 24,
        m_contentHwnd, (HMENU)4003, hInst, NULL);

    m_dataDirEditHwnd = CreateWindowEx(0, L"EDIT", m_settings.GetDataDirectory().c_str(),
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
        210, 150, 300, 24,
        m_contentHwnd, (HMENU)4004, hInst, NULL);

    m_browseBtnHwnd = CreateWindowEx(0, L"BUTTON", L"浏览...",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        520, 150, 80, 24,
        m_contentHwnd, (HMENU)4005, hInst, NULL);

    // 保存按钮
    m_saveSettingsBtnHwnd = CreateWindowEx(0, L"BUTTON", L"保存设置",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        width / 2 - 60, 220, 120, 30,
        m_contentHwnd, (HMENU)4006, hInst, NULL);

    // 重绘内容区
    InvalidateRect(m_contentHwnd, NULL, TRUE);
}

void MainWindow::ShowGameListPage() {
    ClearContentControls(); // 先清理内容区控件
    m_currentPageType = ContentPage::GameList;
    RefreshGameDisplay();
}

void MainWindow::DrawSettingsPage(HDC hdc, const RECT& rcClient) {
    // 获取当前主题颜色
    const ThemeColors& colors = ThemeManager::GetInstance().GetColors();

    // 设置文本颜色和背景模式
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, colors.titleText); // 使用主题标题颜色

    // 创建标题字体
    HFONT hTitleFont = CreateFont(28, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH, L"微软雅黑");
    HFONT oldFont = (HFONT)SelectObject(hdc, hTitleFont);

    // 绘制标题
    RECT titleRect = rcClient;
    titleRect.top += 30;
    DrawText(hdc, L"设置", -1, &titleRect, DT_CENTER | DT_TOP | DT_SINGLELINE);

    // 创建说明文字字体
    HFONT hDescFont = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH, L"微软雅黑");
    SelectObject(hdc, hDescFont);

    // 绘制说明文字
    SetTextColor(hdc, colors.secondaryText); // 使用主题次要文本颜色
    RECT descRect = rcClient;
    descRect.top += 70;
    DrawText(hdc, L"在此页面可以修改应用的外观和数据存储位置", -1, &descRect, DT_CENTER | DT_TOP | DT_SINGLELINE);

    // 清理资源
    SelectObject(hdc, oldFont);
    DeleteObject(hTitleFont);
    DeleteObject(hDescFont);
}

void MainWindow::SaveSettings() {
    OutputDebugString(L"正在执行SaveSettings()方法...\n");

    // 获取主题设置
    int themeIndex = SendMessage(m_themeComboHwnd, CB_GETCURSEL, 0, 0);
    AppTheme newTheme = (themeIndex == 0) ? AppTheme::Light : AppTheme::Dark;
    m_settings.SetTheme(newTheme);

    // 获取数据目录
    wchar_t buffer[MAX_PATH];
    GetWindowText(m_dataDirEditHwnd, buffer, MAX_PATH);
    m_settings.SetDataDirectory(buffer);

    OutputDebugString((L"设置内容 - 主题:" + std::wstring(newTheme == AppTheme::Light ? L"浅色" : L"深色") +
        L", 数据目录:" + buffer + L"\n").c_str());

    // 保存设置
    bool saveResult = m_settings.Save();
    OutputDebugString((L"保存结果:" + std::wstring(saveResult ? L"成功" : L"失败") + L"\n").c_str());

    if (saveResult) {
        MessageBox(m_hwnd, L"设置已保存", L"设置", MB_OK | MB_ICONINFORMATION);
        ApplyTheme(newTheme);
    }
    else {
        MessageBox(m_hwnd, L"保存设置失败，请检查文件权限", L"错误", MB_OK | MB_ICONERROR);
    }
}

void MainWindow::BrowseForDataDirectory() {
    wchar_t buffer[MAX_PATH] = { 0 };

    // 获取当前数据目录
    GetWindowText(m_dataDirEditHwnd, buffer, MAX_PATH);

    // 创建浏览文件夹对话框
    BROWSEINFO bi = { 0 };
    bi.hwndOwner = m_hwnd;
    bi.lpszTitle = L"请选择数据目录";
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

    LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
    if (pidl != NULL) {
        // 获取选择的目录路径
        if (SHGetPathFromIDList(pidl, buffer)) {
            // 更新显示
            SetWindowText(m_dataDirEditHwnd, buffer);
        }

        // 释放内存
        CoTaskMemFree(pidl);
    }
}

void MainWindow::ApplyTheme(AppTheme theme) {
    // 记录当前和新主题信息
    AppTheme currentTheme = ThemeManager::GetInstance().GetCurrentTheme();
    OutputDebugString((L"ApplyTheme: 切换主题从 " +
        std::wstring(currentTheme == AppTheme::Light ? L"浅色" : L"深色") +
        L" 到 " +
        std::wstring(theme == AppTheme::Light ? L"浅色" : L"深色") + L"\n").c_str());

    // 应用主题到界面
    ThemeManager::GetInstance().SwitchTheme(theme);

    // 重绘界面以立即应用主题变更
    InvalidateRect(m_hwnd, NULL, TRUE);
    InvalidateRect(m_sidebarHwnd, NULL, TRUE);
    InvalidateRect(m_contentHwnd, NULL, TRUE);

    // 为其他子窗口设置主题
    EnumChildWindows(m_hwnd, [](HWND hwnd, LPARAM lParam) -> BOOL {
        InvalidateRect(hwnd, NULL, TRUE);
        return TRUE;
        }, 0);

    OutputDebugString(L"主题已切换\n");
}

void MainWindow::ClearContentControls() {
    if (m_themeComboHwnd) { DestroyWindow(m_themeComboHwnd); m_themeComboHwnd = nullptr; }
    if (m_dataDirEditHwnd) { DestroyWindow(m_dataDirEditHwnd); m_dataDirEditHwnd = nullptr; }
    if (m_browseBtnHwnd) { DestroyWindow(m_browseBtnHwnd); m_browseBtnHwnd = nullptr; }
    if (m_saveSettingsBtnHwnd) { DestroyWindow(m_saveSettingsBtnHwnd); m_saveSettingsBtnHwnd = nullptr; }
    if (m_themeLabelHwnd) { DestroyWindow(m_themeLabelHwnd); m_themeLabelHwnd = nullptr; }
    if (m_dataDirLabelHwnd) { DestroyWindow(m_dataDirLabelHwnd); m_dataDirLabelHwnd = nullptr; }
}

// 刷新游戏显示
void MainWindow::RefreshGameDisplay(const std::vector<Game*>& games) {
    // 保存当前要显示的游戏列表
    m_currentGames = games;

    // 计算总页数
    m_totalPages = (games.size() + m_gamesPerPage - 1) / m_gamesPerPage;  // 向上取整
    if (m_totalPages == 0) m_totalPages = 1;  // 至少有1页

    // 确保当前页码在有效范围内
    if (m_currentPage >= m_totalPages) m_currentPage = m_totalPages - 1;
    if (m_currentPage < 0) m_currentPage = 0;

    // 添加调试输出
    WCHAR debugStr[100];
    swprintf_s(debugStr, L"当前页: %d, 总页数: %d, 游戏总数: %d\n",
        m_currentPage + 1, m_totalPages, (int)games.size());
    OutputDebugString(debugStr);

    // 更新页码输入框
    if (m_pageInputHwnd) {
        TCHAR pageText[16];
        _stprintf_s(pageText, _T("%d"), m_currentPage + 1);
        SetWindowText(m_pageInputHwnd, pageText);
    }

    // 记录游戏信息用于调试
    std::wstring gameList;
    for (const auto& game : games) {
        if (!gameList.empty()) gameList += L"\n";
        gameList += game->GetName() + L" (" + game->GetCategory() + L")";
    }

    if (!gameList.empty()) {
        OutputDebugString((L"游戏列表:\n" + gameList + L"\n").c_str());
    }
    else {
        OutputDebugString(L"游戏列表为空\n");
    }

    // 重绘内容区
    InvalidateRect(m_contentHwnd, NULL, TRUE);

    // 重绘状态栏区域
    RECT rcClient;
    GetClientRect(m_hwnd, &rcClient);
    RECT rcStatusBar = { 0, rcClient.bottom - STATUSBAR_HEIGHT, rcClient.right, rcClient.bottom };
    InvalidateRect(m_hwnd, &rcStatusBar, TRUE);
}

// 重载无参数版本，刷新游戏显示
void MainWindow::RefreshGameDisplay() {
    OutputDebugString((L"RefreshGameDisplay(): 当前分类=" + (m_currentCategory.empty() ? L"[空]" : m_currentCategory) +
        L", 当前页=" + std::to_wstring(m_currentPage) + L"\n").c_str());

    if (m_currentCategory.empty() || m_currentCategory == L"全部游戏") {
        // 显示全部游戏
        OutputDebugString(L"RefreshGameDisplay(): 正在显示全部游戏\n");
        std::vector<Game*> allGames;
        allGames.reserve(m_gameCollection.GetGameCount());

        for (size_t i = 0; i < m_gameCollection.GetGameCount(); i++) {
            allGames.push_back(m_gameCollection.GetGame(i));
        }
        RefreshGameDisplay(allGames);
    }
    else {
        // 显示当前分类的游戏
        OutputDebugString((L"RefreshGameDisplay(): 正在显示分类 '" + m_currentCategory + L"' 的游戏\n").c_str());
        std::vector<Game*> games = m_gameCollection.FindGamesByCategory(m_currentCategory);
        RefreshGameDisplay(games);
    }
}

// 添加游戏
void MainWindow::OnAddGame() {
    OutputDebugString(L"正在执行 OnAddGame() 方法\n");
    Game newGame;
    GameDialog dialog;
    try {
        if (dialog.ShowAddDialog(m_hwnd, newGame, m_gameCollection)) {
            OutputDebugString(L"对话框返回OK，正在添加游戏\n");
            if (m_gameCollection.AddGame(newGame)) {
                MessageBox(m_hwnd, L"游戏添加成功", L"添加游戏", MB_OK | MB_ICONINFORMATION);
                RefreshSidebar();
                // 按当前分类刷新
                RefreshGameDisplay();
            }
            else {
                MessageBox(m_hwnd, L"保存游戏信息失败", L"错误", MB_OK | MB_ICONERROR);
                OutputDebugString(L"m_gameCollection.AddGame 返回 false\n");
            }
        }
        else {
            OutputDebugString(L"用户取消了游戏添加对话框\n");
        }
    }
    catch (const std::exception& e) {
        char buffer[1024];
        sprintf_s(buffer, "添加游戏时发生异常: %s", e.what());
        OutputDebugStringA(buffer);
        MessageBoxA(m_hwnd, buffer, "错误", MB_OK | MB_ICONERROR);
    }
    catch (...) {
        OutputDebugString(L"添加游戏时发生未知异常\n");
        MessageBox(m_hwnd, L"添加游戏时发生未知异常", L"错误", MB_OK | MB_ICONERROR);
    }
}

// 绘制游戏视图
void MainWindow::DrawGameCard(HDC hdc, const Game* game, int x, int y, int width, int height, int gameIndex) {
    // 设置文字渲染模式
    SetBkMode(hdc, TRANSPARENT);

    // 卡片背景色 - 深蓝灰色
    HBRUSH hBrushBg = CreateSolidBrush(RGB(45, 53, 80));
    RECT cardRect = { x, y, x + width, y + height };
    FillRect(hdc, &cardRect, hBrushBg);
    DeleteObject(hBrushBg);

    // 图标区域 - 上方三分之一区域
    int iconHeight = height / 3;
    RECT iconRect = { x, y, x + width, y + iconHeight };

    // 绘制图标背景
    HBRUSH hBrushIconBg = CreateSolidBrush(RGB(30, 35, 50));
    FillRect(hdc, &iconRect, hBrushIconBg);
    DeleteObject(hBrushIconBg);

    // 创建适合中文显示的字体
    HFONT hLabelFont = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH, L"微软雅黑");

    HFONT hValueFont = CreateFont(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH, L"微软雅黑");


    // 删除按钮区域，适当加大，避免太靠边
    RECT deleteButtonRect = { x + width - 35, y + 15, x + width - 15, y + 35 };

    // 1. 填充红色背景
    HBRUSH hDeleteBrush = CreateSolidBrush(RGB(220, 60, 60));
    FillRect(hdc, &deleteButtonRect, hDeleteBrush);
    DeleteObject(hDeleteBrush);

    // 2. 只画白色边框，不覆盖填充色
    HPEN hWhitePen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
    HPEN hOldPen = (HPEN)SelectObject(hdc, hWhitePen);
    HBRUSH hNullBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hNullBrush);
    Rectangle(hdc, deleteButtonRect.left, deleteButtonRect.top, deleteButtonRect.right, deleteButtonRect.bottom);
    SelectObject(hdc, hOldBrush);
    SelectObject(hdc, hOldPen);
    DeleteObject(hWhitePen);

    // 3. 绘制“X”
    HFONT hButtonFont = CreateFont(18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH, L"微软雅黑");
    HFONT hOldFont = (HFONT)SelectObject(hdc, hButtonFont);
    SetTextColor(hdc, RGB(255, 255, 255));
    SetBkMode(hdc, TRANSPARENT);
    DrawTextW(hdc, L"X", -1, &deleteButtonRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    SelectObject(hdc, hOldFont);
    DeleteObject(hButtonFont);

    // 保存删除按钮信息
    DeleteButtonInfo deleteBtn;
    deleteBtn.rect = deleteButtonRect;
    deleteBtn.gameIndex = gameIndex;
    m_deleteButtons.push_back(deleteBtn);

    // 启动按钮区域
    RECT launchButtonRect = { x + 15, y + 15, x + 75, y + 35 };

    // 1. 填充绿色背景
    HBRUSH hLaunchBrush = CreateSolidBrush(RGB(60, 180, 60));
    FillRect(hdc, &launchButtonRect, hLaunchBrush);
    DeleteObject(hLaunchBrush);

    // 2. 画白色边框
    HPEN hGreenPen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
    hOldPen = (HPEN)SelectObject(hdc, hGreenPen);
    hOldBrush = (HBRUSH)SelectObject(hdc, hNullBrush);
    Rectangle(hdc, launchButtonRect.left, launchButtonRect.top, launchButtonRect.right, launchButtonRect.bottom);
    SelectObject(hdc, hOldBrush);
    SelectObject(hdc, hOldPen);
    DeleteObject(hGreenPen);

    // 3. 绘制“启动”文字
    hButtonFont = CreateFont(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH, L"微软雅黑");
    hOldFont = (HFONT)SelectObject(hdc, hButtonFont);
    SetTextColor(hdc, RGB(255, 255, 255));
    DrawTextW(hdc, L"启动", -1, &launchButtonRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    SelectObject(hdc, hOldFont);
    DeleteObject(hButtonFont);

    // 保存启动按钮信息
    DeleteButtonInfo launchBtn;
    launchBtn.rect = launchButtonRect;
    launchBtn.gameIndex = gameIndex + 100000; // 用大于当前游戏数的偏移区分启动按钮
    m_deleteButtons.push_back(launchBtn);

    // 尝试加载游戏图标
    bool iconLoaded = false;
    HICON hIcon = NULL;

    if (!game->GetExecutablePath().empty()) {
        // 首先尝试从可执行文件提取图标
        HINSTANCE hInst = GetModuleHandle(NULL);
        hIcon = ExtractIcon(hInst, game->GetExecutablePath().c_str(), 0);

        // 如果可执行文件没有图标，尝试使用指定的图标路径
        if (hIcon == NULL || hIcon == (HICON)1) {
            if (!game->GetIconPath().empty()) {
                hIcon = (HICON)LoadImage(NULL, game->GetIconPath().c_str(), IMAGE_ICON,
                    0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE);
            }
        }
    }
    else if (!game->GetIconPath().empty()) {
        // 如果没有可执行文件路径，直接尝试图标路径
        hIcon = (HICON)LoadImage(NULL, game->GetIconPath().c_str(), IMAGE_ICON,
            0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE);
    }

    if (hIcon && hIcon != (HICON)1) {
        // 计算图标在区域中居中的位置
        int iconSize = std::min(64, std::min(width - 20, iconHeight - 20));
        int iconX = x + (width - iconSize) / 2;
        int iconY = y + (iconHeight - iconSize) / 2;

        // 绘制图标
        DrawIconEx(hdc, iconX, iconY, hIcon, iconSize, iconSize, 0, NULL, DI_NORMAL);
        DestroyIcon(hIcon);
        iconLoaded = true;
    }

    // 如果没有图标，绘制"NOT FOUND"占位符
    if (!iconLoaded) {
        SetTextColor(hdc, RGB(220, 220, 220));
        DrawTextW(hdc, L"NOT FOUND", -1, &iconRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }

    // 信息区域 - 中部
    RECT infoRect = { x, y + iconHeight, x + width, y + height - 60 };
    HBRUSH hBrushInfoBg = CreateSolidBrush(RGB(60, 70, 100));
    FillRect(hdc, &infoRect, hBrushInfoBg);
    DeleteObject(hBrushInfoBg);

    // 改善信息显示 - 使用表格式布局
    const int padding = 8;        // 边距
    const int labelWidth = 65;    // 标签宽度
    const int lineHeight = 20;    // 行高

    // 游戏名称行
    int textY = y + iconHeight + padding;
    RECT labelRect = { x + padding, textY, x + padding + labelWidth, textY + lineHeight };
    RECT valueRect = { x + padding + labelWidth, textY, x + width - padding, textY + lineHeight };

    // 绘制标签
    SelectObject(hdc, hLabelFont);
    SetTextColor(hdc, RGB(200, 200, 200));
    DrawTextW(hdc, L"游戏名称", -1, &labelRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    // 绘制值（使用粗体）
    SelectObject(hdc, hValueFont);
    SetTextColor(hdc, RGB(220, 220, 255));

    // 如果游戏名称太长，截断并添加省略号
    std::wstring gameName = game->GetName();
    if (gameName.length() > 15) { // 根据实际情况调整截断长度
        gameName = gameName.substr(0, 15) + L"...";
    }
    DrawTextW(hdc, gameName.c_str(), -1, &valueRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    // 发行年代行
    textY += lineHeight + 5;
    labelRect = { x + padding, textY, x + padding + labelWidth, textY + lineHeight };
    valueRect = { x + padding + labelWidth, textY, x + width - padding, textY + lineHeight };

    SelectObject(hdc, hLabelFont);
    SetTextColor(hdc, RGB(200, 200, 200));
    DrawTextW(hdc, L"发行年代", -1, &labelRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    SelectObject(hdc, hValueFont);
    SetTextColor(hdc, RGB(220, 220, 255));
    DrawTextW(hdc, game->GetReleaseYear().c_str(), -1, &valueRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    // 制作发行行
    textY += lineHeight + 5;
    labelRect = { x + padding, textY, x + padding + labelWidth, textY + lineHeight };
    valueRect = { x + padding + labelWidth, textY, x + width - padding, textY + lineHeight };

    SelectObject(hdc, hLabelFont);
    SetTextColor(hdc, RGB(200, 200, 200));
    DrawTextW(hdc, L"制作发行", -1, &labelRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    SelectObject(hdc, hValueFont);
    SetTextColor(hdc, RGB(220, 220, 255));

    // 如果发行商名称太长，截断并添加省略号
    std::wstring publisher = game->GetPublisher();
    if (publisher.length() > 15) {
        publisher = publisher.substr(0, 15) + L"...";
    }
    DrawTextW(hdc, publisher.c_str(), -1, &valueRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    // 底部信息区域 - 两行
    // 第一行: 语言和分类
    RECT langRect = { x, y + height - 60, x + width / 2, y + height - 30 };
    HBRUSH hBrushLangBg = CreateSolidBrush(RGB(34, 96, 120)); // 蓝绿色
    FillRect(hdc, &langRect, hBrushLangBg);
    DeleteObject(hBrushLangBg);

    RECT catRect = { x + width / 2, y + height - 60, x + width, y + height - 30 };
    HBRUSH hBrushCatBg = CreateSolidBrush(RGB(70, 100, 150)); // 蓝色
    FillRect(hdc, &catRect, hBrushCatBg);
    DeleteObject(hBrushCatBg);

    // 语言和分类文本 - 使用标签字体
    SelectObject(hdc, hLabelFont);
    SetTextColor(hdc, RGB(240, 240, 240));
    DrawTextW(hdc, game->GetLanguage().c_str(), -1, &langRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    DrawTextW(hdc, game->GetCategory().c_str(), -1, &catRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    // 第二行: 平台和编号
    RECT platformRect = { x, y + height - 30, x + width / 2, y + height };
    HBRUSH hBrushPlatformBg = CreateSolidBrush(RGB(40, 40, 80)); // 深蓝色
    FillRect(hdc, &platformRect, hBrushPlatformBg);
    DeleteObject(hBrushPlatformBg);

    RECT numRect = { x + width / 2, y + height - 30, x + width, y + height };
    HBRUSH hBrushNumBg = CreateSolidBrush(RGB(40, 40, 80)); // 深蓝色
    FillRect(hdc, &numRect, hBrushNumBg);
    DeleteObject(hBrushNumBg);

    // 平台和编号文本
    DrawTextW(hdc, game->GetPlatform().c_str(), -1, &platformRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    // 显示游戏编号
    std::wstring indexStr = L"- " + std::to_wstring(gameIndex + 1) + L" -";
    DrawTextW(hdc, indexStr.c_str(), -1, &numRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    // 恢复原字体并清理资源
    SelectObject(hdc, hOldFont);
    DeleteObject(hLabelFont);
    DeleteObject(hValueFont);
    DeleteObject(hButtonFont);
}

// 绘制状态栏
void MainWindow::DrawStatusBar(HDC hdc, const RECT& rect) {
    // 获取当前主题颜色
    const ThemeColors& colors = ThemeManager::GetInstance().GetColors();

    // 绘制状态栏背景
    HBRUSH hBrushStatusBar = CreateSolidBrush(colors.statusBarBackground);
    FillRect(hdc, &rect, hBrushStatusBar);
    DeleteObject(hBrushStatusBar);

    // 绘制顶部边框线
    HPEN hPen = CreatePen(PS_SOLID, 1, RGB(180, 180, 180));
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
    MoveToEx(hdc, rect.left, rect.top, NULL);
    LineTo(hdc, rect.right, rect.top);
    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);

    // 绘制游戏总数和页码信息
    SetTextColor(hdc, colors.primaryText);
    SetBkMode(hdc, TRANSPARENT);

    // 计算显示文本，使用与图片示例相同的格式
    std::wstring statusText = L"共有 " + std::to_wstring(m_currentGames.size()) +
        L" 款游戏，每页显示 " + std::to_wstring(m_gamesPerPage) +
        L" 款，共 " + std::to_wstring(m_totalPages) + L" 页";

    // 计算文本位置，在按钮上方绘制
    const int leftMargin = 20; // 左侧边距
    int textY = rect.top + (rect.bottom - rect.top - 20) / 2; // 垂直居中
    RECT textRect = { rect.left + leftMargin, textY, rect.right, textY + 20 };
    DrawText(hdc, statusText.c_str(), -1, &textRect, DT_LEFT | DT_SINGLELINE);
}

void MainWindow::ShowHelpDialog() {
    // 创建帮助信息文本
    std::wstring helpText =
        L"游戏启动器 - 使用帮助\n\n"
        L"基本操作：\n"
        L"1. 添加游戏：点击顶部菜单游戏或工具栏中的+按钮\n"
        L"2. 删除游戏：在游戏卡片右上角点击X按钮\n"
        L"3. 启动游戏：在游戏卡片左上角点击启动按钮\n"
        L"4. 搜索游戏：在右上角搜索框输入关键词，点击Go按钮\n"
        L"5. 分类浏览：在左侧分类栏选择游戏分类\n"
        L"6. 翻页浏览：使用底部的页码导航按钮，或使用鼠标滚轮\n\n"
        L"提示：\n"
        L"- 添加游戏时，选择可执行文件可自动填充部分游戏信息\n"
        L"- 在任何分类下都可以通过页码进行导航\n"
        L"- 支持鼠标滚轮快速翻页\n\n"
        L"DGplayer 游戏启动器 Ver 0.1\n"
        L"HuaJH制作。禁止贩卖！";

    // 显示帮助对话框
    MessageBox(m_hwnd, helpText.c_str(), L"帮助", MB_OK | MB_ICONINFORMATION);
}

// 主窗口
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
        // 阻止擦除背景以减少闪烁
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

            // 1. 侧边栏分类按钮
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

            // 2. 顶部菜单
            switch (wmId) {
            case ID_MENU_GAME:
            {
                pThis->ShowGameListPage();
            }
                break;
            case ID_MENU_BOX:
            case ID_MENU_COMMUNITY:
                MessageBox(pThis->m_hwnd, L"该功能暂未实现", L"提示", MB_OK | MB_ICONINFORMATION);
                break;
            case ID_MENU_CONFIG:
            {
                pThis->ShowSettingsPage();
            }
                break;
            case ID_MENU_HELP:
            {
                pThis->ShowHelpDialog();
            }
                break;

                // 3. 分页按钮
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
                    MessageBox(pThis->m_hwnd, L"请输入有效页码", L"提示", MB_OK | MB_ICONINFORMATION);
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

// 游戏内容窗口
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
        // 阻止擦除背景以减少闪烁
        return 1;
    case WM_MOUSEWHEEL:
    {
        if (!pThis) break;

        // 获取滚轮增量并调整敏感度
        short zDelta = GET_WHEEL_DELTA_WPARAM(wParam);

        // 同时检查Ctrl键是否按下，实现快速翻页
        bool isCtrlPressed = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
        int pagesToScroll = isCtrlPressed ? 3 : 1;

        if (zDelta > 0) {
            // 向上滚动，上一页 (支持多页快速翻页)
            for (int i = 0; i < pagesToScroll && pThis->m_currentPage > 0; i++) {
                pThis->m_currentPage--;
            }
            pThis->RefreshGameDisplay();
        }
        else if (zDelta < 0) {
            // 向下滚动，下一页 (支持多页快速翻页)
            for (int i = 0; i < pagesToScroll && pThis->m_currentPage < pThis->m_totalPages - 1; i++) {
                pThis->m_currentPage++;
            }
            pThis->RefreshGameDisplay();
        }
        return 0; // 返回0表示消息已处理
    }

    case WM_KEYDOWN:
    {
        if (pThis) {
            switch (wParam) {
            case VK_PRIOR: // Page Up键
                if (pThis->m_currentPage > 0) {
                    pThis->m_currentPage--;
                    pThis->RefreshGameDisplay();
                }
                return 0;

            case VK_NEXT: // Page Down键
                if (pThis->m_currentPage < pThis->m_totalPages - 1) {
                    pThis->m_currentPage++;
                    pThis->RefreshGameDisplay();
                }
                return 0;

            case VK_HOME: // Home键跳转到第一页
                pThis->m_currentPage = 0;
                pThis->RefreshGameDisplay();
                return 0;

            case VK_END: // End键跳转到最后一页
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

            // 获取内容区大小
            RECT rcClient;
            GetClientRect(hwnd, &rcClient);
            int width = rcClient.right - rcClient.left;
            int height = rcClient.bottom - rcClient.top;

            // 创建内存DC和位图用于双缓冲
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP memBitmap = CreateCompatibleBitmap(hdc, width, height);
            HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

            // 使用主题颜色填充背景
            const ThemeColors& colors = ThemeManager::GetInstance().GetColors();
            HBRUSH hBrushBg = CreateSolidBrush(colors.contentBackground);
            FillRect(memDC, &rcClient, hBrushBg);
            DeleteObject(hBrushBg);

            // 清空之前的删除按钮信息
            pThis->m_deleteButtons.clear();

            if (pThis->m_currentPageType == ContentPage::Settings) {
                // 绘制设置页面
                pThis->DrawSettingsPage(memDC, rcClient);
            }
            else {
                // 游戏卡片和间距
                int cardWidth = 180;    // 从200改为180
                int cardHeight = 250;   // 从280改为250
                int gapX = 15;          // 可选择减小间距
                int gapY = 15;          // 可选择减小间距
                int startX = 10;
                int startY = 10;

                // 动态计算列数
                int columns = std::max(1, (width - gapX) / (cardWidth + gapX));
                const std::vector<Game*>& gamesToShow = pThis->m_currentGames;
                int startIndex = pThis->m_currentPage * pThis->m_gamesPerPage;
                int endIndex = std::min((int)gamesToShow.size(), startIndex + pThis->m_gamesPerPage);

                // 绘制当前页的游戏
                for (int i = startIndex; i < endIndex; i++) {
                    int idx = i - startIndex;
                    int col = idx % columns;
                    int row = idx / columns;
                    int x = startX + col * (cardWidth + gapX);
                    int y = startY + row * (cardHeight + gapY);
                    pThis->DrawGameCard(memDC, gamesToShow[i], x, y, cardWidth, cardHeight, i);
                }

                // 如果当前页没有游戏，显示提示信息
                if (startIndex >= (int)gamesToShow.size() || gamesToShow.empty()) {
                    RECT textRect = rcClient;
                    SetTextColor(memDC, RGB(100, 100, 100));
                    SetBkMode(memDC, TRANSPARENT);
                    HFONT hFont = CreateFont(20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                        CLEARTYPE_QUALITY, DEFAULT_PITCH, L"微软雅黑");
                    HFONT oldFont = (HFONT)SelectObject(memDC, hFont);
                    DrawText(memDC, L"暂无游戏，请点击顶部菜单的游戏或工具栏中的 + 添加游戏",
                        -1, &textRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                    SelectObject(memDC, oldFont);
                    DeleteObject(hFont);
                }

                // 显示翻页提示（如果有多页）
                if (pThis->m_totalPages > 1) {
                    RECT navHintRect = { 10, height - 30, width - 10, height - 10 };
                    SetTextColor(memDC, RGB(80, 80, 80));
                    SetBkMode(memDC, TRANSPARENT);
                    HFONT hHintFont = CreateFont(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                        CLEARTYPE_QUALITY, DEFAULT_PITCH, L"微软雅黑");
                    HFONT oldHintFont = (HFONT)SelectObject(memDC, hHintFont);
                    std::wstring navHint = L"提示：可使用鼠标滚轮、Page Up/Down、Home/End 进行翻页";
                    DrawText(memDC, navHint.c_str(), -1, &navHintRect, DT_RIGHT | DT_BOTTOM | DT_SINGLELINE);
                    SelectObject(memDC, oldHintFont);
                    DeleteObject(hHintFont);
                }
            }

            // 完成绘制后，将内存DC内容复制到屏幕
            BitBlt(hdc, 0, 0, width, height, memDC, 0, 0, SRCCOPY);

            // 清理资源
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
            // 获取鼠标点击位置
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);

            // 检查是否点击了删除或启动按钮
            for (const auto& btn : pThis->m_deleteButtons) {
                if (PtInRect(&btn.rect, { x, y })) {
                    if (btn.gameIndex >= 100000) {
                        // 启动按钮（gameIndex偏移100000）
                        int realIndex = btn.gameIndex - 100000;
                        Game* game = pThis->m_currentGames[realIndex];
                        if (game && game->Launch()) {
                            MessageBox(pThis->m_hwnd, L"游戏已启动", L"提示", MB_OK | MB_ICONINFORMATION);
                        }
                        else {
                            MessageBox(pThis->m_hwnd, L"启动失败，请检查可执行文件路径", L"启动失败", MB_OK | MB_ICONERROR);
                        }
                    }
                    else {
                        // 删除按钮
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
            // 获取鼠标点击位置
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);

            // 遍历所有游戏卡片区域，检查点击位置
            int cardWidth = 180;    // 从200改为180
            int cardHeight = 250;   // 从280改为250
            int gapX = 15;
            int gapY = 15;
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
                    // 创建右键菜单
                    HMENU hPopMenu = CreatePopupMenu();
                    AppendMenu(hPopMenu, MF_STRING, 1, L"启动游戏");
                    AppendMenu(hPopMenu, MF_STRING, 2, L"编辑游戏信息");
                    AppendMenu(hPopMenu, MF_SEPARATOR, 0, NULL);
                    AppendMenu(hPopMenu, MF_STRING, 3, L"删除游戏");

                    // 转换坐标到屏幕坐标
                    POINT pt = { x, y };
                    ClientToScreen(hwnd, &pt);

                    // 显示右键菜单，记录选中的游戏索引
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
        // 处理右键菜单命令
        if (pThis) {
            // 首先处理设置页面按钮，这应该与游戏选择无关
            switch (LOWORD(wParam)) {
            case 4005: // 浏览按钮
            {
                if (HIWORD(wParam) == BN_CLICKED) {
                    OutputDebugString(L"浏览按钮被点击\n");
                    pThis->BrowseForDataDirectory();
                    return 0;
                }
            }
                break;

            case 4006: // 保存设置按钮
            {
                if (HIWORD(wParam) == BN_CLICKED) {
                    OutputDebugString(L"保存设置按钮被点击\n");
                    pThis->SaveSettings();
                    return 0;
                }
            }
                break;
            }

            int gameIndex = (int)(INT_PTR)GetProp(hwnd, L"SelectedGameIndex");
            RemoveProp(hwnd, L"SelectedGameIndex");

            if (gameIndex >= 0 && gameIndex < (int)pThis->m_currentGames.size()) {
                Game* game = pThis->m_currentGames[gameIndex];

                switch (LOWORD(wParam)) {
                case 1: // 启动游戏
                {
                    if (game && game->Launch()) {
                        MessageBox(pThis->m_hwnd, L"游戏已启动", L"提示", MB_OK | MB_ICONINFORMATION);
                    }
                    else {
                        MessageBox(pThis->m_hwnd, L"启动失败，请检查可执行文件路径", L"启动失败", MB_OK | MB_ICONERROR);
                    }
                }
                    break;
                case 2: // 编辑游戏信息
                {
                    MessageBox(pThis->m_hwnd, L"编辑游戏功能暂未实现", L"提示", MB_OK | MB_ICONINFORMATION);
                }
                    break;
                case 3: // 删除游戏
                {
                    pThis->OnDeleteGame(gameIndex);
                }
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

// 侧边栏自定义窗口过程
LRESULT CALLBACK MainWindow::SidebarWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    // 获取主窗口指针
    HWND hMainWnd = GetParent(hwnd);
    MainWindow* pThis = (MainWindow*)GetWindowLongPtr(hMainWnd, GWLP_USERDATA);

    switch (msg) {
    case WM_ERASEBKGND:
        // 阻止擦除背景以减少闪烁
        return 1;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        // 获取侧边栏大小
        RECT rcClient;
        GetClientRect(hwnd, &rcClient);
        int width = rcClient.right - rcClient.left;
        int height = rcClient.bottom - rcClient.top;

        // 创建内存DC和位图用于双缓冲
        HDC memDC = CreateCompatibleDC(hdc);
        HBITMAP memBitmap = CreateCompatibleBitmap(hdc, width, height);
        HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

        // 填充侧边栏背景 (淡蓝色)
        HBRUSH hBrushBg = CreateSolidBrush(RGB(240, 245, 250));
        FillRect(memDC, &rcClient, hBrushBg);
        DeleteObject(hBrushBg);

        // 完成绘制后，将内存DC内容复制到屏幕
        BitBlt(hdc, 0, 0, width, height, memDC, 0, 0, SRCCOPY);

        // 清理资源
        SelectObject(memDC, oldBitmap);
        DeleteObject(memBitmap);
        DeleteDC(memDC);

        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_COMMAND:
        // 转发到主窗口
        if (hMainWnd) {
            SendMessage(hMainWnd, WM_COMMAND, wParam, lParam);
            return 0;
        }
        break;
    }
    // 默认处理
    return DefWindowProc(hwnd, msg, wParam, lParam);
}