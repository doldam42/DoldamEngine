#pragma once
#include "Contact.h"
class Maniford
{
    static const int MAX_CONTACTS = 4;
    
    Contact          m_contacts[MAX_CONTACTS];

    int m_numContacts;

    RigidBody *m_pBodyA;
    RigidBody *m_pBodyB;

};
