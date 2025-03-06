#pragma once
class ControllerRegistry
{
    ControllerRegistry() = default;
    std::vector<IController *> controllers;

  public:
    static ControllerRegistry &GetInstance()
    {
        static ControllerRegistry instance;
        return instance;
    }
    void Register(IController *controller) { controllers.push_back(controller); }

    void RegisterAll(IGameManager *pGame)
    {
        for (IController *&c : controllers)
            pGame->Register(c);
    }
};
