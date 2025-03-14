#pragma once

struct Particle
{
    Vector3 position;
    Vector3 velocity;
    Vector3 acceleration;

    float damping;  // 감쇠 ex) 공기 저항
    float invMass;
};
