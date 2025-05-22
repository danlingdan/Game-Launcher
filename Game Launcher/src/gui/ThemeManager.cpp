#include "ThemeManager.h"

// 单例实例
ThemeManager& ThemeManager::GetInstance() {
    static ThemeManager instance;
    return instance;
}

// 构造函数
ThemeManager::ThemeManager()
    : m_currentTheme(AppTheme::Light),
    m_currentColors(&m_lightTheme) {
    // 初始化浅色和深色主题
    InitLightTheme();
    InitDarkTheme();
}

// 初始化主题
void ThemeManager::Initialize(AppTheme theme) {
    m_currentTheme = theme;
    m_currentColors = (theme == AppTheme::Light) ? &m_lightTheme : &m_darkTheme;
}

// 切换主题
void ThemeManager::SwitchTheme(AppTheme theme) {
    if (m_currentTheme != theme) {
        m_currentTheme = theme;
        m_currentColors = (theme == AppTheme::Light) ? &m_lightTheme : &m_darkTheme;
        // 通知监听器
        NotifyThemeChanged();
    }
}

// 获取当前主题
AppTheme ThemeManager::GetCurrentTheme() const {
    return m_currentTheme;
}

// 获取当前主题颜色
const ThemeColors& ThemeManager::GetColors() const {
    return *m_currentColors;
}

// 添加主题变更监听器
void ThemeManager::AddThemeChangedListener(ThemeChangedCallback callback) {
    m_listeners.push_back(callback);
}

// 通知所有监听器主题已变更
void ThemeManager::NotifyThemeChanged() {
    for (const auto& listener : m_listeners) {
        listener(m_currentTheme);
    }
}

// 定义浅色主题颜色
void ThemeManager::InitLightTheme() {
    // 背景色
    m_lightTheme.mainBackground = RGB(245, 245, 245);     // 浅灰色背景
    m_lightTheme.contentBackground = RGB(255, 255, 255);  // 白色内容区
    m_lightTheme.sidebarBackground = RGB(240, 240, 240);  // 浅灰色侧边栏
    m_lightTheme.topBarBackground = RGB(230, 230, 230);   // 稍深的浅灰色顶部栏
    m_lightTheme.categoryBarBackground = RGB(235, 235, 235); // 分类栏背景
    m_lightTheme.statusBarBackground = RGB(220, 220, 220); // 状态栏背景

    // 文字颜色
    m_lightTheme.primaryText = RGB(30, 30, 30);         // 近黑色文字
    m_lightTheme.secondaryText = RGB(100, 100, 100);    // 灰色次要文字
    m_lightTheme.titleText = RGB(20, 20, 20);           // 深色标题文字
    m_lightTheme.menuText = RGB(40, 40, 40);            // 菜单文字颜色

    // 控件颜色
    m_lightTheme.buttonBackground = RGB(220, 220, 220);   // 浅灰色按钮
    m_lightTheme.buttonHoverBackground = RGB(200, 200, 200); // 按钮悬停颜色
    m_lightTheme.buttonText = RGB(30, 30, 30);            // 按钮文字颜色

    // 卡片颜色
    m_lightTheme.cardBackground = RGB(240, 240, 240);     // 卡片背景
    m_lightTheme.cardTitleBackground = RGB(220, 220, 220); // 卡片标题区
    m_lightTheme.cardInfoBackground = RGB(230, 230, 230);  // 卡片信息区
    m_lightTheme.cardFooterBackground1 = RGB(225, 225, 225); // 卡片底部区域1
    m_lightTheme.cardFooterBackground2 = RGB(215, 215, 215); // 卡片底部区域2
}

// 定义深色主题颜色
void ThemeManager::InitDarkTheme() {
    // 背景色
    m_darkTheme.mainBackground = RGB(35, 35, 35);        // 深灰色背景
    m_darkTheme.contentBackground = RGB(45, 45, 45);     // 稍浅的深色内容区
    m_darkTheme.sidebarBackground = RGB(35, 35, 35);     // 深灰色侧边栏
    m_darkTheme.topBarBackground = RGB(30, 30, 30);      // 更深的顶部栏
    m_darkTheme.categoryBarBackground = RGB(40, 40, 40); // 分类栏背景
    m_darkTheme.statusBarBackground = RGB(25, 25, 25);   // 状态栏背景

    // 文字颜色
    m_darkTheme.primaryText = RGB(220, 220, 220);       // 浅色文字
    m_darkTheme.secondaryText = RGB(180, 180, 180);     // 次要浅色文字
    m_darkTheme.titleText = RGB(240, 240, 240);         // 标题文字颜色
    m_darkTheme.menuText = RGB(220, 220, 220);          // 菜单文字颜色

    // 控件颜色
    m_darkTheme.buttonBackground = RGB(60, 60, 60);      // 深色按钮
    m_darkTheme.buttonHoverBackground = RGB(80, 80, 80); // 按钮悬停颜色
    m_darkTheme.buttonText = RGB(220, 220, 220);         // 按钮文字颜色

    // 卡片颜色
    m_darkTheme.cardBackground = RGB(45, 45, 45);        // 卡片背景
    m_darkTheme.cardTitleBackground = RGB(55, 55, 55);   // 卡片标题区
    m_darkTheme.cardInfoBackground = RGB(50, 50, 50);    // 卡片信息区
    m_darkTheme.cardFooterBackground1 = RGB(40, 40, 40); // 卡片底部区域1
    m_darkTheme.cardFooterBackground2 = RGB(35, 35, 35); // 卡片底部区域2
}