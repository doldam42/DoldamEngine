#pragma once
class DBVH
{
  public:
    BOOL Initialize(int maxObjectCount);
    BOOL InsertObject(const Bounds &b, void *pObj);
    void Build();
};
