#pragma once

class BaseObject
{
    WCHAR m_name[MAX_NAME] = {L'\0'};

    Transform m_localTransform = Transform();

    int m_parentIndex = -1;
    int m_childCount = 0;

  public:
    BOOL Initialize(const WCHAR* name, const Transform* pLocalTransform, int parentIndex, int childCount);

    virtual void ReadFile(FILE *fp);
    virtual void WriteFile(FILE *fp);

    // Getter
    inline const WCHAR     *GetName() const { return m_name; }
    inline const Transform *GetLocalTransform() const { return &m_localTransform; }

    inline int GetParentIndex() const { return m_parentIndex; }
    inline int GetChildCount() const { return m_childCount; }

    // Setter
    void SetName(const WCHAR *name);

    inline void AddChildCount() { m_childCount++; }

    BaseObject() = default;
    virtual ~BaseObject(){};
};
