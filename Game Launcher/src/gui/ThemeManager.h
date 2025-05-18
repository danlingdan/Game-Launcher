#pragma once
#pragma once
#include "../framework.h"
#include "../core/setting.h"

// 主题颜色结构体
struct ThemeColors {
    // 背景色
    COLORREF mainBackground;     // 主背景色
    COLORREF contentBackground;  // 内容区背景色
    COLORREF sidebarBackground;  // 侧边栏背景色
    COLORREF topBarBackground;   // 顶部栏背景色
    COLORREF categoryBarBackground; // 分类栏背景色
    COLORREF statusBarBackground;// 状态栏背景色

    // 文字颜色
    COLORREF primaryText;        // 主要文字颜色
    COLORREF secondaryText;      // 次要文字颜色
    COLORREF titleText;          // 标题文字颜色
    COLORREF menuText;           // 菜单文字颜色

    // 控件颜色
    COLORREF buttonBackground;   // 按钮背景色
    COLORREF buttonHoverBackground; // 按钮悬停背景色
    COLORREF buttonText;         // 按钮文字颜色

    // 卡片颜色
    COLORREF cardBackground;     // 游戏卡片背景色
    COLORREF cardTitleBackground; // 卡片标题区背景色
    COLORREF cardInfoBackground; // 卡片信息区背景色
    COLORREF cardFooterBackground1; // 卡片底部区域1背景色
    COLORREF cardFooterBackground2; // 卡片底部区域2背景色
};

// 主题变更监听器回调函数类型
using ThemeChangedCallback = std::function<void(AppTheme)>;

// 主题管理器
class ThemeManager {
public:
    // 获取单例实例
    static ThemeManager& GetInstance();

    // 初始化主题
    void Initialize(AppTheme theme);

    // 切换主题
    void SwitchTheme(AppTheme theme);

    // 获取当前主题
    AppTheme GetCurrentTheme() const;

    // 获取当前主题颜色
    const ThemeColors& GetColors() const;

    // 添加主题变更监听器
    void AddThemeChangedListener(ThemeChangedCallback callback);

private:
    ThemeManager();

    // 定义浅色主题颜色
    void InitLightTheme();

    // 定义深色主题颜色
    void InitDarkTheme();

    // 通知所有监听器主题已变更
    void NotifyThemeChanged();

private:
    AppTheme m_currentTheme;
    ThemeColors m_lightTheme;
    ThemeColors m_darkTheme;
    ThemeColors* m_currentColors;

    // 主题变更监听器列表
    std::vector<ThemeChangedCallback> m_listeners;
};