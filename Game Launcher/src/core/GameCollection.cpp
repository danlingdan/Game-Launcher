#include "GameCollection.h"

// ����������ȥ��ǰ��ո�
static std::wstring Trim(const std::wstring& str) {
    size_t first = str.find_first_not_of(L" \t\r\n");
    size_t last = str.find_last_not_of(L" \t\r\n");
    if (first == std::wstring::npos || last == std::wstring::npos) return L"";
    return str.substr(first, last - first + 1);
}

// ���������������ִ�Сд�Ƚ�
static bool IEquals(const std::wstring& a, const std::wstring& b) {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i) {
        if (towlower(a[i]) != towlower(b[i])) return false;
    }
    return true;
}

GameCollection::GameCollection() {
    // ���������ļ�·��
    WCHAR appDataPath[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, appDataPath))) {
        m_dataFilePath = appDataPath;
        m_dataFilePath += L"\\GameLauncher\\games.dat";
    }
    else {
        m_dataFilePath = L"games.dat";
    }
}

GameCollection::~GameCollection() {
    SaveGames();
}

bool GameCollection::LoadGames() {
    m_games.clear();

    // ʹ��Windows APIֱ�Ӷ�ȡ�ļ���ȷ����SaveGames����һ��
    HANDLE hFile = CreateFileW(
        m_dataFilePath.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        DWORD error = GetLastError();
        OutputDebugStringW((L"���ļ�ʧ�ܣ�������: " + std::to_wstring(error) + L"\n").c_str());

        // ������ļ������ڵĴ��󣬴���ʾ������
        if (error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND) {
            OutputDebugStringW(L"�ļ������ڣ�����ʾ����Ϸ����\n");
            CreateSampleGames();
        }
        return false;
    }

    // ��ȡ�ļ���С
    DWORD fileSize = GetFileSize(hFile, NULL);
    if (fileSize == INVALID_FILE_SIZE || fileSize == 0) {
        CloseHandle(hFile);
        OutputDebugStringW(L"�ļ�Ϊ�ջ��޷���ȡ�ļ���С\n");
        CreateSampleGames();
        return false;
    }

    // ���仺����
    std::vector<BYTE> buffer(fileSize);
    DWORD bytesRead;

    // ��ȡ�ļ�����
    if (!ReadFile(hFile, buffer.data(), fileSize, &bytesRead, NULL) || bytesRead == 0) {
        CloseHandle(hFile);
        OutputDebugStringW(L"��ȡ�ļ�ʧ��\n");
        return false;
    }

    CloseHandle(hFile);

    // ���BOM��ǲ�����
    size_t dataOffset = 0;
    if (fileSize >= 2 && buffer[0] == 0xFF && buffer[1] == 0xFE) {
        // ����UTF-16LE BOM
        dataOffset = 2;
    }

    // ��������ת��Ϊwchar_t��ʽ
    const wchar_t* wideBuffer = reinterpret_cast<const wchar_t*>(buffer.data() + dataOffset);
    size_t wideLength = (fileSize - dataOffset) / sizeof(wchar_t);

    // ���д���
    std::wstring line;
    for (size_t i = 0; i < wideLength; i++) {
        if (wideBuffer[i] == L'\r' && i + 1 < wideLength && wideBuffer[i + 1] == L'\n') {
            // ���������У�������
            if (!line.empty()) {
                Game game = Game::Deserialize(line);
                m_games.push_back(game);
                line.clear();
            }
            i++; // ����\n
        }
        else if (wideBuffer[i] == L'\n') {
            // ����ֻ��\n�����
            if (!line.empty()) {
                Game game = Game::Deserialize(line);
                m_games.push_back(game);
                line.clear();
            }
        }
        else {
            // �ۻ��ַ�
            line += wideBuffer[i];
        }
    }

    // �������һ�У����û�л��з���β��
    if (!line.empty()) {
        Game game = Game::Deserialize(line);
        m_games.push_back(game);
    }

    OutputDebugStringW((L"�ɹ����� " + std::to_wstring(m_games.size()) + L" ����Ϸ\n").c_str());
    return true;
}

// ������Ϸ���ݵ��ļ�
bool GameCollection::SaveGames() {
    try {
        // ȷ��Ŀ¼���� - ʹ��SHCreateDirectoryExֱ�Ӵ�������·�����
        std::wstring directory = m_dataFilePath.substr(0, m_dataFilePath.find_last_of(L"\\"));
        if (!directory.empty()) {
            int result = SHCreateDirectoryEx(NULL, directory.c_str(), NULL);
            if (result != ERROR_SUCCESS && result != ERROR_ALREADY_EXISTS) {
                OutputDebugStringW((L"����Ŀ¼ʧ��, ������: " + std::to_wstring(result) + L"\n").c_str());
                // ���Դ���Ŀ¼ʧ�ܣ����Ա��浽��ǰĿ¼
                wchar_t currentDir[MAX_PATH];
                GetCurrentDirectoryW(MAX_PATH, currentDir);
                m_dataFilePath = std::wstring(currentDir) + L"\\games.dat";
                OutputDebugStringW((L"��Ϊ���浽��ǰĿ¼: " + m_dataFilePath + L"\n").c_str());
            }
        }

        // ʹ��Windows APIֱ��д���ļ�������C++����Ǳ������
        HANDLE hFile = CreateFileW(
            m_dataFilePath.c_str(),
            GENERIC_WRITE,
            0,                     // ������
            NULL,                  // Ĭ�ϰ�ȫ����
            CREATE_ALWAYS,         // ���Ǵ������ļ�
            FILE_ATTRIBUTE_NORMAL,
            NULL
        );

        if (hFile == INVALID_HANDLE_VALUE) {
            DWORD error = GetLastError();
            OutputDebugStringW((L"�����ļ�ʧ�ܣ�������: " + std::to_wstring(error) + L"\n").c_str());

            // ���Ա���·��
            wchar_t tempPath[MAX_PATH];
            GetTempPathW(MAX_PATH, tempPath);
            std::wstring backupPath = std::wstring(tempPath) + L"games_backup.dat";

            OutputDebugStringW((L"���Ա���·��: " + backupPath + L"\n").c_str());

            hFile = CreateFileW(
                backupPath.c_str(),
                GENERIC_WRITE,
                0,
                NULL,
                CREATE_ALWAYS,
                FILE_ATTRIBUTE_NORMAL,
                NULL
            );

            if (hFile == INVALID_HANDLE_VALUE) {
                OutputDebugStringW(L"����·��Ҳʧ�ܣ��޷�������Ϸ����\n");
                return false;
            }

            // ���±���·��Ϊ�ɹ��ı���·��
            m_dataFilePath = backupPath;
        }

        // д��UTF-16LE BOM���
        const BYTE BOM[] = { 0xFF, 0xFE };
        DWORD bytesWritten = 0;
        WriteFile(hFile, BOM, sizeof(BOM), &bytesWritten, NULL);

        // ���д����Ϸ����
        for (const auto& game : m_games) {
            std::wstring line = game.Serialize() + L"\r\n";
            WriteFile(
                hFile,
                line.c_str(),
                static_cast<DWORD>(line.length() * sizeof(wchar_t)),
                &bytesWritten,
                NULL
            );

            if (bytesWritten != line.length() * sizeof(wchar_t)) {
                OutputDebugStringW(L"д���ļ����ݲ�����\n");
                CloseHandle(hFile);
                return false;
            }
        }

        // �ر��ļ�
        CloseHandle(hFile);
        OutputDebugStringW((L"��Ϸ���ݳɹ����浽: " + m_dataFilePath + L"\n").c_str());
        return true;
    }
    catch (const std::exception& e) {
        std::string errorMsg = "������Ϸʱ�����쳣: ";
        errorMsg += e.what();
        OutputDebugStringA(errorMsg.c_str());
        return false;
    }
    catch (...) {
        OutputDebugStringW(L"������Ϸʱ����δ֪�쳣\n");
        return false;
    }
}

bool GameCollection::AddGame(const Game& game) {
    m_games.push_back(game);
    return SaveGames();
}

Game* GameCollection::GetGame(size_t index) {
    if (index >= m_games.size()) {
        return nullptr;
    }
    return &m_games[index];
}

// ɾ����Ϸ
bool GameCollection::RemoveGame(int index) {
    if (index < 0 || index >= static_cast<int>(m_games.size())) {
        return false;
    }

    try {
        // ���ڴ���ɾ��
        m_games.erase(m_games.begin() + index);

        // ���浽�ļ�
        return SaveGames();
    }
    catch (const std::exception&) {
        return false;
    }
}

std::vector<Game*> GameCollection::FindGames(const std::function<bool(const Game&)>& predicate) {
    std::vector<Game*> result;
    for (auto& game : m_games) {
        if (predicate(game)) {
            result.push_back(&game);
        }
    }
    return result;
}

std::vector<Game*> GameCollection::FindGamesByName(const std::wstring& name) {
    return FindGames([&name](const Game& game) {
        std::wstring gameName = game.GetName();
        std::wstring searchName = name;

        // ת��ΪСд���бȽ�
        std::transform(gameName.begin(), gameName.end(), gameName.begin(), ::towlower);
        std::transform(searchName.begin(), searchName.end(), searchName.begin(), ::towlower);

        return gameName.find(searchName) != std::wstring::npos;
        });
}

std::vector<Game*> GameCollection::FindGamesByCategory(const std::wstring& category) const {
    std::vector<Game*> result;
    std::wstring cat = Trim(category);

    // �Ż���Ԥ�ȼ��������Ա���Ƶ�����·���
    result.reserve(m_games.size() / 2);

    for (const auto& game : m_games) {
        if (IEquals(Trim(game.GetCategory()), cat)) {
            result.push_back(const_cast<Game*>(&game));
        }
    }

    return result;
}

std::vector<std::wstring> GameCollection::GetAllCategories() const {
    // ʹ��set�Զ�ȥ��
    std::set<std::wstring> categories;

    // ���������Ϸ����
    for (const auto& game : m_games) {
        std::wstring cat = Trim(game.GetCategory());
        if (!cat.empty())
            categories.insert(cat);
    }

    // ���û�з��࣬���Ĭ�Ϸ���
    if (categories.empty()) {
        const std::vector<std::wstring> defaultCategories = {
            L"��ɫ����", L"��ʱս��", L"��һ�˳�",
            L"�������", L"ս�����", L"��������", L"���з���"
        };

        categories.insert(defaultCategories.begin(), defaultCategories.end());
    }

    // ���������ķ����б�
    std::vector<std::wstring> result(categories.begin(), categories.end());
    std::sort(result.begin(), result.end());

    return result;
}

void GameCollection::CreateSampleGames() {
    // ����һЩʾ����Ϸ����
    m_games.push_back(Game(L"ģ�����3000", L"1999", L"EA Games", L"����", L"ģ�⾭Ӫ", L"WIN98"));
    m_games.push_back(Game(L"�۹�ʱ��II", L"1999", L"Microsoft", L"����", L"��ʱս��", L"WIN98"));
    m_games.push_back(Game(L"������", L"1998", L"Valve", L"Ӣ��", L"��һ�˳����", L"WIN98"));
    m_games.push_back(Game(L"ħ������III", L"2002", L"��ѩ", L"����", L"��ʱս��", L"WIN98"));
    m_games.push_back(Game(L"ģ������", L"2000", L"EA Games", L"����", L"ģ�⾭Ӫ", L"WIN98"));
    m_games.push_back(Game(L"��ɫ����2", L"2000", L"EA Games", L"����", L"��ʱս��", L"WIN98"));

    // ����ʾ������
    SaveGames();
}