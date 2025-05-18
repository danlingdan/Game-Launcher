#include "GameCollection.h"

// 使用更简短的别名
using json = nlohmann::json;

// 从其他地方导入的UTF-16<->UTF-8转换函数
extern std::string wstring_to_utf8(const std::wstring& wstr);
extern std::wstring utf8_to_wstring(const std::string& str);

// 辅助函数：去除前后空格
static std::wstring Trim(const std::wstring& str) {
    size_t first = str.find_first_not_of(L" \t\r\n");
    size_t last = str.find_last_not_of(L" \t\r\n");
    if (first == std::wstring::npos || last == std::wstring::npos) return L"";
    return str.substr(first, last - first + 1);
}

// 辅助函数：不区分大小写比较
static bool IEquals(const std::wstring& a, const std::wstring& b) {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i) {
        if (towlower(a[i]) != towlower(b[i])) return false;
    }
    return true;
}

GameCollection::GameCollection() {
    // 默认数据目录：用户文档/GameLauncherData（与设置使用相同目录）
    WCHAR path[MAX_PATH] = { 0 };
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, path))) {
        std::wstring dataDir = std::wstring(path) + L"\\GameLauncherData";
        // 确保目录存在
        SHCreateDirectoryExW(NULL, dataDir.c_str(), NULL);
        m_dataFilePath = dataDir + L"\\games.json";
    }
    else {
        // 如果无法获取Documents目录，使用程序当前目录
        m_dataFilePath = L"games.json";
    }

    OutputDebugStringW((L"游戏数据文件路径: " + m_dataFilePath + L"\n").c_str());
}

GameCollection::~GameCollection() {
    SaveGames();
}

bool GameCollection::LoadGames() {
    m_games.clear();

    // 确保目录存在
    std::wstring dirPath = m_dataFilePath.substr(0, m_dataFilePath.find_last_of(L"\\"));
    if (!dirPath.empty()) {
        SHCreateDirectoryExW(NULL, dirPath.c_str(), NULL);
    }

    OutputDebugStringW((L"正在加载游戏数据从: " + m_dataFilePath + L"\n").c_str());

    try {
        // 尝试打开并读取JSON文件
        std::ifstream file(m_dataFilePath);
        if (!file.is_open()) {
            DWORD error = GetLastError();
            OutputDebugStringW((L"打开文件失败，错误码: " + std::to_wstring(error) + L"\n").c_str());

            // 如果是文件不存在的错误，创建示例数据
            if (error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND) {
                OutputDebugStringW(L"文件不存在，创建示例游戏数据\n");
                CreateSampleGames(); // 这会调用SaveGames保存到正确位置
                return true; // 返回成功，因为已经创建了示例数据
            }
            return false;
        }

        json gamesJson;
        file >> gamesJson;
        file.close();

        // 检查是否有games数组
        if (!gamesJson.contains("games") || !gamesJson["games"].is_array()) {
            OutputDebugStringW(L"JSON格式错误：没有games数组\n");
            CreateSampleGames();
            return false;
        }

        // 读取所有游戏信息
        for (const auto& gameJson : gamesJson["games"]) {
            Game game;

            // 解析游戏属性
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

            // 添加到游戏集合
            m_games.push_back(game);
        }

        OutputDebugStringW((L"成功加载 " + std::to_wstring(m_games.size()) + L" 个游戏\n").c_str());
        return true;
    }
    catch (const std::exception& e) {
        std::string errorMsg = "加载游戏数据时发生异常: ";
        errorMsg += e.what();
        OutputDebugStringA(errorMsg.c_str());

        // 如果加载失败，创建示例数据
        CreateSampleGames();
        return false;
    }
    catch (...) {
        OutputDebugStringW(L"加载游戏数据时发生未知异常\n");

        // 如果加载失败，创建示例数据
        CreateSampleGames();
        return false;
    }
}

// 保存游戏数据到文件
bool GameCollection::SaveGames() {
    try {
        // 确保目录存在
        std::wstring directory = m_dataFilePath.substr(0, m_dataFilePath.find_last_of(L"\\"));
        if (!directory.empty()) {
            int result = SHCreateDirectoryExW(NULL, directory.c_str(), NULL);
            if (result != ERROR_SUCCESS && result != ERROR_ALREADY_EXISTS) {
                OutputDebugStringW((L"创建目录失败, 错误码: " + std::to_wstring(result) + L"\n").c_str());

                // 尝试使用当前目录作为备选
                wchar_t currentDir[MAX_PATH];
                GetCurrentDirectoryW(MAX_PATH, currentDir);
                m_dataFilePath = std::wstring(currentDir) + L"\\games.json";
                OutputDebugStringW((L"改为保存到当前目录: " + m_dataFilePath + L"\n").c_str());
            }
        }

        OutputDebugStringW((L"正在保存游戏数据到: " + m_dataFilePath + L"\n").c_str());

        // 创建JSON结构
        json gamesJson;
        json gamesArray = json::array();

        // 添加所有游戏
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

        // 保存到文件
        std::ofstream file(m_dataFilePath);
        if (!file.is_open()) {
            DWORD error = GetLastError();
            OutputDebugStringW((L"创建文件失败，错误码: " + std::to_wstring(error) + L"\n").c_str());

            // 尝试备用路径
            wchar_t tempPath[MAX_PATH];
            GetTempPathW(MAX_PATH, tempPath);
            std::wstring backupPath = std::wstring(tempPath) + L"games_backup.json";

            OutputDebugStringW((L"尝试备用路径: " + backupPath + L"\n").c_str());

            std::ofstream backupFile(backupPath);
            if (!backupFile.is_open()) {
                OutputDebugStringW(L"备用路径也失败，无法保存游戏数据\n");
                return false;
            }

            // 使用pretty print格式保存JSON
            backupFile << gamesJson.dump(4);
            backupFile.close();

            // 更新保存路径为成功的备用路径
            m_dataFilePath = backupPath;
            OutputDebugStringW((L"游戏数据保存到备用路径: " + backupPath + L"\n").c_str());
            return true;
        }

        // 使用pretty print格式保存JSON
        file << gamesJson.dump(4);
        file.close();

        OutputDebugStringW((L"游戏数据成功保存到: " + m_dataFilePath + L"\n").c_str());
        return true;
    }
    catch (const std::exception& e) {
        std::string errorMsg = "保存游戏时发生异常: ";
        errorMsg += e.what();
        OutputDebugStringA(errorMsg.c_str());
        return false;
    }
    catch (...) {
        OutputDebugStringW(L"保存游戏时发生未知异常\n");
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

// 删除游戏
bool GameCollection::RemoveGame(int index) {
    if (index < 0 || index >= static_cast<int>(m_games.size())) {
        return false;
    }

    try {
        // 从内存中删除
        m_games.erase(m_games.begin() + index);

        // 保存到文件
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

        // 转换为小写进行比较
        std::transform(gameName.begin(), gameName.end(), gameName.begin(), ::towlower);
        std::transform(searchName.begin(), searchName.end(), searchName.begin(), ::towlower);

        return gameName.find(searchName) != std::wstring::npos;
        });
}

std::vector<Game*> GameCollection::FindGamesByCategory(const std::wstring& category) const {
    std::vector<Game*> result;
    std::wstring cat = Trim(category);

    // 优化：预先计算容量以避免频繁重新分配
    result.reserve(m_games.size() / 2);

    for (const auto& game : m_games) {
        if (IEquals(Trim(game.GetCategory()), cat)) {
            result.push_back(const_cast<Game*>(&game));
        }
    }

    return result;
}

std::vector<std::wstring> GameCollection::GetAllCategories() const {
    // 使用set自动去重
    std::set<std::wstring> categories;

    // 添加现有游戏分类
    for (const auto& game : m_games) {
        std::wstring cat = Trim(game.GetCategory());
        if (!cat.empty())
            categories.insert(cat);
    }

    // 如果没有分类，添加默认分类
    if (categories.empty()) {
        const std::vector<std::wstring> defaultCategories = {
            L"角色扮演", L"即时战略", L"第一人称",
            L"动作射击", L"战棋策略", L"桌面棋牌", L"休闲分类"
        };

        categories.insert(defaultCategories.begin(), defaultCategories.end());
    }

    // 返回排序后的分类列表
    std::vector<std::wstring> result(categories.begin(), categories.end());
    std::sort(result.begin(), result.end());

    return result;
}

void GameCollection::CreateSampleGames() {
    // 创建一些示例游戏数据
    m_games.push_back(Game(L"模拟城市3000", L"1999", L"EA Games", L"中文", L"模拟经营", L"WIN98"));
    m_games.push_back(Game(L"帝国时代II", L"1999", L"Microsoft", L"中文", L"即时战略", L"WIN98"));
    m_games.push_back(Game(L"半条命", L"1998", L"Valve", L"英文", L"第一人称射击", L"WIN98"));
    m_games.push_back(Game(L"魔兽争霸III", L"2002", L"暴雪", L"中文", L"即时战略", L"WIN98"));
    m_games.push_back(Game(L"模拟人生", L"2000", L"EA Games", L"中文", L"模拟经营", L"WIN98"));
    m_games.push_back(Game(L"红色警戒2", L"2000", L"EA Games", L"中文", L"即时战略", L"WIN98"));

    // 保存示例数据
    SaveGames();
}