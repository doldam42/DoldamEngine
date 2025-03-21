#include "pch.h"
#include "Particle.h"
void Particle::AddForce(const Vector3 &force) { forceAccum += force; }
void Particle::Integrate(float dt)
{
    if (invMass == 0.0f)
        return;
    assert(dt > 0.0f);

    position += velocity * dt;
    position += acceleration * dt * dt * 0.5f;

    Vector3 resultAcc = acceleration;
    resultAcc += forceAccum * invMass;

    velocity += resultAcc * dt;
    velocity *= powf(damping, dt);

    ClearAccumulator();
}

void Particle::ClearAccumulator() { forceAccum = Vector3::Zero; }
