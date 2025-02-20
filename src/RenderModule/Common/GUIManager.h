#pragma once
#include <d3d12.h>

#include "../Common/RendererInterface.h"
// #include "RendererTypedef.h"

class SingleDescriptorAllocator;
struct ImGuiIO;
class GUIManager : public IRenderGUI
{
  public:
    const UINT  MAX_GUI_RESOURCE_COUNT = 64;
    const char *EMPTY_LABEL = "##xx";

  private:
    ID3D12Device *m_pD3DDevice = nullptr;

    ID3D12DescriptorHeap *m_pDescriptorHeap = nullptr;
    // Descriptor Index (0) is Reserved For ImGui Font Texture
    UINT                  m_reservedDescriptorCount = 1;
    UINT                  m_allocatedDescriptorCount = 1;
    UINT                  m_srvDescriptorSize = 0;

    DXGI_FORMAT m_rtvFormat;
    DXGI_FORMAT m_dsvFormat;

    UINT m_width = 0;
    UINT m_height = 0;

    ULONG m_refCount = 1;

    BOOL Alloc(D3D12_CPU_DESCRIPTOR_HANDLE *pOutCPUHandle, D3D12_GPU_DESCRIPTOR_HANDLE *pOutGPUHandle);

    BOOL InitDescriptorHeap();

    void Cleanup();

  public:
    BOOL Initialize(HWND hWnd, ID3D12Device5 *pD3DDevice, ID3D12CommandQueue *pCmdQueue, DXGI_FORMAT rtvFormat,
                    DXGI_FORMAT dsvFormat, UINT frameCount);

    // Inherited via IRenderGUI
    HRESULT __stdcall QueryInterface(REFIID riid, void **ppvObject) override;
    ULONG __stdcall AddRef(void) override;
    ULONG __stdcall Release(void) override;
    void BeginRender();
    void EndRender(ID3D12GraphicsCommandList *pCommandList);

    GUIManager() = default;
    ~GUIManager();

    // Inherited via IRenderGUI
    BOOL Begin(const char *name, bool *pOpen, GUI_WINDOW_FLAGS flags) override;
    void End() override;

    BOOL TreeNode(const char *name) override;
    void TreePop() override;

    void SameLine() override;
    void Text(const char *fmt) override;
    void SliderFloat(const char *label, float *v, float vMin, float vMax, const char *fmt) override;
    BOOL DragFloat3(const char *label, Vector3 *v, float delta = 0.1f) override;
    void CheckBox(const char *label, bool *v) override;
    BOOL Button(const char *label) override;
    BOOL ColoredButton(const char *label, RGBA color) override;
    BOOL InvisibleButton() override;

    void Image(ITextureHandle *pTexHanlde) override;

    LRESULT WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override;

    void OnUpdateWindowSize(UINT width, UINT height);

    // Inherited via IRenderGUI
    void SetNextWindowPosA(UINT posX, UINT posY, bool onlyAtFirst = false) override;
    void SetNextWindowSizeA(UINT width, UINT height, bool onlyAtFirst = false) override;

    // Inherited via IRenderGUI
    void SetNextWindowPosR(float posX, float posY, bool onlyAtFirst = false) override;
    void SetNextWindowSizeR(float width, float height, bool onlyAtFirst = false) override;

    // Inherited via IRenderGUI
    BOOL BeginMenuBar() override;
    void EndMenuBar() override;
    BOOL BeginMenu(const char *label) override;
    void EndMenu() override;

    BOOL MenuItem(const char *label, const char *shortcut) override;

    // Inherited via IRenderGUI
    BOOL BeginChild(const char *label, float width = 0.0f, float height = 0.0f) override;
    void EndChild() override;

    void DockSpace(const char *label) override;

    // Inherited via IRenderGUI
    void BeginGroup() override;
    void EndGroup() override;

    // Inherited via IRenderGUI
    BOOL BeginTabBar(const char *label) override;
    void EndTabBar() override;
    BOOL BeginTabItem(const char *label) override;
    void EndTabItem() override;

    // Inherited via IRenderGUI
    BOOL BeginDragDropSource() override;
    void EndDragDropSource() override;
    BOOL BeginDragDropTarget() override;
    void EndDragDropTarget() override;
    void SetDragDropPayload(const char *type, const void *data, size_t dataSize) override;
    BOOL AcceptDragDropPayload(const char *type, RENDER_GUI_PAYLOAD *pOutPayload) override;
};