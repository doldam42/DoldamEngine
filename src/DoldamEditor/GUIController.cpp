#include "pch.h"
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

    m_pGUI->Begin("Hello World!");
    m_pGUI->Begin("Not Hello World");
    //m_pGUI->Text("Hello World!");
    //m_pGUI->CheckBox("Check Box", &b);
    m_pGUI->SliderFloat("float", &f, 0.0f, 1.0f);
    m_pGUI->End();
    m_pGUI->End();
}

GUIController::~GUIController() { Cleanup(); }
