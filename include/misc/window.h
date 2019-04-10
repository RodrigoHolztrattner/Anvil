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

/** Implements a simple window wrapper for Windows and Linux (XCB) environments.
 *
 *  NOTE: This wrapper does not support scaling (yet).
 **/
#ifndef MISC_WINDOW_H
#define MISC_WINDOW_H

#include "misc/callbacks.h"
#include "misc/io.h"
#include "misc/ref_counter.h"
#include "misc/types.h"
#include "misc/debug.h"

namespace Anvil
{
	/** Prototype of functions, witch receive input and/or window updates */
    typedef std::function<void(InputMouseButton, InputAction, uint32_t)>   MouseButtonCallbackFunction;
    typedef std::function<void(uint32_t, uint32_t)>                        CursorPosCallbackFunction;
    typedef std::function<void(int, int)>                                  ScrollCallbackFunction;
    typedef std::function<void(InputKey, uint32_t, InputAction, uint32_t)> KeyCallbackFunction;
    typedef std::function<void(uint32_t)>                                  CharCallbackFunction;
    typedef std::function<void(uint32_t, uint32_t)>                        WindowPosCallbackFunction;
    typedef std::function<void(uint32_t, uint32_t)>                        WindowSizeCallbackFunction;
    typedef std::function<void()>                                          WindowCloseCallbackFunction;
    typedef std::function<void(bool)>                                      WindowFocusCallbackFunction;
    typedef std::function<void(bool)>                                      WindowMinimizeCallbackFunction;
    typedef std::function<void(std::vector<std::wstring>)>                 WindowDropCallbackFunction;

    /** Prototype of a function, which renders frame contents & presents it */
    typedef std::function<void()> PresentCallbackFunction;

    /** A collection of input callbacks */
    struct InputCallbacks
    {
        MouseButtonCallbackFunction    mouseButtonCallback;
        CursorPosCallbackFunction      cursorPosCallback;
        ScrollCallbackFunction         scrollCallback;
        KeyCallbackFunction            keyCallback;
        CharCallbackFunction           charCallback;
        WindowPosCallbackFunction      windowPosCallback;
        WindowSizeCallbackFunction     windowSizeCallback;
        WindowCloseCallbackFunction    windowCloseCallback;
        WindowFocusCallbackFunction    windowFocusCallback;
        WindowMinimizeCallbackFunction windowMinimizeCallback;
        WindowDropCallbackFunction     windowDropCallback;
    };

    /* Enumerates available window call-back types.*/
    enum WindowCallbackID
    {
        /* Call-back issued right before OS is requested to close the window.
         *
         * callback_arg: Pointer to OnWindowAboutToCloseCallbackArgument instance.
         */
        WINDOW_CALLBACK_ID_ABOUT_TO_CLOSE,

        /* Call-back issued when the user releases a pressed key.
         *
         * callback_arg: pointer to a OnKeypressReleasedCallbackArgument instance.
         **/
        WINDOW_CALLBACK_ID_KEYPRESS_RELEASED,

        /* Always last */
        WINDOW_CALLBACK_ID_COUNT
    };

    /* Enumerates available window call-back types.*/
    enum WindowPlatform
    {
        /* Stub window implementation - useful for off-screen rendering */
        WINDOW_PLATFORM_DUMMY,

        /* Stub window implementation - useful for off-screen rendering.
         *
         * This dummy window saves each "presented" frame in a PNG file. For that process to be successful,
         * the application MUST ensure the swapchain image is transitioned to Anvil::ImageLayout::GENERAL before
         * it is presented.
         **/
        WINDOW_PLATFORM_DUMMY_WITH_PNG_SNAPSHOTS,

    #ifdef _WIN32
        /* win32 */
        #if defined(ANVIL_INCLUDE_WIN3264_WINDOW_SYSTEM_SUPPORT)
            WINDOW_PLATFORM_SYSTEM,
        #endif

    #else
        #if defined(ANVIL_INCLUDE_XCB_WINDOW_SYSTEM_SUPPORT)
            /* linux xcb */
            WINDOW_PLATFORM_XCB,
        #endif

        /* linux xlib */
        WINDOW_PLATFORM_XLIB,

        /* linux xlib */
        WINDOW_PLATFORM_WAYLAND,
    #endif

        /* Always last */
        WINDOW_PLATFORM_COUNT,
        WINDOW_PLATFORM_UNKNOWN = WINDOW_PLATFORM_COUNT
    };

    class Window : public CallbacksSupportProvider
    {
    public:
        /* Public functions */

        /** Creates a single Window instance and presents it.
         *
         *  When returned execution, the created window will not be functional. In order for it
         *  to become responsive, a dedicated thread should invoke run() to host the message pump.
         *
         *  A window instance should be created in the same thread, from which the run() function
         *  is going to be invoked.
         *
         *  @param in_title                 Name to use for the window's title bar.
         *  @param in_width                 Window's width. Note that this value should not exceed screen's width.
         *  @param in_height                Window's height. Note that this value should not exceed screen's height.
         *  @param in_closable              Tells if the user should be able to close the button. Depending on the OS,
         *                                  this may translate to greyed out close button or close button clicks simply
         *                                  being ignored.
         *  @param in_present_callback_func Call-back to use in order to render & present the updated frame contents.
         *                                  Must not be nullptr.
         *
         **/
        Window(const std::string&      in_title,
               unsigned int            in_width,
               unsigned int            in_height,
               bool                    in_closable,
               PresentCallbackFunction in_present_callback_func, 
               InputCallbacks          in_input_callback_collection);

        virtual ~Window();

        /** Closes the window and unblocks the thread executing the message pump. */
        virtual void close() { /* Stub */ }

        /** Returns system XCB connection, should be used by linux only */
        virtual void* get_connection() const { return nullptr; }

        /** Returns system window handle. */
        WindowHandle get_handle() const
        {
            return m_window;
        }

        /** Returns window's height, as specified at creation time */
        uint32_t get_height_at_creation_time() const
        {
            return m_height;
        }

        /* Returns window's platform */
        virtual WindowPlatform get_platform() const = 0;

        /* Returns window's width */
        uint32_t get_width_at_creation_time() const
        {
            return m_width;
        }

		/* Returns window's cursor position */
        virtual void get_cursor_position(uint32_t& x, uint32_t& y) const;

		/* Set the window's cursor position */
        virtual void set_cursor_position(uint32_t, uint32_t);

		/* Hide/show the window's cursor */
        virtual void show_cursor(bool);

        /* Returns if the window is currently focused */
        virtual bool get_is_focused() const;

        /* Force the window to be on focus */
        virtual void set_focused();

        /* Returns if the window if currently hovered */
        virtual bool get_is_hovered() const;

        /* Returns if the window is currently minimized */
        virtual bool get_is_minimized() const;

        /* Returns the window opacity */
        virtual float get_opacity() const;

        /* Set the window opacity */
        virtual void set_opacity(float);

        /* Hide/show this window on the taskbar (if supported) */
        virtual void set_taskbar_visibility(bool visible, Window* opt_parent_window);

        /* Hide/show this window */
        virtual void set_visibility(bool visible);

        /* Returns the current width/height for this window */
        virtual uint32_t get_current_width()  const;
        virtual uint32_t get_current_height() const;

        /* Returns the current position for this window */
        virtual uint32_t get_current_x() const;
        virtual uint32_t get_current_y() const;

        /* Set this window's ordering */
        virtual void set_ordering(WindowOrdering order);

        /* Set the window position */
        virtual void set_pos(uint32_t x, uint32_t y);

        /* Set the window size */
        virtual void set_size(uint32_t width, uint32_t height);

        /* Set the clipboard text (if supported) */
        virtual void set_clipboard_text(std::string text);

        /* Returns the clipboard text (if supported) */
        virtual std::wstring get_clipboard_text() const;

        /* Returns a vector containing the information of all monitors */
        virtual std::vector<MonitorInfo> get_monitors() const;

        /* Set that this window should use manual rendering and event pooling */
        void set_manual_event_polling_and_rendering()
        {
            m_manual_poll_render = true;
        }

        /* Poll events for this window */
        virtual void poll_events(bool cursor_pass_through = false);

        /* Call the rendering callback associated with this window */
        virtual void render();

        /** Makes the window responsive to user's action and starts updating window contents.
         *
         *  This function will *block* the calling thread. To unblock it, call close().
         *
         *  This function can only be called once throughout Window instance's lifetime.
         *  This function can only be called for window instances which have opened a system window.
         *
         **/
        virtual void run() = 0;

        /** Changes the window title.
         *
         *  @param new_title Null-terminated string, holding the new title. Must not be NULL.
         */
        virtual void set_title(const std::string& in_new_title)
        {
            ANVIL_REDUNDANT_ARGUMENT_CONST(in_new_title);

            /* Nop by default */
        }

        /* Tells if the window closure process has finished */
        bool is_window_close_finished() const
        {
            return m_window_close_finished;
        }

        /* Returns if this window is polling its events with the cursor pass-through option */
        bool is_cursor_pass_through_set() const
        {
            return m_cursor_pass_through;
        }

    protected:
        /* protected variables */
        PresentCallbackFunction m_present_callback_func;
        InputCallbacks          m_input_callback_collection;

        bool            m_closable;
        unsigned int    m_height;
        std::string     m_title;
        unsigned int    m_width;
        volatile bool   m_window_should_close;
        volatile bool   m_window_close_finished;

        /* Window handle */
        WindowHandle    m_window;
        bool            m_window_owned;
        bool            m_hovered = false;
        bool            m_minimized = false;
        bool            m_visible = true;
        bool            m_manual_poll_render = false;
        bool            m_cursor_pass_through = false;

        /* protected functions */

    private:
        /* Private functions */

    /* Private variables */

    };
}; /* namespace Anvil */

#endif /* MISC_WINDOW_H */
