

#include "shader_compiler.hpp"

#include <glslang/Public/ShaderLang.h>
#include <SPIRV/GlslangToSpv.h>
#include <StandAlone/ResourceLimits.h>
#include <StandAlone/DirStackFileIncluder.h>

sandbox::shader_compiler::shader_compiler()
{
    glslang::InitializeProcess();
}


sandbox::shader_compiler::~shader_compiler()
{
    glslang::FinalizeProcess();
}


std::string sandbox::shader_compiler::compile_shader(const std::string& shader_source, sandbox::shader_type type) const
{
    EShLanguage lang;

    switch (type) {
        case shader_type::vertex:
            lang = EShLangVertex;
            break;
        case shader_type::fragment:
            lang = EShLangFragment;
            break;
        default:
            throw std::runtime_error("invalid shader type.");
    }

    glslang::TShader shader(lang);
    int ClientInputSemanticsVersion = 100; // maps to, say, #define VULKAN 100
    glslang::EShTargetClientVersion client_version = glslang::EShTargetVulkan_1_0;
    glslang::EShTargetLanguageVersion target_version = glslang::EShTargetSpv_1_0;

    shader.setEnvInput(glslang::EShSourceGlsl, lang, glslang::EShClientVulkan, ClientInputSemanticsVersion);
    shader.setEnvClient(glslang::EShClientVulkan, client_version);
    shader.setEnvTarget(glslang::EShTargetSpv, target_version);
    TBuiltInResource resources;
    resources = glslang::DefaultTBuiltInResource;
    EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);

    constexpr int32_t default_version = 100;

    DirStackFileIncluder includer;

    const auto* glsl_shader_source_c_str = shader_source.c_str();
    shader.setStrings(&glsl_shader_source_c_str, 1);
    std::string preprocessed;

    if (!shader.preprocess(&resources, default_version, ENoProfile, false, false, messages, &preprocessed, includer)) {
        throw std::runtime_error(shader.getInfoLog());
    }

    const char* preprocessed_c_str = preprocessed.c_str();
    shader.setStrings(&preprocessed_c_str, 1);

    if (!shader.parse(&resources, 100, false, messages)) {
        throw std::runtime_error(shader.getInfoLog());
    }

    glslang::TProgram program;
    program.addShader(&shader);

    if (!program.link(messages)) {
        throw std::runtime_error(shader.getInfoLog());
    }

    std::vector<uint32_t> spv;
    spv::SpvBuildLogger logger;
    glslang::SpvOptions spvOptions;
    glslang::GlslangToSpv(*program.getIntermediate(lang), spv, &logger, &spvOptions);

    std::string out;
    out.resize(spv.size() * sizeof(spv.front()));
    std::memcpy(out.data(), spv.data(), spv.size() * sizeof(spv.front()));

    return out;
}
