

#pragma once

#include <memory>

struct GLFWwindow;

namespace sandbox
{
    class window
    {
    public:
        window(size_t w, size_t h);
        ~window();
        const GLFWwindow* get_window() const;
        void* get_metal_layer();
    private:
        struct window_impl;
        std::unique_ptr<window_impl> m_impl;
    };
}

