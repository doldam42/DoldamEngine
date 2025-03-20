#include "pch.h"

#include "Client.h"
#include "InputManager.h"

#include "BallisticDemo.h"

//REGISTER_CONTROLLER(BallisticDemo)

BOOL BallisticDemo::Start() { return 0; }

void BallisticDemo::Update(float dt)
{
    InputManager *pI = g_pClient->GetInputManager();
    IGameManager *pGame = g_pClient->GetGameManager();

    Vector3 camPos = pGame->GetCameraPos();
    Vector3 camDir = pGame->GetCameraLookTo();

    if (pI->IsKeyPressed('1', false))
    {
        m_shotType = SHOT_TYPE_PISTOL;
    }
    if (pI->IsKeyPressed('2', false))
    {
        m_shotType = SHOT_TYPE_ARTILLERY;
    }
    if (pI->IsKeyPressed('3', false))
    {
        m_shotType = SHOT_TYPE_FIREBALL;
    }
    if (pI->IsKeyPressed('4', false))
    {
        m_shotType = SHOT_TYPE_LASER;
    }

    if (pI->IsKeyPressed(VK_LBUTTON, false))
    {
        IGameModel *pModel = pGame->GetPrimitiveModel(PRIMITIVE_MODEL_TYPE_SPHERE);
        Shot        shot;
        shot.pObj = pGame->CreateGameObject();
        shot.pObj->SetModel(pModel);
        shot.shotType = m_shotType;
        shot.startTime = g_pClient->LastFrameTimestamp();

        switch (m_shotType)
        {
        case SHOT_TYPE_PISTOL:
            shot.pObj->SetScale(0.5f);
            shot.particle = Particle(2.0f, camPos, camDir * 35.0f, Vector3::Down, 0.99f);
            break;
        case SHOT_TYPE_ARTILLERY:
            shot.particle = Particle(200.0f, camPos, camDir * 50.0f, Vector3::Down * 20.0f, 0.99f);
            break;
        case SHOT_TYPE_FIREBALL:
            shot.particle = Particle(1.0f, camPos, camDir * 10.0f, Vector3::Down * 0.6f, 0.9f);
            break;
        case SHOT_TYPE_LASER:
            shot.pObj->SetScale(0.5f);
            shot.particle = Particle(0.1f, camPos, camDir * 100.0f, Vector3::Zero, 0.99f);
            break;
        default:
            break;
        }

        shot.particle.ClearAccumulator();

        m_shots.push_back(shot);
    }

    for (int i = 0; i < m_shots.size(); i++)
    {
        Shot &shot = m_shots[i];
        shot.particle.Integrate(dt);

        const Vector3 pos = shot.particle.position;
        shot.pObj->SetPosition(pos.x, pos.y, pos.z);

        if (shot.particle.position.y < 0.0f || (pos - camPos).LengthSquared() > 40000.0f ||
            shot.startTime + 5000 < g_pClient->LastFrameTimestamp())
        {
            pGame->DeleteGameObject(shot.pObj);
            shot = m_shots.back();
            m_shots.pop_back();
        }
    }
}
