#include "pch.h"

#include "BaseObject.h"

BOOL BaseObject::Initialize(const WCHAR *name, const Transform *pLocalTransform,
                            int parentIndex, int childCount)
{
    wcscpy_s(m_name, wcslen(name) + 1, name);
    m_localTransform = *pLocalTransform;
    m_parentIndex = parentIndex;
    m_childCount = childCount;
    return TRUE;
}

void BaseObject::WriteFile(FILE *fp) 
{ 
    fwrite(m_name, sizeof(m_name), 1, fp);
    fwrite(&m_localTransform, sizeof(Transform), 1, fp);
    fwrite(&m_parentIndex, sizeof(int), 1, fp);
    fwrite(&m_childCount, sizeof(int), 1, fp);
}

void BaseObject::SetName(const WCHAR *name) 
{
    memset(m_name, L'\0', sizeof(m_name));
    wcscpy_s(m_name, wcslen(name) + 1, name);
}

void BaseObject::ReadFile(FILE *fp) 
{
    fread(m_name, sizeof(m_name), 1, fp);
    fread(&m_localTransform, sizeof(Transform), 1, fp);
    fread(&m_parentIndex, sizeof(int), 1, fp);
    fread(&m_childCount, sizeof(int), 1, fp);
}
