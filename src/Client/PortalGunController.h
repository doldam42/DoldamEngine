#pragma once
class PortalGunController : public IController
{
    IGameObject *m_pBluePortal = nullptr;
    IGameObject *m_pRedPortal = nullptr;


    public:
    // Inherited via IController
    BOOL Start() override;
    void Update(float dt) override;
};
