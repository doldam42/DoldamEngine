#include "pch.h"

void ParticleForceRegistry::Add(Particle *particle, ParticleForceGenerator *fg)
{
    registrations.push_back({particle, fg});
}

void ParticleForceRegistry::Remove(Particle *particle, ParticleForceGenerator *fg)
{
    for (int i = 0; i < registrations.size(); i++)
    {
        auto &reg = registrations[i];
        if (reg.fg == fg && reg.particle == particle)
        {
            registrations[i] = registrations.back();
            registrations.pop_back();
        }
    }
}

void ParticleForceRegistry::Clear() {}

void ParticleForceRegistry::UpdateForces(float dt)
{
    for (auto &i : registrations)
    {
        i.fg->UpdateForce(i.particle, dt);
    }
}

void ParticleGravity::UpdateForce(Particle *particle, float duration)
{
    if (particle->invMass == 0.0f)
        return;

    // ������ ������ ���� �� �ۿ�
    particle->AddForce(gravity * particle->invMass);
}

void ParticleDrag::UpdateForce(Particle *particle, float duration)
{
    Vector3 force = particle->velocity;
    float   dragCoeff = force.Length();
    dragCoeff = k1 * dragCoeff + k2 * dragCoeff * dragCoeff;

    force.Normalize();
    force *= -dragCoeff;
    particle->AddForce(force);
}

void ParticleSpring::UpdateForce(Particle *particle, float duration)
{
    Vector3 d = (particle->position - other->position);
    float   f = -springConstant * fabsf(d.Length() - restLength);

    d.Normalize();
    d *= f;

    particle->AddForce(d);
}

void ParticleAnchoredSpring::UpdateForce(Particle *particle, float duration)
{
    Vector3 d = (particle->position - *anchor);
    float   f = fabsf(d.Length() - restLength);
    f *= springConstant;

    d.Normalize();
    d *= -f;

    particle->AddForce(d);
}

void ParticleBungee::UpdateForce(Particle *particle, float duration)
{
    Vector3 d = (particle->position - other->position);

    float l = d.Length() - restLength;

    if (l < 0)
        return;

    float   f = -springConstant * l;

    d.Normalize();
    d *= f;

    particle->AddForce(d);
}

void ParticleBuoyancy::UpdateForce(Particle *particle, float duration) 
{
    float depth = particle->position.y;

    // �� ������ ������ �˻�
    if (depth >= waterHeight + maxDepth)
        return;

    // ħ������ ���
    Vector3 force(0.0f);
    if (depth <= waterHeight - maxDepth)
    {
        force.y = liquidDensity * volume;
        particle->AddForce(force);
        return;
    }

    // �κ������� ����� �ִ� ���
    force.y = liquidDensity * volume * (depth - maxDepth - waterHeight) / 2 * maxDepth;
    particle->AddForce(force);
}
