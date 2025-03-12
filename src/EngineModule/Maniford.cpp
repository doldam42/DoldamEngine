#include "pch.h"
#include "Maniford.h"

void Maniford::AddContact(const Contact &contact_old) 
{ 
    // Make sure the contact's BodyA and BodyB are of the correct order
	Contact contact = contact_old;
	if (contact_old.pA != m_pBodyA || contact_old.pB != m_pBodyB)
	{
        contact.contactPointALocalSpace = contact_old.contactPointBLocalSpace;
        contact.contactPointBLocalSpace = contact_old.contactPointALocalSpace;
        contact.contactPointAWorldSpace = contact_old.contactPointBWorldSpace;
        contact.contactPointBWorldSpace = contact_old.contactPointAWorldSpace;
        
        contact.pA = m_pBodyA;
        contact.pB = m_pBodyB;
	}

    // If this contact is close to another contact, then keep the old contact
    for (int i = 0; i < m_numContacts; i++)
    {
        const RigidBody *bodyA = m_contacts[i].pA;
        const RigidBody *bodyB = m_contacts[i].pB;

        const Vector3 oldA = bodyA->LocalSpaceToWorldSpace(m_contacts[i].contactPointALocalSpace);
        const Vector3 oldB = bodyB->LocalSpaceToWorldSpace(m_contacts[i].contactPointBLocalSpace);

        const Vector3 newA = contact.pA->LocalSpaceToWorldSpace(contact.contactPointALocalSpace);
        const Vector3 newB = contact.pB->LocalSpaceToWorldSpace(contact.contactPointBLocalSpace);

        const Vector3 aa = newA - oldA;
        const Vector3 bb = newB - oldB;

        const float distanceThreshold = 0.02f;
        if (aa.LengthSquared() < distanceThreshold * distanceThreshold)
        {
            return;
        }
        if (bb.LengthSquared() < distanceThreshold * distanceThreshold)
        {
            return;
        }
    }

    // If we're all full on contacts, then keep the contacts that are furthest away from each other
    int newSlot = m_numContacts;
    if (newSlot >= MAX_CONTACTS)
    {
        Vector3 avg = Vector3::Zero;
        avg += m_contacts[0].contactPointALocalSpace;
        avg += m_contacts[1].contactPointALocalSpace;
        avg += m_contacts[2].contactPointALocalSpace;
        avg += m_contacts[3].contactPointALocalSpace;
        avg += contact.contactPointALocalSpace;
        avg *= 0.2f;

        float minDist = (avg - contact.contactPointALocalSpace).LengthSquared();
        int   newIdx = -1;
        for (int i = 0; i < MAX_CONTACTS; i++)
        {
            float dist2 = (avg - m_contacts[i].contactPointALocalSpace).LengthSquared();

            if (dist2 < minDist)
            {
                minDist = dist2;
                newIdx = i;
            }
        }

        if (-1 != newIdx)
        {
            newSlot = newIdx;
        }
        else
        {
            return;
        }
    }

    m_contacts[newSlot] = contact;

    if (newSlot == m_numContacts)
    {
        m_numContacts++;
    }
}

void Maniford::RemoveExpiredContacts() 
{
    // remove any contacts that have drifted too far
    for (int i = 0; i < m_numContacts; i++)
    {
        Contact &contact = m_contacts[i];

        RigidBody *bodyA = contact.pA;
        RigidBody *bodyB = contact.pB;

        // Get the tangential distance of the point on A and the point on B
        const Vector3 a = bodyA->LocalSpaceToWorldSpace(contact.contactPointALocalSpace);
        const Vector3 b = bodyB->LocalSpaceToWorldSpace(contact.contactPointBLocalSpace);

        Vector3 normal = Vector3::TransformNormal(contact.normal, Matrix::CreateFromQuaternion(bodyA->GetOrient()));

        // Calculate the tangential separation and penetration depth
        const Vector3 ab = b - a;
        float      penetrationDepth = normal.Dot(ab);
        Vector3    abNormal = normal * penetrationDepth;
        Vector3    abTangent = ab - abNormal;

        // If the tangential displacement is less than a specific threshold, it's okay to keep it
        const float distanceThreshold = 0.02f;
        if (abTangent.LengthSquared() < distanceThreshold * distanceThreshold && penetrationDepth <= 0.0f)
        {
            continue;
        }

        // This contact has moved beyond its threshold and should be removed
        for (int j = i; j < MAX_CONTACTS - 1; j++)
        {
            m_contacts[j] = m_contacts[j + 1];
        }
        m_numContacts--;
        i--;
    }
}
