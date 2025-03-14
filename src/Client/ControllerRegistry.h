#pragma once
#include <string_view>

class ControllerRegistry
{
    std::unordered_map<std::string_view, IController *> controllers;

  public:
    static ControllerRegistry &GetInstance()
    {
        static ControllerRegistry instance;
        return instance;
    }

    void Register(const std::string_view &name, IController *controller)
    {
        if (controllers.find(name) == controllers.end())
        {
            controllers.insert({name, controller});
        }
    }

    void RegisterAll(IGameManager *pGame)
    {
        for (auto &item : controllers)
        {
            pGame->Register(item.second);
        }
        controllers.clear();
    }
};

#define REGISTER_CONTROLLER(ClassName)                                                                                 \
    namespace                                                                                                          \
    {                                                                                                                  \
    struct ClassName##Registrar                                                                                        \
    {                                                                                                                  \
        ClassName _##ClassName##;                                                                                      \
        ClassName##Registrar() { ControllerRegistry::GetInstance().Register(#ClassName, &_##ClassName##); }            \
    };                                                                                                                 \
    static ClassName##Registrar g_##ClassName##Registrar;                                                              \
    }

template <typename T> class AutoRegisterController : public IController
{
  private:
    static T instance;

  public:
    AutoRegisterController() { ControllerRegistry::GetInstance().Register(&instance); }
};

template <typename T> T AutoRegisterController<T>::instance;