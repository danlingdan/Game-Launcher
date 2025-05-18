#pragma once
#include "../framework.h"
#include "../core/Game.h"
#include "../../Resource.h"
#include "MainWindow.h"
#include "../core/GameManager.h"

// ��Ϸ���/�༭�Ի�����
class GameDialog {
public:
    GameDialog();
    ~GameDialog();

    // ��ʾ�����Ϸ�Ի���
    bool ShowAddDialog(HWND hParent, Game& game, GameCollection& collection);

    // ��ʾ�༭��Ϸ�Ի���
    bool ShowEditDialog(HWND hParent, Game& game, GameCollection& collection);

private:
    // �Ի�����̺���
    static INT_PTR CALLBACK DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

    // ʵ�ʴ���Ի�����Ϣ�ĺ���
    INT_PTR HandleMessage(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

    // ��ʼ���Ի���
    void InitDialog(HWND hDlg);
    void InitCategoryComboBox(HWND hDlg);

    // �ӶԻ����ȡ����
    void GetDialogData(HWND hDlg);

    // �����ִ���ļ�
    void BrowseExecutable(HWND hDlg);

    // ���ͼ���ļ�
    void BrowseIcon(HWND hDlg);

    // ��֤�Ի�������
    bool ValidateDialog(HWND hDlg);

    Game* m_pGame;     // ָ��Ҫ��ӻ�༭����Ϸ����
    bool m_bEditing;   // ��ʶ����ӻ��Ǳ༭
    GameCollection* m_pCollection = nullptr;
};
