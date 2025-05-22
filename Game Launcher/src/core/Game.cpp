#include "Game.h"

//===============================================================
// ���캯������������
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
    // ������������Ϊ�գ�����Ҫ��������
}

//===============================================================
// Getter��Setter����
//===============================================================

const std::wstring& Game::GetLaunchParams() const {
    return m_launchParams;
}

void Game::SetLaunchParams(const std::wstring& params) {
    m_launchParams = params;
}

//===============================================================
// ��Ϸ��������
//===============================================================

bool Game::Launch() const {
    // �����ִ���ļ�·��Ϊ�գ��޷�����
    if (m_executablePath.empty()) {
        return false;
    }

    // ���������У�������ѡ������������
    std::wstring commandLine;
    if (!m_launchParams.empty()) {
        commandLine = L"\"" + m_executablePath + L"\" " + m_launchParams;
    }
    else {
        commandLine = L"\"" + m_executablePath + L"\"";
    }

    // ׼������������Ϣ
    STARTUPINFO si = { sizeof(STARTUPINFO) };
    PROCESS_INFORMATION pi = { 0 };

    // ��������������Ϸ
    BOOL result = CreateProcess(
        NULL,                       // Ӧ�ó�������ʹ�������У�
        (LPWSTR)commandLine.c_str(),// �����У�����������
        NULL,                       // ���̰�ȫ����
        NULL,                       // �̰߳�ȫ����
        FALSE,                      // ����̳б�־
        0,                          // ������־
        NULL,                       // ������
        NULL,                       // ��ǰĿ¼
        &si,                        // STARTUPINFO
        &pi                         // PROCESS_INFORMATION
    );

    // ��������ɹ����رվ��
    if (result) {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return true;
    }

    return false;
}

//===============================================================
// ���л��ͷ����л�
//===============================================================

// ���л�
std::wstring Game::Serialize() const {
    // ʹ��JSON��ʽ���л���Ϸ����
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

    return utf8_to_wstring(j.dump(4)); // ʹ��4���ո������������
}

// �����л�
Game Game::Deserialize(const std::wstring& data) {
    Game game;
    try {
        nlohmann::json j = nlohmann::json::parse(wstring_to_utf8(data));

        // ������
        game.m_name = utf8_to_wstring(j["name"].get<std::string>());

        // ��ѡ�ʹ���쳣��ȫ�Ļ�ȡ��ʽ
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
        // ��¼�����л�������Ϣ
        OutputDebugStringA("��Ϸ�����л�ʧ��: ");
        OutputDebugStringA(e.what());
        OutputDebugStringA("\n");

        // ����Ĭ�ϵĿ���Ϸ����
    }

    return game;
}