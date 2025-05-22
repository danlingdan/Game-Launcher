#pragma once
#include "../framework.h"
#include "../../Resource.h"
#include "../core/GameCollection.h"
#include "GameDialog.h"
#include "../core/GameManager.h"
#include "../core/setting.h"
#include "../gui/ThemeManager.h"

#define SIDEBAR_WIDTH 180
#define TOOLBAR_HEIGHT 40
#define MENU_HEIGHT 30
#define STATUSBAR_HEIGHT 30
#define TOPBAR_HEIGHT 45  // �����˵����߶�
#define CATEGORY_HEIGHT 35  // ��Ϸ�����и߶�

// �˵���ṹ
struct MenuItem {
    std::wstring text;
    RECT rect;
    int id;
    bool isHovered;
};

// ɾ����ť�ṹ
struct DeleteButtonInfo {
    RECT rect;
    int gameIndex;
};

enum class ContentPage {
    GameList,
    Settings
};

class MainWindow {
public:
    MainWindow();
    ~MainWindow();

    bool Create(HINSTANCE hInstance, int nCmdShow);

    void OnAddGame();
    void OnDeleteGame(int gameIndex);
    void DrawGameCard(HDC hdc, const Game* game, int x, int y, int width, int height, int gameIndex);
    void AsyncLoadGames();
private:
    void OnCreate(HWND hwnd);
    void OnDestroy();
    void OnSize(int width, int height);
    void OnPaint();
    void OnMouseMove(int x, int y);
    void OnLButtonDown(int x, int y);
    void AddSidebarButtons();
    void CalculateMenuItemPositions();
    void RefreshGameDisplay();
    void RefreshGameDisplay(const std::vector<Game*>& games);
    void RefreshSidebar();
    void DrawStatusBar(HDC hdc, const RECT& rect);
    void ShowHelpDialog();
    void ClearContentControls();

    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK ContentWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK SidebarWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK ThemeButtonProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    HWND m_hwnd;
    HWND m_sidebarHwnd;
    HWND m_toolbarHwnd;
    HWND m_contentHwnd;
    HWND m_statusbarHwnd;
    HWND m_pageInputHwnd;  // ҳ�������
    HWND m_searchHwnd;
    HMENU m_hMenu;
    HWND m_themeLabelHwnd = nullptr;
    HWND m_dataDirLabelHwnd = nullptr;

    ContentPage m_currentPageType = ContentPage::GameList;
    void ShowSettingsPage();
    void ShowGameListPage();
    void DrawSettingsPage(HDC hdc, const RECT& rcClient);

    // ��ҳ���
    int m_currentPage;     // ��ǰҳ��
    int m_totalPages;      // ��ҳ��
    int m_gamesPerPage;    // ÿҳ��ʾ����Ϸ��

    // �Զ��嶥���˵����
    std::vector<MenuItem> m_menuItems;
    std::vector<MenuItem> m_toolbarButtons;
    int m_hoverMenuItem;

    std::vector<DeleteButtonInfo> m_deleteButtons;


    // ����
    LOGFONT m_titleFont;
    LOGFONT m_menuFont;
    HFONT m_hTitleFont = NULL;
    HFONT m_hMenuFont = NULL;

    // �����Ϸ���ϳ�Ա
    GameCollection m_gameCollection;
    std::vector<std::wstring> m_sidebarCategories;
    std::vector<Game*> m_currentGames; // ���浱ǰҪ��ʾ����Ϸ
    std::wstring m_currentCategory;    // ��ǰɸѡ�ķ��ࣨ��Ϊȫ����
    bool m_isLoading;  // ����״̬��־

    // ���ö���
    Setting m_settings;

    // ����ҳ��ؼ�
    HWND m_themeComboHwnd;
    HWND m_dataDirEditHwnd;
    HWND m_browseBtnHwnd;
    HWND m_saveSettingsBtnHwnd;

    // ����ҳ����ط���
    void InitializeSettings();
    void SaveSettings();
    void BrowseForDataDirectory();
    void ApplyTheme(AppTheme theme);
};
