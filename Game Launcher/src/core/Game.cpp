#include "Game.h"

//===============================================================
// 构造函数和析构函数
//===============================================================

Game::Game() :
    m_name(L""),
    m_releaseYear(L""),
    m_publisher(L""),
    m_language(L""),
    m_category(L""),
    m_platform(L"WIN98"),
    m_executablePath(L""),
    m_iconPath(L""),
    m_launchParams(L"") {
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
    m_iconPath(L""),
    m_launchParams(L"") {
}

Game::~Game() {
    // 析构函数保持为空，不需要特殊清理
}

//===============================================================
// Getter和Setter方法
//===============================================================

const std::wstring& Game::GetLaunchParams() const {
    return m_launchParams;
}

void Game::SetLaunchParams(const std::wstring& params) {
    m_launchParams = params;
}

//===============================================================
// 游戏操作方法
//===============================================================

bool Game::Launch() const {
    // 如果可执行文件路径为空，无法启动
    if (m_executablePath.empty()) {
        return false;
    }

    // 构建命令行（包含可选的启动参数）
    std::wstring commandLine;
    if (!m_launchParams.empty()) {
        commandLine = L"\"" + m_executablePath + L"\" " + m_launchParams;
    }
    else {
        commandLine = L"\"" + m_executablePath + L"\"";
    }

    // 准备进程启动信息
    STARTUPINFO si = { sizeof(STARTUPINFO) };
    PROCESS_INFORMATION pi = { 0 };

    // 创建进程启动游戏
    BOOL result = CreateProcess(
        NULL,                       // 应用程序名（使用命令行）
        (LPWSTR)commandLine.c_str(),// 命令行（包含参数）
        NULL,                       // 进程安全属性
        NULL,                       // 线程安全属性
        FALSE,                      // 句柄继承标志
        0,                          // 创建标志
        NULL,                       // 环境块
        NULL,                       // 当前目录
        &si,                        // STARTUPINFO
        &pi                         // PROCESS_INFORMATION
    );

    // 如果创建成功，关闭句柄
    if (result) {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return true;
    }

    return false;
}

//===============================================================
// 序列化和反序列化
//===============================================================

// 序列化
std::wstring Game::Serialize() const {
    // 使用JSON格式序列化游戏对象
    nlohmann::json j;
    j["name"] = wstring_to_utf8(m_name);
    j["releaseYear"] = wstring_to_utf8(m_releaseYear);
    j["publisher"] = wstring_to_utf8(m_publisher);
    j["language"] = wstring_to_utf8(m_language);
    j["category"] = wstring_to_utf8(m_category);
    j["platform"] = wstring_to_utf8(m_platform);
    j["executablePath"] = wstring_to_utf8(m_executablePath);
    j["iconPath"] = wstring_to_utf8(m_iconPath);
    j["launchParams"] = wstring_to_utf8(m_launchParams);

    return utf8_to_wstring(j.dump(4)); // 使用4个空格缩进美化输出
}

// 反序列化
Game Game::Deserialize(const std::wstring& data) {
    Game game;
    try {
        nlohmann::json j = nlohmann::json::parse(wstring_to_utf8(data));

        // 必填项
        game.m_name = utf8_to_wstring(j["name"].get<std::string>());

        // 可选项，使用异常安全的获取方式
        game.m_releaseYear = j.contains("releaseYear") ?
            utf8_to_wstring(j["releaseYear"].get<std::string>()) : L"";
        game.m_publisher = j.contains("publisher") ?
            utf8_to_wstring(j["publisher"].get<std::string>()) : L"";
        game.m_language = j.contains("language") ?
            utf8_to_wstring(j["language"].get<std::string>()) : L"";
        game.m_category = j.contains("category") ?
            utf8_to_wstring(j["category"].get<std::string>()) : L"";
        game.m_platform = j.contains("platform") ?
            utf8_to_wstring(j["platform"].get<std::string>()) : L"WIN98";
        game.m_executablePath = j.contains("executablePath") ?
            utf8_to_wstring(j["executablePath"].get<std::string>()) : L"";
        game.m_iconPath = j.contains("iconPath") ?
            utf8_to_wstring(j["iconPath"].get<std::string>()) : L"";
        game.m_launchParams = j.contains("launchParams") ?
            utf8_to_wstring(j["launchParams"].get<std::string>()) : L"";
    }
    catch (const std::exception& e) {
        // 记录反序列化错误信息
        OutputDebugStringA("游戏反序列化失败: ");
        OutputDebugStringA(e.what());
        OutputDebugStringA("\n");

        // 返回默认的空游戏对象
    }

    return game;
}