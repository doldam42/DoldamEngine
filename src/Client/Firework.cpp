#include "pch.h"

#include "InputManager.h"
#include "Client.h"
#include <random>

#include "Firework.h"

//REGISTER_CONTROLLER(FireworkDemo)

void FireworkDemo::InitFireworkRules()
{
    // Go through the firework types and create their rules.
    rules[0].Init(2);
    rules[0].SetParameters(1,                   // type
                           0.5f, 1.4f,          // age range
                           Vector3(-5, 25, -5), // min velocity
                           Vector3(5, 28, 5),   // max velocity
                           0.1                  // damping
    );
    rules[0].payloads[0].Set(3, 5);
    rules[0].payloads[1].Set(5, 5);

    rules[1].Init(1);
    rules[1].SetParameters(2,                   // type
                           0.5f, 1.0f,          // age range
                           Vector3(-5, 10, -5), // min velocity
                           Vector3(5, 20, 5),   // max velocity
                           0.8                  // damping
    );
    rules[1].payloads[0].Set(4, 2);

    rules[2].Init(0);
    rules[2].SetParameters(3,                   // type
                           0.5f, 1.5f,          // age range
                           Vector3(-5, -5, -5), // min velocity
                           Vector3(5, 5, 5),    // max velocity
                           0.1                  // damping
    );

    rules[3].Init(0);
    rules[3].SetParameters(4,                   // type
                           0.25f, 0.5f,         // age range
                           Vector3(-20, 5, -5), // min velocity
                           Vector3(20, 5, 5),   // max velocity
                           0.2                  // damping
    );

    rules[4].Init(1);
    rules[4].SetParameters(5,                   // type
                           0.5f, 1.0f,          // age range
                           Vector3(-20, 2, -5), // min velocity
                           Vector3(20, 18, 5),  // max velocity
                           0.01                 // damping
    );
    rules[4].payloads[0].Set(3, 5);

    rules[5].Init(0);
    rules[5].SetParameters(6,                  // type
                           3, 5,               // age range
                           Vector3(-5, 5, -5), // min velocity
                           Vector3(5, 10, 5),  // max velocity
                           0.95                // damping
    );

    rules[6].Init(1);
    rules[6].SetParameters(7,                   // type
                           4, 5,                // age range
                           Vector3(-5, 50, -5), // min velocity
                           Vector3(5, 60, 5),   // max velocity
                           0.01                 // damping
    );
    rules[6].payloads[0].Set(8, 10);

    rules[7].Init(0);
    rules[7].SetParameters(8,                   // type
                           0.25f, 0.5f,         // age range
                           Vector3(-1, -1, -1), // min velocity
                           Vector3(1, 1, 1),    // max velocity
                           0.01                 // damping
    );

    rules[8].Init(0);
    rules[8].SetParameters(9,                    // type
                           3, 5,                 // age range
                           Vector3(-15, 10, -5), // min velocity
                           Vector3(15, 15, 5),   // max velocity
                           0.95                  // damping
    );
}

void FireworkDemo::Create(UINT type, const Firework *parent)
{
    FireworkRule *rule = rules + (type - 1);
    rule->Create(fireworks + nextFirework, parent);
    nextFirework = (nextFirework + 1) % maxFireworks;
}

void FireworkDemo::Create(UINT type, UINT number, const Firework *parent)
{
    for (UINT i = 0; i < number; i++)
    {
        Create(type, parent);
    }
}

BOOL FireworkDemo::Start()
{
    IGameManager *pGame = g_pClient->GetGameManager();
    for (int i = 0; i < maxFireworks; i++)
    {
        IGameModel  *pModel = pGame->GetPrimitiveModel(PRIMITIVE_MODEL_TYPE_SPHERE);
        IGameObject *pObj = pGame->CreateGameObject();
        pObj->SetScale(0.1f);
        pObj->SetModel(pModel);
        pObj->SetActive(false);
        fireworks[i].pObj = pObj;
        fireworks[i].type = 0;
    }

    InitFireworkRules();

    return TRUE;
}

void FireworkDemo::Update(float dt)
{
    InputManager *pI = g_pClient->GetInputManager();

    if (pI->IsKeyPressed('1', false))
    {
        Create(1, 1, NULL);
    }
    if (pI->IsKeyPressed('2', false))
    {
        Create(2, 1, NULL);
    }
    if (pI->IsKeyPressed('3', false))
    {
        Create(3, 1, NULL);
    }
    if (pI->IsKeyPressed('4', false))
    {
        Create(4, 1, NULL);
    }
    if (pI->IsKeyPressed('5', false))
    {
        Create(5, 1, NULL);
    }
    if (pI->IsKeyPressed('6', false))
    {
        Create(6, 1, NULL);
    }
    if (pI->IsKeyPressed('7', false))
    {
        Create(7, 1, NULL);
    }
    if (pI->IsKeyPressed('8', false))
    {
        Create(8, 1, NULL);
    }
    if (pI->IsKeyPressed('9', false))
    {
        Create(9, 1, NULL);
    }
    for (Firework *firework = fireworks; firework < fireworks + maxFireworks; firework++)
    {
        if (firework->type > 0)
        {
            if (firework->Update(dt))
            {
                FireworkRule *rule = rules + (firework->type - 1);
                firework->type = 0;
                firework->pObj->SetActive(false);
                for (UINT i = 0; i < rule->payloadCount; i++)
                {
                    FireworkRule::Payload *payload = rule->payloads + i;
                    Create(payload->type, payload->count, firework);
                }
            }
        }
    }
}

void FireworkRule::Init(UINT payloadCount)
{
    FireworkRule::payloadCount = payloadCount;
    payloads = new Payload[payloadCount];
}

void FireworkRule::SetParameters(UINT type, float minAge, float maxAge, const Vector3 &minVelocity,
                                 const Vector3 &maxVelocity, float damping)
{
    this->type = type;
    this->minAge = minAge;
    this->maxAge = maxAge;
    this->minVelocity = minVelocity;
    this->maxVelocity = maxVelocity;
    this->damping = damping;
}

void FireworkRule::Create(Firework *firework, const Firework *parent) const
{
    firework->pObj->SetActive(true);
    firework->type = type;
    float rf = (float)rand() / (float)(RAND_MAX);
    firework->age = minAge + ((maxAge - minAge) * rf);
    Vector3 vel = Vector3::Zero;
    if (parent)
    {
        firework->position = parent->position;
        vel += parent->velocity;
    }
    else
    {
        Vector3 start;
        int     x = rand() % 3 - 1;
        start.x = 5.0f * x;
        firework->position = start;
    }
    rf = (float)rand() / (float)(RAND_MAX);
    vel += Vector3::Lerp(minVelocity, maxVelocity, rf);
    firework->velocity = vel;
    firework->invMass = 1.0f;
    firework->damping = damping;
    firework->acceleration = Vector3::Down * 0.98f;
    firework->ClearAccumulator();
}

BOOL Firework::Update(float dt)
{
    Integrate(dt);
    age -= dt;

    pObj->SetPosition(position.x, position.y, position.z);

    return (age < 0) || position.y < 0.0f;
}
