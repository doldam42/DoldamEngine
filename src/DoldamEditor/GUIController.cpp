#include "pch.h"

#include "FileDialogUtils.h"

#include "GUIController.h"

void GUIController::Cleanup()
{
    if (m_pGUI)
    {
        m_pGUI->Release();
        m_pGUI = nullptr;
    }
}

BOOL GUIController::Initilize(IRenderGUI *pGUI)
{
    m_pGUI = pGUI;
    return TRUE;
}

BOOL GUIController::Start() { return 0; }

void GUIController::Update(float dt) {}

void GUIController::Render() 
{
    static bool b;
    static float f;
    
    m_pGUI->SetNextWindowPosR(0.80f, 0.0f);
    m_pGUI->SetNextWindowSizeR(0.20f, 1.0f);
    m_pGUI->Begin("Hierarchy", false, GUI_WINDOW_FLAG_NO_MOVE | GUI_WINDOW_FLAG_NO_RESIZE | GUI_WINDOW_FLAG_NO_TITLE_BAR);
    m_pGUI->Text("Hierarchy");
    if (m_pGUI->TreeNode("Root"))
    {
        if (m_pGUI->TreeNode("Sub Node"))
        {
            m_pGUI->SliderFloat(nullptr, &f, 0.0f, 1.0f);
            m_pGUI->Text("Hello World!");
            if (m_pGUI->Button("Open File"))
            {
                BasicFileOpen();
            }
            m_pGUI->TreePop();
        }
        m_pGUI->TreePop();
    }
    m_pGUI->End();
}

GUIController::~GUIController() { Cleanup(); }
