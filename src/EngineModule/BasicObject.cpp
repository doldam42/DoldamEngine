#include "pch.h"

#include "BasicObject.h"

void BasicObject::WriteFile(FILE *fp) { fwrite(&m_header, sizeof(OBJECT_HEADER), 1, fp); }

const wchar_t *BasicObject::GetName() { return m_header.name; }

const Transform *BasicObject::GetTrasform() { return &m_header.TM; }

int BasicObject::GetParentIdx() const { return m_header.ParentIndex; }

void BasicObject::InitMeshHandle(IRenderer *pRnd, const Material *pMaterials, const wchar_t *basePath)
{
    __debugbreak();
}

bool BasicObject::IsInFrustum(const Matrix &worldMat, const BoundingFrustum &frustum) const { return false; }

void BasicObject::Render(IRenderer *pRnd, const Matrix &worldMat) { }

BasicObject::~BasicObject() {}
