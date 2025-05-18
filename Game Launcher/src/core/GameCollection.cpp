#include "GameCollection.h"

// ʹ�ø���̵ı���
using json = nlohmann::json;

// �������ط������UTF-16<->UTF-8ת������
extern std::string wstring_to_utf8(const std::wstring& wstr);
extern std::wstring utf8_to_wstring(const std::string& str);

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
    // Ĭ������Ŀ¼���û��ĵ�/GameLauncherData��������ʹ����ͬĿ¼��
    WCHAR path[MAX_PATH] = { 0 };
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, path))) {
        std::wstring dataDir = std::wstring(path) + L"\\GameLauncherData";
        // ȷ��Ŀ¼����
        SHCreateDirectoryExW(NULL, dataDir.c_str(), NULL);
        m_dataFilePath = dataDir + L"\\games.json";
    }
    else {
        // ����޷���ȡDocumentsĿ¼��ʹ�ó���ǰĿ¼
        m_dataFilePath = L"games.json";
    }

    OutputDebugStringW((L"��Ϸ�����ļ�·��: " + m_dataFilePath + L"\n").c_str());
}

GameCollection::~GameCollection() {
    SaveGames();
}

bool GameCollection::LoadGames() {
    m_games.clear();

    // ȷ��Ŀ¼����
    std::wstring dirPath = m_dataFilePath.substr(0, m_dataFilePath.find_last_of(L"\\"));
    if (!dirPath.empty()) {
        SHCreateDirectoryExW(NULL, dirPath.c_str(), NULL);
    }

    OutputDebugStringW((L"���ڼ�����Ϸ���ݴ�: " + m_dataFilePath + L"\n").c_str());

    try {
        // ���Դ򿪲���ȡJSON�ļ�
        std::ifstream file(m_dataFilePath);
        if (!file.is_open()) {
            DWORD error = GetLastError();
            OutputDebugStringW((L"���ļ�ʧ�ܣ�������: " + std::to_wstring(error) + L"\n").c_str());

            // ������ļ������ڵĴ��󣬴���ʾ������
            if (error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND) {
                OutputDebugStringW(L"�ļ������ڣ�����ʾ����Ϸ����\n");
                CreateSampleGames(); // ������SaveGames���浽��ȷλ��
                return true; // ���سɹ�����Ϊ�Ѿ�������ʾ������
            }
            return false;
        }

        json gamesJson;
        file >> gamesJson;
        file.close();

        // ����Ƿ���games����
        if (!gamesJson.contains("games") || !gamesJson["games"].is_array()) {
            OutputDebugStringW(L"JSON��ʽ����û��games����\n");
            CreateSampleGames();
            return false;
        }

        // ��ȡ������Ϸ��Ϣ
        for (const auto& gameJson : gamesJson["games"]) {
            Game game;

            // ������Ϸ����
            if (gameJson.contains("name")) {
                game.SetName(utf8_to_wstring(gameJson["name"]));
            }
            if (gameJson.contains("releaseYear")) {
                game.SetReleaseYear(utf8_to_wstring(gameJson["releaseYear"]));
            }
            if (gameJson.contains("publisher")) {
                game.SetPublisher(utf8_to_wstring(gameJson["publisher"]));
            }
            if (gameJson.contains("language")) {
                game.SetLanguage(utf8_to_wstring(gameJson["language"]));
            }
            if (gameJson.contains("category")) {
                game.SetCategory(utf8_to_wstring(gameJson["category"]));
            }
            if (gameJson.contains("platform")) {
                game.SetPlatform(utf8_to_wstring(gameJson["platform"]));
            }
            if (gameJson.contains("executablePath")) {
                game.SetExecutablePath(utf8_to_wstring(gameJson["executablePath"]));
            }
            if (gameJson.contains("iconPath")) {
                game.SetIconPath(utf8_to_wstring(gameJson["iconPath"]));
            }

            // ��ӵ���Ϸ����
            m_games.push_back(game);
        }

        OutputDebugStringW((L"�ɹ����� " + std::to_wstring(m_games.size()) + L" ����Ϸ\n").c_str());
        return true;
    }
    catch (const std::exception& e) {
        std::string errorMsg = "������Ϸ����ʱ�����쳣: ";
        errorMsg += e.what();
        OutputDebugStringA(errorMsg.c_str());

        // �������ʧ�ܣ�����ʾ������
        CreateSampleGames();
        return false;
    }
    catch (...) {
        OutputDebugStringW(L"������Ϸ����ʱ����δ֪�쳣\n");

        // �������ʧ�ܣ�����ʾ������
        CreateSampleGames();
        return false;
    }
}

// ������Ϸ���ݵ��ļ�
bool GameCollection::SaveGames() {
    try {
        // ȷ��Ŀ¼����
        std::wstring directory = m_dataFilePath.substr(0, m_dataFilePath.find_last_of(L"\\"));
        if (!directory.empty()) {
            int result = SHCreateDirectoryExW(NULL, directory.c_str(), NULL);
            if (result != ERROR_SUCCESS && result != ERROR_ALREADY_EXISTS) {
                OutputDebugStringW((L"����Ŀ¼ʧ��, ������: " + std::to_wstring(result) + L"\n").c_str());

                // ����ʹ�õ�ǰĿ¼��Ϊ��ѡ
                wchar_t currentDir[MAX_PATH];
                GetCurrentDirectoryW(MAX_PATH, currentDir);
                m_dataFilePath = std::wstring(currentDir) + L"\\games.json";
                OutputDebugStringW((L"��Ϊ���浽��ǰĿ¼: " + m_dataFilePath + L"\n").c_str());
            }
        }

        OutputDebugStringW((L"���ڱ�����Ϸ���ݵ�: " + m_dataFilePath + L"\n").c_str());

        // ����JSON�ṹ
        json gamesJson;
        json gamesArray = json::array();

        // ���������Ϸ
        for (const auto& game : m_games) {
            json gameJson;
            gameJson["name"] = wstring_to_utf8(game.GetName());
            gameJson["releaseYear"] = wstring_to_utf8(game.GetReleaseYear());
            gameJson["publisher"] = wstring_to_utf8(game.GetPublisher());
            gameJson["language"] = wstring_to_utf8(game.GetLanguage());
            gameJson["category"] = wstring_to_utf8(game.GetCategory());
            gameJson["platform"] = wstring_to_utf8(game.GetPlatform());
            gameJson["executablePath"] = wstring_to_utf8(game.GetExecutablePath());
            gameJson["iconPath"] = wstring_to_utf8(game.GetIconPath());

            gamesArray.push_back(gameJson);
        }

        gamesJson["games"] = gamesArray;

        // ���浽�ļ�
        std::ofstream file(m_dataFilePath);
        if (!file.is_open()) {
            DWORD error = GetLastError();
            OutputDebugStringW((L"�����ļ�ʧ�ܣ�������: " + std::to_wstring(error) + L"\n").c_str());

            // ���Ա���·��
            wchar_t tempPath[MAX_PATH];
            GetTempPathW(MAX_PATH, tempPath);
            std::wstring backupPath = std::wstring(tempPath) + L"games_backup.json";

            OutputDebugStringW((L"���Ա���·��: " + backupPath + L"\n").c_str());

            std::ofstream backupFile(backupPath);
            if (!backupFile.is_open()) {
                OutputDebugStringW(L"����·��Ҳʧ�ܣ��޷�������Ϸ����\n");
                return false;
            }

            // ʹ��pretty print��ʽ����JSON
            backupFile << gamesJson.dump(4);
            backupFile.close();

            // ���±���·��Ϊ�ɹ��ı���·��
            m_dataFilePath = backupPath;
            OutputDebugStringW((L"��Ϸ���ݱ��浽����·��: " + backupPath + L"\n").c_str());
            return true;
        }

        // ʹ��pretty print��ʽ����JSON
        file << gamesJson.dump(4);
        file.close();

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