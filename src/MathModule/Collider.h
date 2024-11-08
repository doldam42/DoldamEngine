#pragma once

struct Contact
{
    Vector3 worldSpaceA;
    Vector3 worldSpaceB;
    Vector3 localSpaceA;
    Vector3 localSpaceB;

    Vector3 normal;
    float   separationDistance;
    float   timeOfImpact;

    /*IShape *pShapeA;
    IShape *pShapeB;*/
};

BOOL CheckCollision(const Box &box, const Sphere &sphere, Contact *pOutContact);
BOOL CheckCollision(const Sphere &a, const Sphere &b, Contact *pOutContact);
//
//static BOOL IsIntersect(const IShape* pA, const IShape* pB, Contact* pOutContact)
//{
//    if (pA->GetType() == SHAPE_TYPE_NONE || pB->GetType() == SHAPE_TYPE_NONE)
//        return FALSE;
//
//    if (pA->GetType() == SHAPE_TYPE_BOX)
//    {
//        Box cur;
//        GetBoxInWorld(&cur);
//        if (pB->GetType() == SHAPE_TYPE_BOX)
//        {
//            Box other;
//            pOther->GetBoxInWorld(&other);
//            return cur.Intersect(other);
//        }
//        else
//        {
//            Sphere  other;
//            Contact c;
//            pOther->GetSphereInWorld(&other);
//            return CheckCollision(cur, other, &c);
//        }
//    }
//    else
//    {
//        Sphere cur;
//        GetSphereInWorld(&cur);
//        if (pB->GetType() == SHAPE_TYPE_BOX)
//        {
//            Box     other;
//            Contact c;
//            pOther->GetBoxInWorld(&other);
//            return CheckCollision(other, cur, &c);
//        }
//        else
//        {
//            Sphere  other;
//            Contact c;
//            pOther->GetSphereInWorld(&other);
//            return CheckCollision(cur, other, &c);
//        }
//    }
//}