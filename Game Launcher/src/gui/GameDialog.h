#pragma once
#include "../framework.h"
#include "../core/Game.h"
#include "../../Resource.h"
#include "MainWindow.h"
#include "../core/GameManager.h"

// 游戏添加/编辑对话框类
class GameDialog {
public:
    GameDialog();
    ~GameDialog();

    // 显示添加游戏对话框
    bool ShowAddDialog(HWND hParent, Game& game, GameCollection& collection);

    // 显示编辑游戏对话框
    bool ShowEditDialog(HWND hParent, Game& game, GameCollection& collection);

private:
    // 对话框过程函数
    static INT_PTR CALLBACK DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

    // 实际处理对话框消息的函数
    INT_PTR HandleMessage(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

    // 初始化对话框
    void InitDialog(HWND hDlg);
    void InitCategoryComboBox(HWND hDlg);

    // 从对话框获取数据
    void GetDialogData(HWND hDlg);

    // 浏览可执行文件
    void BrowseExecutable(HWND hDlg);

    // 浏览图标文件
    void BrowseIcon(HWND hDlg);

    // 验证对话框数据
    bool ValidateDialog(HWND hDlg);

    Game* m_pGame;     // 指向要添加或编辑的游戏对象
    bool m_bEditing;   // 标识是添加还是编辑
    GameCollection* m_pCollection = nullptr;
};
