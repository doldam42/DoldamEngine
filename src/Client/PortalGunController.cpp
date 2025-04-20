#include "pch.h"
#include "Client.h"
#include "ControllerRegistry.h"
#include "InputManager.h"
#include "PortalGunController.h"

REGISTER_CONTROLLER(PortalGunController)

BOOL PortalGunController::Start()
{
    IGameManager *pGame = g_pClient->GetGameManager();
    IPhysicsManager *pPhysics = g_pClient->GetPhysics();

    Material portalMaterial = {};
    portalMaterial.metallicFactor = 0.0f;
    wcscpy_s(portalMaterial.name, L"red_portal");
    wcscpy_s(portalMaterial.basePath, L"..\\..\\assets\\textures\\");
    wcscpy_s(portalMaterial.albedoTextureName, L"red_portal.png");

    IRenderMaterial *pPortalMaterial = pGame->GetRenderer()->CreateMaterialHandle(&portalMaterial);

    m_pRedPortal = pGame->CreateGameObject();
    m_pRedPortal->SetModel(pGame->GetPrimitiveModel(PRIMITIVE_MODEL_TYPE_SQUARE));
    m_pRedPortal->SetScale(1.5f, 2.5f, 1.5f);
    m_pRedPortal->SetMaterials(&pPortalMaterial, 1);
    m_pRedPortal->SetActive(FALSE);

    /*ICollider *pCollider = pPhysics->CreateBoxCollider(m_pRedPortal, Vector3(1.5f, 2.5f, 0.1f));
    m_pRedPortal->SetCollider(pCollider);*/

    wcscpy_s(portalMaterial.name, L"blue_portal");
    wcscpy_s(portalMaterial.albedoTextureName, L"blue_portal.png");
    pPortalMaterial = pGame->GetRenderer()->CreateMaterialHandle(&portalMaterial);
    m_pBluePortal = pGame->CreateGameObject();
    m_pBluePortal->SetModel(pGame->GetPrimitiveModel(PRIMITIVE_MODEL_TYPE_SQUARE));
    m_pBluePortal->SetScale(1.5f, 2.5f, 1.5f);
    m_pBluePortal->SetMaterials(&pPortalMaterial, 1);
    m_pBluePortal->SetActive(FALSE);

    //pCollider = pPhysics->CreateBoxCollider(m_pBluePortal, Vector3(1.5f, 2.5f, 0.1f));
    //m_pBluePortal->SetCollider(pCollider);

    // Set CrossHair
    {
        UINT width = g_pClient->GetScreenWidth();
        UINT height = g_pClient->GetScreenHeight();
        m_crossHairPosX = (width / 2) - m_crossHairImageSize * m_crossHairScale * 0.5f;
        m_crossHairPosY = (height / 2) - m_crossHairImageSize * m_crossHairScale * 0.5f;

        IGameSprite *pSprite = pGame->CreateSpriteFromFile(L"../../assets/textures/", L"crosshair.dds",
                                                           m_crossHairImageSize, m_crossHairImageSize);
        pSprite->SetScale(0.25);
        pSprite->SetPosition(m_crossHairPosX, m_crossHairPosY);

        m_pCrossHairSprite = pSprite;
    }

    return TRUE;
}

void PortalGunController::Update(float dt)
{
    IGameManager *pGame = g_pClient->GetGameManager();
    InputManager *pI = g_pClient->GetInputManager();

    if (pI->IsKeyPressed(VK_LBUTTON, false))
    {
        Vector3 rayDir = pGame->GetCameraLookTo();
        Vector3 rayPos = pGame->GetCameraPos();

        RayHit hit;
        if (pGame->Raycast(rayPos, rayDir, &hit))
        {
            Vector3 pos = hit.point + hit.normal * 0.01f;

            Vector3 portalForward = -hit.normal;
            portalForward.Normalize();

            Vector3 upCandidate = Vector3::Transform(Vector3::UnitY, hit.pHitted->GetRotation());

            // forward°¡ ¹Ù´ÚÀÌ¸é rayDir°ú ÆòÇàÇÏ°Ô ¹Ù²ãÁÜ
            if (fabs(portalForward.Dot(Vector3::UnitY)) > 0.99f)
            {
                upCandidate = rayDir;
            }

            Vector3 portalRight = upCandidate.Cross(portalForward);

            portalRight.Normalize();
            upCandidate.Normalize();

            Matrix rotationMatrix = Matrix::Identity;
            rotationMatrix = Matrix::CreateWorld(Vector3::Zero, portalForward, upCandidate);

            Quaternion rot = Quaternion::CreateFromRotationMatrix(rotationMatrix);
            m_pRedPortal->SetActive(TRUE);
            m_pRedPortal->SetRotation(rot);
            m_pRedPortal->SetPosition(pos.x, pos.y, pos.z);

            // IGameObject     *pHitted = hit.pHitted;
            // IRenderMaterial *pMaterial = pHitted->GetMaterialAt(0);
            // pMaterial->UpdateEmissive(Vector3(1.0f, 0.0f, 0.0f));
        }
    }

    else if (pI->IsKeyPressed(VK_RBUTTON, false))
    {
        Vector3 rayDir = pGame->GetCameraLookTo();
        Vector3 rayPos = pGame->GetCameraPos();

        RayHit hit;
        if (pGame->Raycast(rayPos, rayDir, &hit))
        {
            Vector3 pos = hit.point + hit.normal * 0.02f;

            Vector3 portalForward = -hit.normal;
            portalForward.Normalize();

            Vector3 upCandidate = Vector3::Transform(Vector3::UnitY, hit.pHitted->GetRotation());

            // forward°¡ ¹Ù´ÚÀÌ¸é rayDir°ú ÆòÇàÇÏ°Ô ¹Ù²ãÁÜ
            if (fabs(portalForward.Dot(Vector3::UnitY)) > 0.99f)
            {
                upCandidate = rayDir;
            }

            Vector3 portalRight = upCandidate.Cross(portalForward);

            portalRight.Normalize();
            upCandidate.Normalize();

            Matrix rotationMatrix = Matrix::Identity;
            rotationMatrix = Matrix::CreateWorld(Vector3::Zero, portalForward, upCandidate);

            Quaternion rot = Quaternion::CreateFromRotationMatrix(rotationMatrix);
            m_pBluePortal->SetActive(TRUE);
            m_pBluePortal->SetRotation(rot);
            m_pBluePortal->SetPosition(pos.x, pos.y, pos.z);

            // IGameObject     *pHitted = hit.pHitted;
            // IRenderMaterial *pMaterial = pHitted->GetMaterialAt(0);
            // pMaterial->UpdateEmissive(Vector3(1.0f, 0.0f, 0.0f));
        }
    }
}
