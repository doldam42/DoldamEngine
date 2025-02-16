#include "pch.h"

#include "GUIView.h"

void GUIView::ShowHierarchy()
{
    pGUI->SetNextWindowPosR(HierarchyPos.x, HierarchyPos.y);
    pGUI->SetNextWindowSizeR(HierarchySize.x, HierarchySize.y);

    if (pGUI->Begin("Hierarchy", nullptr, g_windowFlags))
    {
        pGUI->Text("Game Objects List");
        pGUI->End();
    }
}