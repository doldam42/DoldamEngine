#include "pch.h"

#include "ControllerManager.h"

void ControllerManager::Register(IController *pController) { m_controllers.push_back(pController); }

void ControllerManager::Register(IRenderableController *pController) { m_renderableControllers.push_back(pController); }

void ControllerManager::Update(float dt)
{
    for (int i = 0; i < m_controllers.size(); i++)
    {
        m_controllers[i]->Update(dt);
    }
    for (int i = 0; i < m_renderableControllers.size(); i++)
    {
        m_renderableControllers[i]->Update(dt);
    }
}

void ControllerManager::Render()
{
    for (int i = 0; i < m_renderableControllers.size(); i++)
    {
        m_renderableControllers[i]->Render();
    }
}

void ControllerManager::Start()
{
    for (int i = 0; i < m_controllers.size(); i++)
    {
        m_controllers[i]->Start();
    }
    for (int i = 0; i < m_renderableControllers.size(); i++)
    {
        m_renderableControllers[i]->Start();
    }
}

ControllerManager::~ControllerManager() {}
