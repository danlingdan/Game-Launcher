#include "setting.h"

// 使用更简短的别名
using json = nlohmann::json;

Setting::Setting()
    : m_theme(AppTheme::Light)
{
    // 默认数据目录：用户文档/GameLauncherData
    WCHAR path[MAX_PATH] = { 0 };
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, path))) {
        m_dataDirectory = std::wstring(path) + L"\\GameLauncherData";
    }
    else {
        m_dataDirectory = L"C:\\GameLauncherData";
    }
}

void Setting::SetTheme(AppTheme theme) {
    m_theme = theme;
}

AppTheme Setting::GetTheme() const {
    return m_theme;
}

void Setting::SetDataDirectory(const std::wstring& dir) {
    m_dataDirectory = dir;
}

std::wstring Setting::GetDataDirectory() const {
    return m_dataDirectory;
}

bool Setting::Save() const {
    try {
        // 准备JSON对象
        json settingsJson;

        // 添加设置信息
        settingsJson["theme"] = (m_theme == AppTheme::Dark ? "Dark" : "Light");
        settingsJson["dataDirectory"] = wstring_to_utf8(m_dataDirectory);

        // 准备保存路径
        std::wstring fullPath;
        bool pathFound = false;

        // 1. 首先尝试使用用户设置的数据目录
        if (!m_dataDirectory.empty()) {
            // 确保目录存在
            if (SHCreateDirectoryEx(NULL, m_dataDirectory.c_str(), NULL) == ERROR_SUCCESS ||
                GetLastError() == ERROR_ALREADY_EXISTS) {
                fullPath = m_dataDirectory + L"\\settings.json";
                pathFound = true;
                OutputDebugStringW((L"尝试将设置保存到用户选择的数据目录: " + fullPath + L"\n").c_str());
            }
        }

        // 2. 如果用户未选择或目录为空或创建失败，尝试使用AppData目录
        if (!pathFound) {
            WCHAR appDataPath[MAX_PATH] = { 0 };
            if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, appDataPath))) {
                std::wstring appDataDir = std::wstring(appDataPath) + L"\\GameLauncher";

                // 确保目录存在
                if (SHCreateDirectoryEx(NULL, appDataDir.c_str(), NULL) == ERROR_SUCCESS ||
                    GetLastError() == ERROR_ALREADY_EXISTS) {
                    fullPath = appDataDir + L"\\settings.json";
                    pathFound = true;
                    OutputDebugStringW((L"尝试将设置保存到AppData目录: " + fullPath + L"\n").c_str());
                }
            }
        }

        // 3. 最后尝试使用程序目录
        if (!pathFound) {
            WCHAR exePath[MAX_PATH] = { 0 };
            GetModuleFileNameW(NULL, exePath, MAX_PATH);

            // 提取目录部分
            std::wstring directory = exePath;
            size_t pos = directory.find_last_of(L"\\");
            if (pos != std::wstring::npos) {
                directory = directory.substr(0, pos + 1); // 包含尾部斜杠
                fullPath = directory + L"settings.json";
                pathFound = true;
                OutputDebugStringW((L"尝试将设置保存到程序目录: " + fullPath + L"\n").c_str());
            }
        }

        // 4. 如果所有尝试都失败，尝试当前目录
        if (!pathFound) {
            wchar_t currentDir[MAX_PATH];
            GetCurrentDirectoryW(MAX_PATH, currentDir);
            fullPath = std::wstring(currentDir) + L"\\settings.json";
            OutputDebugStringW((L"尝试备用路径: " + fullPath + L"\n").c_str());
        }

        // 保存JSON到文件
        std::ofstream file(fullPath, std::ios::out);
        if (!file) {
            OutputDebugStringW(L"无法创建设置文件\n");
            return false;
        }

        // 使用pretty print格式将JSON保存到文件
        file << settingsJson.dump(4);
        file.close();

        OutputDebugStringW((L"设置成功保存到: " + fullPath + L"\n").c_str());
        return true;
    }
    catch (const std::exception& e) {
        std::string errorMsg = "保存设置时发生异常: ";
        errorMsg += e.what();
        OutputDebugStringA(errorMsg.c_str());
        return false;
    }
    catch (...) {
        OutputDebugStringW(L"保存设置时发生未知异常\n");
        return false;
    }
}

bool Setting::Load() {
    // 构建所有可能的设置文件路径
    std::vector<std::wstring> possiblePaths;

    // 添加当前数据目录路径
    if (!m_dataDirectory.empty()) {
        possiblePaths.push_back(m_dataDirectory + L"\\settings.json");
    }

    // 添加AppData目录路径
    WCHAR appDataPath[MAX_PATH] = { 0 };
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, appDataPath))) {
        std::wstring appDataDir = std::wstring(appDataPath) + L"\\GameLauncher";
        possiblePaths.push_back(appDataDir + L"\\settings.json");
    }

    // 添加程序目录路径
    WCHAR exePath[MAX_PATH] = { 0 };
    GetModuleFileNameW(NULL, exePath, MAX_PATH);
    std::wstring directory = exePath;
    size_t pos = directory.find_last_of(L"\\");
    if (pos != std::wstring::npos) {
        directory = directory.substr(0, pos + 1);
        possiblePaths.push_back(directory + L"settings.json");
    }

    // 添加当前目录路径
    wchar_t currentDir[MAX_PATH];
    GetCurrentDirectoryW(MAX_PATH, currentDir);
    possiblePaths.push_back(std::wstring(currentDir) + L"\\settings.json");

    // 依次尝试所有路径
    for (const auto& path : possiblePaths) {
        OutputDebugStringW((L"尝试从 " + path + L" 加载设置\n").c_str());

        try {
            // 尝试打开并读取JSON文件
            std::ifstream file(path);
            if (!file.is_open()) continue;

            json settingsJson;
            file >> settingsJson;
            file.close();

            bool themeFound = false;
            bool dirFound = false;
            AppTheme loadedTheme = AppTheme::Light; // 默认值
            std::wstring loadedDataDir;

            // 读取主题设置
            if (settingsJson.contains("theme")) {
                std::string themeStr = settingsJson["theme"];
                loadedTheme = (themeStr == "Dark") ? AppTheme::Dark : AppTheme::Light;
                themeFound = true;
                OutputDebugStringW((L"读取到主题设置: " + std::wstring(loadedTheme == AppTheme::Dark ? L"深色" : L"浅色") + L"\n").c_str());
            }

            // 读取数据目录设置
            if (settingsJson.contains("dataDirectory")) {
                std::string dirStr = settingsJson["dataDirectory"];
                loadedDataDir = utf8_to_wstring(dirStr);
                dirFound = true;
                OutputDebugStringW((L"读取到数据目录: " + loadedDataDir + L"\n").c_str());
            }

            // 如果读取到主题，更新内存中的设置
            if (themeFound) {
                m_theme = loadedTheme;
            }

            // 如果读取到数据目录，更新内存中的设置
            if (dirFound && !loadedDataDir.empty()) {
                m_dataDirectory = loadedDataDir;
            }

            // 如果发现新的数据目录且不是当前路径，查看是否存在新数据目录下的设置文件
            if (dirFound && !loadedDataDir.empty()) {
                std::wstring newSettingsPath = loadedDataDir + L"\\settings.json";

                // 如果当前尝试的路径不是新目录下的设置文件，尝试加载新路径
                if (newSettingsPath != path) {
                    OutputDebugStringW((L"发现新数据目录，尝试从 " + newSettingsPath + L" 加载设置\n").c_str());

                    std::ifstream newFile(newSettingsPath);
                    if (newFile.is_open()) {
                        json newSettingsJson;
                        try {
                            newFile >> newSettingsJson;
                            newFile.close();

                            // 读取新主题设置
                            if (newSettingsJson.contains("theme")) {
                                std::string themeStr = newSettingsJson["theme"];
                                m_theme = (themeStr == "Dark") ? AppTheme::Dark : AppTheme::Light;
                                OutputDebugStringW((L"从新目录读取到主题设置: " + std::wstring(m_theme == AppTheme::Dark ? L"深色" : L"浅色") + L"\n").c_str());
                            }

                            OutputDebugStringW((L"成功从新目录加载设置: " + newSettingsPath + L"\n").c_str());
                            return true;
                        }
                        catch (...) {
                            OutputDebugStringW(L"新数据目录设置文件格式错误，忽略\n");
                        }
                    }
                }
            }

            // 如果至少读取到一个设置，则认为加载成功
            if (themeFound || dirFound) {
                OutputDebugStringW((L"成功从 " + path + L" 加载设置\n").c_str());
                OutputDebugStringW((L"当前主题: " + std::wstring(m_theme == AppTheme::Dark ? L"深色" : L"浅色") + L"\n").c_str());
                OutputDebugStringW((L"当前数据目录: " + m_dataDirectory + L"\n").c_str());
                return true;
            }
        }
        catch (const std::exception& e) {
            std::string errorMsg = "加载设置时发生异常: ";
            errorMsg += e.what();
            OutputDebugStringA(errorMsg.c_str());
            // 继续尝试下一个路径
        }
        catch (...) {
            OutputDebugStringW((L"从 " + path + L" 加载设置时发生未知异常\n").c_str());
            // 继续尝试下一个路径
        }
    }

    OutputDebugStringW(L"无法从任何位置加载设置，使用默认值\n");
    return false;
}