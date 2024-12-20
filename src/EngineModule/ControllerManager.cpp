#include "pch.h"
#include "ControllerManager.h"

void ControllerManager::Register(IController *pController) { m_controllers.push_back(pController); }

void ControllerManager::Update(float dt)
{
    for (int i = 0; i < m_controllers.size(); i++)
    {
        m_controllers[i]->Update(dt);
    }
}

void ControllerManager::Start()
{
    for (int i = 0; i < m_controllers.size(); i++)
    {
        m_controllers[i]->Start();
    }
}

ControllerManager::~ControllerManager()
{
}
