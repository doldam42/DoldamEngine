#pragma once

struct Particle
{
    Vector3 position;
    Vector3 velocity;
    Vector3 acceleration;

    float damping;  // ���� ex) ���� ����
    float invMass;
};
