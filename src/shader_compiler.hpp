

#pragma once

#include <string>

#include <memory>

namespace sandbox
{
    enum class shader_type
    {
        vertex, fragment
    };

    class shader_compiler
    {
    public:
        shader_compiler();
        ~shader_compiler();
        shader_compiler(const shader_compiler&) = delete;
        shader_compiler& operator=(const shader_compiler&) = delete;
        shader_compiler(shader_compiler&&) = delete;
        shader_compiler& operator=(shader_compiler&&) = delete;

        std::string compile_shader(const std::string& source, shader_type type) const;
    };
}

