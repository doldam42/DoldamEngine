#include "pch.h"

#include "Collider.h"
#include "RigidBody.h"

#include "Contact.h"

void ResolveContact(const Contact &contact)
{
    Collider *pColliderA = contact.pA;
    Collider *pColliderB = contact.pB;

    RigidBody *pA = pColliderA->pBody;
    RigidBody *pB = pColliderB->pBody;

    if (!pA || !pB)
        return;

    const Vector3 ptOnA = contact.contactPointAWorldSpace;
    const Vector3 ptOnB = contact.contactPointBWorldSpace;

    const float invMassA = pA->GetInvMass();
    const float invMassB = pB->GetInvMass();
    const float invMassAB = invMassA + invMassB;

    const float elasticity = pA->GetElasticity() * pB->GetElasticity();

    const Matrix invWorldInertiaA = pA->GetInverseInertiaTensorWorldSpace();
    const Matrix invWorldInertiaB = pB->GetInverseInertiaTensorWorldSpace();

    const Vector3 &n = contact.normal;

    const Vector3 ra = ptOnA - pA->GetCenterOfMassWorldSpace();
    const Vector3 rb = ptOnB - pB->GetCenterOfMassWorldSpace();

    const Vector3 angularJA = (Vector3::Transform(ra.Cross(n), invWorldInertiaA)).Cross(ra);
    const Vector3 angularJB = (Vector3::Transform(rb.Cross(n), invWorldInertiaB)).Cross(rb);
    const float   angularFactor = (angularJA + angularJB).Dot(n);

    const Vector3 velA = pA->GetLinearVelocity() + pA->GetAngularVelocity().Cross(ra);
    const Vector3 velB = pB->GetLinearVelocity() + pB->GetAngularVelocity().Cross(rb);

    const Vector3 vab = velA - velB;
    //const float   oldImpulse = contact.normalImpulse;

    float impulseJ = (1.0f + elasticity) * vab.Dot(n) / (invMassAB + angularFactor);
    /*contact.normalImpulse = max(oldImpulse + impulseJ, 0.0f);
    impulseJ -= oldImpulse;*/
    const Vector3 vectorImpulseJ = n * impulseJ;

    pA->ApplyImpulse(ptOnA, -vectorImpulseJ);
    pB->ApplyImpulse(ptOnB, vectorImpulseJ);

    const float frictionA = pA->GetFriction();
    const float frictionB = pB->GetFriction();
    const float friction = frictionA * frictionB;

    const Vector3 velNorm = n * n.Dot(vab);

    const Vector3 velTang = vab - velNorm;

    Vector3 relativeVelGTang = velTang;
    relativeVelGTang.Normalize();

    const Vector3 inertiaA = (Vector3::Transform(ra.Cross(relativeVelGTang), invWorldInertiaA)).Cross(ra);
    const Vector3 inertiaB = (Vector3::Transform(rb.Cross(relativeVelGTang), invWorldInertiaB)).Cross(rb);
    const float   invIntertia = (inertiaA * inertiaB).Dot(relativeVelGTang);

    const float   reduceMass = 1.0f / (invMassAB + invIntertia);
    const Vector3 impulseFrition = velTang * reduceMass * friction;

    pA->ApplyImpulse(ptOnA, -impulseFrition);
    pB->ApplyImpulse(ptOnB, impulseFrition);

    //
    // Let's also move our colliding objects to just outside of each other (projection method)
    //

    if (contact.timeOfImpact == 0.0f)
    {
        const Vector3 &n = contact.normal;
        const Vector3  ds = ptOnB - ptOnA;

        const float tA = invMassA / invMassAB;
        const float tB = invMassB / invMassAB;

        pColliderA->AddPosition(ds * tA);
        pColliderB->AddPosition(-ds * tB);
    }
}