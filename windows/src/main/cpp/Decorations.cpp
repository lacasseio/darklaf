/*
 * MIT License
 *
 * Copyright (c) 2020 Jannis Weis
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */
#include "Decorations.h"
#include "com_github_weisj_darklaf_platform_windows_JNIDecorationsWindows.h"

#ifndef WM_NCUAHDRAWCAPTION
#define WM_NCUAHDRAWCAPTION (0x00AE)
#endif
#ifndef WM_NCUAHDRAWFRAME
#define WM_NCUAHDRAWFRAME (0x00AF)
#endif

std::map<HWND, WindowWrapper*> wrapper_map = std::map<HWND, WindowWrapper*>();

bool Maximized(HWND hwnd)
{
    WINDOWPLACEMENT placement;
    if (!GetWindowPlacement(hwnd, &placement))
        return false;
    return placement.showCmd == SW_MAXIMIZE;
}

LRESULT HandleHitTest(WindowWrapper *wrapper, int x, int y)
{
    if (wrapper->popup_menu)
        return HTCLIENT;

    POINT ptMouse = { x,
                      y };

    // Get the window rectangle.
    RECT rcWindow;
    GetWindowRect(wrapper->window, &rcWindow);

    // Determine if the hit test is for resizing. Default middle (1,1).
    USHORT uRow = 1;
    USHORT uCol = 1;

    if (!Maximized(wrapper->window))
    {
        /* The horizontal frame should be the same size as the vertical frame,
         since the NONCLIENTMETRICS structure does not distinguish between them */
        int frame_size = GetSystemMetrics(SM_CXFRAME) + GetSystemMetrics(SM_CXPADDEDBORDER);
        /* The diagonal size handles are wider than the frame */
        int diagonal_width = frame_size * 2 + GetSystemMetrics(SM_CXBORDER);

        bool top = ptMouse.y >= rcWindow.top && ptMouse.y < rcWindow.top + frame_size;
        bool bottom = !top && (ptMouse.y < rcWindow.bottom && ptMouse.y >= rcWindow.bottom - frame_size);

        bool left = ptMouse.x >= rcWindow.left && ptMouse.x < rcWindow.left + frame_size;
        bool right = !left && (ptMouse.x < rcWindow.right && ptMouse.x >= rcWindow.right - frame_size);

        bool diag_top = ptMouse.y >= rcWindow.top && ptMouse.y < rcWindow.top + diagonal_width;
        bool diag_bottom = !diag_top && (ptMouse.y < rcWindow.bottom && ptMouse.y >= rcWindow.bottom - diagonal_width);

        bool diag_left = ptMouse.x >= rcWindow.left && ptMouse.x < rcWindow.left + frame_size;
        bool diag_right = !diag_left && (ptMouse.x < rcWindow.right && ptMouse.x >= rcWindow.right - diagonal_width);

        if (top)
            uRow = 0;
        if (bottom)
            uRow = 2;

        if (top || bottom)
        {
            if (diag_left)
                uCol = 0;
            else if (diag_right)
                uCol = 2;
        }
        else
        {
            if (left)
                uCol = 0;
            if (right)
                uCol = 2;

            if (left || right)
            {
                if (diag_top)
                    uRow = 0;
                else if (diag_bottom)
                    uRow = 2;
            }
        }
    }

    // Hit test (HTTOPLEFT, ... HTBOTTOMRIGHT)
    LRESULT hitTests[3][3] = { { HTTOPLEFT,
                                 HTTOP,
                                 HTTOPRIGHT },
                               { HTLEFT,
                                 HTNOWHERE,
                                 HTRIGHT },
                               { HTBOTTOMLEFT,
                                 HTBOTTOM,
                                 HTBOTTOMRIGHT } };
    LRESULT hit = hitTests[uRow][uCol];
    if (hit == HTNOWHERE || !wrapper->resizable)
    {
        //Handle window drag.
        if (ptMouse.y < rcWindow.top + wrapper->title_height && ptMouse.x >= rcWindow.left + wrapper->left
            && ptMouse.x <= rcWindow.right - wrapper->right)
        {
            return HTCAPTION;
        }
        return HTCLIENT;
    }
    else
    {
        return hit;
    }
}

void UpdateRegion(WindowWrapper *wrapper)
{
    if (wrapper->popup_menu)
        return;
    RECT old_rgn = wrapper->rgn;

    if (Maximized(wrapper->window))
    {
        WINDOWINFO wi;
        wi.cbSize = sizeof(WINDOWINFO);
        GetWindowInfo(wrapper->window, &wi);

        /* For maximized windows, a region is needed to cut off the non-client
         borders that hang over the edge of the screen */
        wrapper->rgn.left = wi.rcClient.left - wi.rcWindow.left;
        wrapper->rgn.top = wi.rcClient.top - wi.rcWindow.top;
        wrapper->rgn.right = wi.rcClient.right - wi.rcWindow.left;
        wrapper->rgn.bottom = wi.rcClient.bottom - wi.rcWindow.top;

    }
    else
    {
        /* Don't mess with the region when composition is enabled and the
         window is not maximized, otherwise it will lose its shadow */
        wrapper->rgn.left = 0;
        wrapper->rgn.top = 0;
        wrapper->rgn.right = 0;
        wrapper->rgn.bottom = 0;
    }

    /* Avoid unnecessarily updating the region to avoid unnecessary redraws */
    if (EqualRect(&wrapper->rgn, &old_rgn))
        return;
    /* Treat empty regions as NULL regions */
    RECT empty = { 0 };
    if (EqualRect(&wrapper->rgn, &empty))
        SetWindowRgn(wrapper->window, NULL, TRUE);
    else
        SetWindowRgn(wrapper->window, CreateRectRgnIndirect(&wrapper->rgn), TRUE);
}

bool AutoHideTaskbar(UINT edge, RECT mon)
{
    APPBARDATA data;
    data.cbSize = sizeof(APPBARDATA);
    data.uEdge = edge;
    data.rc = mon;
    return SHAppBarMessage(ABM_GETAUTOHIDEBAREX, &data);
}

void AdjustMaximizedClientArea(HWND window, RECT &rect)
{
    if (!Maximized(window))
        return;

    HMONITOR monitor = MonitorFromWindow(window, MONITOR_DEFAULTTOPRIMARY);
    if (!monitor)
        return;

    MONITORINFO monitor_info {};
    monitor_info.cbSize = sizeof(MONITORINFO);
    if (!GetMonitorInfo(monitor, &monitor_info))
        return;

    rect = monitor_info.rcWork;
}

void HandleNCCalcSize(WindowWrapper *wrapper, WPARAM wparam, LPARAM lparam)
{
    union
    {
            LPARAM lparam;
            RECT *rect;
    } params;
    params.lparam = lparam;

    /* DefWindowProc must be called in both the maximized and non-maximized
     cases, otherwise tile/cascade windows won't work */
    RECT nonclient = *params.rect;
    DefWindowProc(wrapper->window, WM_NCCALCSIZE, wparam, lparam);
    RECT client = *params.rect;

    if (Maximized(wrapper->window))
    {
        WINDOWINFO wi;
        wi.cbSize = sizeof(wi);
        GetWindowInfo(wrapper->window, &wi);

        /* Maximized windows always have a non-client border that hangs over
         the edge of the screen, so the size proposed by WM_NCCALCSIZE is
         fine. Just adjust the top border to remove the window title. */
        (*params.rect).left = client.left;
        (*params.rect).top = nonclient.top + wi.cyWindowBorders;
        (*params.rect).right = client.right;
        (*params.rect).bottom = client.bottom;

        HMONITOR mon = MonitorFromWindow(wrapper->window, MONITOR_DEFAULTTOPRIMARY);
        MONITORINFO mi;
        mi.cbSize = sizeof(mi);
        GetMonitorInfoW(mon, &mi);

        /*
         * If the client rectangle is the same as the monitor's rectangle,
         * the shell assumes that the window has gone fullscreen, so it removes
         * the topmost attribute from any auto-hide appbars, making them
         * inaccessible. To avoid this, reduce the size of the client area by
         * one pixel on a certain edge. The edge is chosen based on which side
         * of the monitor is likely to contain an auto-hide appbar, so the
         * missing client area is covered by it.
         */
        if (EqualRect(params.rect, &mi.rcMonitor))
        {
            if (AutoHideTaskbar(ABE_BOTTOM, mi.rcMonitor))
                params.rect->bottom--;
            else if (AutoHideTaskbar(ABE_LEFT, mi.rcMonitor))
                params.rect->left++;
            else if (AutoHideTaskbar(ABE_TOP, mi.rcMonitor))
                params.rect->top++;
            else if (AutoHideTaskbar(ABE_RIGHT, mi.rcMonitor))
                params.rect->right--;
        }
    }
    else
    {
        /* For the non-maximized case, set the output RECT to what it was
         before WM_NCCALCSIZE modified it. This will make the client size the
         same as the non-client size. */
        *params.rect = nonclient;
    }
}

void HandleWindowPosChanged(WindowWrapper *wrapper, const WINDOWPOS *pos)
{
    RECT client;
    GetClientRect(wrapper->window, &client);
    LONG old_width = wrapper->width;
    LONG old_height = wrapper->height;
    wrapper->width = client.right;
    wrapper->height = client.bottom;
    bool client_changed = wrapper->width != old_width || wrapper->height != old_height;

    if (client_changed || (pos->flags & SWP_FRAMECHANGED))
        UpdateRegion(wrapper);
}

void PaintBackground(HWND hwnd, WPARAM wParam, WindowWrapper *wrapper)
{
    HDC hdc = reinterpret_cast<HDC>(wParam);
    RECT clientRect;
    GetClientRect(hwnd, &clientRect);
    FillRect(hdc, &clientRect, wrapper->bgBrush);
}

LRESULT CALLBACK WindowWrapper::WindowProc(_In_ HWND hwnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
    HWND handle = reinterpret_cast<HWND>(hwnd);
    auto wrapper = wrapper_map[handle];

    switch(uMsg)
    {
        case WM_NCACTIVATE:
        return TRUE;
        case WM_NCCALCSIZE:
        if (wParam == TRUE)
        {
            HandleNCCalcSize(wrapper, wParam, lParam);
            UpdateRegion(wrapper);
            return 0;
        }
        break;
        case WM_NCHITTEST:
        return HandleHitTest(wrapper, GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
        case WM_MOVE:
        wrapper->moving = wrapper->move_mode;
        break;
        case WM_ENTERSIZEMOVE:
        wrapper->move_mode = TRUE;
        break;
        case WM_EXITSIZEMOVE:
        wrapper->moving = FALSE;
        wrapper->move_mode = FALSE;
        break;
        case WM_ERASEBKGND:
        case WM_PAINT:
        if (!wrapper->bgBrush)
        break;
        if (!wrapper->moving)
        PaintBackground(hwnd, wParam, wrapper);
        if (uMsg == WM_ERASEBKGND)
        return TRUE;
        break;
        case WM_NCUAHDRAWCAPTION:
        case WM_NCUAHDRAWFRAME:
        /* These undocumented messages are sent to draw themed window borders.
         Block them to prevent drawing borders over the client area. */
        return 0;
        case WM_WINDOWPOSCHANGED:
        HandleWindowPosChanged(wrapper, (WINDOWPOS*)lParam);
        break;
        default:
        break;
    }

    return CallWindowProc(wrapper->prev_proc, hwnd, uMsg, wParam, lParam);
}

JNIEXPORT void JNICALL
Java_com_github_weisj_darklaf_platform_windows_JNIDecorationsWindows_setResizable(JNIEnv *env, jclass obj, jlong hwnd, jboolean res)
{
    HWND handle = reinterpret_cast<HWND>(hwnd);
    auto wrap = wrapper_map[handle];
    if (wrap)
    {
        wrap->resizable = res;
    }
}

JNIEXPORT void JNICALL
Java_com_github_weisj_darklaf_platform_windows_JNIDecorationsWindows_updateValues(JNIEnv *env, jclass obj, jlong hwnd,
        jint l, jint r, jint h)
{
    HWND handle = reinterpret_cast<HWND>(hwnd);
    auto wrap = wrapper_map[handle];
    if (wrap)
    {
        wrap->left = l;
        wrap->right = r;
        wrap->title_height = h;
    }
}

JNIEXPORT void JNICALL
Java_com_github_weisj_darklaf_platform_windows_JNIDecorationsWindows_setBackground(JNIEnv *env, jclass obj, jlong hwnd, jint r, jint g, jint b)
{
    HWND handle = reinterpret_cast<HWND>(hwnd);
    auto wrap = wrapper_map[handle];
    if (wrap)
    {
        wrap->bgBrush = CreateSolidBrush(RGB(r, g, b));
    }
}

void ExtendClientFrame(HWND handle)
{
    MARGINS margins = { 0,
                        0,
                        1,
                        0 };
    DwmExtendFrameIntoClientArea(handle, &margins);
}

void SetupWindowStyle(HWND handle, bool is_popup)
{
    auto style = GetWindowLongPtr(handle, GWL_STYLE);
    style |= WS_THICKFRAME;
    SetWindowLongPtr(handle, GWL_STYLE, style);
}

bool InstallDecorations(HWND handle, bool is_popup)
{
    //Prevent multiple installations overriding the real window procedure.
    auto it = wrapper_map.find(handle);
    if (it != wrapper_map.end())
        return false;

    SetupWindowStyle(handle, is_popup);
    ExtendClientFrame(handle);

    WNDPROC proc = reinterpret_cast<WNDPROC>(GetWindowLongPtr(handle, GWLP_WNDPROC));

    WindowWrapper *wrapper = new WindowWrapper();
    wrapper->window = handle;
    wrapper->prev_proc = proc;
    wrapper->popup_menu = is_popup;
    wrapper_map[handle] = wrapper;
    SetWindowLongPtr(handle, GWLP_WNDPROC, (LONG_PTR) WindowWrapper::WindowProc);
    UINT flags = SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED;
    SetWindowPos(handle, NULL, 0, 0, 0, 0, flags);
    return true;
}

JNIEXPORT jboolean JNICALL
Java_com_github_weisj_darklaf_platform_windows_JNIDecorationsWindows_installDecorations(JNIEnv *env, jclass obj, jlong hwnd)
{
    HWND handle = reinterpret_cast<HWND>(hwnd);
    return (jboolean) InstallDecorations(handle, false);
}

JNIEXPORT void JNICALL
Java_com_github_weisj_darklaf_platform_windows_JNIDecorationsWindows_uninstallDecorations(JNIEnv *env, jclass obj, jlong hwnd)
{
    HWND handle = reinterpret_cast<HWND>(hwnd);
    auto wrap = wrapper_map[handle];
    if (wrap)
    {
        SetWindowLongPtr(handle, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(wrap->prev_proc));
        wrapper_map.erase(handle);
        delete (wrap);
    }
}

JNIEXPORT jboolean JNICALL
Java_com_github_weisj_darklaf_platform_windows_JNIDecorationsWindows_installPopupMenuDecorations(JNIEnv *env, jclass obj, jlong hwnd)
{
    HWND handle = reinterpret_cast<HWND>(hwnd);
    return (jboolean) InstallDecorations(handle, true);
}

//Window functions.

JNIEXPORT void JNICALL
Java_com_github_weisj_darklaf_platform_windows_JNIDecorationsWindows_minimize(JNIEnv *env, jclass obj, jlong hwnd)
{
    HWND handle = reinterpret_cast<HWND>(hwnd);
    ShowWindow(handle, SW_MINIMIZE);
}

JNIEXPORT void JNICALL
Java_com_github_weisj_darklaf_platform_windows_JNIDecorationsWindows_maximize(JNIEnv *env, jclass obj, jlong hwnd)
{
    HWND handle = reinterpret_cast<HWND>(hwnd);
    ShowWindow(handle, SW_MAXIMIZE);
}

JNIEXPORT void JNICALL
Java_com_github_weisj_darklaf_platform_windows_JNIDecorationsWindows_restore(JNIEnv *env, jclass obj, jlong hwnd)
{
    HWND handle = reinterpret_cast<HWND>(hwnd);
    ShowWindow(handle, SW_RESTORE);
}
