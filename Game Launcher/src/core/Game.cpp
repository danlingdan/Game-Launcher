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
    // ����ִ���ļ�·���Ƿ�Ϊ��
    if (m_executablePath.empty()) {
        MessageBoxW(NULL, L"δ���ÿ�ִ���ļ�·�����޷�������Ϸ��", L"����ʧ��", MB_OK | MB_ICONERROR);
        return false;
    }

    // ����ļ��Ƿ����
    if (GetFileAttributesW(m_executablePath.c_str()) == INVALID_FILE_ATTRIBUTES) {
        MessageBoxW(NULL, (L"�Ҳ�����ִ���ļ�:\n" + m_executablePath).c_str(), L"����ʧ��", MB_OK | MB_ICONERROR);
        return false;
    }

    // ʹ��ShellExecute������Ϸ
    HINSTANCE result = ShellExecuteW(
        NULL,           // �����ھ��
        L"open",        // ����
        m_executablePath.c_str(), // �ļ�·��
        NULL,           // ����
        NULL,           // ����Ŀ¼
        SW_SHOWNORMAL   // ��ʾ����
    );

    // ShellExecute����ֵС�ڵ���32��ʾʧ��
    if ((INT_PTR)result <= 32) {
        MessageBoxW(NULL, L"������Ϸʧ�ܣ������ļ�·����Ȩ�ޡ�", L"����ʧ��", MB_OK | MB_ICONERROR);
        return false;
    }

    return true;
}

// ����Ϸ�������л�Ϊ�ַ���
std::wstring Game::Serialize() const {
    // ʹ��ֱ���ַ������Ӵ���stringstream�����ܸ���
    return m_name + L"|" +
        m_releaseYear + L"|" +
        m_publisher + L"|" +
        m_language + L"|" +
        m_category + L"|" +
        m_platform + L"|" +
        m_executablePath + L"|" +
        m_iconPath;
}

// ���ַ��������л�Ϊ��Ϸ����
Game Game::Deserialize(const std::wstring& data) {
    Game game;
    if (data.empty()) {
        return game; // ����Ĭ����Ϸ����
    }

    std::vector<std::wstring> parts;
    parts.reserve(8); // Ԥ����ռ䣬����Ƶ���ط���

    size_t start = 0;
    size_t end = 0;

    // �ֶ��ָ��ַ�������wstringstream����Ч
    while ((end = data.find(L'|', start)) != std::wstring::npos) {
        parts.push_back(data.substr(start, end - start));
        start = end + 1;
    }

    // ������һ����
    if (start < data.length()) {
        parts.push_back(data.substr(start));
    }

    // ��ȫ�ذ������������θ�ֵ
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