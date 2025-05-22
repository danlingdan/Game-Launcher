#pragma once
#include "../framework.h"
#include "../tools/StringUtils.h"

// 游戏类 - 存储单个游戏的信息
class Game {
public:
    Game();
    Game(const std::wstring& name, const std::wstring& releaseYear,
        const std::wstring& publisher, const std::wstring& language,
        const std::wstring& category, const std::wstring& platform);
    ~Game();

    // Getter/Setter
    const std::wstring& GetName() const { return m_name; }
    void SetName(const std::wstring& name) { m_name = name; }

    const std::wstring& GetReleaseYear() const { return m_releaseYear; }
    void SetReleaseYear(const std::wstring& year) { m_releaseYear = year; }

    const std::wstring& GetPublisher() const { return m_publisher; }
    void SetPublisher(const std::wstring& publisher) { m_publisher = publisher; }

    const std::wstring& GetLanguage() const { return m_language; }
    void SetLanguage(const std::wstring& language) { m_language = language; }

    const std::wstring& GetCategory() const { return m_category; }
    void SetCategory(const std::wstring& category) { m_category = category; }

    const std::wstring& GetPlatform() const { return m_platform; }
    void SetPlatform(const std::wstring& platform) { m_platform = platform; }

    const std::wstring& GetExecutablePath() const { return m_executablePath; }
    void SetExecutablePath(const std::wstring& path) { m_executablePath = path; }

    const std::wstring& GetIconPath() const { return m_iconPath; }
    void SetIconPath(const std::wstring& path) { m_iconPath = path; }

    // 添加启动参数的getter和setter
    const std::wstring& GetLaunchParams() const;
    void SetLaunchParams(const std::wstring& params);

    // 启动游戏
    bool Launch() const;

    // 序列化和反序列化
    std::wstring Serialize() const;
    static Game Deserialize(const std::wstring& data);

private:
    std::wstring m_name;           // 游戏名称
    std::wstring m_releaseYear;    // 发行年代
    std::wstring m_publisher;      // 制作发行
    std::wstring m_language;       // 语言
    std::wstring m_category;       // 游戏分类
    std::wstring m_platform;       // 平台
    std::wstring m_executablePath; // 可执行文件路径
    std::wstring m_iconPath;       // 图标路径

    // 新增启动参数成员变量
    std::wstring m_launchParams;

    // 可以添加其他属性如评分、标签等
};