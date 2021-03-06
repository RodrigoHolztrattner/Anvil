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

/** Implements a simple window wrapper for Windows environments.
 *
 *  NOTE: This wrapper does not support scaling (yet).
 **/
#ifndef WINDOW_WIN3264_H
#define WINDOW_WIN3264_H

#include "misc/window.h"

namespace Anvil
{
    class WindowWin3264 : public Window
    {
    public:
        /* Public functions */

        /* Creates a window wrapper instance by opening a new system window.
         *
         * NOTE: This function resets that last system error assigned to the calling thread.
         *
         * @param in_title                 Title to use for the window.
         * @param in_width                 New window's width. Must not be 0.
         * @param in_height                New window's height. Must not be 0.
         * @param in_closable              Determines if window's close button should be greyed out, prohibiting
         *                                 the user from being able to destroy the window on their own.
         * @param in_present_callback_func Func pointer to a function which is going to render frame contents to
         *                                 the swapchain image. Must not be null.
         * @param in_visible               Should the window be made visible at creation time?
         *
         * @return New Anvil::Window instance if successful, or null otherwise.
         */
        static Anvil::WindowUniquePtr create(const std::string&             in_title,
                                             unsigned int                   in_width,
                                             unsigned int                   in_height,
                                             bool                           in_closable,
                                             Anvil::PresentCallbackFunction in_present_callback_func,
											 Anvil::InputCallbacks          in_input_callback_collection,
                                             bool                           in_visible);

        /* Creates a window wrapper instance from an existing window handle.
         *
         * It is assumed that:
         * 1) the application is going to run the message pump on its own.
         * 2) the application is going to explicitly call the presentation callback function at expose/paint/etc. system requests.
         * 3) the application only needs the wrapper instance for interaction with other Anvil wrappers (such as swapchains).
         *
         *
         * @param in_window_handle Existing, valid window handle.
         *
         * @return New Anvil::Window instance if successful, or null otherwise.
         */
        static Anvil::WindowUniquePtr create(HWND in_window_handle);

        virtual ~WindowWin3264(){ /* Stub */ }

        virtual void close(bool terminate = true);
        virtual void run();

        /* Returns window's platform */
        WindowPlatform get_platform() const
        {
            return WINDOW_PLATFORM_SYSTEM;
        }

        /* This function should never be called under Windows */
        virtual void* get_connection() const
        {
            anvil_assert_fail();

            return nullptr;
        }

        /** Changes the window title.
         *
         *  @param in_new_title Null-terminated string, holding the new title.
         */
        void set_title(const std::string& in_new_title) override;

		/* Returns window's cursor position */
		void get_cursor_position(int32_t& x, int32_t& y) const override;

		/* Set the window's cursor position */
		void set_cursor_position(int32_t x, int32_t y) override;

		/* Hide/show the window's cursor */
		void show_cursor(bool show) override;

        /* Returns if the window is currently focused */
        bool get_is_focused() const override;

        /* Force the window to be on focus */
        void set_focused() override;

        /* Returns if the window if currently hovered */
        bool get_is_hovered() const override;

        /* Returns if the window is currently minimized */
        bool get_is_minimized() const override;

        /* Returns the window opacity */
        float get_opacity() const override;

        /* Set the window opacity */
        void set_opacity(float opacity) override;

        /* Hide/show this window on the taskbar (if supported) */
        void set_taskbar_visibility(bool visible, Window* opt_parent_window) override;

        /* Hide/show this window */
        void set_visibility(bool visible) override;

        /* Returns the current width/height for this window */
        uint32_t get_current_width()  const override;
        uint32_t get_current_height() const override;

        /* Returns the current position for this window */
        int32_t get_current_x() const override;
        int32_t get_current_y() const override;

        /* Set this window's ordering */
        virtual void set_ordering(WindowOrdering order) override;

        /* Set the window position */
        virtual void set_pos(int32_t x, int32_t y) override;

        /* Set the window size */
        virtual void set_size(uint32_t width, uint32_t height) override;

        /* Set the clipboard text (if supported) */
        void set_clipboard_text(std::string text) override;

        /* Returns the clipboard text (if supported) */
        std::wstring get_clipboard_text() const override;

        /* Returns a vector containing the information of all monitors */
        virtual std::vector<MonitorInfo> get_monitors() const;

        /* Poll events for this window */
        virtual void poll_events(bool cursor_pass_through = false);

        /* Call the rendering callback associated with this window */
        virtual void render();

    private:
        /* Private functions */

        WindowWin3264(const std::string&             in_title,
                      unsigned int                   in_width,
                      unsigned int                   in_height,
                      bool                           in_closable,
                      Anvil::PresentCallbackFunction in_present_callback_func, 
					  Anvil::InputCallbacks          in_input_callback_collection);
        WindowWin3264(HWND                           in_handle,
                      const std::string&             in_title,
                      unsigned int                   in_width,
                      unsigned int                   in_height,
                      PresentCallbackFunction        in_present_callback_func, 
					  Anvil::InputCallbacks          in_input_callback_collection);

        /** Creates a new system window and prepares it for usage. */
        bool init(const bool& in_visible);

        static LRESULT CALLBACK msg_callback_pfn_proc(HWND   in_window_handle,
                                                      UINT   in_message_id,
                                                      WPARAM in_param_wide,
                                                      LPARAM in_param_long);

        static BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, 
                                             HDC hdcMonitor,
                                             LPRECT lprcMonitor,
                                             LPARAM dwData);

        void process_message(HWND in_window_handle,
                             UINT   in_message_id,
                             WPARAM in_param_wide,
                             LPARAM in_param_long);

		HCURSOR m_WindowCursor;
        std::array<bool, static_cast<int>(InputKey::MAX_COUNT)> m_KeyStatus;

        /* Private variables */
    };
}; /* namespace Anvil */

#endif /* WINDOW_WIN3264_H */

