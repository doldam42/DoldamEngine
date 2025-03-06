#pragma once
#include "AutoRegisterController.h"

class TimeController : public AutoRegisterController<TimeController>
{
  private:
  public:
    BOOL Start() override { return TRUE; }
    void Update(float dt) override;
    void Render() override {}

    TimeController() = default;
    ~TimeController(){};
};
