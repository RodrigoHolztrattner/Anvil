//
// Copyright (c) 2017-2018 Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include "misc/window_win3264.h"
#include <sstream>
#include <shellscalingapi.h>

#define WM_DESTROY_WINDOW (WM_USER + 1)


/* See create() for documentation */
Anvil::WindowWin3264::WindowWin3264(const std::string&             in_title,
                                    unsigned int                   in_width,
                                    unsigned int                   in_height,
                                    bool                           in_closable,
                                    Anvil::PresentCallbackFunction in_present_callback_func, 
									Anvil::InputCallbacks          in_input_callback_collection)
    :Window(in_title,
            in_width,
            in_height,
            in_closable,
            in_present_callback_func, 
			in_input_callback_collection)
{
    m_window_owned = true;
}

/* See create() for documentation */
Anvil::WindowWin3264::WindowWin3264(HWND                           in_handle,
                                    const std::string&             in_title,
                                    unsigned int                   in_width,
                                    unsigned int                   in_height,
                                    Anvil::PresentCallbackFunction in_present_callback_func,
									Anvil::InputCallbacks          in_input_callback_collection)
    :Window(in_title,
            in_width,
            in_height,
            true, /* in_closable */
            in_present_callback_func, 
            in_input_callback_collection)
{
    m_window       = in_handle;
    m_window_owned = false;
}

/** Please see header for specification */
Anvil::WindowUniquePtr Anvil::WindowWin3264::create(const std::string&             in_title,
                                                    unsigned int                   in_width,
                                                    unsigned int                   in_height,
                                                    bool                           in_closable,
                                                    Anvil::PresentCallbackFunction in_present_callback_func,
													Anvil::InputCallbacks          in_input_callback_collection,
                                                    bool                           in_visible)
{
    WindowUniquePtr result_ptr(
        new Anvil::WindowWin3264(in_title,
                                 in_width,
                                 in_height,
                                 in_closable,
                                 in_present_callback_func, 
								 in_input_callback_collection),
        std::default_delete<Anvil::Window>()
    );

    if (result_ptr)
    {
        if (!dynamic_cast<WindowWin3264*>(result_ptr.get() )->init(in_visible) )
        {
            result_ptr.reset();
        }
    }

    return result_ptr;
}

/** Please see header for specification */
Anvil::WindowUniquePtr Anvil::WindowWin3264::create(HWND in_window_handle)
{
    WindowUniquePtr result_ptr (nullptr,
                                std::default_delete<Window>() );
    RECT            window_rect;
    uint32_t        window_size[2] = {0};

    /* The window has already been spawned by the user. Gather all the info we need in order to instantiate
     * the wrapper instance.
     */
    if (::IsWindow(in_window_handle) == 0)
    {
        anvil_assert_fail();

        goto end;
    }

    if (::GetClientRect(in_window_handle,
                       &window_rect) == 0)
    {
        anvil_assert_fail();

        goto end;
    }

    window_size[0] = static_cast<uint32_t>(window_rect.right  - window_rect.left);
    window_size[1] = static_cast<uint32_t>(window_rect.bottom - window_rect.top);

    /* Go ahead and create the window wrapper instance */
    result_ptr.reset(
        new Anvil::WindowWin3264(in_window_handle,
                                 "HWND wrapper window instance",
                                 window_size[0],
                                 window_size[1],
                                 nullptr, /* present_callback_func_ptr */
                                 Anvil::InputCallbacks()) /* _input_callback_func_ptr */
    );

    if (result_ptr)
    {
        if (!dynamic_cast<WindowWin3264*>(result_ptr.get() )->init(::IsWindowVisible(in_window_handle) == TRUE))
        {
            result_ptr.reset();
        }
    }

end:
    return result_ptr;
}

/** Please see header for specification */
void Anvil::WindowWin3264::close()
{
    anvil_assert(m_window_owned);

    if (!m_window_should_close)
    {
        m_window_should_close = true;

        /* NOTE: When the call below leaves, the window is guaranteed to be gone */
        ::SendMessage(m_window,
                      WM_DESTROY_WINDOW,
                      0,  /* wParam */
                      0); /* lParam */
    }
}

/** Creates a new system window and prepares it for usage. */
bool Anvil::WindowWin3264::init(const bool& in_visible)
{
    bool        result            = false;
    const char* window_class_name = (m_closable) ? "Anvil window (closable)"
                                                 : "Anvil window (non-closable)";

    if (m_window_owned)
    {
        std::stringstream class_name_sstream;
        HINSTANCE         instance           = ::GetModuleHandle(nullptr);
        WNDCLASSEX        window_class;
        RECT              window_rect;
        const uint32_t    window_size[2] =
        {
            m_width,
            m_height
        };


        // Initialize the window class structure:
        window_class.cbSize        = sizeof(WNDCLASSEX);
        window_class.style         = CS_HREDRAW | CS_VREDRAW;
        window_class.lpfnWndProc   = msg_callback_pfn_proc;
        window_class.cbClsExtra    = 0;
        window_class.cbWndExtra    = 0;
        window_class.hbrBackground = static_cast<HBRUSH>(::GetStockObject(WHITE_BRUSH) );
        window_class.hInstance     = instance;
        window_class.hIcon         = ::LoadIcon  (nullptr,  /* hInstance */
                                                  IDI_APPLICATION);
        window_class.hCursor       = ::LoadCursor(nullptr,  /* hInstance */
                                                  IDC_ARROW);
        window_class.lpszMenuName  = nullptr;
        window_class.lpszClassName = window_class_name;
        window_class.hIconSm       = ::LoadIcon(nullptr, /* hInstance */
                                                IDI_WINLOGO);

        if (!m_closable)
        {
            window_class.style |= CS_NOCLOSE;
        }

        /* Register window class. If more than one window is instantiated, this call may fail
         * but we don't really care.
         **/
        ::RegisterClassEx(&window_class);

		m_WindowCursor = window_class.hCursor;

        /* Create the window */
        window_rect.left   = 0;
        window_rect.top    = 0;
        window_rect.right  = static_cast<LONG>(window_size[0]);
        window_rect.bottom = static_cast<LONG>(window_size[1]);

        if (::AdjustWindowRect(&window_rect,
                               WS_OVERLAPPEDWINDOW,
                               FALSE /* bMenu */) == 0)
        {
            anvil_assert_fail();

            goto end;
        }

        /* NOTE: Anvil currently does not support automatic swapchain resizing so make sure the window is not scalable. */
        const auto visibility_flag = (in_visible) ? WS_VISIBLE : 0;

        m_window = ::CreateWindowEx(0,                    /* dwExStyle */
                                    window_class_name,
                                    m_title.c_str(),
                                    (WS_OVERLAPPEDWINDOW | visibility_flag | WS_SYSMENU) ^ WS_THICKFRAME,
                                    0, /* X */
                                    0, /* Y */
                                    window_rect.right - window_rect.left,
                                    window_rect.bottom - window_rect.top,
                                    nullptr, /* hWndParent */
                                    nullptr, /* hMenu */
                                    instance,
                                    nullptr); /* lParam */

        if (m_window == nullptr)
        {
            anvil_assert_fail();

            goto end;
        }
    }

    /* In order to verify if SetWindowLongPtr() has succeeded, we need to check the last thread-local error's value */
    ::SetLastError(0);

    if ((::SetWindowLongPtr(m_window,
                            GWLP_USERDATA,
                            reinterpret_cast<LONG_PTR>(this) ) == 0) &&
        (::GetLastError    ()                                  != 0) )
    {
        anvil_assert_fail();

        goto end;
    }

    result = true;
end:
    return result;
}

/** Window message handler.
 *
 *  @param in_window_handle Window handle.
 *  @param in_message_id    Message ID
 *  @param in_param_wide    Wide window message parameter.
 *  @param in_param_long    Long window message parameter.
 *
 *  @return Window message-specific return value.
 **/
LRESULT CALLBACK Anvil::WindowWin3264::msg_callback_pfn_proc(HWND   in_window_handle,
                                                             UINT   in_message_id,
                                                             WPARAM in_param_wide,
                                                             LPARAM in_param_long)
{
    WindowWin3264* window_ptr = reinterpret_cast<WindowWin3264*>(::GetWindowLongPtr(in_window_handle,
                                                                                    GWLP_USERDATA) );

	// Custom -> process the message
    if (window_ptr != nullptr)
    {
        window_ptr->process_message(in_window_handle,
                                    in_message_id,
                                    in_param_wide,
                                    in_param_long);
    }

    switch (in_message_id)
    {
        case WM_DESTROY:
        {
            window_ptr->m_window_should_close = true;

            ::PostQuitMessage(0);

            return 0;
        }

        case WM_CLOSE:
        case WM_DESTROY_WINDOW:
        {
            OnWindowAboutToCloseCallbackArgument callback_argument(window_ptr);

            window_ptr->callback(WINDOW_CALLBACK_ID_ABOUT_TO_CLOSE,
                                &callback_argument);

            ::DestroyWindow(window_ptr->m_window);

            break;
        }

        case WM_KEYUP:
        {
            OnKeypressReleasedCallbackArgument callback_data(window_ptr,
                                                             static_cast<Anvil::KeyID>(LOWORD(in_param_wide) & 0xFF) );

            window_ptr->callback(WINDOW_CALLBACK_ID_KEYPRESS_RELEASED,
                                &callback_data);

            return 0;
        }

        default:
        {
            break;
        }
    }

    return DefWindowProc(in_window_handle,
                         in_message_id,
                         in_param_wide,
                         in_param_long);
}

void Anvil::WindowWin3264::process_message(HWND   in_window_handle,
                                           UINT   in_message_id,
                                           WPARAM in_param_wide,
                                           LPARAM in_param_long)
{
	WORD x, y;

	x = LOWORD(in_param_long);
	y = HIWORD(in_param_long);

    auto GetInputMods = [&]()
    {
        uint32_t mods = 0;

        if (m_KeyStatus[static_cast<int>(InputKey::KEY_LEFT_SHIFT)] || m_KeyStatus[static_cast<int>(InputKey::KEY_RIGHT_SHIFT)])
        {
            mods = mods | static_cast<int>(InputMods::SHIFT);
        }

        if (m_KeyStatus[static_cast<int>(InputKey::KEY_LEFT_ALT)] || m_KeyStatus[static_cast<int>(InputKey::KEY_RIGHT_ALT)])
        {
            mods = mods | static_cast<int>(InputMods::ALT);
        }

        if (m_KeyStatus[static_cast<int>(InputKey::KEY_LEFT_CONTROL)] || m_KeyStatus[static_cast<int>(InputKey::KEY_RIGHT_CONTROL)])
        {
            mods = mods | static_cast<int>(InputMods::CONTROL);
        }

        if (m_KeyStatus[static_cast<int>(InputKey::KEY_LEFT_SUPER)] || m_KeyStatus[static_cast<int>(InputKey::KEY_RIGHT_SUPER)])
        {
            mods = mods | static_cast<int>(InputMods::SUPER);
        }

        return mods;
    };

	switch (in_message_id)
	{
        case WM_CHAR:
        {
            if (m_input_callback_collection.charCallback)
            {
                m_input_callback_collection.charCallback(static_cast<uint32_t>(in_param_wide));
            }

            break;
        }

	    case WM_KEYDOWN:
	    {
            if (static_cast<int>(in_param_wide) < static_cast<int>(InputKey::MAX_COUNT))
            {
                m_KeyStatus[static_cast<int>(in_param_wide)] = true;
            }

            if (m_input_callback_collection.keyCallback)
            {
                m_input_callback_collection.keyCallback(static_cast<InputKey>(in_param_wide), static_cast<uint32_t>(in_param_wide), Anvil::InputAction::PRESS, GetInputMods());
            }
    
		    break;
	    }

	    case WM_KEYUP:
	    {
            if (static_cast<int>(in_param_wide) < static_cast<int>(InputKey::MAX_COUNT))
            {
                m_KeyStatus[static_cast<int>(in_param_wide)] = false;
            }

            if (m_input_callback_collection.keyCallback)
            {
                m_input_callback_collection.keyCallback(static_cast<InputKey>(in_param_wide), static_cast<uint32_t>(in_param_wide), Anvil::InputAction::RELEASE, GetInputMods());
            }		
        
            break;
	    }

	    case WM_LBUTTONDOWN:
	    {
            if (m_input_callback_collection.mouseButtonCallback)
            {
                m_input_callback_collection.mouseButtonCallback(InputMouseButton::BUTTON_LEFT, InputAction::PRESS, GetInputMods());
            }
            
            break;
	    }

	    case WM_LBUTTONUP:
	    {
            if (m_input_callback_collection.mouseButtonCallback)
            {
                m_input_callback_collection.mouseButtonCallback(InputMouseButton::BUTTON_LEFT, InputAction::RELEASE, GetInputMods());
            }

            break;
	    }

	    case WM_RBUTTONDOWN:
	    {
            if (m_input_callback_collection.mouseButtonCallback)
            {
                m_input_callback_collection.mouseButtonCallback(InputMouseButton::BUTTON_RIGHT, InputAction::PRESS, GetInputMods());
            }

            break;
	    }

	    case WM_RBUTTONUP:
	    {
            if (m_input_callback_collection.mouseButtonCallback)
            {
                m_input_callback_collection.mouseButtonCallback(InputMouseButton::BUTTON_RIGHT, InputAction::RELEASE, GetInputMods());
            }

            break;
	    }

	    case WM_MBUTTONDOWN:
	    {
            if (m_input_callback_collection.mouseButtonCallback)
            {
                m_input_callback_collection.mouseButtonCallback(InputMouseButton::BUTTON_MIDDLE, InputAction::PRESS, GetInputMods());
            }

            break;
	    }

	    case WM_MBUTTONUP:
	    {
            if (m_input_callback_collection.mouseButtonCallback)
            {
                m_input_callback_collection.mouseButtonCallback(InputMouseButton::BUTTON_MIDDLE, InputAction::RELEASE, GetInputMods());
            }

            break;
	    }

	    case WM_MOUSEWHEEL:
	    {
		    // Determine the wheel roll amount
		    int amount = (int)(short)HIWORD(in_param_wide);
		    if (amount != 0) amount /= WHEEL_DELTA; // Adjust to -1~1 range

            if (m_input_callback_collection.scrollCallback)
            {
                m_input_callback_collection.scrollCallback(amount, 0);
            }

		    break;
	    }

        case WM_MOUSEMOVE:
        {
            if (m_input_callback_collection.cursorPosCallback)
            {
                POINT p;
                GetCursorPos(&p);
                ScreenToClient(in_window_handle, &p);

                m_input_callback_collection.cursorPosCallback(static_cast<uint32_t>(p.x), static_cast<uint32_t>(p.y));
            }

            break;
        }

        case WM_MOVING:
        {
            if (m_input_callback_collection.windowPosCallback)
            {
                m_input_callback_collection.windowPosCallback(static_cast<uint32_t>(x), static_cast<uint32_t>(y));
            }

            break;
        }

        case WM_SIZE:
        {
            if (m_input_callback_collection.windowSizeCallback)
            {
                m_input_callback_collection.windowSizeCallback(static_cast<uint32_t>(x), static_cast<uint32_t>(y));
            }

            break;
        }

        case WM_CLOSE:
        {
            if (m_input_callback_collection.windowCloseCallback)
            {
                m_input_callback_collection.windowCloseCallback();
            }

            break;
        }

        case WM_ACTIVATE:
        {
            if (m_input_callback_collection.windowFocusCallback)
            {
                // activation flag
                auto fActive = LOWORD(in_param_wide); 
                
                m_input_callback_collection.windowFocusCallback(static_cast<bool>(!(fActive == WA_INACTIVE)));
            }

            break;
        }

        case WM_SYSCOMMAND:
        {
            if ((in_param_wide & 0xFFF0) == SC_MINIMIZE)
            {
                m_minimized = true;

                if (m_input_callback_collection.windowMinimizeCallback)
                {
                    m_input_callback_collection.windowMinimizeCallback(true);
                }
            }
            else if ((in_param_wide & 0xFFF0) == SC_MAXIMIZE && m_input_callback_collection.windowMinimizeCallback)
            {
                m_minimized = false;

                if (m_input_callback_collection.windowMinimizeCallback)
                {
                    m_input_callback_collection.windowMinimizeCallback(false);
                }
            }

            break;
        }

        case WM_DROPFILES:
        {
            if (m_input_callback_collection.windowDropCallback)
            {
                TCHAR lpszFile[MAX_PATH] = { 0 };
                UINT uFile = 0;
                HDROP hDrop = (HDROP)in_param_wide;
                std::vector<std::wstring> files;

                uFile = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, NULL);

                for (unsigned i = 0; i < uFile; i++)
                {
                    lpszFile[0] = '\0';
                    if (DragQueryFile(hDrop, i, lpszFile, MAX_PATH))
                    {
#ifdef _UNICODE
                        files.push_back(lpszFile);
#else
                        // Need to convert the string
                        // First get the length needed
                        size_t length = mbstowcs(nullptr, lpszFile, 0);

                        // Allocate a temporary string
                        wchar_t* tmpstr = new wchar_t[length + 1];

                        // Do the actual conversion
                        mbstowcs(tmpstr, lpszFile, length + 1);

                        files.push_back(tmpstr);

                        // Free the temporary string
                        delete[] tmpstr;
#endif
                    }
                }

                DragFinish(hDrop);

                m_input_callback_collection.windowDropCallback(files);
            }
            
            break;
        }

        case WM_MOUSEHOVER:
        {
            m_hovered = true;

            break;
        }

        case WM_MOUSELEAVE:
        {
            m_hovered = false;

            break;
        }
	}
}

/* Please see header for specification */
void Anvil::WindowWin3264::run()
{
    if (m_manual_poll_render)
    {
        anvil_assert_fail();
    }

    int done = 0;

    /* This function should only be called for wrapper instances which have created the window! */
    anvil_assert(m_window_owned);

    /* Run the message loop */
    while (!done)
    {
        MSG msg;

        while (::PeekMessage(&msg,
                             nullptr,
                             0,
                             0,
                             PM_REMOVE) )
        {
            if (msg.message == WM_QUIT)
            {
                done = 1;

                break;
            }

            ::TranslateMessage(&msg);
            ::DispatchMessage (&msg);
        }

        if (msg.message == WM_QUIT)
        {
            done = 1;
        }
        else
        if (m_present_callback_func != nullptr)
        {
            m_present_callback_func();
        }
    }

    m_window_close_finished = true;
}

void Anvil::WindowWin3264::set_title(const std::string& in_new_title)
{
    /* This function should only be called for wrapper instances which have created the window! */
    anvil_assert(m_window_owned);

    ::SetWindowText(m_window,
                    in_new_title.c_str() );
}

void Anvil::WindowWin3264::get_cursor_position(uint32_t& x, uint32_t& y) const
{
	// Get the cursor position (currently only works on windows
	POINT p;
	GetCursorPos(&p);
	HWND Handle = WindowFromPoint(p);
	ScreenToClient(Handle, &p);

	// Set the mouse x and y
	x = static_cast<uint32_t>(p.x);
	y = static_cast<uint32_t>(p.y);
}

void Anvil::WindowWin3264::set_cursor_position(uint32_t x, uint32_t y)
{
	SetCursorPos(x, y);
}

void Anvil::WindowWin3264::show_cursor(bool show)
{
	if (show)
	{
		SetCursor(m_WindowCursor);
	}
	else
	{
		SetCursor(NULL);
	}
}

bool Anvil::WindowWin3264::get_is_focused() const
{
    return m_window == GetFocus();
}

void Anvil::WindowWin3264::set_focused()
{
    SetFocus(m_window);
}

bool Anvil::WindowWin3264::get_is_hovered() const
{
    return m_hovered;
}

bool Anvil::WindowWin3264::get_is_minimized() const
{
    return m_minimized;
}

float Anvil::WindowWin3264::get_opacity() const
{
    COLORREF pcrKey;
    BYTE pbAlpha;
    DWORD pdwFlags;
    GetLayeredWindowAttributes(m_window, &pcrKey, &pbAlpha, &pdwFlags);

    return min(1.0f, static_cast<float>(pbAlpha) / 255.0f);
}

void Anvil::WindowWin3264::set_opacity(float opacity)
{
    SetLayeredWindowAttributes(m_window, RGB(0, 0, 0), max(255, static_cast<BYTE>(opacity * 255.0)), LWA_ALPHA);
}

void Anvil::WindowWin3264::set_taskbar_visibility(bool visible)
{
    long style = GetWindowLong(m_window, GWL_STYLE);
    style &= ~(WS_VISIBLE);

    style |= WS_EX_TOOLWINDOW;

    if (visible)
    {
        style &= WS_EX_APPWINDOW;

    }
    else
    {
        style &= ~(WS_EX_APPWINDOW);
    }

    ShowWindow(m_window, SW_HIDE);
    SetWindowLong(m_window, GWL_STYLE, style);
    ShowWindow(m_window, SW_SHOW);

    if (!m_visible)
    {
        ShowWindow(m_window, SW_HIDE);
    }
}

void Anvil::WindowWin3264::set_visibility(bool visible)
{
    m_visible = visible;

    ShowWindow(m_window, visible ? SW_SHOW : SW_HIDE);
}

uint32_t Anvil::WindowWin3264::get_current_width()  const
{
    RECT rect;
    if (GetWindowRect(m_window, &rect))
    {
        return static_cast<uint32_t>(rect.right - rect.left);
    }

    return 0;
}

uint32_t Anvil::WindowWin3264::get_current_height() const
{
    RECT rect;
    if (GetWindowRect(m_window, &rect))
    {
        return static_cast<uint32_t>(rect.bottom - rect.top);
    }

    return 0;
}

uint32_t Anvil::WindowWin3264::get_current_x() const
{
    RECT rect;
    if (GetWindowRect(m_window, &rect))
    {
        return static_cast<uint32_t>(rect.left);
    }

    return 0;
}

uint32_t Anvil::WindowWin3264::get_current_y() const
{
    RECT rect;
    if (GetWindowRect(m_window, &rect))
    {
        return static_cast<uint32_t>(rect.top);
    }

    return 0;
}

void Anvil::WindowWin3264::set_pos(uint32_t x, uint32_t y)
{
    SetWindowPos(m_window,
                 NULL,
                 static_cast<int>(x), 
                 static_cast<int>(y),
                 static_cast<int>(get_current_width()), 
                 static_cast<int>(get_current_height()),
                 NULL);
}

/* Set the window size */
void Anvil::WindowWin3264::set_size(uint32_t width, uint32_t height)
{
    SetWindowPos(m_window,
                 NULL,
                 static_cast<int>(get_current_x()),
                 static_cast<int>(get_current_y()),
                 static_cast<int>(width),
                 static_cast<int>(height),
                 NULL);
}

/* Set this window's ordering */
void Anvil::WindowWin3264::set_ordering(WindowOrdering order)
{
    SetWindowPos(m_window,
        ((HWND)order),
                 static_cast<int>(get_current_x()),
                 static_cast<int>(get_current_y()),
                 static_cast<int>(get_current_width()),
                 static_cast<int>(get_current_height()),
                 NULL);
}

void Anvil::WindowWin3264::set_clipboard_text(std::string text)
{
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, text.size() + 1);
    memcpy(GlobalLock(hMem), text.data(), text.size() + 1);
    GlobalUnlock(hMem);
    OpenClipboard(0);
    EmptyClipboard();
    SetClipboardData(CF_TEXT, hMem);
    CloseClipboard();
}

std::wstring Anvil::WindowWin3264::get_clipboard_text() const
{
    // Try opening the clipboard
    if (!OpenClipboard(nullptr))
    {
#ifdef _UNICODE
        return "";
#else
        return L"";
#endif
    }

    // Get handle of clipboard object for ANSI text
    HANDLE hData = GetClipboardData(CF_TEXT);
    if (hData == nullptr)
    {
#ifdef _UNICODE
        return "";
#else
        return L"";
#endif
    }

    // Lock the handle to get the actual text pointer
    char* pszText = static_cast<char*>(GlobalLock(hData));
    if (pszText == nullptr)
    {
#ifdef _UNICODE
        return "";
#else
        return L"";
#endif
    }

    // Save text in a string class instance
    std::string text(pszText);

    // Release the lock
    GlobalUnlock(hData);

    // Release the clipboard
    CloseClipboard();

#ifdef _UNICODE
    return pszText;
#else
    // Need to convert the string
    // First get the length needed
    size_t length = mbstowcs(nullptr, pszText, 0);

    // Allocate a temporary string
    wchar_t* tmpstr = new wchar_t[length + 1];

    // Do the actual conversion
    mbstowcs(tmpstr, pszText, length + 1);

    std::wstring result = tmpstr;

    // Free the temporary string
    delete[] tmpstr;

    return result;
#endif
}

BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC, LPRECT lprcMonitor, LPARAM dwData)
{
    Anvil::MonitorInfo monitor_info;

    UINT dpiX;
    UINT dpiY;

    // DPI
    monitor_info.x_scale = 0; // TODO: Not necessary if optional is available
    monitor_info.y_scale = 0; // TODO: Not necessary if optional is available
    if(GetDpiForMonitor(hMonitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY) == S_OK)
    {
        monitor_info.x_scale = static_cast<uint32_t>(dpiX);
        monitor_info.y_scale = static_cast<uint32_t>(dpiY);
    }

    // Pos/size/primary
    {
        RECT Rect;
        MONITORINFO mi;
        mi.cbSize = sizeof(mi);
        Rect = *lprcMonitor;
        GetMonitorInfo(hMonitor, &mi);
        
        monitor_info.x = mi.rcMonitor.right;
        monitor_info.y = mi.rcMonitor.top;
        monitor_info.width = mi.rcMonitor.left - mi.rcMonitor.right;
        monitor_info.height = mi.rcMonitor.bottom - mi.rcMonitor.top;

        monitor_info.is_primary = (monitor_info.x == 0 && monitor_info.y == 0);
    }

    std::vector<Anvil::MonitorInfo>& monitors = *(std::vector<Anvil::MonitorInfo>*)dwData;
    monitors.push_back(monitor_info);

    return TRUE;
}

/* Returns a vector containing the information of all monitors */
std::vector<Anvil::MonitorInfo> Anvil::WindowWin3264::get_monitors() const
{
    std::vector<Anvil::MonitorInfo> monitors;

    if (!EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, (LPARAM)& monitors))
    {
        return {};
    }

    return monitors;
}

void Anvil::WindowWin3264::poll_events()
{
    if (!m_manual_poll_render)
    {
        anvil_assert_fail();
    }

    /* This function should only be called for wrapper instances which have created the window! */
    anvil_assert(m_window_owned);

    /* Run the message loop */
    MSG msg;
    bool wants_exit = false;

    while (::PeekMessage(&msg,
                            nullptr,
                            0,
                            0,
                            PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
        {
            wants_exit = true;

            break;
        }

        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
    }

    if (msg.message == WM_QUIT)
    {
        wants_exit = true;
    }
    else
    if (m_present_callback_func != nullptr)
    {
        m_present_callback_func();
    }
    
    if (wants_exit)
    {
        m_window_close_finished = true;
    }
}

/* Call the rendering callback associated with this window */
void Anvil::WindowWin3264::render()
{
    if (!m_manual_poll_render)
    {
        anvil_assert_fail();
    }

    if (m_present_callback_func != nullptr)
    {
        m_present_callback_func();
    }
}