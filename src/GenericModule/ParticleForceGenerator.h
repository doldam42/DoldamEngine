#pragma once

interface ParticleForceGenerator { virtual void UpdateForce(Particle * particle, float duration) = 0; };

class ParticleForceRegistry
{
  protected:
    struct ParticleForceRegistration
    {
        Particle               *particle;
        ParticleForceGenerator *fg;
    };
    typedef std::vector<ParticleForceRegistration> Registry;
    Registry                                       registrations;

  public:
    // 주어진 힘 발생기를 주어진 입자에 등록
    void Add(Particle *particle, ParticleForceGenerator *fg);

    // 주어진 쌍을 목록에서 제거한다.
    void Remove(Particle *particle, ParticleForceGenerator *fg);

    // 모든 등록 항목을 제거한다.
    // 입자나 힘 발생기 자체를 제거하지 않고, 연결만을 제거한다.
    void Clear();

    // 해당하는 입자의 힘을 업데이트하도록 모든 힘 발생기를 호출
    void UpdateForces(float dt);
};

class ParticleGravity : public ParticleForceGenerator
{
    Vector3 gravity;

  public:
    ParticleGravity(const Vector3 &inGravity) : gravity(inGravity) {}

    virtual void UpdateForce(Particle *particle, float duration) override;
};

class ParticleDrag : public ParticleForceGenerator
{
    // 속도에 비례하는 드래그 비례 상수
    float k1;

    // 속도의 제곱에 비례하는 드래그 비례 상수
    float k2;

  public:
    ParticleDrag(float _k1, float _k2) : k1(_k1), k2(_k2) {}

    virtual void UpdateForce(Particle *particle, float duration) override;
};

class ParticleSpring : public ParticleForceGenerator
{
    Particle *other;

    float springConstant;
    float restLength;

  public:
    ParticleSpring(Particle *_other, float constant, float _restLength)
        : springConstant(constant), restLength(_restLength)
    {
        other = _other;
    }

    virtual void UpdateForce(Particle *particle, float duration) override;
};

class ParticleAnchoredSpring : public ParticleForceGenerator
{
  protected:
    Vector3 *anchor;

    float springConstant;
    float restLength;

  public:
    // Anchor는 Local 변수 절대 금지
    ParticleAnchoredSpring(Vector3 *_anchor, float constant, float _restLength)
        : springConstant(constant), restLength(_restLength)
    {
        anchor = _anchor;
    }

    virtual void UpdateForce(Particle *particle, float duration) override;
};

class ParticleBungee : public ParticleForceGenerator
{
    Particle *other;

    float springConstant;
    float restLength;

  public:
    ParticleBungee(Particle *_other, float constant, float _restLength)
        : springConstant(constant), restLength(_restLength)
    {
        other = _other;
    }

    virtual void UpdateForce(Particle *particle, float duration) override;
};

class ParticleBuoyancy :public ParticleForceGenerator
{
    // 퇴대 부력을 발생시키기 전 개체의 최대 침수 깊이
    float maxDepth;
    
    // 물체의 부피
    float volume;

    // 수면이 y = 0 평면으로부터 이동한 높이 
    float waterHeight;

    // 액체의 밀도
    float liquidDensity;

    public:
    ParticleBuoyancy(float _maxDepth, float _volume, float _waterHeight, float _liquidDensity = 1000.f)
          : maxDepth(_maxDepth), volume(_volume), waterHeight(_waterHeight), liquidDensity(_liquidDensity)
      {
      }

    virtual void UpdateForce(Particle *particle, float duration) override;
};