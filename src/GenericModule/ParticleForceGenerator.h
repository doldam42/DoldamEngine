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
    // �־��� �� �߻��⸦ �־��� ���ڿ� ���
    void Add(Particle *particle, ParticleForceGenerator *fg);

    // �־��� ���� ��Ͽ��� �����Ѵ�.
    void Remove(Particle *particle, ParticleForceGenerator *fg);

    // ��� ��� �׸��� �����Ѵ�.
    // ���ڳ� �� �߻��� ��ü�� �������� �ʰ�, ���Ḹ�� �����Ѵ�.
    void Clear();

    // �ش��ϴ� ������ ���� ������Ʈ�ϵ��� ��� �� �߻��⸦ ȣ��
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
    // �ӵ��� ����ϴ� �巡�� ��� ���
    float k1;

    // �ӵ��� ������ ����ϴ� �巡�� ��� ���
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
    // Anchor�� Local ���� ���� ����
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
    // ��� �η��� �߻���Ű�� �� ��ü�� �ִ� ħ�� ����
    float maxDepth;
    
    // ��ü�� ����
    float volume;

    // ������ y = 0 ������κ��� �̵��� ���� 
    float waterHeight;

    // ��ü�� �е�
    float liquidDensity;

    public:
    ParticleBuoyancy(float _maxDepth, float _volume, float _waterHeight, float _liquidDensity = 1000.f)
          : maxDepth(_maxDepth), volume(_volume), waterHeight(_waterHeight), liquidDensity(_liquidDensity)
      {
      }

    virtual void UpdateForce(Particle *particle, float duration) override;
};