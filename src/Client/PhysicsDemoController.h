#pragma once
class PhysicsDemoController : public IController
{
  public:
    // Inherited via IController
    BOOL Start() override;
    void Update(float dt) override;
    void Render() override;

    PhysicsDemoController() = default;
    ~PhysicsDemoController() {}
};
