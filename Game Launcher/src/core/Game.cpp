#include "Game.h"

Game::Game() :
    m_name(L""),
    m_releaseYear(L""),
    m_publisher(L""),
    m_language(L""),
    m_category(L""),
    m_platform(L"WIN98"),
    m_executablePath(L""),
    m_iconPath(L"") {
}

Game::Game(const std::wstring& name, const std::wstring& releaseYear,
    const std::wstring& publisher, const std::wstring& language,
    const std::wstring& category, const std::wstring& platform) :
    m_name(name),
    m_releaseYear(releaseYear),
    m_publisher(publisher),
    m_language(language),
    m_category(category),
    m_platform(platform),
    m_executablePath(L""),
    m_iconPath(L"") {
}

Game::~Game() {
}

bool Game::Launch() const {
    // 检查可执行文件路径是否为空
    if (m_executablePath.empty()) {
        MessageBoxW(NULL, L"未设置可执行文件路径，无法启动游戏。", L"启动失败", MB_OK | MB_ICONERROR);
        return false;
    }

    // 检查文件是否存在
    if (GetFileAttributesW(m_executablePath.c_str()) == INVALID_FILE_ATTRIBUTES) {
        MessageBoxW(NULL, (L"找不到可执行文件:\n" + m_executablePath).c_str(), L"启动失败", MB_OK | MB_ICONERROR);
        return false;
    }

    // 使用ShellExecute启动游戏
    HINSTANCE result = ShellExecuteW(
        NULL,           // 父窗口句柄
        L"open",        // 操作
        m_executablePath.c_str(), // 文件路径
        NULL,           // 参数
        NULL,           // 工作目录
        SW_SHOWNORMAL   // 显示窗口
    );

    // ShellExecute返回值小于等于32表示失败
    if ((INT_PTR)result <= 32) {
        MessageBoxW(NULL, L"启动游戏失败，请检查文件路径和权限。", L"启动失败", MB_OK | MB_ICONERROR);
        return false;
    }

    return true;
}

// 将游戏数据序列化为字符串
std::wstring Game::Serialize() const {
    // 使用直接字符串连接代替stringstream，性能更高
    return m_name + L"|" +
        m_releaseYear + L"|" +
        m_publisher + L"|" +
        m_language + L"|" +
        m_category + L"|" +
        m_platform + L"|" +
        m_executablePath + L"|" +
        m_iconPath;
}

// 从字符串反序列化为游戏数据
Game Game::Deserialize(const std::wstring& data) {
    Game game;
    if (data.empty()) {
        return game; // 返回默认游戏对象
    }

    std::vector<std::wstring> parts;
    parts.reserve(8); // 预分配空间，避免频繁重分配

    size_t start = 0;
    size_t end = 0;

    // 手动分割字符串，比wstringstream更高效
    while ((end = data.find(L'|', start)) != std::wstring::npos) {
        parts.push_back(data.substr(start, end - start));
        start = end + 1;
    }

    // 添加最后一部分
    if (start < data.length()) {
        parts.push_back(data.substr(start));
    }

    // 安全地按部分数量依次赋值
    if (parts.size() >= 1) game.m_name = parts[0];
    if (parts.size() >= 2) game.m_releaseYear = parts[1];
    if (parts.size() >= 3) game.m_publisher = parts[2];
    if (parts.size() >= 4) game.m_language = parts[3];
    if (parts.size() >= 5) game.m_category = parts[4];
    if (parts.size() >= 6) game.m_platform = parts[5];
    if (parts.size() >= 7) game.m_executablePath = parts[6];
    if (parts.size() >= 8) game.m_iconPath = parts[7];

    return game;
}