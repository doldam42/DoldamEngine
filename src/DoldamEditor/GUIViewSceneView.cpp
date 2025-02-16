#include "pch.h"
#include "GUIView.h"

void GUIView::ShowSceneView()
{
    pGUI->SetNextWindowPosR(SceneViewPos.x, SceneViewPos.y);
    pGUI->SetNextWindowSizeR(SceneViewSize.x, SceneViewSize.y);

    if (pGUI->Begin("Scene View", nullptr, g_windowFlags))
    {
        pGUI->Text("Scene Rendering...");
    }
    pGUI->End();
}