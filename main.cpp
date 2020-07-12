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

#include <array>

constexpr auto vss = R"(
#version 450
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_color;

layout (location = 0) out vec3 v_color;

void main()
{
    v_color = a_color;
    gl_Position = vec4(a_pos, 1.0);
}
)";

constexpr auto fss = R"(
#version 450

layout (location = 0) in vec3 v_color;

layout (location = 0) out vec4 frag_color;

void main()
{
    frag_color = vec4(v_color, 1.0);
}
)";

constexpr float vert_data[][6] = {
    {-1, -1, 0, 1, 0, 0},
    {0, 1, 0, 0, 1, 0},
    {1, -1, 0, 0, 0, 1},
};

constexpr uint16_t i_data[] = {
    0, 1, 2
};


int main()
{
    // init
    sandbox::window window(800, 600);
    sandbox::shader_compiler compiler;

    WGPUSurfaceId surface = wgpu_create_surface_from_metal_layer(window.get_metal_layer());

    WGPURequestAdapterOptions const request_adapter_options =
        {
            .power_preference = WGPUPowerPreference_LowPower,
            .compatible_surface = surface,
        };

    WGPUAdapterId adapter = { 0 };

    wgpu_request_adapter_async(
        &request_adapter_options,
        2 | 4 | 8,
        []( WGPUAdapterId received, void *userdata ) { *(WGPUAdapterId*)userdata = received; },
        (void *)&adapter );


    WGPUDeviceDescriptor const device_desc =
        {
            .extensions = { .anisotropic_filtering = false, },
            .limits = { .max_bind_groups = 2 },
        };
    WGPUDeviceId wgpu_device = wgpu_adapter_request_device( adapter, &device_desc, nullptr );
    WGPUQueueId queue = wgpu_device_get_default_queue( wgpu_device );

    // buffers
    WGPUBufferDescriptor vbuf_descr
    {
        .size = sizeof(vert_data),
        .usage = WGPUBufferUsage_VERTEX
    };

    uint8_t * vert_staging_memory;
    auto vbuf = wgpu_device_create_buffer_mapped(wgpu_device, &vbuf_descr, &vert_staging_memory);
    std::memcpy(vert_staging_memory, vert_data, sizeof(vert_data));
    wgpu_buffer_unmap(vbuf);

    WGPUBufferDescriptor ibuf_descr {
        .size = sizeof(i_data),
        .usage = WGPUBufferUsage_VERTEX
    };

    uint8_t* ind_staging_memory;
    auto ibuf = wgpu_device_create_buffer_mapped(wgpu_device, &ibuf_descr, &ind_staging_memory);
    std::memcpy(ind_staging_memory, i_data, sizeof(i_data));
    wgpu_buffer_unmap(ibuf);

    // uniforms

    WGPUPipelineLayoutDescriptor unifrom_layout_desc {
        .bind_group_layouts_length = 0
    };

    auto unifrom_layout = wgpu_device_create_pipeline_layout(wgpu_device, &unifrom_layout_desc);
//
//    //shaders
//
    auto vss_spv = compiler.compile_shader(vss, sandbox::shader_type::vertex);
    auto fss_spv = compiler.compile_shader(fss, sandbox::shader_type::fragment);

    WGPUShaderModuleDescriptor vss_desc {
        .code = {
            .bytes = reinterpret_cast<uint32_t*>(vss_spv.data()),
            .length = vss_spv.size() / sizeof(uint32_t)
        }
    };

    WGPUShaderModuleDescriptor fss_desc {
        .code = {
            .bytes = reinterpret_cast<uint32_t*>(fss_spv.data()),
            .length = fss_spv.size() / sizeof(uint32_t)
        }
    };

    auto vs = wgpu_device_create_shader_module(wgpu_device, &vss_desc);
    auto fs = wgpu_device_create_shader_module(wgpu_device, &fss_desc);

    std::array<WGPUVertexAttributeDescriptor, 2> attrs {
        WGPUVertexAttributeDescriptor {
            .offset = 0,
            .format = WGPUVertexFormat_Float3,
            .shader_location = 0
        },
        {
            .offset = sizeof(float[3]),
            .format = WGPUVertexFormat_Float3,
            .shader_location = 1
        }
    };

    std::array<WGPUVertexBufferLayoutDescriptor, 1> pos_l_desc {
        WGPUVertexBufferLayoutDescriptor {
            .array_stride = sizeof(float[6]),
            .step_mode = WGPUInputStepMode_Vertex,
            .attributes = attrs.data(),
            .attributes_length = attrs.size()
        }
    };

    WGPUVertexStateDescriptor vert_state_desc {
        .index_format = WGPUIndexFormat_Uint16,
        .vertex_buffers = pos_l_desc.data(),
        .vertex_buffers_length = pos_l_desc.size()
    };

    WGPUProgrammableStageDescriptor v_stage {
        .module = vs,
        .entry_point = "main"
    };

    WGPUProgrammableStageDescriptor f_stage {
        .module = fs,
        .entry_point = "main"
    };

    WGPURasterizationStateDescriptor rastr_desc {
        .front_face = WGPUFrontFace_Ccw,
        .cull_mode = WGPUCullMode_None,
        .depth_bias = 0,
        .depth_bias_slope_scale = 0.0,
        .depth_bias_clamp = 0.0,
    };

    WGPUColorStateDescriptor color_state_desc {
        .format = WGPUTextureFormat_Bgra8UnormSrgb,
        .alpha_blend = {
            .src_factor = WGPUBlendFactor_SrcAlpha,
            .dst_factor = WGPUBlendFactor_OneMinusSrcAlpha,
            .operation = WGPUBlendOperation_Add,
        },
        .color_blend = {
            .src_factor = WGPUBlendFactor_SrcAlpha,
            .dst_factor = WGPUBlendFactor_OneMinusSrcAlpha,
            .operation = WGPUBlendOperation_Add,
        },
        .write_mask = WGPUColorWrite_ALL
    };

//    WGPUDepthStencilStateDescriptor depth_stencil_desc {
//        .format = WGPUTextureFormat_Depth24PlusStencil8,
//        .depth_write_enabled = true,
//        .depth_compare = WGPUCompareFunction_LessEqual,
//        .stencil_front =
//            {
//                .compare = WGPUCompareFunction_Always,
//                .fail_op = WGPUStencilOperation_Keep,
//                .depth_fail_op = WGPUStencilOperation_Keep,
//                .pass_op = WGPUStencilOperation_Keep,
//            },
//        .stencil_back =
//            {
//                .compare = WGPUCompareFunction_Always,
//                .fail_op = WGPUStencilOperation_Keep,
//                .depth_fail_op = WGPUStencilOperation_Keep,
//                .pass_op = WGPUStencilOperation_Keep,
//            },
//        .stencil_read_mask = 0,
//        .stencil_write_mask = 0,
//    };

WGPURenderPipelineDescriptor render_pipeline_desc {
        .layout = unifrom_layout,
        .vertex_stage = v_stage,
        .fragment_stage = &f_stage,
        .primitive_topology = WGPUPrimitiveTopology_TriangleList,
        .rasterization_state = &rastr_desc,
        .color_states = &color_state_desc,
        .color_states_length = 1,
        .depth_stencil_state = nullptr,
        .vertex_state = vert_state_desc,
        .sample_count = 1,
        .sample_mask = ~0u,
        .alpha_to_coverage_enabled = false,
    };

    auto pipeline = wgpu_device_create_render_pipeline(wgpu_device, &render_pipeline_desc);
//
//    // attachments
//
//    WGPUTextureDescriptor d_tex_desc {
//        .size = {800, 800, 1},
//        .mip_level_count = 1,
//        .sample_count = 1,
//        .dimension = WGPUTextureDimension_D2,
//        .format = WGPUTextureFormat_Depth24PlusStencil8,
//        .usage = WGPUTextureUsage_OUTPUT_ATTACHMENT
//    };
//
//    WGPUTextureViewDescriptor d_tex_view_desc {
//        .format = WGPUTextureFormat_Depth24PlusStencil8,
//        .dimension = WGPUTextureViewDimension_D2,
//        .aspect = WGPUTextureAspect_All,
//        .base_mip_level = 0,
//        .level_count = 1,
//        .base_array_layer = 0,
//        .array_layer_count = 1,
//    };
//
//    auto depth_tex = wgpu_device_create_texture(wgpu_device, &d_tex_desc);
//    auto depth_tex_view = wgpu_texture_create_view( wgpu_device, &d_tex_view_desc);
//
//    // commands
//
//    WGPURenderPassColorAttachmentDescriptor color_attachment_desc {
//        .attachment = uint32_t(-1),
//        .load_op = WGPULoadOp_Clear,
//        .store_op = WGPUStoreOp_Store,
//        .clear_color = {1, 1, 0, 1}
//    };
//
//    WGPURenderPassDepthStencilAttachmentDescriptor depth_attachment_desc {
//        .attachment = depth_tex_view,
//        .depth_load_op = WGPULoadOp_Clear,
//        .depth_store_op = WGPUStoreOp_Store,
//        .clear_depth = 1,
//        .stencil_load_op = WGPULoadOp_Clear,
//        .stencil_store_op = WGPUStoreOp_Store,
//        .clear_stencil = 0
//    };
//
//    WGPURenderPassDescriptor render_pass_desc {
//        .color_attachments = &color_attachment_desc,
//        .color_attachments_length = 1,
//        .depth_stencil_attachment = &depth_attachment_desc
//    };

    WGPUSwapChainId swapchain = {};

    int32_t last_width = -1;
    int32_t last_height = -1;

    while(!glfwWindowShouldClose(const_cast<GLFWwindow*>(window.get_window()))) {
        glfwPollEvents();
        int32_t w, h;
        glfwGetFramebufferSize(const_cast<GLFWwindow*>(window.get_window()), &w, &h);
        if (w != last_width || h != last_height) {
            last_width = w;
            last_height = h;

//            wgpu_texture_view_destroy(depth_tex_view);
//            wgpu_texture_destroy(depth_tex);
//
//            d_tex_desc.size = {.width = uint32_t(last_width), .height = uint32_t(last_height), .depth = 1u};
//            depth_tex = wgpu_device_create_texture(wgpu_device, &d_tex_desc);
//            depth_tex_view = wgpu_texture_create_view(depth_tex, &d_tex_view_desc);
//            depth_attachment_desc.attachment = depth_tex_view;

            WGPUSwapChainDescriptor swapchain_desc {
                .usage = WGPUTextureUsage_OUTPUT_ATTACHMENT,
                .format = WGPUTextureFormat_Bgra8UnormSrgb,
                .width = uint32_t(last_width),
                .height = uint32_t(last_height),
                .present_mode = WGPUPresentMode_Fifo,
            };

            swapchain = wgpu_device_create_swap_chain(wgpu_device, surface, &swapchain_desc);
        }

        const auto tex = wgpu_swap_chain_get_next_texture(swapchain);

        std::array<WGPURenderPassColorAttachmentDescriptor, 1> color_attachments = {
            WGPURenderPassColorAttachmentDescriptor {
                .attachment = tex.view_id,
                .resolve_target = 0,
                .load_op = WGPULoadOp_Clear,
                .store_op = WGPUStoreOp_Store,
                .clear_color = {0.1, 0.2, 0.3, 1.0}
            },
        };

        auto pass_desc = WGPURenderPassDescriptor {
            .color_attachments = color_attachments.data(),
            .color_attachments_length = color_attachments.size(),
            .depth_stencil_attachment = nullptr,
        };

        WGPUCommandEncoderDescriptor command_desc {
            .label = "encoder"
        };
        auto encoder = wgpu_device_create_command_encoder(wgpu_device, &command_desc);

        WGPURenderPassId pass = wgpu_command_encoder_begin_render_pass(encoder, &pass_desc);
        wgpu_render_pass_set_pipeline(pass, pipeline);
        wgpu_render_pass_set_vertex_buffer(pass, 0, vbuf, 0, sizeof(vert_data));
        wgpu_render_pass_draw(pass, 3, 1, 0, 0);
        wgpu_render_pass_end_pass(pass);
        WGPUCommandBufferId cmd_buf = wgpu_command_encoder_finish( encoder, nullptr );
        wgpu_queue_submit(queue, &cmd_buf, 1);
        wgpu_swap_chain_present(swapchain);
    }


    wgpu_buffer_destroy(vbuf);
    wgpu_buffer_destroy(ibuf);
    wgpu_pipeline_layout_destroy(unifrom_layout);
    wgpu_render_pipeline_destroy(pipeline);
    wgpu_shader_module_destroy(vs);
    wgpu_shader_module_destroy(fs);
    wgpu_adapter_destroy(adapter);

    return 0;
}
