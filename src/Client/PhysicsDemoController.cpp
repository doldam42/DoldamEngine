#include "pch.h"

#include "Client.h"
#include "ControllerRegistry.h"
#include "InputManager.h"

#include "PhysicsDemoController.h"

REGISTER_CONTROLLER(PhysicsDemoController)

BOOL PhysicsDemoController::Start()
{
    IGameManager *pGame = g_pClient->GetGameManager();
    IRenderer    *pRenderer = pGame->GetRenderer();

    Material ChristmasTreeOrnamentMaterial = {};
    ChristmasTreeOrnamentMaterial.reflectionFactor = 0.2f;
    wcscpy_s(ChristmasTreeOrnamentMaterial.name, L"ChristmasTreeOrnament");
    wcscpy_s(ChristmasTreeOrnamentMaterial.basePath, L"..\\..\\assets\\textures\\ChristmasTreeOrnament014\\");
    wcscpy_s(ChristmasTreeOrnamentMaterial.albedoTextureName, L"ChristmasTreeOrnament014_2K-JPG_Color.jpg");
    wcscpy_s(ChristmasTreeOrnamentMaterial.normalTextureName, L"ChristmasTreeOrnament014_2K-JPG_NormalDX.jpg");
    wcscpy_s(ChristmasTreeOrnamentMaterial.roughnessTextureName, L"ChristmasTreeOrnament014_2K-JPG_Roughness.jpg");
    wcscpy_s(ChristmasTreeOrnamentMaterial.metallicTextureName, L"ChristmasTreeOrnament014_2K-JPG_Metalness.jpg");

    IRenderMaterial *pSphereMaterial = pRenderer->CreateMaterialHandle(&ChristmasTreeOrnamentMaterial);

    float dx = -10.f;
    for (int i = 0; i < 100; i++)
    {
        
        float height = 5.0f + 3.0f * i;

        IGameModel  *pSphereModel = pGame->GetPrimitiveModel(PRIMITIVE_MODEL_TYPE_SPHERE);
        IGameObject *pSphere = pGame->CreateGameObject(FALSE);
        pSphere->SetModel(pSphereModel);
        pSphere->SetPosition(dx, height, 0.0f);
        pSphere->SetScale(2.0f);
        pSphere->SetMaterials(&pSphereMaterial, 1);
        pSphere->InitSphereCollider(Vector3::Zero, 2.0f);
        pSphere->InitRigidBody(1.0f, 1.0f, 0.5f);

        dx += 0.5f;
    }

    m_pMaterial = pSphereMaterial;

    return TRUE;
}

void PhysicsDemoController::Update(float dt)
{
    constexpr float SHOOT_CYCLE = 0.5f;
    static float    cycle = 0.0f;

    InputManager *pI = g_pClient->GetInputManager();
    IGameManager *pGame = g_pClient->GetGameManager();

    cycle += dt;

    if (cycle > SHOOT_CYCLE)
    {
        IGameModel *pSphereModel = pGame->GetPrimitiveModel(PRIMITIVE_MODEL_TYPE_SPHERE);
        IGameObject *pSphere = pGame->CreateGameObject(FALSE);
        pSphere->SetModel(pSphereModel);
        pSphere->SetPosition(0.0f, 10.0f, -25.0f);
        pSphere->SetScale(2.0f);
        pSphere->SetMaterials(&m_pMaterial, 1);
        pSphere->InitSphereCollider(Vector3::Zero, 2.0f);
        pSphere->InitRigidBody(1.0f, 1.0f, 0.5f);
        
        pSphere->GetRigidBody()->ApplyImpulseLinear(Vector3(0.0f, 2.0f, 15.0f));
        
        cycle = 0.0f;
    }
}
