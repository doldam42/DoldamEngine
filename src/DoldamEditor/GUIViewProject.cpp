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
            RGBA buttonColor = RGBA::BLACK;

            const fs::path ext = entry.path().filename().extension();
            if (ext == L".dca")
            {
                buttonColor = RGBA::ORANGE;
            }
            else if (ext == L".png" || ext == L".jpg" || ext == L".dds" || ext == L".DDS" || ext == L".bmp" ||
                     ext == L".BMP" || ext == L"hdr")
            {
                buttonColor = RGBA::GREEN;
            }
            else if (ext == L".dom" || ext == L".fbx" || ext == L".gltf")
            {
                buttonColor = RGBA::BLUE_II;
            }
            else
            {
                buttonColor = RGBA::VIOLET_I;
            }

            if (pGUI->ColoredButton(entry.path().filename().string().c_str(), buttonColor))
            {

            }
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
