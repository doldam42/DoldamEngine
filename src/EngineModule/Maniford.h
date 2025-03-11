#pragma once
#include "Contact.h"
class Maniford
{
    static const int MAX_CONTACTS = 4;

    Contact m_contacts[MAX_CONTACTS];

    int m_numContacts;

    RigidBody *m_pBodyA;
    RigidBody *m_pBodyB;

  public:
    /*void AddContact(const Contact &contact);
    void RemoveExpiredContacts();*/

    Contact GetContact(const int idx) const { return m_contacts[idx]; }
    int     GetNumContact() const { return m_numContacts; }
};
