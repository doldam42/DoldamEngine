#pragma once

#include "pch.h"
#include "ControllerRegistry.h"

template <typename T>
class AutoRegisterController : public IController
{
  private:
    static T instance;

  public:
    AutoRegisterController() 
    { 
        ControllerRegistry::GetInstance().Register(&instance);
    }
};

template<typename T> 
T AutoRegisterController<T>::instance;