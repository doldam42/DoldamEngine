#include "pch.h"

#include "GUIView.h"

void GUIView::ShowHierarchy()
{
    pGUI->SetNextWindowPosR(HierarchyPos.x, HierarchyPos.y);
    pGUI->SetNextWindowSizeR(HierarchySize.x, HierarchySize.y);

    if (pGUI->Begin("Hierarchy", nullptr, g_windowFlags))
    {
        pGUI->Text("Scene Hierarchy");
        for (IGameObject* pObj : objects)
        {
            char buff[20] = {'\0'};
            sprintf_s(buff, "%d", pObj->GetID());
            pGUI->Text(buff);
        }
        pGUI->End();
    }
}