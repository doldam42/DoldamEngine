#pragma once

enum SHAPE_TYPE : UINT
{
    SHAPE_TYPE_NONE = 0,
    SHAPE_TYPE_BOX,
    SHAPE_TYPE_SPHERE
};

interface IShape
{
    virtual SHAPE_TYPE GetType() const = 0;
    virtual Vector3    GetCenterOfMass() const = 0;
    virtual Matrix     InertiaTensor() const = 0;
};