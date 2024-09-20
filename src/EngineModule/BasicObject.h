#pragma once

#include <cstdio>
#include "../MathModule/MathHeaders.h"
#include "../RenderModule/RenderModule.h"

enum OBJECT_TYPE
{
    OBJECT_TYPE_MESH = 0,
    OBJECT_TYPE_CHARACTER,
};

struct OBJECT_HEADER
{
    OBJECT_TYPE Type;
    int         ParentIndex = 1;
    int         ChildCount = 0;
    Transform   TM;
    wchar_t     name[MAX_NAME] = {0};
};

class BasicObject
{
    OBJECT_HEADER m_header;

  public:
    virtual void WriteFile(FILE *fp);
    virtual void ReadFile(FILE *fp) = 0;

    inline OBJECT_TYPE GetType() const { return m_header.Type; }

    BasicObject(const OBJECT_HEADER &header) : m_header(header) {}

    const wchar_t *GetName();

    const Transform *GetTrasform();

    // return -1 if root object
    int GetParentIdx() const;

    void AddChildCount() { m_header.ChildCount++; }

    virtual void InitMeshHandle(IRenderer *pRnd, const Material *pMaterials, const wchar_t *basePath);

    virtual bool IsInFrustum(const Matrix &worldMat, const BoundingFrustum &frustum) const;

    virtual void Render(IRenderer *pRnd, const Matrix &worldMat);

    virtual ~BasicObject();
};
