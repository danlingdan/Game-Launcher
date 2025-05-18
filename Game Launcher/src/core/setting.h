#pragma once
#include "../framework.h"
#include "../tools/StringUtils.h"

// 支持的主题
enum class AppTheme {
    Light,
    Dark
};

class Setting {
public:
    Setting();

    // 主题
    void SetTheme(AppTheme theme);
    AppTheme GetTheme() const;

    // 数据目录
    void SetDataDirectory(const std::wstring& dir);
    std::wstring GetDataDirectory() const;

    // 加载/保存设置
    bool Load();
    bool Save() const;

private:
    AppTheme m_theme;
    std::wstring m_dataDirectory;
};