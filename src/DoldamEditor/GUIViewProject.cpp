#include "pch.h"

#include <filesystem>

#include "FileManager.h"

#include "GUIView.h"

namespace fs = std::filesystem;

static void ShowDirectoryTreeRecursive(IRenderGUI *pGUI, GUIView* pView, FileNode *pNode, fs::path parentPath)
{
    char name[MAX_NAME] = {'\0'};
    ws2s(pNode->name, name);

    if (pNode->isDirectory)
    {
        if (pGUI->TreeNode(name))
        {
            parentPath /= pNode->name;
            // Recursive call for nested directories
            SORT_LINK *pCur = pNode->childrenLinkHead;
            while (pCur)
            {
                FileNode *pChild = (FileNode *)pCur->pItem;

                ShowDirectoryTreeRecursive(pGUI, pView, pChild, parentPath);

                pCur = pCur->pNext;
            }
            pGUI->TreePop();
        }
    }
    else
    {
        RGBA buttonColor = RGBA::BLACK;

        GAME_ITEM_TYPE gameItemType = GAME_ITEM_TYPE_NONE;
        switch (pNode->itemType)
        {
        case FILE_ITEM_TYPE_TEXTURE:
            buttonColor = RGBA::GREEN;
            break;
        case FILE_ITEM_TYPE_MESH:
            buttonColor = RGBA::BLUE_II;
            gameItemType = GAME_ITEM_TYPE_MODEL;
            break;
        case FILE_ITEM_TYPE_ANIMATION:
            buttonColor = RGBA::ORANGE;
            break;
        default:
            buttonColor = buttonColor = RGBA::VIOLET_I;
            break;
        }

        if (pGUI->ColoredButton(name, buttonColor))
        {
            /*pView->isItemSelected = true;

            ZeroMemory(pView->basePath, sizeof(pView->basePath));
            ZeroMemory(pView->filename, sizeof(pView->filename));

            wcscpy_s(pView->basePath, MAX_PATH, parentPath.c_str());
            wcscpy_s(pView->filename, MAX_PATH, pNode->name);*/
        }
        if (pGUI->BeginDragDropSource())
        {
            pGUI->SetDragDropPayload("SELECT_FILE_PAYLOAD", NULL, 0);

            pView->isItemSelected = true;

            ZeroMemory(pView->basePath, sizeof(pView->basePath));
            ZeroMemory(pView->filename, sizeof(pView->filename));

            wcscpy_s(pView->basePath, parentPath.c_str());
            wcscat_s(pView->basePath, L"\\");
            wcscpy_s(pView->filename, pNode->name);

            pView->selectedItemType = gameItemType;

            pGUI->EndDragDropSource();
        }
    }
}

void GUIView::ShowProject()
{
    fs::path fullPath(assetPath);

    pGUI->SetNextWindowPosR(ProjectPos.x, ProjectPos.y);
    pGUI->SetNextWindowSizeR(ProjectSize.x, ProjectSize.y);

    if (pGUI->Begin("Project", nullptr, g_windowFlags))
    {
        pGUI->Text("Project & Assets");

        pGUI->BeginChild("AssetNode", WINDOW_WIDTH, 0.0f);

        ShowDirectoryTreeRecursive(pGUI, this, assetDir, fullPath);

        pGUI->EndChild();
        pGUI->SameLine();

        pGUI->BeginChild("Selected Item");
        pGUI->Text("Drop Here");
        if (pGUI->BeginDragDropTarget())
        {
            RENDER_GUI_PAYLOAD payload;
            if (pGUI->AcceptDragDropPayload("SELECT_FILE_PAYLOAD", &payload))
            {
                shouldCreateItem = true;
            }
            pGUI->EndDragDropTarget();
        }

        if (pGUI->TreeNode("Assets"))
        {
            for (IGameModel* pModel : models)
            {
                char buff[10] = {L'\0'};
                sprintf_s(buff, "%d", pModel->GetID());
                pGUI->Text(buff);
            }
            pGUI->TreePop();
        }
        
        pGUI->EndChild();

        pGUI->End();
    }
}
