#pragma once
#include "ControllerRegistry.h"

class TimeController : public IController // or AutoRegisterController<TimeController>
{
  private:
  public:
    BOOL Start() override { return TRUE; }
    void Update(float dt) override;

    TimeController() = default;
    ~TimeController(){};
};
