#pragma once

#include <string_view>

interface IScene
{
    virtual void Load() = 0;
    virtual void Update(float dt) = 0;
    virtual void UnLoad() = 0;
    virtual ~IScene() = default;
};

class SceneRegistry
{
    std::unordered_map<std::string_view, IScene *> scenes;

    IScene *currentScene = nullptr;

  public:
    static SceneRegistry &GetInstance()
    {
        static SceneRegistry instance;
        return instance;
    }

    void Register(const std::string_view &name, IScene *scene)
    {
        if (scenes.find(name) == scenes.end())
        {
            scenes.insert({name, scene});
        }
    }

    inline void ChangeScene(const std::string_view &name)
    {
        UnLoad();
        currentScene = scenes[name];
        currentScene->Load();
    }
    inline void Update(float dt) { currentScene->Update(dt); }
    inline void UnLoad()
    {
        if (currentScene)
        {
            currentScene->UnLoad();
            currentScene = nullptr;
        }
    }

    ~SceneRegistry()
    {
    }
};

#define REGISTER_SCENE(ClassName)                                                                                      \
    namespace                                                                                                          \
    {                                                                                                                  \
    struct ClassName##Registrar                                                                                        \
    {                                                                                                                  \
        ClassName _##ClassName##;                                                                                      \
        ClassName##Registrar() { SceneRegistry::GetInstance().Register(#ClassName, &_##ClassName##); }                 \
    };                                                                                                                 \
    static ClassName##Registrar g_##ClassName##Registrar;                                                              \
    }
