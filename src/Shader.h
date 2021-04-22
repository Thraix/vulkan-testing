#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include <vector>
#include <stdexcept>
#include <fstream>

#include "VulkanContext.h"
#include "Utils.h"

#include <shaderc/shaderc.h>

class Shader
{
  enum class ShaderType
  {
    Vertex = shaderc_glsl_default_vertex_shader,
    Fragment = shaderc_glsl_default_fragment_shader,
  };

  static shaderc_compiler_t compiler;
  static shaderc_compile_options_t compileOptions;

  private:
    VkShaderModule vertShaderModule;
    VkShaderModule fragShaderModule;

  public:
    Shader(const std::string& vertex, const std::string& fragment)
    {
      std::vector<char> data = readFile(vertex, ShaderType::Vertex);
      vertShaderModule = CreateShaderModule(readFile(vertex, ShaderType::Vertex));
      fragShaderModule = CreateShaderModule(readFile(fragment, ShaderType::Fragment));
    }

    virtual ~Shader()
    {
      vkDestroyShaderModule(VulkanContext::GetDevice(), vertShaderModule, nullptr);
      vkDestroyShaderModule(VulkanContext::GetDevice(), fragShaderModule, nullptr);
    }

    VkShaderModule GetVertexModule()
    {
      return vertShaderModule;
    }

    VkShaderModule GetFragmentModule()
    {
      return fragShaderModule;
    }

  private:
    VkShaderModule CreateShaderModule(const std::vector<char>& code)
    {
      VkShaderModuleCreateInfo createInfo = {};
      createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
      createInfo.codeSize = code.size();
      createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
      VkShaderModule shaderModule;
      if(vkCreateShaderModule(VulkanContext::GetDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
        throw std::runtime_error("Failed to create shader module!");

      return shaderModule;
    }

    static std::vector<char> readFile(const std::string& shaderFile, ShaderType type)
    {
      static shaderc_compiler_t compiler = shaderc_compiler_initialize();
      const std::string& spvFile= shaderFile + ".spv";
      std::ifstream file(spvFile, std::ios::ate | std::ios::binary);
      if(!file.is_open())
      {
        std::vector<char> shaderText = readFile(shaderFile);
        shaderc_compilation_result_t result = shaderc_compile_into_spv(compiler, shaderText.data(), shaderText.size(), (shaderc_shader_kind)type, shaderFile.c_str(), "main", compileOptions);
      (void)result;
        const char* c = shaderc_result_get_bytes(result);
        return std::vector<char>{c, c + shaderc_result_get_length(result)};
      }
      size_t fileSize = (size_t) file.tellg();
      std::vector<char> buffer(fileSize);
      file.seekg(0);
      file.read(buffer.data(), fileSize);
      file.close();
      return buffer;
    }

    static std::vector<char> readFile(const std::string& filename)
    {
      std::ifstream file(filename, std::ios::ate | std::ios::binary);
      if(!file.is_open())
      {
        throw std::runtime_error{"Failed to open file: " + filename};
      }
      size_t fileSize = (size_t) file.tellg();
      std::vector<char> buffer(fileSize);
      file.seekg(0);
      file.read(buffer.data(), fileSize);
      file.close();
      return buffer;
    }
};
