#include "pch.h"
#include "GUIView.h"

void GUIView::ShowSceneView()
{
    pGUI->SetNextWindowPosR(SceneViewPos.x, SceneViewPos.y);
    pGUI->SetNextWindowSizeR(SceneViewSize.x, SceneViewSize.y);

    if (pGUI->Begin("Scene View", nullptr, g_windowFlags))
    {
        pGUI->Image(pSceneViewTex, width * SceneViewSize.x, height * SceneViewSize.y);

        if (pGUI->BeginDragDropTarget())
        {
            RENDER_GUI_PAYLOAD payload;

            if (pGUI->AcceptDragDropPayload("SELECT_FILE_PAYLOAD", &payload))
            {
                currentRequestType = GUI_REQUEST_TYPE_CREATE_GAMEOBJECT;
            }
            pGUI->EndDragDropTarget();
        }
    }
    pGUI->End();
}