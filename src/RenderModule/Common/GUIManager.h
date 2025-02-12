#pragma once
#include <d3d12.h>

#include "../Common/RendererInterface.h"
//#include "RendererTypedef.h"

struct ImGuiIO;
class GUIManager : public IRenderGUI
{
  public:
    const char* EMPTY_LABEL = "##xx";

  private:
    ID3D12Device         *m_pD3DDevice = nullptr;
    ID3D12CommandQueue   *pCmdQueue = nullptr;
    ID3D12DescriptorHeap *m_pHeap = nullptr;

    DXGI_FORMAT m_rtvFormat;
    DXGI_FORMAT m_dsvFormat;

    UINT m_width = 0;
    UINT m_height = 0;

    ULONG m_refCount = 1;

    void Cleanup();

  public:
    BOOL Initialize(HWND hWnd, ID3D12Device *pD3DDevice, ID3D12CommandQueue *pCmdQueue, ID3D12DescriptorHeap *pHeap,
                    D3D12_CPU_DESCRIPTOR_HANDLE rtvCpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE rtvGpuHandle, DXGI_FORMAT rtvFormat,
                    DXGI_FORMAT dsvFormat, UINT frameCount);

    // Inherited via IRenderGUI
    HRESULT __stdcall QueryInterface(REFIID riid, void **ppvObject) override;
    ULONG __stdcall AddRef(void) override;
    ULONG __stdcall Release(void) override;
    void BeginRender();
    void EndRender(ID3D12GraphicsCommandList* pCommandList);

    GUIManager() = default;
    ~GUIManager();

    // Inherited via IRenderGUI
    BOOL Begin(const char *name, bool showAnotherWindow, GUI_WINDOW_FLAGS flags) override;
    void End() override;

    BOOL TreeNode(const char *name);
    void TreePop();

    void Text(const char *txt) override;
    void SliderFloat(const char *label, float *v, float vMin, float vMax, const char *fmt) override;
    void CheckBox(const char *label, bool *v) override;
    BOOL Button(const char *label) override;

    LRESULT WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override;

    void OnUpdateWindowSize(UINT width, UINT height);

    // Inherited via IRenderGUI
    void SetNextWindowPosA(UINT posX, UINT posY) override;
    void SetNextWindowSizeA(UINT width, UINT height) override;

    // Inherited via IRenderGUI
    void SetNextWindowPosR(float posX, float posY) override;
    void SetNextWindowSizeR(float width, float height) override;
};