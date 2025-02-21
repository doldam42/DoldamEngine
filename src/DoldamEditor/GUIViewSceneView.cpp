#include "pch.h"
#include "GUIView.h"

void GUIView::ShowSceneView()
{
    static char selected[MAX_NAME] = {"Drop Here"};
    pGUI->SetNextWindowPosR(SceneViewPos.x, SceneViewPos.y);
    pGUI->SetNextWindowSizeR(SceneViewSize.x, SceneViewSize.y);

    if (pGUI->Begin("Scene View", nullptr, g_windowFlags))
    {
        pGUI->Text(selected);

        pGUI->Image(pSceneViewTex, width * SceneViewSize.x, height * SceneViewSize.y);

        if (pGUI->BeginDragDropTarget())
        {
            RENDER_GUI_PAYLOAD payload;

            if (pGUI->AcceptDragDropPayload("SELECT_FILE_PAYLOAD", &payload))
            {
                ZeroMemory(selected, sizeof(selected));
                strcpy_s(selected, "Selected!");
            }
            pGUI->EndDragDropTarget();
        }
    }
    pGUI->End();
}