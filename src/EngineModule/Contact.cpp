#include "pch.h"

#include "Contact.h"

void ResolveContact(Contact &contact)
{
    RigidBody *pA = contact.pA;
    RigidBody *pB = contact.pB;

    const Vector3 ptOnA = contact.contactPointAWorldSpace;
    const Vector3 ptOnB = contact.contactPointBWorldSpace;

    const float invMassA = pA->m_invMass;
    const float invMassB = pB->m_invMass;

    const float elasticity = pA->m_elasticity * pB->m_elasticity;

    const Matrix invWorldInertiaA = pA->GetInverseInertiaTensorWorldSpace();
    const Matrix invWorldInertiaB = pB->GetInverseInertiaTensorWorldSpace();

    const Vector3 &n = contact.normal;

    const Vector3 ra = ptOnA - pA->GetCenterOfMassWorldSpace();
    const Vector3 rb = ptOnB - pB->GetCenterOfMassWorldSpace();

    const Vector3 angularJA = (Vector3::Transform(ra.Cross(n), invWorldInertiaA)).Cross(ra);
    const Vector3 angularJB = (Vector3::Transform(rb.Cross(n), invWorldInertiaB)).Cross(rb);
    const float   angularFactor = (angularJA + angularJB).Dot(n);

    const Vector3 velA = pA->m_linearVelocity + pA->m_angularVelocity.Cross(ra);
    const Vector3 velB = pB->m_linearVelocity + pB->m_angularVelocity.Cross(rb);

    const Vector3 vab = velA - velB;
    const float   impulseJ = (1.0f + elasticity) * vab.Dot(n) / (invMassA + invMassB + angularFactor);
    const Vector3 vectorImpulseJ = n * impulseJ;

    pA->ApplyImpulse(ptOnA, -vectorImpulseJ);
    pB->ApplyImpulse(ptOnB, vectorImpulseJ);

    const float frictionA = pA->m_friction;
    const float frictionB = pB->m_friction;
    const float friction = frictionA * frictionB;

    const Vector3 velNorm = n * n.Dot(vab);

    const Vector3 velTang = vab - velNorm;

    Vector3 relativeVelGTang = velTang;
    relativeVelGTang.Normalize();

    const Vector3 inertiaA = (Vector3::Transform(ra.Cross(relativeVelGTang), invWorldInertiaA)).Cross(ra);
    const Vector3 inertiaB = (Vector3::Transform(rb.Cross(relativeVelGTang), invWorldInertiaB)).Cross(rb);
    const float   invIntertia = (inertiaA * inertiaB).Dot(relativeVelGTang);

    const float   reduceMass = 1.0f / (invMassA + invMassB + invIntertia);
    const Vector3 impulseFrition = velTang * reduceMass * friction;

    pA->ApplyImpulse(ptOnA, -impulseFrition);
    pB->ApplyImpulse(ptOnB, impulseFrition);

    //
    // Let's also move our colliding objects to just outside of each other (projection method)
    //
    if (contact.timeOfImpact == 0.0f)
    {
        const Vector3 ds = ptOnB - ptOnA;

        const float tA = invMassA / (invMassA + invMassB);
        const float tB = invMassB / (invMassA + invMassB);

        pA->AddPosition(ds * tA);
        pB->AddPosition(-ds * tB);
    }
}
