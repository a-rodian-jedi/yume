#include "platform/platform.h"
#include "core/logger.h"

#if YPLATFORM_WINDOWS

#include <Windows.h>
#include <windowsx.h>
#include <stdlib.h>

typedef struct internal_state {
    HINSTANCE h_instance;
    HWND hwnd;
} internal_state;

static f64 clock_frequency;
static LARGE_INTEGER start_time;

// win32 api setup

LRESULT CALLBACK win32_process_message(HWND hwnd, u32 msg, WPARAM w_param, LPARAM l_param);

b8 platform_startup(
    platform_state* pl_state,
    const char* application_name,
    i32 x,
    i32 y,
    i32 width,
    i32 height) {
        pl_state->internal_state = malloc(sizeof(internal_state));
        internal_state *state = (internal_state *)pl_state->internal_state;

        state->h_instance = GetModuleHandleA(0);

        // set up window class
        // WNDCLASSA
        HICON icon = LoadIcon(state->h_instance, IDI_APPLICATION);
        WNDCLASSA wc;
        memset(&wc, 0, sizeof(wc));                 // zero out memory
        wc.style = CS_DBLCLKS;                      // capture double clicks
        wc.lpfnWndProc = win32_process_message;     // pointer to window procedure (handles events)
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = state->h_instance;
        wc.hIcon = icon;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);   // manually manage cursor
        wc.hbrBackground = NULL;
        wc.lpszClassName = "yume_window_class";     // name window class

        // created the class, now register it
        if (!RegisterClassA(&wc)) {
            MessageBoxA(0, "Window registration failed", "Error", MB_ICONEXCLAMATION | MB_OK);
            return FALSE;
        }

        // create window
        u32 client_x = x;
        u32 client_y = y;
        u32 client_width = width;
        u32 client_height = height;

        u32 window_x = client_x;
        u32 window_y = client_y;
        u32 window_width = client_width;
        u32 window_height = client_height;

        u32 window_style = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION;
        u32 window_ex_style = WS_EX_APPWINDOW;

        window_style |= WS_MAXIMIZEBOX;
        window_style |= WS_MINIMIZEBOX;
        window_style |= WS_THICKFRAME;

        // get border size, because framing is not included
        // pain.
        RECT border_rect = {0, 0, 0, 0};
        AdjustWindowRectEx(&border_rect, window_style, 0, window_ex_style);

        window_x += border_rect.left;
        window_y += border_rect.top;

        // grow by size of border
        window_width += border_rect.right - border_rect.left;
        window_height += border_rect.bottom - border_rect.top;

        // finally do the actual creation
        HWND handle = CreateWindowExA(
            window_ex_style, "yume_window_class", application_name,
            window_style, window_x, window_y, window_width, window_height,
            0, 0, state->h_instance, 0);
        
        if (handle == 0) {
            MessageBoxA(NULL, "Window creation failed", "Error", MB_ICONEXCLAMATION | MB_OK);

            YFATAL("Window creation failed");
            return FALSE;
        } else {
            state->hwnd = handle;
        }

        // TODO: temporary value
        b32 should_activate = 1;
        i32 show_window_command_flags = should_activate ? SW_SHOW : SW_SHOWNOACTIVATE;
        ShowWindow(state->hwnd, show_window_command_flags);

        LARGE_INTEGER frequency;
        // get CPU speed
        QueryPerformanceFrequency(&frequency);
        // convert clock speed to f64 from int
        clock_frequency = 1.0 / (f64)frequency.QuadPart;
        // get current time
        QueryPerformanceCounter(&start_time);

        return TRUE;
}

void platform_shutdown(platform_state *pl_state) {
    internal_state *state = (internal_state *)pl_state->internal_state;

    if (state->hwnd) {
        DestroyWindow(state->hwnd);
        state->hwnd = 0;
    }
}

// process messages off the queue or window will lock up
// handles wc.lpfnWndProc = win32_process_message
b8 platform_pump_messages(platform_state* pl_state) {
    MSG message;

    while(PeekMessageA(&message, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&message);
        DispatchMessageA(&message);
    }

    return TRUE;
}

void* platform_allocate(u64 size, b8 aligned) {
    return malloc(size);
}

void platform_free(void* block, b8 aligned) {
    free(block);
}

void* platform_zero_memory(void* block, u64 size) {
    return memset(block, 0, size);
}

void* platform_copy_memory(void* dest, const void* source, u64 size) {
    return memcpy(dest, source, size);
}

void* platform_set_memory(void* dest, i32 value, u64 size) {
    return memset(dest, value, size);
}

// TODO: could be made to work with Windows Terminal/Powershell
void platform_console_write(const char* message, u8 color) {
    // stdout, cout in CPP
    HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    // color mapping
    static u8 levels[6] = {64, 4, 6, 2, 1, 8};
    SetConsoleTextAttribute(console_handle, levels[color]);
    OutputDebugStringA(message);
    u64 length = strlen(message);
    LPDWORD number_written = 0;
    WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), message, (DWORD)length, number_written, 0);
}

void platform_console_write_error(const char* message, u8 color) {
    // stderr, cerr in CPP
    HANDLE console_handle = GetStdHandle(STD_ERROR_HANDLE);
    // color mapping
    static u8 levels[6] = {64, 4, 6, 2, 1, 8};
    SetConsoleTextAttribute(console_handle, levels[color]);
    OutputDebugStringA(message);
    u64 length = strlen(message);
    LPDWORD number_written = 0;
    WriteConsole(GetStdHandle(STD_ERROR_HANDLE), message, (DWORD)length, number_written, 0);
}

f64 platform_get_absolute_time() {
    LARGE_INTEGER now_time;
    QueryPerformanceCounter(&now_time);
    return (f64)now_time.QuadPart * clock_frequency;
}

void platform_sleep(u64 ms) {
    Sleep(ms);
}

// event handler!
LRESULT CALLBACK win32_process_message(HWND hwnd, u32 msg, WPARAM w_param, LPARAM l_param) {
    switch (msg) {
        case WM_ERASEBKGND:
            // Notify OS that clearing screen will be handled by application so it does not
            // mess with Vulkan (causes flickering)
            return 1;
        case WM_CLOSE:
            // TODO: fire an event for the application to quit/clean up
            return 0;
        case WM_DESTROY:
            // adds WM_QUIT to stack
            PostQuitMessage(0);
            return 0;
        case WM_SIZE: {
            // RECT r;
            // GetClientRect(hwnd, &r);
            // u32 width = r.right - r.left;
            // u32 height = r.bottom - r.top;

            // TODO: resize window
        } break;
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP: {
            // b8 pressed = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);
            // TODO: process input
        } break;
        case WM_MOUSEMOVE: {
            // l_param stores mouse x,y
            // i32 x_pos = GET_X_LPARAM(l_param);
            // i32 y_pos = GET_Y_LPARAM(l_param);
            // TODO: process input
        } break;
        case WM_MOUSEHWHEEL: {
            // w_param stores mousewheel data
            // i32 z_delta = GET_WHEEL_DELTA_WPARAM(w_param);
            // if (z_delta != 0) {
            //     // flatten input (clamp) to -1 to 1
            //     z_delta = (z_delta < 0) ? -1 : 1;
            // }
            // TODO: process input
        } break;
        // mouse clicky
        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP: {
            // b8 pressed = msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN || msg == WM_MBUTTONDOWN;
            // TODO: input processing
        } break;
    }

    // anything we did not handle, handle it here
    return DefWindowProcA(hwnd, msg, w_param, l_param);
}

#endif
