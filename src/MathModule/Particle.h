#pragma once

struct Particle
{
    Vector3 position;
    Vector3 velocity;
    Vector3 acceleration;
    Vector3 forceAccum;

    float damping = 0.0f; // 감쇠 ex) 공기 저항
    float invMass = 0.0f;

    void Integrate(float dt);

    void ClearAccumulator();

    Particle(float mass, Vector3 pos, Vector3 velocity, Vector3 acc, float damping)
        : position(pos), velocity(velocity), acceleration(acc), damping(damping), invMass(1.0f / mass)
    {
    }
    Particle() = default;
};
