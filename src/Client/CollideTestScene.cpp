#include "pch.h"

#include "Client.h"

#include "CollideTestScene.h"

REGISTER_SCENE(CollideTestScene)

void CollideTestScene::Load()
{
    IGameManager    *pGame = g_pClient->GetGameManager();
    IPhysicsManager *pPhysics = g_pClient->GetPhysics();
    IRenderer       *pRenderer = pGame->GetRenderer();

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

    ICollider *pCollider = pPhysics->CreateBoxCollider(pGround, Vector3(30.0f, 0.2f, 30.0f));
    pGround->SetCollider(pCollider);

    for (int col = 0; col < 5; col++)
    {
        float z = -8.0f + col * 4.0f;
        for (int row = 0; row < 5; row++)
        {
            WCHAR name[MAX_NAME];
            ZeroMemory(name, sizeof(name));
            wsprintfW(name, L"material%d%d", col, row);
            Material material = {};
            material.metallicFactor = 0.0f;
            material.reflectionFactor = 0.9f;
            wcscpy_s(material.name, name);
            wcscpy_s(material.basePath, L"..\\..\\assets\\textures\\Tiles074\\");
            wcscpy_s(material.albedoTextureName, L"Tiles074_2K-JPG_Color.jpg");
            wcscpy_s(material.normalTextureName, L"Tiles074_2K-JPG_NormalDX.jpg");
            wcscpy_s(material.roughnessTextureName, L"Tiles074_2K-JPG_Roughness.jpg");
            IRenderMaterial *pMaterial = pRenderer->CreateMaterialHandle(&material);

            float        x = -8.0f + row * 4.0f;
            IGameObject *pObj = pGame->CreateGameObject();

            if (row % 2) // odd
            {
                /*IGameModel *pModel = pGame->GetPrimitiveModel(PRIMITIVE_MODEL_TYPE_BOX);
                pObj->SetModel(pModel);
                ICollider *pCollider = pPhysics->CreateBoxCollider(pObj, Vector3::One);
                pObj->SetCollider(pCollider);*/

                IGameModel *pModel = pGame->GetPrimitiveModel(PRIMITIVE_MODEL_TYPE_SPHERE);
                pObj->SetModel(pModel);
                ICollider *pCollider = pPhysics->CreateEllpsoidCollider(pObj, 1.5f, 1.0f);
                pObj->SetCollider(pCollider);
                pObj->SetScale(1.0f, 1.5f, 1.0f);
            }
            else
            {
                IGameModel *pModel = pGame->GetPrimitiveModel(PRIMITIVE_MODEL_TYPE_SPHERE);
                pObj->SetModel(pModel);
                ICollider *pCollider = pPhysics->CreateSphereCollider(pObj, 1.0f);
                pObj->SetCollider(pCollider);

                /*IGameModel *pModel = pGame->GetPrimitiveModel(PRIMITIVE_MODEL_TYPE_SPHERE);
                pObj->SetModel(pModel);
                ICollider *pCollider = pPhysics->CreateEllpsoidCollider(pObj, 1.5f, 1.0f);
                pObj->SetCollider(pCollider);
                pObj->SetScale(1.0f, 1.5f, 1.0f);*/
            }

            /*IGameModel *pModel = pGame->GetPrimitiveModel(PRIMITIVE_MODEL_TYPE_BOX);
            pObj->SetModel(pModel);
            ICollider *pCollider = pPhysics->CreateBoxCollider(pObj, Vector3::One);
            pObj->SetCollider(pCollider);*/
            pObj->SetPosition(x, 2.0f, z);
            pObj->SetRotationY(XM_PIDIV2 / 2);
            pObj->SetMaterials(&pMaterial, 1);
            m_pSpheres[col * 5 + row] = pObj;
        }
    }

    // ICollider  *pCollider = pPhysics->CreateBoxCollider(Vector3(30.0f, 0.2f, 30.0f));
    // IRigidBody *pBody = pPhysics->CreateRigidBody(pCollider, Vector3::Zero, 0.0f, 0.5f, 0.5f, FALSE);
    // pGround->SetCollider(pCollider);
    // pGround->SetRigidBody(pBody);

    pGame->SetCameraPosition(0.0f, 2.0f, -2.0f);
}

void CollideTestScene::Update(float dt)
{

    const float boundary = 10.0f;

    static float offset = 0.0f;
    static float speed = 5.0f;

    float deltaPos = speed * dt;
    for (int col = 0; col < 5; col++)
    {
        for (int row = 0; row < 5; row++)
        {
            int          idx = col * 5 + row;
            IGameObject *pObj = m_pSpheres[idx];
            if (row % 2) // odd
            {
                pObj->AddPosition(Vector3(deltaPos, 0.0f, 0.0f));
            }
            else
            {
                pObj->AddPosition(Vector3(0.0f, 0.0f, deltaPos));
            }

            if (pObj->GetCollider()->IsCollisionEnter())
            {
                pObj->GetMaterialAt(0)->UpdateEmissive(Vector3(1.0f));
            }
            if (pObj->GetCollider()->IsCollisionExit())
            {
                pObj->GetMaterialAt(0)->UpdateEmissive(Vector3(0.0f));
            }
        }
    }

    offset += deltaPos;

    if (abs(offset) > boundary)
    {
        speed = -speed;
        offset = (offset > 0) ? boundary : -boundary;
    }
}

void CollideTestScene::UnLoad() {}
