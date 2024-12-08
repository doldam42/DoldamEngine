#include "pch.h"
#include "ControllerManager.h"

void ControllerManager::RegisterController(IController *pController) { m_controllers.push_back(pController); }

void ControllerManager::UpdateControllers(float dt)
{
    for (int i = 0; i < m_controllers.size(); i++)
    {
        m_controllers[i]->Update(dt);
    }
}

void ControllerManager::StartControllers()
{
    for (int i = 0; i < m_controllers.size(); i++)
    {
        m_controllers[i]->Start();
    }
}

ControllerManager::~ControllerManager()
{
}
