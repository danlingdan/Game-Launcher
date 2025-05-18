#include "GameCollection.h"

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
    // 设置数据文件路径
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

    // 使用Windows API直接读取文件，确保与SaveGames保持一致
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
        OutputDebugStringW((L"打开文件失败，错误码: " + std::to_wstring(error) + L"\n").c_str());

        // 如果是文件不存在的错误，创建示例数据
        if (error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND) {
            OutputDebugStringW(L"文件不存在，创建示例游戏数据\n");
            CreateSampleGames();
        }
        return false;
    }

    // 获取文件大小
    DWORD fileSize = GetFileSize(hFile, NULL);
    if (fileSize == INVALID_FILE_SIZE || fileSize == 0) {
        CloseHandle(hFile);
        OutputDebugStringW(L"文件为空或无法获取文件大小\n");
        CreateSampleGames();
        return false;
    }

    // 分配缓冲区
    std::vector<BYTE> buffer(fileSize);
    DWORD bytesRead;

    // 读取文件内容
    if (!ReadFile(hFile, buffer.data(), fileSize, &bytesRead, NULL) || bytesRead == 0) {
        CloseHandle(hFile);
        OutputDebugStringW(L"读取文件失败\n");
        return false;
    }

    CloseHandle(hFile);

    // 检查BOM标记并跳过
    size_t dataOffset = 0;
    if (fileSize >= 2 && buffer[0] == 0xFF && buffer[1] == 0xFE) {
        // 跳过UTF-16LE BOM
        dataOffset = 2;
    }

    // 将缓冲区转换为wchar_t格式
    const wchar_t* wideBuffer = reinterpret_cast<const wchar_t*>(buffer.data() + dataOffset);
    size_t wideLength = (fileSize - dataOffset) / sizeof(wchar_t);

    // 按行处理
    std::wstring line;
    for (size_t i = 0; i < wideLength; i++) {
        if (wideBuffer[i] == L'\r' && i + 1 < wideLength && wideBuffer[i + 1] == L'\n') {
            // 发现完整行，处理它
            if (!line.empty()) {
                Game game = Game::Deserialize(line);
                m_games.push_back(game);
                line.clear();
            }
            i++; // 跳过\n
        }
        else if (wideBuffer[i] == L'\n') {
            // 处理只有\n的情况
            if (!line.empty()) {
                Game game = Game::Deserialize(line);
                m_games.push_back(game);
                line.clear();
            }
        }
        else {
            // 累积字符
            line += wideBuffer[i];
        }
    }

    // 处理最后一行（如果没有换行符结尾）
    if (!line.empty()) {
        Game game = Game::Deserialize(line);
        m_games.push_back(game);
    }

    OutputDebugStringW((L"成功加载 " + std::to_wstring(m_games.size()) + L" 个游戏\n").c_str());
    return true;
}

// 保存游戏数据到文件
bool GameCollection::SaveGames() {
    try {
        // 确保目录存在 - 使用SHCreateDirectoryEx直接创建完整路径层次
        std::wstring directory = m_dataFilePath.substr(0, m_dataFilePath.find_last_of(L"\\"));
        if (!directory.empty()) {
            int result = SHCreateDirectoryEx(NULL, directory.c_str(), NULL);
            if (result != ERROR_SUCCESS && result != ERROR_ALREADY_EXISTS) {
                OutputDebugStringW((L"创建目录失败, 错误码: " + std::to_wstring(result) + L"\n").c_str());
                // 尝试创建目录失败，尝试保存到当前目录
                wchar_t currentDir[MAX_PATH];
                GetCurrentDirectoryW(MAX_PATH, currentDir);
                m_dataFilePath = std::wstring(currentDir) + L"\\games.dat";
                OutputDebugStringW((L"改为保存到当前目录: " + m_dataFilePath + L"\n").c_str());
            }
        }

        // 使用Windows API直接写入文件，避免C++流的潜在问题
        HANDLE hFile = CreateFileW(
            m_dataFilePath.c_str(),
            GENERIC_WRITE,
            0,                     // 不共享
            NULL,                  // 默认安全属性
            CREATE_ALWAYS,         // 总是创建新文件
            FILE_ATTRIBUTE_NORMAL,
            NULL
        );

        if (hFile == INVALID_HANDLE_VALUE) {
            DWORD error = GetLastError();
            OutputDebugStringW((L"创建文件失败，错误码: " + std::to_wstring(error) + L"\n").c_str());

            // 尝试备用路径
            wchar_t tempPath[MAX_PATH];
            GetTempPathW(MAX_PATH, tempPath);
            std::wstring backupPath = std::wstring(tempPath) + L"games_backup.dat";

            OutputDebugStringW((L"尝试备用路径: " + backupPath + L"\n").c_str());

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
                OutputDebugStringW(L"备用路径也失败，无法保存游戏数据\n");
                return false;
            }

            // 更新保存路径为成功的备用路径
            m_dataFilePath = backupPath;
        }

        // 写入UTF-16LE BOM标记
        const BYTE BOM[] = { 0xFF, 0xFE };
        DWORD bytesWritten = 0;
        WriteFile(hFile, BOM, sizeof(BOM), &bytesWritten, NULL);

        // 逐个写入游戏数据
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
                OutputDebugStringW(L"写入文件数据不完整\n");
                CloseHandle(hFile);
                return false;
            }
        }

        // 关闭文件
        CloseHandle(hFile);
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