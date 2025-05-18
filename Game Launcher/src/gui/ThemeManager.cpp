#include "ThemeManager.h"

ThemeManager& ThemeManager::GetInstance() {
    static ThemeManager instance;
    return instance;
}

ThemeManager::ThemeManager() : m_currentTheme(AppTheme::Light), m_currentColors(&m_lightTheme) {
    InitLightTheme();
    InitDarkTheme();
}

void ThemeManager::Initialize(AppTheme theme) {
    m_currentTheme = theme;
    m_currentColors = (theme == AppTheme::Light) ? &m_lightTheme : &m_darkTheme;
}

void ThemeManager::SwitchTheme(AppTheme theme) {
    if (m_currentTheme != theme) {
        m_currentTheme = theme;
        m_currentColors = (theme == AppTheme::Light) ? &m_lightTheme : &m_darkTheme;

        // 通知UI需要刷新
        NotifyThemeChanged();
    }
}

AppTheme ThemeManager::GetCurrentTheme() const {
    return m_currentTheme;
}

const ThemeColors& ThemeManager::GetColors() const {
    return *m_currentColors;
}

void ThemeManager::AddThemeChangedListener(ThemeChangedCallback callback) {
    m_listeners.push_back(callback);
}

void ThemeManager::NotifyThemeChanged() {
    for (auto& listener : m_listeners) {
        listener(m_currentTheme);
    }
}

void ThemeManager::InitLightTheme() {
    // 浅色主题配色方案
    m_lightTheme.mainBackground = RGB(250, 250, 250);
    m_lightTheme.contentBackground = RGB(240, 240, 245);
    m_lightTheme.sidebarBackground = RGB(240, 245, 250);
    m_lightTheme.topBarBackground = RGB(30, 35, 70);  // 保持原始颜色
    m_lightTheme.categoryBarBackground = RGB(240, 240, 245);
    m_lightTheme.statusBarBackground = RGB(235, 235, 235);

    m_lightTheme.primaryText = RGB(40, 40, 80);
    m_lightTheme.secondaryText = RGB(100, 100, 100);
    m_lightTheme.titleText = RGB(40, 40, 80);
    m_lightTheme.menuText = RGB(220, 220, 220);

    m_lightTheme.buttonBackground = RGB(230, 230, 235);
    m_lightTheme.buttonHoverBackground = RGB(200, 210, 230);
    m_lightTheme.buttonText = RGB(50, 50, 50);

    m_lightTheme.cardBackground = RGB(45, 53, 80);
    m_lightTheme.cardTitleBackground = RGB(30, 35, 50);
    m_lightTheme.cardInfoBackground = RGB(60, 70, 100);
    m_lightTheme.cardFooterBackground1 = RGB(34, 96, 120);
    m_lightTheme.cardFooterBackground2 = RGB(70, 100, 150);
}

void ThemeManager::InitDarkTheme() {
    // 深色主题配色方案
    m_darkTheme.mainBackground = RGB(30, 30, 35);
    m_darkTheme.contentBackground = RGB(45, 45, 50);
    m_darkTheme.sidebarBackground = RGB(40, 40, 45);
    m_darkTheme.topBarBackground = RGB(20, 20, 30);
    m_darkTheme.categoryBarBackground = RGB(35, 35, 40);
    m_darkTheme.statusBarBackground = RGB(35, 35, 40);

    m_darkTheme.primaryText = RGB(220, 220, 230);
    m_darkTheme.secondaryText = RGB(180, 180, 200);
    m_darkTheme.titleText = RGB(220, 220, 240);
    m_darkTheme.menuText = RGB(220, 220, 220);

    m_darkTheme.buttonBackground = RGB(60, 60, 70);
    m_darkTheme.buttonHoverBackground = RGB(80, 80, 100);
    m_darkTheme.buttonText = RGB(220, 220, 230);

    m_darkTheme.cardBackground = RGB(50, 50, 65);
    m_darkTheme.cardTitleBackground = RGB(40, 40, 55);
    m_darkTheme.cardInfoBackground = RGB(55, 60, 80);
    m_darkTheme.cardFooterBackground1 = RGB(45, 85, 110);
    m_darkTheme.cardFooterBackground2 = RGB(60, 80, 120);
}