#pragma once
#include "ControllerRegistry.h"

struct Firework : public Particle
{
    UINT  type;
    float age;

    IGameObject *pObj;

    BOOL Update(float dt);
};

struct FireworkRule
{
    UINT    type;
    float   minAge;
    float   maxAge;
    Vector3 minVelocity;
    Vector3 maxVelocity;
    float   damping;

    void Init(UINT payloadCount);

    void SetParameters(UINT type, float minAge, float maxAge, const Vector3 &minVelocity, const Vector3 &maxVelocity,
                       float damping);

    struct Payload
    {
        UINT type;
        UINT count;
        void Set(UINT type, UINT count)
        {
            Payload::count = count;
            Payload::type = type;
        }
    };

    UINT     payloadCount;
    Payload *payloads;

    void Create(Firework *firework, const Firework *parent = nullptr) const;

    ~FireworkRule()
    {
        if (payloads)
        {
            delete[] payloads;
            payloads = nullptr;
        }
    };
    FireworkRule() = default;
};

class FireworkDemo : public IController
{
    const static unsigned maxFireworks = 1024;
    Firework              fireworks[maxFireworks];

    UINT         nextFirework = 0;
    const static unsigned ruleCount = 9;
    FireworkRule rules[ruleCount];

    void InitFireworkRules();

    void Create(UINT type, const Firework *parent);
    void Create(UINT type, UINT number, const Firework *parent);
  public:
    BOOL Start() override;
    void Update(float dt) override;
};