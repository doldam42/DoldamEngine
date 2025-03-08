#pragma once
class PhysicsDemoController : public IController
{
  public:
    // Inherited via IController
    BOOL Start() override;
    void Update(float dt) override;

    PhysicsDemoController() = default;
    ~PhysicsDemoController() {}
};
