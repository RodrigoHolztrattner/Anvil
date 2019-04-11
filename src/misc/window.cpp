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

#include "misc/debug.h"
#include "misc/window.h"

/** Please see header for specification */
Anvil::Window::Window(const std::string&      in_title,
                      unsigned int            in_width,
                      unsigned int            in_height,
                      bool                    in_closable,
                      PresentCallbackFunction in_present_callback_func,
                      InputCallbacks          in_input_callback_collection)
    :CallbacksSupportProvider(WINDOW_CALLBACK_ID_COUNT),
     m_closable                 (in_closable),
     m_height                   (in_height),
     m_present_callback_func    (in_present_callback_func),
     m_input_callback_collection(in_input_callback_collection),
     m_title                    (in_title),
     m_width                    (in_width),
     m_window_should_close      (false),
     m_window_close_finished    (false)
{
    /* Stub */
}

/** Destructor */
Anvil::Window::~Window()
{
    /* Stub */
}

void Anvil::Window::get_cursor_position(int32_t& x, int32_t& y) const
{
    x = 0;
    y = 0;
}

/* Set the window's cursor position */
void Anvil::Window::set_cursor_position(int32_t, int32_t)
{
    /* Stub */
}

/* Hide/show the window's cursor */
void Anvil::Window::show_cursor(bool)
{
    /* Stub */
}

/* Returns if the window is currently focused */
bool Anvil::Window::get_is_focused() const
{
    return false;
}

/* Force the window to be on focus */
void Anvil::Window::set_focused()
{
    /* Stub */
}

/* Returns if the window is currently focused */
bool Anvil::Window::get_is_hovered() const
{
    return false;
}

/* Returns if the window is currently minimized */
bool Anvil::Window::get_is_minimized() const
{
    return false;
}

/* Returns the window opacity */
float Anvil::Window::get_opacity() const
{
    return 1.0;
}

/* Set the window opacity */
void Anvil::Window::set_opacity(float)
{
    /* Stub */
}

/* Hide/show this window on the taskbar (if supported) */
void Anvil::Window::set_taskbar_visibility(bool, Window*)
{
    /* Stub */
}

/* Hide/show this window */
void Anvil::Window::set_visibility(bool)
{
    /* Stub */
};

/* Returns the current width/height for this window */
uint32_t Anvil::Window::get_current_width()  const
{
    return 0;
}

uint32_t Anvil::Window::get_current_height() const
{
    return 0;
}

/* Returns the current position for this window */
int32_t Anvil::Window::get_current_x() const
{
    return 0;
}

int32_t Anvil::Window::get_current_y() const
{
    return 0;
}

/* Set this window's ordering */
void Anvil::Window::set_ordering(WindowOrdering)
{
    /* Stub */
}

void Anvil::Window::set_pos(int32_t, int32_t)
{
    /* Stub */
}

/* Set the window size */
void Anvil::Window::set_size(uint32_t, uint32_t)
{
    /* Stub */
}

/* Set the clipboard text (if supported) */
void Anvil::Window::set_clipboard_text(std::string)
{
    /* Stub */
}

/* Returns the clipboard text (if supported) */
std::wstring Anvil::Window::get_clipboard_text() const
{
#ifdef _UNICODE
    return "";
#else
    return L"";
#endif
}

/* Returns a vector containing the information of all monitors */
std::vector<Anvil::MonitorInfo> Anvil::Window::get_monitors() const
{
    return {};
}

void Anvil::Window::poll_events(bool)
{
    /* Stub */
}

/* Call the rendering callback associated with this window */
void Anvil::Window::render()
{
    /* Stub */
}