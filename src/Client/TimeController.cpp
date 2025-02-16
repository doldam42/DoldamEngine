#include "pch.h"

#include "Client.h"
#include "InputManager.h"

#include "TimeController.h"

BOOL TimeController::Start() 
{
    IGameManager  *pG = g_pClient->GetGameManager();
    InputManager *pI = g_pClient->GetInputManager();

    pI->AddKeyListener(VK_SPACE, [pG](void *) { pG->TogglePause(); });
    pI->AddKeyListener('1', [pG](void *) { pG->SetTimeSpeed(0.5f); });
    pI->AddKeyListener('2', [pG](void *) { pG->SetTimeSpeed(1.0f); });
    pI->AddKeyListener('3', [pG](void *) { pG->SetTimeSpeed(2.0f); });

    return TRUE;
}

void TimeController::Update(float dt)
{}

void TimeController::Render() {}
