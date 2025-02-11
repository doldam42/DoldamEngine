#include <d3d12.h>
#include <d3dx12.h>

#include "imgui.h"
#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"

#include "GUIManager.h"

static D3D12_CPU_DESCRIPTOR_HANDLE g_cpuHandle = {};
static D3D12_GPU_DESCRIPTOR_HANDLE g_gpuHandle = {};

void GUIManager::Cleanup()
{
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

BOOL GUIManager::Initialize(HWND hWnd, ID3D12Device *pD3DDevice, ID3D12CommandQueue *pCmdQueue,
                            ID3D12DescriptorHeap *pHeap, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle,
                            D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle, DXGI_FORMAT rtvFormat, DXGI_FORMAT dsvFormat,
                            UINT frameCount)
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Ű���� ����
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // �����е� ����

    // Setup Dear ImGui style
    //ImGui::StyleColorsDark();
    ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hWnd);

    ImGui_ImplDX12_InitInfo init_info = {};
    init_info.Device = pD3DDevice;
    init_info.CommandQueue = pCmdQueue;
    init_info.NumFramesInFlight = frameCount;
    init_info.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    init_info.DSVFormat = DXGI_FORMAT_UNKNOWN;
    // Allocating SRV descriptors (for textures) is up to the application, so we provide callbacks.
    // (current version of the backend will only allocate one descriptor, future versions will need to allocate more)

    g_cpuHandle = cpuHandle;
    g_gpuHandle = gpuHandle;
    init_info.SrvDescriptorHeap = pHeap;
    init_info.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo *, D3D12_CPU_DESCRIPTOR_HANDLE *out_cpu_handle,
                                        D3D12_GPU_DESCRIPTOR_HANDLE *out_gpu_handle) {
        *out_cpu_handle = g_cpuHandle;
        *out_gpu_handle = g_gpuHandle;
    };
    init_info.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo *, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle,
                                       D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle) {};
    ImGui_ImplDX12_Init(&init_info);

    return TRUE;
}

HRESULT __stdcall GUIManager::QueryInterface(REFIID riid, void **ppvObject) { return E_NOTIMPL; }

ULONG __stdcall GUIManager::AddRef(void)
{
    m_refCount++;
    return m_refCount;
}

ULONG __stdcall GUIManager::Release(void)
{
    ULONG ref_count = --m_refCount;
    if (!m_refCount)
        delete this;

    return ref_count;
}

void GUIManager::BeginRender()
{
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void GUIManager::EndRender(ID3D12GraphicsCommandList *pCommandList)
{
    // Rendering
    ImGui::Render();

    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), pCommandList);
}

GUIManager::~GUIManager() { Cleanup(); }

void GUIManager::Begin(const char *name, bool showAnotherWindow) { ImGui::Begin(name, &showAnotherWindow); }

void GUIManager::End() { ImGui::End(); }

void GUIManager::Text(const char *txt) { ImGui::Text(txt); }

void GUIManager::SliderFloat(const char *label, float *v, float vMin, float vMax, const char *fmt)
{
    ImGui::SliderFloat(label, v, vMin, vMax, fmt);
}

void GUIManager::CheckBox(const char *label, bool *v) { ImGui::Checkbox(label, v); }

void GUIManager::Button(const char *label) { ImGui::Button(label); }

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT GUIManager::WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) 
{
    return ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
}
