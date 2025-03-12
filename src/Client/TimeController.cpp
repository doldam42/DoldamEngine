#include "pch.h"

#include "Client.h"
#include "InputManager.h"

#include "TimeController.h"

REGISTER_CONTROLLER(TimeController)
void TimeController::Update(float dt)
{ 
    static float  speed = 0.0f;

    InputManager *pI = g_pClient->GetInputManager();
    if (pI->IsKeyPressed(VK_TAB, false))
    {
        speed = fmodf(speed + 0.5f, 2.0f);
        g_pClient->SetTimeSpeed(speed);
    }
}

