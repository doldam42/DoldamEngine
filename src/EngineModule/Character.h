#pragma once
#include "GameObject.h"

class Character : public GameObject
{
  private:
    int m_clipId = 0;
    int frame = 0;

  public:
    void LateUpdate(float dt);

    Character() = default;
    ~Character();
};
