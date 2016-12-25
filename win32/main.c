#define _WIN32_IE 0x0500
#define _WIN32_WINNT 0x0501
#define WINVER _WIN32_WINNT

#include <stdio.h>
#include <windows.h>
#include <commctrl.h>

#include "../core/core.h"



HRESULT APIENTRY WindowProc(HWND hWnd, UINT uMsg, WPARAM wPrm, LPARAM lPrm) {
    switch (uMsg) {
        case WM_CREATE:
            SetWindowLongPtr(hWnd, GWLP_USERDATA, 0);
            return 0;

        case WM_KEYUP:
        case WM_KEYDOWN: {
            ENGC *engc = (ENGC*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
            BOOL down = (uMsg == WM_KEYDOWN)? TRUE : FALSE;

            switch (wPrm & 0xFF) {
                case VK_LEFT:
                    cKbdInput(engc, KEY_LEFT, down);
                    break;

                case VK_RIGHT:
                    cKbdInput(engc, KEY_RIGHT, down);
                    break;

                case VK_UP:
                    cKbdInput(engc, KEY_UP, down);
                    break;

                case VK_DOWN:
                    cKbdInput(engc, KEY_DOWN, down);
                    break;

                case VK_F1:
                    cKbdInput(engc, KEY_F1, down);
                    break;

                case VK_F2:
                    cKbdInput(engc, KEY_F2, down);
                    break;

                case VK_F3:
                    cKbdInput(engc, KEY_F3, down);
                    break;

                case VK_F4:
                    cKbdInput(engc, KEY_F4, down);
                    break;

                case VK_F5:
                    cKbdInput(engc, KEY_F5, down);
                    break;

                case VK_SPACE:
                    cKbdInput(engc, KEY_SPACE, down);
                    break;

                case 'W':
                    cKbdInput(engc, KEY_W, down);
                    break;

                case 'S':
                    cKbdInput(engc, KEY_S, down);
                    break;

                case 'A':
                    cKbdInput(engc, KEY_A, down);
                    break;

                case 'D':
                    cKbdInput(engc, KEY_D, down);
                    break;
            }
            return 0;
        }
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP:
        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN: {
            ENGC *engc = (ENGC*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
            POINT movp;

            if ((uMsg == WM_LBUTTONDOWN)
            ||  (uMsg == WM_MBUTTONDOWN)
            ||  (uMsg == WM_RBUTTONDOWN))
                SetCapture(hWnd);
            else
                ReleaseCapture();
            GetCursorPos(&movp);
            ScreenToClient(hWnd, &movp);
            cMouseInput(engc, movp.x, movp.y,
                       ((uMsg == WM_LBUTTONDOWN)? 1 << 1 : 0) |
                       ((uMsg == WM_MBUTTONDOWN)? 1 << 2 : 0) |
                       ((uMsg == WM_RBUTTONDOWN)? 1 << 3 : 0));
            return 0;
        }
        case WM_MOUSEMOVE: {
            ENGC *engc = (ENGC*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
            POINT movp;

            GetCursorPos(&movp);
            ScreenToClient(hWnd, &movp);
            cMouseInput(engc, movp.x, movp.y,
                       ((wPrm & MK_LBUTTON)? 2 : 0) |
                       ((wPrm & MK_MBUTTON)? 4 : 0) |
                       ((wPrm & MK_RBUTTON)? 8 : 0) | 1);
            return 0;
        }
        case WM_SIZE: {
            ENGC *engc = (ENGC*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

            if (engc && (wPrm != SIZE_MINIMIZED)) {
                cResizeWindow(engc, LOWORD(lPrm), HIWORD(lPrm));
            }
            return 0;
        }
        case WM_CLOSE:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hWnd, uMsg, wPrm, lPrm);
}



int APIENTRY WinMain(HINSTANCE inst, HINSTANCE prev, LPSTR cmdl, int show) {
    INITCOMMONCONTROLSEX icct = {sizeof(icct), ICC_STANDARD_CLASSES};
    WNDCLASSEX wndc = {sizeof(wndc), CS_HREDRAW | CS_VREDRAW, WindowProc,
                       0, 0, GetModuleHandle(0), 0, LoadCursor(0, IDC_ARROW),
                       0, 0, " ", 0};
    PIXELFORMATDESCRIPTOR ppfd = {sizeof(ppfd), 1, PFD_SUPPORT_OPENGL |
                                  PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER,
                                  PFD_TYPE_RGBA, 32};
    RECT rect = {};
    MSG pmsg = {};
    HGLRC mwrc;
    HDC mwdc;
    HWND hwnd;
    ENGC *engc;

    int64_t oldt = 0, time = 0;
    int32_t lbit = 0, hbit = 0;

//    AllocConsole();
//    freopen("CONOUT$", "wb", stdout);

    InitCommonControlsEx(&icct);
    RegisterClassEx(&wndc);
    hwnd = CreateWindowEx(0, wndc.lpszClassName, 0, WS_TILEDWINDOW,
                          0, 0, 0, 0, 0, 0, wndc.hInstance, 0);
    SendMessage(hwnd, WM_SETICON, ICON_BIG,
               (LPARAM)LoadIcon(wndc.hInstance, MAKEINTRESOURCE(1)));

    mwdc = GetDC(hwnd);
    ppfd.iLayerType = PFD_MAIN_PLANE;
    SetPixelFormat(mwdc, ChoosePixelFormat(mwdc, &ppfd), &ppfd);
    wglMakeCurrent(mwdc, mwrc = wglCreateContext(mwdc));
    SetWindowLongPtr(hwnd, GWLP_USERDATA,
                    (LONG_PTR)(engc = cMakeEngine((intptr_t)mwdc)));

    rect.right = 800;
    rect.bottom = 600;
    rect.left = (GetSystemMetrics(SM_CXSCREEN) - rect.right) >> 1;
    rect.top = (GetSystemMetrics(SM_CYSCREEN) - rect.bottom) >> 1;
    SetWindowPos(hwnd, HWND_TOP, rect.left, rect.top,
                 rect.right, rect.bottom, SWP_SHOWWINDOW);

    while (pmsg.message != WM_QUIT) {
        if (PeekMessage(&pmsg, 0, 0, 0, PM_REMOVE)) {
            TranslateMessage(&pmsg);
            DispatchMessage(&pmsg);
            continue;
        }
        lbit = GetTickCount();
        if ((time < 0) && (lbit >= 0))
            hbit++;
        time = ((uint64_t)lbit & 0xFFFFFFFF) | ((uint64_t)hbit << 32);
        if (time - oldt >= DEF_UTMR) {
            cUpdateState(engc);
            oldt = time;
        }
        cRedrawWindow(engc);
        SwapBuffers((HDC)cGetUserData(engc));
    }
    cFreeEngine(&engc);
    wglMakeCurrent(0, 0);
    wglDeleteContext(mwrc);
    ReleaseDC(hwnd, mwdc);
    DeleteDC(mwdc);
    DestroyWindow(hwnd);

//    fclose(stdout);
//    FreeConsole();

    exit(0);
    return 0;
}
