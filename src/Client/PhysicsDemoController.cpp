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

    IGameModel  *pBoxModel = pGame->GetPrimitiveModel(PRIMITIVE_MODEL_TYPE_BOX);
    IGameObject *pBox = pGame->CreateGameObject();
    //pBox->SetModel(pBoxModel);
    //pBox->SetPosition(0.0f, 10.0f, 0.0f);
    //pBox->SetScale(2.0f);
    ////pBox->SetRotationX(XM_PIDIV2 / 4);
    //pBox->SetMaterials(&pSphereMaterial, 1);
    //pBox->InitBoxCollider(Vector3::Zero, Vector3(2.0f));
    //pBox->InitRigidBody(2.0f, 0.2f, 0.5f);

    //pBoxModel = pGame->GetPrimitiveModel(PRIMITIVE_MODEL_TYPE_BOX);
    //pBox = pGame->CreateGameObject();
    pBox->SetModel(pBoxModel);
    pBox->SetPosition(5.0f, 10.0f, 5.0f);
    pBox->SetScale(2.0f);
    pBox->SetMaterials(&pSphereMaterial, 1);
    pBox->InitBoxCollider(Vector3::Zero, Vector3(2.0f));
    pBox->InitRigidBody(2.0f, 0.2f, 0.5f);

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
                    Vector3((float)xx * scaleHeight, deltaHeight + (float)z * scaleHeight, (float) yy * scaleHeight);
                pBox->SetPosition(pos.x, pos.y, pos.z);
                pBox->SetMaterials(&pSphereMaterial, 1);
                pBox->InitBoxCollider(Vector3::Zero, Vector3::One);
                pBox->InitRigidBody(1.0f, 0.0f, 0.5f);
            }
        }
    }

    /*float y = -20.0f;
    float height = 25.0f;
    for (int i = 0; i < 10; i++)
    {
        float x = -20.0f;
        for (int j = 0; j < 10; j++)
        {
            IGameModel  *pSphereModel = pGame->GetPrimitiveModel(PRIMITIVE_MODEL_TYPE_SPHERE);
            IGameObject *pSphere = pGame->CreateGameObject();
            pSphere->SetModel(pSphereModel);
            pSphere->SetPosition(x, height, y);
            pSphere->SetScale(2.0f);
            pSphere->SetMaterials(&pSphereMaterial, 1);
            pSphere->InitSphereCollider(Vector3::Zero, 2.0f);
            pSphere->InitRigidBody(1.0f, 1.0f, 0.5f);
            x += 4.0f;
        }
        y += 4.0f;
    }*/
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
        IGameModel  *pSphereModel = pGame->GetPrimitiveModel(PRIMITIVE_MODEL_TYPE_SPHERE);
        IGameObject *pSphere1 = pGame->CreateGameObject();
        pSphere1->SetModel(pSphereModel);
        pSphere1->SetPosition(0.0f, 10.0f, -25.0f);
        pSphere1->SetScale(2.0f);
        pSphere1->SetMaterials(&m_pMaterial, 1);
        pSphere1->InitSphereCollider(Vector3::Zero, 2.0f);
        pSphere1->InitRigidBody(1.0f, 1.0f, 0.5f);

        pSphere1->GetRigidBody()->ApplyImpulseLinear(Vector3(0.0f, 2.0f, 15.0f));

        //pSphereModel->AddRef();
        //IGameObject *pSphere2 = pGame->CreateGameObject();
        //pSphere2->SetModel(pSphereModel);
        //pSphere2->SetPosition(0.0f, 10.0f, 25.0f);
        //pSphere2->SetScale(2.0f);
        //pSphere2->SetMaterials(&m_pMaterial, 1);
        //pSphere2->InitSphereCollider(Vector3::Zero, 2.0f);
        //pSphere2->InitRigidBody(1.0f, 1.0f, 0.5f);

        //pSphere2->GetRigidBody()->ApplyImpulseLinear(Vector3(0.0f, 2.0f, -15.0f));

        cycle = 0.0f;
    }
}
