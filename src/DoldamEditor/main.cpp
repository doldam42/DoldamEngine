// DoldamEditor.cpp : Defines the entry point for the application.
//

#include "pch.h"

#include "imgui.h"
#include "imgui_impl_win32.h"

#include "DoldamEditor.h"
#include "GUIController.h"
#include "GameEditor.h"

#define MAX_LOADSTRING 100

#pragma comment(lib, "DirectXTK12.lib")
#pragma comment(lib, "GenericModule.lib")
#pragma comment(lib, "MathModule.lib")

// Global Variables:

HINSTANCE hInst = nullptr; // current instance

GameEditor *g_pEditor = nullptr;

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
    LoadStringW(hInstance, IDC_DOLDAMEDITOR, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    g_hMainWindow = InitInstance(hInstance, nCmdShow);
    if (!g_hMainWindow)
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DOLDAMEDITOR));

    MSG msg;

    g_pEditor = new GameEditor;
    g_pEditor->Initialize(g_hMainWindow);

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
            g_pEditor->Process();
        }
    }

    if (g_pEditor)
    {
        delete g_pEditor;
        g_pEditor = nullptr;
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
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DOLDAMEDITOR));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_DOLDAMEDITOR);
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

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                              CW_USEDEFAULT, nullptr, nullptr, hInstance, nullptr);

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
    if (g_pEditor != nullptr && g_pEditor->WndProcHandler(hWnd, message, wParam, lParam))
        return true;
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
        // TODO
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
            // TODO
            if (vkCode == VK_ESCAPE)
            {
                DestroyWindow(hWnd);
                break;
            }
        }
    }
    break;

    case WM_KEYUP: {
        UINT uiScanCode = (0x00ff0000 & lParam) >> 16;
        UINT vkCode = MapVirtualKey(uiScanCode, MAPVK_VSC_TO_VK);
        // TODO
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
        // TODO
    }
    break;

    case WM_MOUSEWHEEL: {
        // TODO
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
