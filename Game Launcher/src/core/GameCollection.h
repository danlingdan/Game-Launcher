#pragma once
#include "../framework.h"
#include "Game.h"
#include "../tools/StringUtils.h"

// ��Ϸ������ - ����������Ϸ
class GameCollection {
public:
    GameCollection();
    ~GameCollection();

    // ������Ϸ����
    bool LoadGames();

    // ������Ϸ����
    bool SaveGames();

    // �����Ϸ
    bool AddGame(const Game& game);

    // ��ȡ��Ϸ����
    size_t GetGameCount() const { return m_games.size(); }

    // ��ȡָ����������Ϸ
    Game* GetGame(size_t index);

    // ��������������Ϸ
    std::vector<Game*> FindGames(const std::function<bool(const Game&)>& predicate);

    // �������Ʋ�����Ϸ
    std::vector<Game*> FindGamesByName(const std::wstring& name);

    // ���ݷ��������Ϸ
    std::vector<Game*> FindGamesByCategory(const std::wstring& category) const;

    // ��ȡ���з���
    std::vector<std::wstring> GetAllCategories() const;

    // ɾ����Ϸ
    bool RemoveGame(int index);
private:
    std::vector<Game> m_games;
    std::wstring m_dataFilePath;

    // ������ʼ����
    void CreateSampleGames();
};