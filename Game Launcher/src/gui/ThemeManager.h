#pragma once
#pragma once
#include "../framework.h"
#include "../core/setting.h"

// ������ɫ�ṹ��
struct ThemeColors {
    // ����ɫ
    COLORREF mainBackground;     // ������ɫ
    COLORREF contentBackground;  // ����������ɫ
    COLORREF sidebarBackground;  // ���������ɫ
    COLORREF topBarBackground;   // ����������ɫ
    COLORREF categoryBarBackground; // ����������ɫ
    COLORREF statusBarBackground;// ״̬������ɫ

    // ������ɫ
    COLORREF primaryText;        // ��Ҫ������ɫ
    COLORREF secondaryText;      // ��Ҫ������ɫ
    COLORREF titleText;          // ����������ɫ
    COLORREF menuText;           // �˵�������ɫ

    // �ؼ���ɫ
    COLORREF buttonBackground;   // ��ť����ɫ
    COLORREF buttonHoverBackground; // ��ť��ͣ����ɫ
    COLORREF buttonText;         // ��ť������ɫ

    // ��Ƭ��ɫ
    COLORREF cardBackground;     // ��Ϸ��Ƭ����ɫ
    COLORREF cardTitleBackground; // ��Ƭ����������ɫ
    COLORREF cardInfoBackground; // ��Ƭ��Ϣ������ɫ
    COLORREF cardFooterBackground1; // ��Ƭ�ײ�����1����ɫ
    COLORREF cardFooterBackground2; // ��Ƭ�ײ�����2����ɫ
};

// �������������ص���������
using ThemeChangedCallback = std::function<void(AppTheme)>;

// ���������
class ThemeManager {
public:
    // ��ȡ����ʵ��
    static ThemeManager& GetInstance();

    // ��ʼ������
    void Initialize(AppTheme theme);

    // �л�����
    void SwitchTheme(AppTheme theme);

    // ��ȡ��ǰ����
    AppTheme GetCurrentTheme() const;

    // ��ȡ��ǰ������ɫ
    const ThemeColors& GetColors() const;

    // ���������������
    void AddThemeChangedListener(ThemeChangedCallback callback);

private:
    ThemeManager();

    // ����ǳɫ������ɫ
    void InitLightTheme();

    // ������ɫ������ɫ
    void InitDarkTheme();

    // ֪ͨ���м����������ѱ��
    void NotifyThemeChanged();

private:
    AppTheme m_currentTheme;
    ThemeColors m_lightTheme;
    ThemeColors m_darkTheme;
    ThemeColors* m_currentColors;

    // �������������б�
    std::vector<ThemeChangedCallback> m_listeners;
};