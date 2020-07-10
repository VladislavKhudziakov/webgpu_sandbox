

#include "window.hpp"

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_COCOA
#include <Foundation/Foundation.h>
#include <QuartzCore/CAMetalLayer.h>
#include <GLFW/glfw3native.h>

namespace sandbox
{
    struct window::window_impl
    {
        window_impl(size_t w, size_t h)
            : glfw_window(glfwCreateWindow(w, h, "wgpu", nullptr, nullptr)),
            ns_window(glfwGetCocoaWindow( glfw_window ))
        {
            assert(glfw_window != nullptr);

            metal_layer = nullptr;
            [ns_window.contentView setWantsLayer:YES];
            metal_layer = [CAMetalLayer layer];
            [ns_window.contentView setLayer:metal_layer];
        }

        ~window_impl() {
            glfwDestroyWindow(glfw_window);
        }

        GLFWwindow* glfw_window = nullptr;
        NSWindow* ns_window;
        id metal_layer;
    };
}


sandbox::window::window(size_t w, size_t h)
{
    const auto initialized = glfwInit();

    if (!initialized) {
        throw std::runtime_error("cannot init glfw");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    m_impl = std::make_unique<window_impl>(w, h);
}


const GLFWwindow* sandbox::window::get_window() const
{
    return m_impl->glfw_window;
}


void* sandbox::window::get_metal_layer()
{
    return m_impl->metal_layer;
}

sandbox::window::~window() = default;
