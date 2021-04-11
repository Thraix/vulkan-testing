#pragma once

#include "VulkanHandle.h"
#include "VulkanContext.h"
#include "Texture.h"
#include <iostream>
#include <vector>
#include <set>
#include <cstring>
#include <stdlib.h>
#include <fstream>
#include <functional>
#include <math/Maths.h>
#include <chrono>
#include <ImageUtils.h>
#include <memory>

const uint32_t DEFAULT_WIDTH = 1280;
const uint32_t DEFAULT_HEIGHT = 720;

const int MAX_FRAMES_IN_FLIGHT = 2;

struct Vertex
{
  Greet::Vec3 position;
  Greet::Vec3 color;
  Greet::Vec2 texCoord;

  static VkVertexInputBindingDescription GetBindingDescription()
  {
    VkVertexInputBindingDescription bindingDescription = {};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescription;

  };

  static std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescriptions()
  {
    std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = {};
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, position);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, color);

    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

    return attributeDescriptions;
  };
};

struct UniformBufferObject
{
  Greet::Mat4 model;
  Greet::Mat4 view;
  Greet::Mat4 proj;
};

const std::vector<Vertex> vertices = {
  {{-0.5f, -0.5f, 0.0f}, {1.0f,1.0f,1.0f}, {0.0f,0.0f}},
  {{ 0.5f, -0.5f, 0.0f}, {1.0f,1.0f,1.0f}, {1.0f,0.0f}},
  {{-0.5f,  0.5f, 0.0f}, {1.0f,1.0f,1.0f}, {0.0f,1.0f}},
  {{ 0.5f,  0.5f, 0.0f}, {1.0f,1.0f,1.0f}, {1.0f,1.0f}},

  {{-0.5f, -0.5f,-0.5f}, {1.0f,1.0f,1.0f}, {0.0f,0.0f}},
  {{ 0.5f, -0.5f,-0.5f}, {1.0f,1.0f,1.0f}, {1.0f,0.0f}},
  {{-0.5f,  0.5f,-0.5f}, {1.0f,1.0f,1.0f}, {0.0f,1.0f}},
  {{ 0.5f,  0.5f,-0.5f}, {1.0f,1.0f,1.0f}, {1.0f,1.0f}},
};

const std::vector<uint16_t> indices = {
  0, 1, 2, 1, 3, 2,
  4, 5, 6, 5, 7, 6,
};

class Application
{
  private:
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    std::shared_ptr<Texture> texture;

    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;

    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;

    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;

    std::vector<VkCommandBuffer> commandBuffers;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    size_t currentFrame = 0;

    bool framebufferResized = false;



  public:
    void run()
    {
      InitVulkan();
      MainLoop();
      Cleanup();
    }

  private:

    void InitVulkan()
    {
      VulkanContext::Init();
      CreateDescriptorSetLayout();
      CreateGraphicsPipeline();
      CreateTexture();
      CreateVertexBuffer();
      CreateIndexBuffer();
      CreateUniformBuffers();
      CreateDescriptorPool();
      CreateDescriptorSets();
      CreateCommandBuffers();
      CreateSyncObjects();
    }

    void ResizeFramebuffer()
    {
      int width = 0;
      int height = 0;
      while(width==0 || height == 0)
      {
        glfwGetFramebufferSize(VulkanContext::GetWindow(), &width, &height);
        //glfwWaitEvents();
      }

      vkDeviceWaitIdle(VulkanContext::GetDevice());

      CleanupGraphicsPipeline();
      VulkanContext::RecreateSwapChain();
      CreateGraphicsPipeline();
      CreateCommandBuffers();
    }
    void CreateDescriptorSetLayout()
    {
      VkDescriptorSetLayoutBinding uboLayoutBinding = {};
      uboLayoutBinding.binding = 0;
      uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      uboLayoutBinding.descriptorCount = 1;
      uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
      uboLayoutBinding.pImmutableSamplers = nullptr;

      VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
      samplerLayoutBinding.binding = 1;
      samplerLayoutBinding.descriptorCount = 1;
      samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      samplerLayoutBinding.pImmutableSamplers = nullptr;
      samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

      std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};

      VkDescriptorSetLayoutCreateInfo layoutInfo = {};
      layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
      layoutInfo.bindingCount = bindings.size();
      layoutInfo.pBindings = bindings.data();

      if(vkCreateDescriptorSetLayout(VulkanContext::GetDevice(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
        throw std::runtime_error("Failed to create descriptor set layout");
    }

    void CreateGraphicsPipeline()
    {
      auto vertShaderCode = readFile("res/shaders/shader.vert.spv");
      auto fragShaderCode = readFile("res/shaders/shader.frag.spv");
      VkShaderModule vertShaderModule = CreateShaderModule(vertShaderCode);
      VkShaderModule fragShaderModule = CreateShaderModule(fragShaderCode);

      VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
      vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
      vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
      vertShaderStageInfo.module = vertShaderModule;
      vertShaderStageInfo.pName = "main"; // Entry point

      VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
      fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
      fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
      fragShaderStageInfo.module = fragShaderModule;
      fragShaderStageInfo.pName = "main"; // Entry point

      VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

      auto bindingDescription = Vertex::GetBindingDescription();
      auto attributeDescriptions = Vertex::GetAttributeDescriptions();
      VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
      vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
      vertexInputInfo.vertexBindingDescriptionCount = 1;
      vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
      vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
      vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

      VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
      inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
      inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
      inputAssembly.primitiveRestartEnable = VK_FALSE;

      VkViewport viewport = {};
      viewport.x = 0.0f;
      viewport.y = 0.0f;
      viewport.width = (float) VulkanContext::GetSwapChainWidth();
      viewport.height = (float) VulkanContext::GetSwapChainHeight();
      viewport.minDepth = 0.0f;
      viewport.maxDepth = 1.0f;

      VkRect2D scissor = {};
      scissor.offset = {0,0};
      scissor.extent = VulkanContext::GetSwapChainExtent();

      VkPipelineViewportStateCreateInfo viewportState = {};
      viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
      viewportState.viewportCount = 1;
      viewportState.pViewports = &viewport;
      viewportState.scissorCount = 1;
      viewportState.pScissors = &scissor;

      VkPipelineRasterizationStateCreateInfo rasterizer = {};
      rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
      rasterizer.depthClampEnable = VK_FALSE;
      rasterizer.rasterizerDiscardEnable = VK_FALSE;
      rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
      rasterizer.lineWidth = 1.0;
      rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
      rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
      rasterizer.depthBiasEnable = VK_FALSE;
      rasterizer.depthBiasConstantFactor = 0.0f;
      rasterizer.depthBiasClamp = 0.0f;
      rasterizer.depthBiasSlopeFactor = 0.0f;

      VkPipelineMultisampleStateCreateInfo multisampling = {};
      multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
      multisampling.sampleShadingEnable = VK_FALSE;
      multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
      multisampling.minSampleShading = 1.0f;
      multisampling.pSampleMask = nullptr;
      multisampling.alphaToCoverageEnable = VK_FALSE;
      multisampling.alphaToOneEnable = VK_FALSE;

      VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
      colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
      colorBlendAttachment.blendEnable = VK_FALSE;
      colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
      colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
      colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
      colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
      colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
      colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

      VkPipelineColorBlendStateCreateInfo colorBlending = {};
      colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
      colorBlending.logicOpEnable = VK_FALSE;
      colorBlending.logicOp = VK_LOGIC_OP_COPY;
      colorBlending.attachmentCount = 1;
      colorBlending.pAttachments = &colorBlendAttachment;
      colorBlending.blendConstants[0] = 0.0f;
      colorBlending.blendConstants[1] = 0.0f;
      colorBlending.blendConstants[2] = 0.0f;
      colorBlending.blendConstants[3] = 0.0f;

      VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
      pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
      pipelineLayoutInfo.setLayoutCount = 1;
      pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

      if(vkCreatePipelineLayout(VulkanContext::GetDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
        throw std::runtime_error("Failed to create pipeline layout");

      VkPipelineDepthStencilStateCreateInfo depthStencil = {};
      depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
      depthStencil.depthTestEnable = VK_TRUE;
      depthStencil.depthWriteEnable = VK_TRUE;
      depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
      depthStencil.depthBoundsTestEnable = VK_FALSE;
      depthStencil.minDepthBounds = 0.0f;
      depthStencil.maxDepthBounds = 1.0f;
      depthStencil.stencilTestEnable = VK_FALSE;
      depthStencil.front = {};
      depthStencil.back = {};


      VkGraphicsPipelineCreateInfo pipelineInfo = {};
      pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
      pipelineInfo.stageCount = 2;
      pipelineInfo.pStages = shaderStages;
      pipelineInfo.pVertexInputState = &vertexInputInfo;
      pipelineInfo.pInputAssemblyState = &inputAssembly;
      pipelineInfo.pViewportState = &viewportState;
      pipelineInfo.pRasterizationState = &rasterizer;
      pipelineInfo.pMultisampleState = &multisampling;
      pipelineInfo.pDepthStencilState = &depthStencil;
      pipelineInfo.pColorBlendState = &colorBlending;
      pipelineInfo.pDynamicState = nullptr;
      pipelineInfo.layout = pipelineLayout;
      pipelineInfo.renderPass = VulkanContext::GetRenderPass();
      pipelineInfo.subpass = 0;
      pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
      pipelineInfo.basePipelineIndex = -1;

      if(vkCreateGraphicsPipelines(VulkanContext::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
        throw std::runtime_error("Failed to create graphics pipeline");


      vkDestroyShaderModule(VulkanContext::GetDevice(), fragShaderModule, nullptr);
      vkDestroyShaderModule(VulkanContext::GetDevice(), vertShaderModule, nullptr);
    }

    void CreateTexture()
    {
      texture = std::make_shared<Texture>("res/textures/test.png");
    }

    void CreateVertexBuffer()
    {
      VkDeviceSize bufferSize = vertices.size() * sizeof(Vertex);
      VkBuffer stagingBuffer;
      VkDeviceMemory stagingBufferMemory;

      VulkanUtils::CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,stagingBufferMemory);

      VulkanUtils::UpdateBuffer(stagingBufferMemory, vertices.data(), bufferSize);

      VulkanUtils::CreateBuffer(bufferSize,VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer,vertexBufferMemory);

      VulkanUtils::CopyBuffer(stagingBuffer, vertexBuffer, bufferSize);

      vkDestroyBuffer(VulkanContext::GetDevice(), stagingBuffer, nullptr);
      vkFreeMemory(VulkanContext::GetDevice(), stagingBufferMemory, nullptr);
    }

    void CreateIndexBuffer()
    {
      VkDeviceSize bufferSize = indices.size() * sizeof(indices[0]);
      VkBuffer stagingBuffer;
      VkDeviceMemory stagingBufferMemory;

      VulkanUtils::CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,stagingBufferMemory);

      VulkanUtils::UpdateBuffer(stagingBufferMemory, indices.data(), bufferSize);

      VulkanUtils::CreateBuffer(bufferSize,VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer,indexBufferMemory);

      VulkanUtils::CopyBuffer(stagingBuffer, indexBuffer, bufferSize);

      vkDestroyBuffer(VulkanContext::GetDevice(), stagingBuffer, nullptr);
      vkFreeMemory(VulkanContext::GetDevice(), stagingBufferMemory, nullptr);

    }

    void CreateUniformBuffers()
    {
      VkDeviceSize bufferSize = sizeof(UniformBufferObject);

      size_t count = VulkanContext::GetSwapChainSize();
      uniformBuffers.resize(count);
      uniformBuffersMemory.resize(count);
      for(size_t i = 0; i < count; i++)
      {
        VulkanUtils::CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);
      }
    }

    void CreateDescriptorPool()
    {
      size_t count = VulkanContext::GetSwapChainSize();
      std::array<VkDescriptorPoolSize,2> poolSizes = {};
      poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      poolSizes[0].descriptorCount = count;
      poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      poolSizes[1].descriptorCount = count;

      VkDescriptorPoolCreateInfo poolInfo = {};
      poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
      poolInfo.poolSizeCount = poolSizes.size();
      poolInfo.pPoolSizes = poolSizes.data();
      poolInfo.maxSets = static_cast<uint32_t>(count);

      if(vkCreateDescriptorPool(VulkanContext::GetDevice(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
        throw std::runtime_error("Failed to create descriptor pool!");
    }

    void CreateDescriptorSets()
    {
      size_t count = VulkanContext::GetSwapChainSize();
      std::vector<VkDescriptorSetLayout> layouts(count, descriptorSetLayout);
      VkDescriptorSetAllocateInfo allocInfo = {};
      allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
      allocInfo.descriptorPool = descriptorPool;
      allocInfo.descriptorSetCount = static_cast<uint32_t>(count);
      allocInfo.pSetLayouts = layouts.data();

      descriptorSets.resize(count);
      if(vkAllocateDescriptorSets(VulkanContext::GetDevice(), &allocInfo, descriptorSets.data()) != VK_SUCCESS)
        throw std::runtime_error("Failed to allocate descriptor sets");

      for(size_t i = 0; i < count; i++)
      {
        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);


        std::array<VkWriteDescriptorSet, 2> descriptorWrite = {};
        descriptorWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite[0].dstSet = descriptorSets[i];
        descriptorWrite[0].dstBinding = 0;
        descriptorWrite[0].dstArrayElement = 0;
        descriptorWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite[0].descriptorCount = 1;
        descriptorWrite[0].pBufferInfo = &bufferInfo;

        VkDescriptorImageInfo imageInfo = texture->GetDescriptor();

        descriptorWrite[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite[1].dstSet = descriptorSets[i];
        descriptorWrite[1].dstBinding = 1;
        descriptorWrite[1].dstArrayElement = 0;
        descriptorWrite[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrite[1].descriptorCount = 1;
        descriptorWrite[1].pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(VulkanContext::GetDevice(), descriptorWrite.size(), descriptorWrite.data(), 0, nullptr);
      }
    }

    void CreateCommandBuffers()
    {
      commandBuffers.resize(VulkanContext::GetSwapChainSize());
      VkCommandBufferAllocateInfo allocInfo = {};
      allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
      allocInfo.commandPool = VulkanContext::GetCommandPool();
      allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
      allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

      if(vkAllocateCommandBuffers(VulkanContext::GetDevice(), &allocInfo, commandBuffers.data()) != VK_SUCCESS)
        throw std::runtime_error("Failed to allocate command buffers");

      for(size_t i = 0; i < commandBuffers.size(); i++)
      {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        beginInfo.pInheritanceInfo = nullptr;

        if(vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS)
          throw std::runtime_error("Failed to begin recording command buffer");

        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = VulkanContext::GetRenderPass();
        renderPassInfo.framebuffer = VulkanContext::GetSwapChainFramebuffer(i);
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = VulkanContext::GetSwapChainExtent();

        std::array<VkClearValue,2> clearColors = {};
        clearColors[0] = { 0.0f, 0.0f, 0.0f, 0.0f };
        clearColors[1] = { 1.0f, 0 };

        renderPassInfo.clearValueCount = clearColors.size();
        renderPassInfo.pClearValues = clearColors.data();

        vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        {
          vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

          VkBuffer vertexBuffers[] = {vertexBuffer};
          VkDeviceSize offsets[] = {0};

          vkCmdBindVertexBuffers(commandBuffers[i], 0, 1,vertexBuffers, offsets);
          vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer, 0,VK_INDEX_TYPE_UINT16);

          vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[i], 0, nullptr);

          vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

        }
        vkCmdEndRenderPass(commandBuffers[i]);
        if(vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
          throw std::runtime_error("Failed to record command buffer");
      }
    }

    void CreateSyncObjects()
    {
      imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
      renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
      inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
      VkSemaphoreCreateInfo semaphoreInfo = {};
      semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

      VkFenceCreateInfo fenceInfo = {};
      fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
      fenceInfo.flags= VK_FENCE_CREATE_SIGNALED_BIT;

      for(int i = 0;i<MAX_FRAMES_IN_FLIGHT;i++)
      {
        if(vkCreateSemaphore(VulkanContext::GetDevice(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(VulkanContext::GetDevice(), &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS || 
            vkCreateFence(VulkanContext::GetDevice(), &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
          throw std::runtime_error("Failed to create semaphores");
      }
    }

    VkShaderModule CreateShaderModule(const std::vector<char>& code)
    {
      VkShaderModuleCreateInfo createInfo = {};
      createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
      createInfo.codeSize = code.size();
      createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
      VkShaderModule shaderModule;
      if(vkCreateShaderModule(VulkanContext::GetDevice(), &createInfo,nullptr, &shaderModule) != VK_SUCCESS)
        throw std::runtime_error("Failed to create shader module!");
      return shaderModule;
    }

    void MainLoop()
    {
      while(!glfwWindowShouldClose(VulkanContext::GetWindow())) {
        glfwPollEvents();
        DrawFrame();
      }

      vkDeviceWaitIdle(VulkanContext::GetDevice());
    }

    void DrawFrame()
    {
      vkWaitForFences(VulkanContext::GetDevice(), 1, &inFlightFences[currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());

      uint32_t imageIndex;
      VkResult result = vkAcquireNextImageKHR(VulkanContext::GetDevice(), VulkanContext::GetSwapChain(), std::numeric_limits<uint64_t>::max(), imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

      if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        ResizeFramebuffer();
        return;
      } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
      }

      UpdateUniformBuffer(imageIndex);

      VkSubmitInfo submitInfo = {};
      submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

      VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
      VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
      submitInfo.waitSemaphoreCount = 1;
      submitInfo.pWaitSemaphores = waitSemaphores;
      submitInfo.pWaitDstStageMask = waitStages;

      submitInfo.commandBufferCount = 1;
      submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

      VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
      submitInfo.signalSemaphoreCount = 1;
      submitInfo.pSignalSemaphores = signalSemaphores;

      vkResetFences(VulkanContext::GetDevice(), 1, &inFlightFences[currentFrame]);

      if (vkQueueSubmit(VulkanContext::GetGraphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
      }

      VkPresentInfoKHR presentInfo = {};
      presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

      presentInfo.waitSemaphoreCount = 1;
      presentInfo.pWaitSemaphores = signalSemaphores;

      VkSwapchainKHR swapChains[] = {VulkanContext::GetSwapChain()};
      presentInfo.swapchainCount = 1;
      presentInfo.pSwapchains = swapChains;

      presentInfo.pImageIndices = &imageIndex;

      result = vkQueuePresentKHR(VulkanContext::GetPresentQueue(), &presentInfo);

      if (result != VK_ERROR_OUT_OF_DATE_KHR && result != VK_SUBOPTIMAL_KHR && result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
      }

      currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void UpdateUniformBuffer(uint32_t currentImage)
    {
      static auto startTime = std::chrono::high_resolution_clock::now();

      auto currentTime = std::chrono::high_resolution_clock::now();
      float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
      UniformBufferObject ubo = {};
      ubo.model = Greet::Mat4::RotateRZ(time);
      ubo.view = Greet::Mat4::LookAt(Greet::Vec3(1,1,1), Greet::Vec3(0,0,0), Greet::Vec3(0,0,-1));
      ubo.proj = Greet::Mat4::ProjectionMatrix(VulkanContext::GetSwapChainWidth()  / (float) VulkanContext::GetSwapChainHeight(), 90, 0.1f, 10.0f);

      VulkanUtils::UpdateBuffer(uniformBuffersMemory[currentImage], &ubo, sizeof(ubo));
    }

    void Cleanup()
    {
      CleanupGraphicsPipeline();

      texture.reset();

      vkDestroyDescriptorPool(VulkanContext::GetDevice(), descriptorPool, nullptr);

      vkDestroyDescriptorSetLayout(VulkanContext::GetDevice(), descriptorSetLayout, nullptr);
      for(size_t i = 0; i < VulkanContext::GetSwapChainSize(); i++)
      {
        vkDestroyBuffer(VulkanContext::GetDevice(), uniformBuffers[i], nullptr);
        vkFreeMemory(VulkanContext::GetDevice(), uniformBuffersMemory[i], nullptr);
      }

      vkDestroyBuffer(VulkanContext::GetDevice(), indexBuffer, nullptr);
      vkFreeMemory(VulkanContext::GetDevice(), indexBufferMemory, nullptr);

      vkDestroyBuffer(VulkanContext::GetDevice(), vertexBuffer, nullptr);
      vkFreeMemory(VulkanContext::GetDevice(), vertexBufferMemory, nullptr);

      for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
      {
        vkDestroySemaphore(VulkanContext::GetDevice(), renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(VulkanContext::GetDevice(), imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(VulkanContext::GetDevice(), inFlightFences[i], nullptr);
      }
      VulkanContext::Cleanup();
    }

    void CleanupGraphicsPipeline()
    {
      vkFreeCommandBuffers(VulkanContext::GetDevice(), VulkanContext::GetCommandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());

      vkDestroyPipeline(VulkanContext::GetDevice(), graphicsPipeline, nullptr);
      vkDestroyPipelineLayout(VulkanContext::GetDevice(), pipelineLayout, nullptr);
    }

    static void FramebufferResizeCallback(GLFWwindow* window, int width, int height)
    {
      auto app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
      app->framebufferResized = true;
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
    {
      if(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
        std::cout << "VK_INFO: " << pCallbackData->pMessage << std::endl;
      if(messageSeverity &  VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
        std::cout << "VK_VERB: " << pCallbackData->pMessage << std::endl;
      if(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        std::cout << "VK_WARN: " << pCallbackData->pMessage << std::endl;
      if(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        std::cout << "VK_ERROR: " << pCallbackData->pMessage << std::endl;
      return VK_FALSE;
    }

    static std::vector<char> readFile(const std::string& filename)
    {
      std::ifstream file(filename, std::ios::ate | std::ios::binary);
      if(!file.is_open())
        throw std::runtime_error("Failed to open file");
      size_t fileSize = (size_t) file.tellg();
      std::vector<char> buffer(fileSize);
      file.seekg(0);
      file.read(buffer.data(), fileSize);
      file.close();
      return buffer;
    }

};
