#include "pch.h"

#include "Client.h"

#include "Resource.h"

#define MAX_LOADSTRING 100

// #pragma comment(linker, "/entry:wWinMainCRTStartup /subsystem:console")

#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avdevice.lib")
#pragma comment(lib, "avfilter.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "swresample.lib")
#pragma comment(lib, "swscale.lib")

#pragma comment(lib, "DirectXTK12.lib")
#pragma comment(lib, "GenericModule.lib")
#pragma comment(lib, "MathModule.lib")
#pragma comment(lib, "RenderModule.lib")
#pragma comment(lib, "EngineModule.lib")
#pragma comment(lib, "ModelExporter.lib")

// Global Variables:

HINSTANCE    hInst = nullptr; // current instance

HWND  g_hMainWindow = nullptr;
WCHAR szTitle[MAX_LOADSTRING];       // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING]; // the main window class name

// Forward declarations of functions included in this code module:
ATOM             MyRegisterClass(HINSTANCE hInstance);
HWND             InitInstance(HINSTANCE hInstance, int nCmdShow);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine,
                      _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.
#ifdef _DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MY01CREATEDEVICE, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    g_hMainWindow = InitInstance(hInstance, nCmdShow);
    if (!g_hMainWindow)
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MY01CREATEDEVICE));

    MSG msg;

    g_pClient = new Client;
    g_pClient->Initialize(g_hMainWindow);

    // Main message loop:
    while (1)
    {
        // call WndProc
        // g_bCanUseWndProc == FALSE이면 DefWndProc호출

        BOOL bHasMsg = PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE);

        if (bHasMsg)
        {
            if (msg.message == WM_QUIT)
            {
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            g_pClient->Process();
        }
    }

    if (g_pClient)
    {
        delete g_pClient;
        g_pClient = nullptr;
    }

#ifdef _DEBUG
    _ASSERT(_CrtCheckMemory());
#endif
    return (int)msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MY01CREATEDEVICE));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_MY01CREATEDEVICE);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
HWND InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // Store instance handle in our global variable

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr,
                              nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return nullptr;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return hWnd;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND: {
        int wmId = LOWORD(wParam);
        // Parse the menu selections:
        switch (wmId)
        {
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;

    case WM_SIZE: {
        if (g_pClient)
        {
            RECT rect;
            GetClientRect(hWnd, &rect);
            DWORD width = rect.right - rect.left;
            DWORD height = rect.bottom - rect.top;
            g_pClient->OnUpdateWindowSize(width, height);
        }
    }

    break;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC         hdc = BeginPaint(hWnd, &ps);
        // TODO: Add any drawing code that uses hdc here...
        EndPaint(hWnd, &ps);
    }
    break;

    case WM_KEYDOWN: {

        UINT uiScanCode = (0x00ff0000 & lParam) >> 16;
        UINT vkCode = MapVirtualKey(uiScanCode, MAPVK_VSC_TO_VK);

        if (!(lParam & 0x40000000))
        {
            if (g_pClient)
            {
                g_pClient->OnKeyDown(vkCode, uiScanCode);
            }
        }
    }
    break;

    case WM_KEYUP: {
        UINT uiScanCode = (0x00ff0000 & lParam) >> 16;
        UINT vkCode = MapVirtualKey(uiScanCode, MAPVK_VSC_TO_VK);
        if (g_pClient)
        {
            g_pClient->OnKeyUp(vkCode, uiScanCode);
        }
    }
    break;

    case WM_SYSKEYDOWN: {
        UINT uiScanCode = (0x00ff0000 & lParam) >> 16;
        UINT vkCode = MapVirtualKey(uiScanCode, MAPVK_VSC_TO_VK);
        BOOL bAltKeyDown = FALSE;
        if ((HIWORD(lParam) & KF_ALTDOWN))
        {
            bAltKeyDown = TRUE;
        }
        /*if (!g_pGame->OnSysKeyDown(vkCode, uiScanCode, bAltKeyDown))
        {
            DefWindowProc(hWnd, message, wParam, lParam);
        }*/
    }
    break;

    case WM_MOUSEMOVE: {
        if (g_pClient)
        {
            g_pClient->OnMouseMove(LOWORD(lParam), HIWORD(lParam));
        }
    }
    break;

    case WM_MOUSEWHEEL: {
        if (g_pClient)
        {
            float deltaWheel = (float)GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA;
            g_pClient->OnMouseWheel(deltaWheel);
        }
        break;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
