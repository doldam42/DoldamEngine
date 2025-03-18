#pragma once
#include "Contact.h"

struct Maniford
{
    static const int MAX_CONTACTS = 4;

    Contact m_contacts[MAX_CONTACTS];
    int     m_numContacts = 0;

    RigidBody *m_pBodyA = nullptr;
    RigidBody *m_pBodyB = nullptr;

    Contact GetContact(const int idx) const { return m_contacts[idx]; }
    int     GetNumContact() const { return m_numContacts; }

    void AddContact(const Contact &contact);
    void RemoveExpiredContacts();
};

struct ManifordKey
{
    ManifordKey(RigidBody *pA, RigidBody *pB) { key = (uintptr_t)pA ^ (uintptr_t)pB; }

    inline bool operator==(const ManifordKey &rhs) { return key == rhs.key; }
    inline bool operator!=(const ManifordKey &rhs) const { return !(this->key == rhs.key); }
    uintptr_t   key;
};

inline bool operator<(const ManifordKey &a1, const ManifordKey &a2) { return a1.key < a2.key; }