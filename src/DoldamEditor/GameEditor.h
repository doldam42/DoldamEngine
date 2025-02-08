#pragma once
#include <Windows.h>
#include "../../imgui/imgui.h"

class GameEditor
{
  private:
    ImGuiIO m_io;
    HWND m_hWnd = nullptr;
    bool m_show_demo_window = true;
    bool m_show_another_window = false;

    bool InitGUI();

    void UpdateGUI();

    void Cleanup();

  public:
    BOOL Initialize(HWND hWnd);
    void Process();

    GameEditor() = default;
    ~GameEditor();
};
