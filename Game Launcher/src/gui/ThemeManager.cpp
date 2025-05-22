#include "ThemeManager.h"

// ����ʵ��
ThemeManager& ThemeManager::GetInstance() {
    static ThemeManager instance;
    return instance;
}

// ���캯��
ThemeManager::ThemeManager()
    : m_currentTheme(AppTheme::Light),
    m_currentColors(&m_lightTheme) {
    // ��ʼ��ǳɫ����ɫ����
    InitLightTheme();
    InitDarkTheme();
}

// ��ʼ������
void ThemeManager::Initialize(AppTheme theme) {
    m_currentTheme = theme;
    m_currentColors = (theme == AppTheme::Light) ? &m_lightTheme : &m_darkTheme;
}

// �л�����
void ThemeManager::SwitchTheme(AppTheme theme) {
    if (m_currentTheme != theme) {
        m_currentTheme = theme;
        m_currentColors = (theme == AppTheme::Light) ? &m_lightTheme : &m_darkTheme;
        // ֪ͨ������
        NotifyThemeChanged();
    }
}

// ��ȡ��ǰ����
AppTheme ThemeManager::GetCurrentTheme() const {
    return m_currentTheme;
}

// ��ȡ��ǰ������ɫ
const ThemeColors& ThemeManager::GetColors() const {
    return *m_currentColors;
}

// ���������������
void ThemeManager::AddThemeChangedListener(ThemeChangedCallback callback) {
    m_listeners.push_back(callback);
}

// ֪ͨ���м����������ѱ��
void ThemeManager::NotifyThemeChanged() {
    for (const auto& listener : m_listeners) {
        listener(m_currentTheme);
    }
}

// ����ǳɫ������ɫ
void ThemeManager::InitLightTheme() {
    // ����ɫ
    m_lightTheme.mainBackground = RGB(245, 245, 245);     // ǳ��ɫ����
    m_lightTheme.contentBackground = RGB(255, 255, 255);  // ��ɫ������
    m_lightTheme.sidebarBackground = RGB(240, 240, 240);  // ǳ��ɫ�����
    m_lightTheme.topBarBackground = RGB(230, 230, 230);   // �����ǳ��ɫ������
    m_lightTheme.categoryBarBackground = RGB(235, 235, 235); // ����������
    m_lightTheme.statusBarBackground = RGB(220, 220, 220); // ״̬������

    // ������ɫ
    m_lightTheme.primaryText = RGB(30, 30, 30);         // ����ɫ����
    m_lightTheme.secondaryText = RGB(100, 100, 100);    // ��ɫ��Ҫ����
    m_lightTheme.titleText = RGB(20, 20, 20);           // ��ɫ��������
    m_lightTheme.menuText = RGB(40, 40, 40);            // �˵�������ɫ

    // �ؼ���ɫ
    m_lightTheme.buttonBackground = RGB(220, 220, 220);   // ǳ��ɫ��ť
    m_lightTheme.buttonHoverBackground = RGB(200, 200, 200); // ��ť��ͣ��ɫ
    m_lightTheme.buttonText = RGB(30, 30, 30);            // ��ť������ɫ

    // ��Ƭ��ɫ
    m_lightTheme.cardBackground = RGB(240, 240, 240);     // ��Ƭ����
    m_lightTheme.cardTitleBackground = RGB(220, 220, 220); // ��Ƭ������
    m_lightTheme.cardInfoBackground = RGB(230, 230, 230);  // ��Ƭ��Ϣ��
    m_lightTheme.cardFooterBackground1 = RGB(225, 225, 225); // ��Ƭ�ײ�����1
    m_lightTheme.cardFooterBackground2 = RGB(215, 215, 215); // ��Ƭ�ײ�����2
}

// ������ɫ������ɫ
void ThemeManager::InitDarkTheme() {
    // ����ɫ
    m_darkTheme.mainBackground = RGB(35, 35, 35);        // ���ɫ����
    m_darkTheme.contentBackground = RGB(45, 45, 45);     // ��ǳ����ɫ������
    m_darkTheme.sidebarBackground = RGB(35, 35, 35);     // ���ɫ�����
    m_darkTheme.topBarBackground = RGB(30, 30, 30);      // ����Ķ�����
    m_darkTheme.categoryBarBackground = RGB(40, 40, 40); // ����������
    m_darkTheme.statusBarBackground = RGB(25, 25, 25);   // ״̬������

    // ������ɫ
    m_darkTheme.primaryText = RGB(220, 220, 220);       // ǳɫ����
    m_darkTheme.secondaryText = RGB(180, 180, 180);     // ��Ҫǳɫ����
    m_darkTheme.titleText = RGB(240, 240, 240);         // ����������ɫ
    m_darkTheme.menuText = RGB(220, 220, 220);          // �˵�������ɫ

    // �ؼ���ɫ
    m_darkTheme.buttonBackground = RGB(60, 60, 60);      // ��ɫ��ť
    m_darkTheme.buttonHoverBackground = RGB(80, 80, 80); // ��ť��ͣ��ɫ
    m_darkTheme.buttonText = RGB(220, 220, 220);         // ��ť������ɫ

    // ��Ƭ��ɫ
    m_darkTheme.cardBackground = RGB(45, 45, 45);        // ��Ƭ����
    m_darkTheme.cardTitleBackground = RGB(55, 55, 55);   // ��Ƭ������
    m_darkTheme.cardInfoBackground = RGB(50, 50, 50);    // ��Ƭ��Ϣ��
    m_darkTheme.cardFooterBackground1 = RGB(40, 40, 40); // ��Ƭ�ײ�����1
    m_darkTheme.cardFooterBackground2 = RGB(35, 35, 35); // ��Ƭ�ײ�����2
}