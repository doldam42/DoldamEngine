#pragma once
class PortalGunController : public IController
{
    IGameObject *m_pBluePortal = nullptr;
    IGameObject *m_pRedPortal = nullptr;
    // crossHair
    int          m_crossHairPosX = 0;
    int          m_crossHairPosY = 0;
    int          m_crossHairImageSize = 256;
    float        m_crossHairScale = 0.25f;
    IGameSprite *m_pCrossHairSprite = nullptr;

    public:
    // Inherited via IController
    BOOL Start() override;
    void Update(float dt) override;
};
