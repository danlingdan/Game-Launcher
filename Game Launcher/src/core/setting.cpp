#include "setting.h"

// ʹ�ø���̵ı���
using json = nlohmann::json;

Setting::Setting()
    : m_theme(AppTheme::Light)
{
    // Ĭ������Ŀ¼���û��ĵ�/GameLauncherData
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
        // ׼��JSON����
        json settingsJson;

        // ���������Ϣ
        settingsJson["theme"] = (m_theme == AppTheme::Dark ? "Dark" : "Light");
        settingsJson["dataDirectory"] = wstring_to_utf8(m_dataDirectory);

        // ׼������·��
        std::wstring fullPath;
        bool pathFound = false;

        // 1. ���ȳ���ʹ���û����õ�����Ŀ¼
        if (!m_dataDirectory.empty()) {
            // ȷ��Ŀ¼����
            if (SHCreateDirectoryEx(NULL, m_dataDirectory.c_str(), NULL) == ERROR_SUCCESS ||
                GetLastError() == ERROR_ALREADY_EXISTS) {
                fullPath = m_dataDirectory + L"\\settings.json";
                pathFound = true;
                OutputDebugStringW((L"���Խ����ñ��浽�û�ѡ�������Ŀ¼: " + fullPath + L"\n").c_str());
            }
        }

        // 2. ����û�δѡ���Ŀ¼Ϊ�ջ򴴽�ʧ�ܣ�����ʹ��AppDataĿ¼
        if (!pathFound) {
            WCHAR appDataPath[MAX_PATH] = { 0 };
            if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, appDataPath))) {
                std::wstring appDataDir = std::wstring(appDataPath) + L"\\GameLauncher";

                // ȷ��Ŀ¼����
                if (SHCreateDirectoryEx(NULL, appDataDir.c_str(), NULL) == ERROR_SUCCESS ||
                    GetLastError() == ERROR_ALREADY_EXISTS) {
                    fullPath = appDataDir + L"\\settings.json";
                    pathFound = true;
                    OutputDebugStringW((L"���Խ����ñ��浽AppDataĿ¼: " + fullPath + L"\n").c_str());
                }
            }
        }

        // 3. �����ʹ�ó���Ŀ¼
        if (!pathFound) {
            WCHAR exePath[MAX_PATH] = { 0 };
            GetModuleFileNameW(NULL, exePath, MAX_PATH);

            // ��ȡĿ¼����
            std::wstring directory = exePath;
            size_t pos = directory.find_last_of(L"\\");
            if (pos != std::wstring::npos) {
                directory = directory.substr(0, pos + 1); // ����β��б��
                fullPath = directory + L"settings.json";
                pathFound = true;
                OutputDebugStringW((L"���Խ����ñ��浽����Ŀ¼: " + fullPath + L"\n").c_str());
            }
        }

        // 4. ������г��Զ�ʧ�ܣ����Ե�ǰĿ¼
        if (!pathFound) {
            wchar_t currentDir[MAX_PATH];
            GetCurrentDirectoryW(MAX_PATH, currentDir);
            fullPath = std::wstring(currentDir) + L"\\settings.json";
            OutputDebugStringW((L"���Ա���·��: " + fullPath + L"\n").c_str());
        }

        // ����JSON���ļ�
        std::ofstream file(fullPath, std::ios::out);
        if (!file) {
            OutputDebugStringW(L"�޷����������ļ�\n");
            return false;
        }

        // ʹ��pretty print��ʽ��JSON���浽�ļ�
        file << settingsJson.dump(4);
        file.close();

        OutputDebugStringW((L"���óɹ����浽: " + fullPath + L"\n").c_str());
        return true;
    }
    catch (const std::exception& e) {
        std::string errorMsg = "��������ʱ�����쳣: ";
        errorMsg += e.what();
        OutputDebugStringA(errorMsg.c_str());
        return false;
    }
    catch (...) {
        OutputDebugStringW(L"��������ʱ����δ֪�쳣\n");
        return false;
    }
}

bool Setting::Load() {
    // �������п��ܵ������ļ�·��
    std::vector<std::wstring> possiblePaths;

    // ��ӵ�ǰ����Ŀ¼·��
    if (!m_dataDirectory.empty()) {
        possiblePaths.push_back(m_dataDirectory + L"\\settings.json");
    }

    // ���AppDataĿ¼·��
    WCHAR appDataPath[MAX_PATH] = { 0 };
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, appDataPath))) {
        std::wstring appDataDir = std::wstring(appDataPath) + L"\\GameLauncher";
        possiblePaths.push_back(appDataDir + L"\\settings.json");
    }

    // ��ӳ���Ŀ¼·��
    WCHAR exePath[MAX_PATH] = { 0 };
    GetModuleFileNameW(NULL, exePath, MAX_PATH);
    std::wstring directory = exePath;
    size_t pos = directory.find_last_of(L"\\");
    if (pos != std::wstring::npos) {
        directory = directory.substr(0, pos + 1);
        possiblePaths.push_back(directory + L"settings.json");
    }

    // ��ӵ�ǰĿ¼·��
    wchar_t currentDir[MAX_PATH];
    GetCurrentDirectoryW(MAX_PATH, currentDir);
    possiblePaths.push_back(std::wstring(currentDir) + L"\\settings.json");

    // ���γ�������·��
    for (const auto& path : possiblePaths) {
        OutputDebugStringW((L"���Դ� " + path + L" ��������\n").c_str());

        try {
            // ���Դ򿪲���ȡJSON�ļ�
            std::ifstream file(path);
            if (!file.is_open()) continue;

            json settingsJson;
            file >> settingsJson;
            file.close();

            bool themeFound = false;
            bool dirFound = false;
            AppTheme loadedTheme = AppTheme::Light; // Ĭ��ֵ
            std::wstring loadedDataDir;

            // ��ȡ��������
            if (settingsJson.contains("theme")) {
                std::string themeStr = settingsJson["theme"];
                loadedTheme = (themeStr == "Dark") ? AppTheme::Dark : AppTheme::Light;
                themeFound = true;
                OutputDebugStringW((L"��ȡ����������: " + std::wstring(loadedTheme == AppTheme::Dark ? L"��ɫ" : L"ǳɫ") + L"\n").c_str());
            }

            // ��ȡ����Ŀ¼����
            if (settingsJson.contains("dataDirectory")) {
                std::string dirStr = settingsJson["dataDirectory"];
                loadedDataDir = utf8_to_wstring(dirStr);
                dirFound = true;
                OutputDebugStringW((L"��ȡ������Ŀ¼: " + loadedDataDir + L"\n").c_str());
            }

            // �����ȡ�����⣬�����ڴ��е�����
            if (themeFound) {
                m_theme = loadedTheme;
            }

            // �����ȡ������Ŀ¼�������ڴ��е�����
            if (dirFound && !loadedDataDir.empty()) {
                m_dataDirectory = loadedDataDir;
            }

            // ��������µ�����Ŀ¼�Ҳ��ǵ�ǰ·�����鿴�Ƿ����������Ŀ¼�µ������ļ�
            if (dirFound && !loadedDataDir.empty()) {
                std::wstring newSettingsPath = loadedDataDir + L"\\settings.json";

                // �����ǰ���Ե�·��������Ŀ¼�µ������ļ������Լ�����·��
                if (newSettingsPath != path) {
                    OutputDebugStringW((L"����������Ŀ¼�����Դ� " + newSettingsPath + L" ��������\n").c_str());

                    std::ifstream newFile(newSettingsPath);
                    if (newFile.is_open()) {
                        json newSettingsJson;
                        try {
                            newFile >> newSettingsJson;
                            newFile.close();

                            // ��ȡ����������
                            if (newSettingsJson.contains("theme")) {
                                std::string themeStr = newSettingsJson["theme"];
                                m_theme = (themeStr == "Dark") ? AppTheme::Dark : AppTheme::Light;
                                OutputDebugStringW((L"����Ŀ¼��ȡ����������: " + std::wstring(m_theme == AppTheme::Dark ? L"��ɫ" : L"ǳɫ") + L"\n").c_str());
                            }

                            OutputDebugStringW((L"�ɹ�����Ŀ¼��������: " + newSettingsPath + L"\n").c_str());
                            return true;
                        }
                        catch (...) {
                            OutputDebugStringW(L"������Ŀ¼�����ļ���ʽ���󣬺���\n");
                        }
                    }
                }
            }

            // ������ٶ�ȡ��һ�����ã�����Ϊ���سɹ�
            if (themeFound || dirFound) {
                OutputDebugStringW((L"�ɹ��� " + path + L" ��������\n").c_str());
                OutputDebugStringW((L"��ǰ����: " + std::wstring(m_theme == AppTheme::Dark ? L"��ɫ" : L"ǳɫ") + L"\n").c_str());
                OutputDebugStringW((L"��ǰ����Ŀ¼: " + m_dataDirectory + L"\n").c_str());
                return true;
            }
        }
        catch (const std::exception& e) {
            std::string errorMsg = "��������ʱ�����쳣: ";
            errorMsg += e.what();
            OutputDebugStringA(errorMsg.c_str());
            // ����������һ��·��
        }
        catch (...) {
            OutputDebugStringW((L"�� " + path + L" ��������ʱ����δ֪�쳣\n").c_str());
            // ����������һ��·��
        }
    }

    OutputDebugStringW(L"�޷����κ�λ�ü������ã�ʹ��Ĭ��ֵ\n");
    return false;
}