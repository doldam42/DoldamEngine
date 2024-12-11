#include "pch.h"

#include "Client.h"

#include "TimeController.h"

BOOL TimeController::Start() { return TRUE; }

void TimeController::Update(float dt)
{
    Client &c = Client::GetInstance();

    IGameManager  *pG = c.GetGameManager();
    IInputManager *pI = pG->GetInputManager();

    pI->AddKeyListener('1', [pG](void *) { pG->SetTimeSpeed(0.5f); });
    pI->AddKeyListener('2', [pG](void *) { pG->SetTimeSpeed(1.0f); });
    pI->AddKeyListener('3', [pG](void *) { pG->SetTimeSpeed(2.0f); });
}
