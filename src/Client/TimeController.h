#pragma once

class TimeController : public IController
{
  private:
  public:
    // Inherited via IController
    BOOL Start() override;
    void Update(float dt) override;

    ~TimeController(){};
};
