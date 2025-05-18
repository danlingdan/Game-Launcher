#pragma once
#include "../framework.h"
#include "Game.h"
#include "../tools/StringUtils.h"

// 游戏集合类 - 管理所有游戏
class GameCollection {
public:
    GameCollection();
    ~GameCollection();

    // 加载游戏集合
    bool LoadGames();

    // 保存游戏集合
    bool SaveGames();

    // 添加游戏
    bool AddGame(const Game& game);

    // 获取游戏数量
    size_t GetGameCount() const { return m_games.size(); }

    // 获取指定索引的游戏
    Game* GetGame(size_t index);

    // 根据条件查找游戏
    std::vector<Game*> FindGames(const std::function<bool(const Game&)>& predicate);

    // 根据名称查找游戏
    std::vector<Game*> FindGamesByName(const std::wstring& name);

    // 根据分类查找游戏
    std::vector<Game*> FindGamesByCategory(const std::wstring& category) const;

    // 获取所有分类
    std::vector<std::wstring> GetAllCategories() const;

    // 删除游戏
    bool RemoveGame(int index);
private:
    std::vector<Game> m_games;
    std::wstring m_dataFilePath;

    // 建立初始数据
    void CreateSampleGames();
};