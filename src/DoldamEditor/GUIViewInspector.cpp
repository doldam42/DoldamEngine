#include "pch.h"

#include "GUIView.h"

void GUIView::ShowInspector()
{
    pGUI->SetNextWindowPosR(InspectorPos.x, InspectorPos.y);
    pGUI->SetNextWindowSizeR(InspectorSize.x, InspectorSize.y);

    if (pGUI->Begin("Inspector", nullptr, g_windowFlags))
    {
        pGUI->Text("Object Properties");
        
        static Vector3 pos, rot, scale;
        if (pGUI->TreeNode("Transform"))
        {
            pGUI->DragFloat3("Position", &pos);
            pGUI->DragFloat3("Rotation", &rot);
            pGUI->DragFloat3("Scale", &scale);
            pGUI->TreePop();
        }
        if (pGUI->TreeNode("Mesh"))
        {
            pGUI->TreePop();
        }
        if (pGUI->TreeNode("Material"))
        {
            pGUI->TreePop();
        }
        //if (pSelectedObject)
        //{
        //    // TODO
        //}
        pGUI->End();
    }
}