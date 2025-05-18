#pragma once
#include "../framework.h"
#include "../tools/StringUtils.h"

// ֧�ֵ�����
enum class AppTheme {
    Light,
    Dark
};

class Setting {
public:
    Setting();

    // ����
    void SetTheme(AppTheme theme);
    AppTheme GetTheme() const;

    // ����Ŀ¼
    void SetDataDirectory(const std::wstring& dir);
    std::wstring GetDataDirectory() const;

    // ����/��������
    bool Load();
    bool Save() const;

private:
    AppTheme m_theme;
    std::wstring m_dataDirectory;
};