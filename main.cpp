#include <iostream>

#include <window.hpp>
#include <shader_compiler.hpp>

#include <GLFW/glfw3.h>

#ifdef __cplusplus
extern "C"
{
#include <wgpu.h>
}
#endif


int main()
{
    sandbox::window window(800, 600);
    sandbox::shader_compiler compiler;

    WGPUSurfaceId surface = wgpu_create_surface_from_metal_layer(window.get_metal_layer());

    WGPURequestAdapterOptions const request_adapter_options =
        {
            .power_preference = WGPUPowerPreference_LowPower,
            .compatible_surface = surface,
        };

//    WGPUAdapterId adapter = { 0 };

//    wgpu_request_adapter_async(
//        &request_adapter_options,
//        2 | 4 | 8,
//        []( WGPUAdapterId received, void *userdata ) { *(WGPUAdapterId*)userdata = received; },
//        (void *)&adapter );
//
//
//    WGPUDeviceDescriptor const device_desc =
//        {
//            .extensions = { .anisotropic_filtering = false, },
//            .limits = { .max_bind_groups = 2 },
//        };
//    WGPUDeviceId device = wgpu_adapter_request_device( adapter, &device_desc, nullptr );
//    WGPUQueueId queue = wgpu_device_get_default_queue( device );
//    WGPUSwapChainId swapchain = {};

    while(!glfwWindowShouldClose(const_cast<GLFWwindow*>(window.get_window()))) {
        glfwPollEvents();
    }

//    wgpu_adapter_destroy(adapter);

    return 0;
}
