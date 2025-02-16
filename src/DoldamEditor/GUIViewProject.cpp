#include "pch.h"

#include <filesystem>

#include "GUIView.h"

namespace fs = std::filesystem;

void ShowDirectoryTreeRecursive(IRenderGUI *pGUI, const fs::path &directory)
{
    for (const auto &entry : fs::directory_iterator(directory))
    {
        if (entry.is_directory())
        {
            if (pGUI->TreeNode(entry.path().filename().string().c_str()))
            {
                // Recursive call for nested directories
                ShowDirectoryTreeRecursive(pGUI, entry.path());
                pGUI->TreePop();
            }
        }
        else
        {
            RGBA color{0, 149, 183, 255};
            pGUI->ColoredButton(entry.path().filename().string().c_str(), color);
            //pGUI->Text(entry.path().filename().string().c_str());
        }
    }
}

void GUIView::ShowProject()
{
    pGUI->SetNextWindowPosR(ProjectPos.x, ProjectPos.y);
    pGUI->SetNextWindowSizeR(ProjectSize.x, ProjectSize.y);

    if (pGUI->Begin("Project", nullptr, g_windowFlags))
    {
        pGUI->Text("Project & Assets");

        pGUI->BeginChild("AssetNode", WINDOW_WIDTH, 0.0f);
        if (pGUI->TreeNode("Assets"))
        {
            ShowDirectoryTreeRecursive(pGUI, fs::path(assetPath));
            pGUI->TreePop();
        }
        pGUI->EndChild();
        pGUI->SameLine();

        pGUI->BeginChild("Images");
        for (int i = 0; i < 100; i++)
            pGUI->Text("scrollable Images");
        pGUI->Text("End");
        pGUI->EndChild();
        pGUI->End();
    }
}
