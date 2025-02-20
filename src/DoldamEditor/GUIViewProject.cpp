#include "pch.h"

#include <filesystem>

#include "FileManager.h"

#include "GUIView.h"

namespace fs = std::filesystem;

static void ShowDirectoryTreeRecursive(IRenderGUI *pGUI, GUIView* pView, FileNode *pNode, fs::path &parentPath)
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

        switch (pNode->itemType)
        {
        case FILE_ITEM_TYPE_TEXTURE:
            buttonColor = RGBA::GREEN;
            break;
        case FILE_ITEM_TYPE_MESH:
            buttonColor = RGBA::BLUE_II;
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
            ZeroMemory(pView->selectedItemName, sizeof(pView->selectedItemName));
            wcscpy_s(pView->selectedItemName, MAX_NAME, (parentPath / pNode->name).c_str());
        }
        if (pGUI->BeginDragDropSource())
        {
            ZeroMemory(pView->selectedItemName, sizeof(pView->selectedItemName));
            wcscpy_s(pView->selectedItemName, MAX_NAME, (parentPath / pNode->name).c_str());
            pGUI->SetDragDropPayload("SELECT_FILE_PAYLOAD", NULL, 0);
            pGUI->EndDragDropSource();
        }
    }
}

void GUIView::ShowProject()
{
    fs::path fullPath(basePath);

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

        static char selected[MAX_NAME] = {'\0'};
        if (pGUI->TreeNode("Selected Item"))
        {
            if (pGUI->BeginDragDropTarget())
            {
                RENDER_GUI_PAYLOAD payload;

                if (pGUI->AcceptDragDropPayload("SELECT_FILE_PAYLOAD", &payload))
                {
                    ZeroMemory(selected, sizeof(selected));
                    ws2s(selectedItemName, selected);
                }
                pGUI->EndDragDropTarget();
            }
            pGUI->Text(selected);
            pGUI->TreePop();
        }
        
        pGUI->EndChild();
        pGUI->End();
    }
}
