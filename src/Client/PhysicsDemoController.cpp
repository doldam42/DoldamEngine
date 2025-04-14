#include "pch.h"

#include "Client.h"
#include "InputManager.h"

#include "PhysicsDemoController.h"

REGISTER_SCENE(PhysicsDemoController)

void PhysicsDemoController::UnLoad() {}

void PhysicsDemoController::Load()
{
    IGameManager    *pGame = g_pClient->GetGameManager();
    IRenderer       *pRenderer = pGame->GetRenderer();
    IPhysicsManager *pPhysics = g_pClient->GetPhysics();

    // Create Material
    Material reflectiveMaterial = {};
    reflectiveMaterial.metallicFactor = 0.0f;
    reflectiveMaterial.reflectionFactor = 0.9f;
    wcscpy_s(reflectiveMaterial.name, L"ground");
    wcscpy_s(reflectiveMaterial.basePath, L"..\\..\\assets\\textures\\Tiles074\\");
    wcscpy_s(reflectiveMaterial.albedoTextureName, L"Tiles074_2K-JPG_Color.jpg");
    wcscpy_s(reflectiveMaterial.normalTextureName, L"Tiles074_2K-JPG_NormalDX.jpg");
    wcscpy_s(reflectiveMaterial.roughnessTextureName, L"Tiles074_2K-JPG_Roughness.jpg");
    IRenderMaterial *pGroundMaterial = pRenderer->CreateMaterialHandle(&reflectiveMaterial);

    IGameModel  *pGroundModel = pGame->GetPrimitiveModel(PRIMITIVE_MODEL_TYPE_BOX);
    IGameObject *pGround = pGame->CreateGameObject();
    pGround->SetModel(pGroundModel);
    pGround->SetPosition(0.0f, 0.0f, 0.0f);
    pGround->SetScale(30.0f, 0.2f, 30.0f);
    pGround->SetMaterials(&pGroundMaterial, 1);

    ICollider  *pCollider = pPhysics->CreateBoxCollider(Vector3(30.0f, 0.2f, 30.0f));
    IRigidBody *pBody = pPhysics->CreateRigidBody(pCollider, Vector3::Zero, 0.0f, 0.5f, 0.5f, FALSE);
    pGround->SetCollider(pCollider);
    pGround->SetRigidBody(pBody);

    Material ChristmasTreeOrnamentMaterial = {};
    ChristmasTreeOrnamentMaterial.reflectionFactor = 0.2f;
    wcscpy_s(ChristmasTreeOrnamentMaterial.name, L"ChristmasTreeOrnament");
    wcscpy_s(ChristmasTreeOrnamentMaterial.basePath, L"..\\..\\assets\\textures\\ChristmasTreeOrnament014\\");
    wcscpy_s(ChristmasTreeOrnamentMaterial.albedoTextureName, L"ChristmasTreeOrnament014_2K-JPG_Color.jpg");
    wcscpy_s(ChristmasTreeOrnamentMaterial.normalTextureName, L"ChristmasTreeOrnament014_2K-JPG_NormalDX.jpg");
    wcscpy_s(ChristmasTreeOrnamentMaterial.roughnessTextureName, L"ChristmasTreeOrnament014_2K-JPG_Roughness.jpg");
    wcscpy_s(ChristmasTreeOrnamentMaterial.metallicTextureName, L"ChristmasTreeOrnament014_2K-JPG_Metalness.jpg");

    IRenderMaterial *pSphereMaterial = pRenderer->CreateMaterialHandle(&ChristmasTreeOrnamentMaterial);

    IGameModel  *pBoxModel = pGame->GetPrimitiveModel(PRIMITIVE_MODEL_TYPE_BOX);
    IGameObject *pBox = pGame->CreateGameObject();
    
    pBox->SetModel(pBoxModel);
    pBox->SetPosition(5.0f, 10.0f, 5.0f);
    pBox->SetScale(2.0f);
    pBox->SetMaterials(&pSphereMaterial, 1);

    pCollider = pPhysics->CreateBoxCollider(Vector3(2.0f));
    pBody = pPhysics->CreateRigidBody(pCollider, Vector3(5.0f, 10.0f, 5.0f), 2.0f, 0.2f, 0.5f);
    pBox->SetCollider(pCollider);
    pBox->SetRigidBody(pBody);
    
    size_t boxIdx = 0;
    m_pBoxs[boxIdx] = pBox;

    const int stackHeight = 5;
    for (int x = 0; x < 1; x++)
    {
        for (int y = 0; y < 1; y++)
        {
            for (int z = 0; z < stackHeight; z++)
            {
                IGameModel  *pBoxModel = pGame->GetPrimitiveModel(PRIMITIVE_MODEL_TYPE_BOX);
                IGameObject *pBox = pGame->CreateGameObject();
                pBox->SetModel(pBoxModel);

                float offset = ((z & 1) == 0) ? 0.0f : 0.15f;
                float xx = (float)x + offset;
                float yy = (float)y + offset;
                float delta = 0.05f;
                float scaleHeight = 2.0f + delta;
                float deltaHeight = 1.0f + delta;

                Vector3 pos =
                    Vector3((float)xx * scaleHeight, deltaHeight + (float)z * scaleHeight, (float)yy * scaleHeight);
                pBox->SetPosition(pos.x, pos.y, pos.z);
                pBox->SetMaterials(&pSphereMaterial, 1);
                ICollider  *pCollider = pPhysics->CreateBoxCollider(Vector3::One);
                IRigidBody *pBody =
                    pPhysics->CreateRigidBody(pCollider, Vector3(pos.x, pos.y, pos.z), 1.0f, 0.2f, 0.5f);
                pBox->SetCollider(pCollider);
                pBox->SetRigidBody(pBody);

                m_pBoxs[boxIdx] = pBox;
                boxIdx++;
            }
        }
    }

    for (int i = 0; i < MAX_BALL_COUNT; i++)
    {
        IGameModel  *pSphereModel = pGame->GetPrimitiveModel(PRIMITIVE_MODEL_TYPE_SPHERE);
        IGameObject *pSphere = pGame->CreateGameObject();

        pSphere->SetModel(pSphereModel);
        pSphere->SetPosition(0.0f, 0.0f, 0.0f);
        pSphere->SetScale(2.0f);
        pSphere->SetMaterials(&pSphereMaterial, 1);

        ICollider  *pCollider = pPhysics->CreateSphereCollider(2.0f);
        IRigidBody *pBody = pPhysics->CreateRigidBody(pCollider, Vector3::Zero, 1.0f, 1.0f, 0.0f);
        
        pSphere->SetCollider(pCollider);
        pSphere->SetRigidBody(pBody);
        pSphere->SetActive(FALSE);

        m_pBalls[i] = pSphere;
    }

    m_pMaterial = pSphereMaterial;
}

void PhysicsDemoController::Update(float dt)
{
    /*constexpr float SHOOT_CYCLE = 0.5f;
    static float    cycle = 0.0f;
    static BOOL      shootLeft = FALSE;

    InputManager    *pI = g_pClient->GetInputManager();
    IGameManager    *pGame = g_pClient->GetGameManager();
    IPhysicsManager *pPhysics = g_pClient->GetPhysics();

    cycle += dt;
    if (cycle > SHOOT_CYCLE)
    {
        if (IsFull())
        {
            IGameObject *pDel = Pop();
            pDel->GetRigidBody()->Reset();
            pDel->SetActive(FALSE);
        }

        IGameObject *pSphere = Back();
        IRigidBody  *pBody = pSphere->GetRigidBody();
        pBody->Reset();
        pSphere->SetActive(TRUE);
        Push();
        
        if (shootLeft)
        {
            pSphere->SetPosition(0.0f, 10.0f, -25.0f);
            pBody->SetPosition(Vector3(0.0f, 10.0f, -25.0f));
            pBody->ApplyImpulseLinear(Vector3(0.0f, 2.0f, 40.0f));
        }
        else
        {
            pSphere->SetPosition(0.0f, 10.0f, 25.0f);
            pBody->SetPosition(Vector3(0.0f, 10.0f, 25.0f));
            pBody->ApplyImpulseLinear(Vector3(0.0f, 2.0f, -40.0f));
        }
        
        cycle = 0.0f;
        shootLeft = !shootLeft;
    }*/
}
