#pragma once
#include "../framework.h"
#include "../tools/StringUtils.h"

// ��Ϸ�� - �洢������Ϸ����Ϣ
class Game {
public:
    Game();
    Game(const std::wstring& name, const std::wstring& releaseYear,
        const std::wstring& publisher, const std::wstring& language,
        const std::wstring& category, const std::wstring& platform);
    ~Game();

    // Getter/Setter
    const std::wstring& GetName() const { return m_name; }
    void SetName(const std::wstring& name) { m_name = name; }

    const std::wstring& GetReleaseYear() const { return m_releaseYear; }
    void SetReleaseYear(const std::wstring& year) { m_releaseYear = year; }

    const std::wstring& GetPublisher() const { return m_publisher; }
    void SetPublisher(const std::wstring& publisher) { m_publisher = publisher; }

    const std::wstring& GetLanguage() const { return m_language; }
    void SetLanguage(const std::wstring& language) { m_language = language; }

    const std::wstring& GetCategory() const { return m_category; }
    void SetCategory(const std::wstring& category) { m_category = category; }

    const std::wstring& GetPlatform() const { return m_platform; }
    void SetPlatform(const std::wstring& platform) { m_platform = platform; }

    const std::wstring& GetExecutablePath() const { return m_executablePath; }
    void SetExecutablePath(const std::wstring& path) { m_executablePath = path; }

    const std::wstring& GetIconPath() const { return m_iconPath; }
    void SetIconPath(const std::wstring& path) { m_iconPath = path; }

    // �������������getter��setter
    const std::wstring& GetLaunchParams() const;
    void SetLaunchParams(const std::wstring& params);

    // ������Ϸ
    bool Launch() const;

    // ���л��ͷ����л�
    std::wstring Serialize() const;
    static Game Deserialize(const std::wstring& data);

private:
    std::wstring m_name;           // ��Ϸ����
    std::wstring m_releaseYear;    // �������
    std::wstring m_publisher;      // ��������
    std::wstring m_language;       // ����
    std::wstring m_category;       // ��Ϸ����
    std::wstring m_platform;       // ƽ̨
    std::wstring m_executablePath; // ��ִ���ļ�·��
    std::wstring m_iconPath;       // ͼ��·��

    // ��������������Ա����
    std::wstring m_launchParams;

    // ��������������������֡���ǩ��
};