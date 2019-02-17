#pragma once

#include "SwapChainHandler.h"
#include "ImageView.h"
#include "VulkanHandle.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <set>
#include <cstring>
#include <optional>
#include <fstream>
#include <functional>
#include <math/Maths.h>
#include <chrono>
#include <ImageUtils.h>

const std::vector<const char*> validationLayers = {
  "VK_LAYER_LUNARG_standard_validation",
};

const std::vector<const char*> deviceExtensions = {
  VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

const uint32_t DEFAULT_WIDTH = 1280;
const uint32_t DEFAULT_HEIGHT = 720;

const int MAX_FRAMES_IN_FLIGHT = 2;

#ifdef _DEBUG
const bool enableValidationLayers = true;
#else
const bool enableValidationLayers = false;
#endif

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
    GLFWwindow* window;

    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkSurfaceKHR surface;

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;

    VkQueue graphicsQueue;
    VkQueue presentQueue;

    SwapChainHandler* swapChains;

    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    VkImage textureImage;
    VkImageView textureImageView;
    VkSampler textureSampler;
    VkDeviceMemory textureImageMemory;

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
      InitWindow();
      InitVulkan();
      MainLoop();
      Cleanup();
    }

  private:
    void InitWindow()
    {
      glfwInit();
      glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
      glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
      window = glfwCreateWindow(DEFAULT_WIDTH, DEFAULT_HEIGHT, "Vulkan window", nullptr, nullptr);
      glfwSetWindowUserPointer(window, this);
      glfwSetFramebufferSizeCallback(window, FramebufferResizeCallback);
    }

    void InitVulkan()
    {
      CreateInstance();
      SetupDebugMessenger();
      CreateSurface();
      PickPhysicalDevice();
      CreateLogicalDevice();
      swapChains = new SwapChainHandler(window, surface, device, physicalDevice,graphicsQueue);
      CreateDescriptorSetLayout();
      CreateGraphicsPipeline();
      CreateTextureImage();
      CreateTextureImageView();
      CreateTextureSampler();
      CreateVertexBuffer();
      CreateIndexBuffer();
      CreateUniformBuffers();
      CreateDescriptorPool();
      CreateDescriptorSets();
      CreateCommandBuffers();
      CreateSyncObjects();
    }

    void RecreateSwapChain()
    {
      int width = 0;
      int height = 0;
      while(width==0 || height == 0)
      {
        glfwGetFramebufferSize(window, &width, &height);
        //glfwWaitEvents();
      }

      vkDeviceWaitIdle(device);

      CleanupSwapChain();

      delete swapChains;
      swapChains = new SwapChainHandler(window, surface, device, physicalDevice, graphicsQueue);
      CreateGraphicsPipeline();
      CreateCommandBuffers();
    }

    void CreateInstance()
    {
      if(enableValidationLayers && !CheckValidationLayerSupport())
        throw std::runtime_error("Validation layers requested, but are not available");

      VkApplicationInfo appInfo = {};
      appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
      appInfo.pApplicationName = "Vulkan Window";
      appInfo.applicationVersion = VK_MAKE_VERSION(1,0,0);
      appInfo.pEngineName = "Greet";
      appInfo.engineVersion = VK_MAKE_VERSION(1,0,0);
      appInfo.apiVersion = VK_API_VERSION_1_1;

      auto extensions = GetRequiredExtensions();

      VkInstanceCreateInfo createInfo = {};
      createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
      createInfo.pApplicationInfo = &appInfo;
      createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
      createInfo.ppEnabledExtensionNames = extensions.data();
      if (enableValidationLayers)
      {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
      }
      else
      {
        createInfo.enabledLayerCount = 0;
      }

      if(vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
        throw std::runtime_error("Failed to create Vulkan instance");
    }

    void SetupDebugMessenger()
    {
      if(!enableValidationLayers) 
        return;

      VkDebugUtilsMessengerCreateInfoEXT createInfo = {};

      createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
      createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | 
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | 
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

      createInfo.messageType = 
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | 
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | 
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
      createInfo.flags = 0;
      createInfo.pfnUserCallback = Application::DebugCallback;

      if(VulkanHandle::CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
        throw std::runtime_error("Failed to setup debug messenger");
    }

    void CreateSurface()
    {
      if(glfwCreateWindowSurface(instance,window, nullptr, &surface) != VK_SUCCESS)
        throw std::runtime_error("Failed to create window surface");
    }

    void PickPhysicalDevice()
    {
      uint32_t deviceCount = 0;
      vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
      if(deviceCount == 0)
        throw std::runtime_error("Failed to find GPU with Vulkan support");

      std::vector<VkPhysicalDevice> devices(deviceCount);
      vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

      for(auto&& device : devices)
      {
        if(IsDeviceSuitable(device))
        {
          physicalDevice = device;
          break;
        }
      }
      if(physicalDevice == VK_NULL_HANDLE)
        throw std::runtime_error("Failed to find GPU with support of all needed operations");
      PrintPhysicalDeviceName(physicalDevice);
    }

    // Check if the GPU has support for the wanted operations
    bool IsDeviceSuitable(VkPhysicalDevice device)
    {
      QueueFamilyIndices indices = VulkanHandle::FindQueueFamilies(surface, device);
      bool extensionSupported = CheckDeviceExtensionSupport(device);
      bool swapChainAdequate = false;
      if(extensionSupported)
      {
        SwapChainSupportDetails swapChainSupport = SwapChainHandler::QuerySwapChainSupport(surface, device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
      }
      VkPhysicalDeviceFeatures supportedFeatures;
      vkGetPhysicalDeviceFeatures(device, &supportedFeatures);
      return indices.IsComplete() && extensionSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
    }

    bool CheckDeviceExtensionSupport(VkPhysicalDevice device)
    {
      uint32_t extensionCount;
      vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
      std::vector<VkExtensionProperties> availableExtensions(extensionCount);
      vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

      std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

      for(auto&& extension : availableExtensions)
        requiredExtensions.erase(extension.extensionName);
      return requiredExtensions.empty();
    }

    void PrintPhysicalDeviceName(VkPhysicalDevice device)
    {
      VkPhysicalDeviceProperties deviceProperties;
      vkGetPhysicalDeviceProperties(device, &deviceProperties);

      uint32_t version;
      vkEnumerateInstanceVersion(&version);
      std::cout << "INFO: Vulkan v" << 
        VK_VERSION_MAJOR(deviceProperties.apiVersion) << "." << 
        VK_VERSION_MINOR(deviceProperties.apiVersion) << "." << 
        VK_VERSION_PATCH(deviceProperties.apiVersion) << std::endl; 

      std::cout << "INFO: " << 
        deviceProperties.deviceName << " v" << 
        VK_VERSION_MAJOR(deviceProperties.driverVersion) << "." << 
        VK_VERSION_MINOR(deviceProperties.driverVersion) << "." << 
        VK_VERSION_PATCH(deviceProperties.driverVersion) << std::endl;
    }

    void CreateLogicalDevice()
    {
      QueueFamilyIndices indices = VulkanHandle::FindQueueFamilies(surface, physicalDevice);
      float queuePriority = 1.0f;
      std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

      std::set<uint32_t> uniqueQueueFamilies = {
        indices.graphicsFamily.value(), 
        indices.presentFamily.value(),
      };
      for(uint32_t queueFamily : uniqueQueueFamilies)
      {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
      }

      VkPhysicalDeviceFeatures deviceFeatures = {};
      deviceFeatures.samplerAnisotropy = VK_TRUE;
      VkDeviceCreateInfo createInfo = {};
      createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
      createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
      createInfo.pQueueCreateInfos = queueCreateInfos.data();
      createInfo.pEnabledFeatures = &deviceFeatures;
      createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
      createInfo.ppEnabledExtensionNames = deviceExtensions.data();
      if constexpr (enableValidationLayers)
      {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
      }
      else
      {
        createInfo.enabledLayerCount = 0;
      }

      if(vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
        throw std::runtime_error("Failed to create logical device");

      vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
      vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
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

      if(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
        throw std::runtime_error("Failed to create descriptor set layout");

      VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
      pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
      pipelineLayoutInfo.setLayoutCount = 1;
      pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
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
      viewport.width = (float) swapChains->GetWidth();
      viewport.height = (float) swapChains->GetHeight();
      viewport.minDepth = 0.0f;
      viewport.maxDepth = 1.0f;

      VkRect2D scissor = {};
      scissor.offset = {0,0};
      scissor.extent = swapChains->GetExtent();

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

      if(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
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
      pipelineInfo.renderPass = swapChains->GetRenderPass();
      pipelineInfo.subpass = 0;
      pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
      pipelineInfo.basePipelineIndex = -1;

      if(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
        throw std::runtime_error("Failed to create graphics pipeline");


      vkDestroyShaderModule(device, fragShaderModule, nullptr);
      vkDestroyShaderModule(device, vertShaderModule, nullptr);
    }

    void CreateTextureImage()
    {
      uint32_t width, height;
      BYTE* bytes = ImageUtils::loadImage("res/textures/test.png", &width, &height);

      VkDeviceSize imageSize = width * height * 4;

      VkBuffer stagingBuffer;
      VkDeviceMemory stagingBufferMemory;
      CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

      UpdateBuffer(stagingBufferMemory, bytes, imageSize);

      delete[] bytes;
      ImageView::CreateImage(device, physicalDevice, width, height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

      ImageView::TransitionImageLayout(device, swapChains->GetCommandPool(), graphicsQueue, textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
      CopyBufferToImage(stagingBuffer, textureImage, width, height);
      ImageView::TransitionImageLayout(device, swapChains->GetCommandPool(), graphicsQueue, textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
      vkDestroyBuffer(device, stagingBuffer, nullptr);
      vkFreeMemory(device, stagingBufferMemory, nullptr);

    }

    void CreateTextureImageView()
    {
      textureImageView = ImageView::CreateImageView(device, textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
    }

    void CreateTextureSampler()
    {
      VkSamplerCreateInfo samplerInfo = {};
      samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
      samplerInfo.magFilter = VK_FILTER_NEAREST;
      samplerInfo.minFilter = VK_FILTER_NEAREST;
      samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
      samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
      samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
      samplerInfo.anisotropyEnable = VK_TRUE;
      samplerInfo.maxAnisotropy = 16;

      samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
      samplerInfo.unnormalizedCoordinates = VK_FALSE;
      samplerInfo.compareEnable = VK_FALSE;
      samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
      samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
      samplerInfo.mipLodBias = 0.0f;
      samplerInfo.minLod = 0.0f;
      samplerInfo.maxLod = 0.0f;

      if(vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS)
        throw std::runtime_error("Failed to create texture sampler");

    }

    void CreateVertexBuffer()
    {
      VkDeviceSize bufferSize = vertices.size() * sizeof(Vertex);
      VkBuffer stagingBuffer;
      VkDeviceMemory stagingBufferMemory;

      CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,stagingBufferMemory);

      UpdateBuffer(stagingBufferMemory, vertices.data(), bufferSize);

      CreateBuffer(bufferSize,VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer,vertexBufferMemory);

      CopyBuffer(stagingBuffer, vertexBuffer, bufferSize);

      vkDestroyBuffer(device, stagingBuffer, nullptr);
      vkFreeMemory(device, stagingBufferMemory, nullptr);
    }

    void CreateIndexBuffer()
    {
      VkDeviceSize bufferSize = indices.size() * sizeof(indices[0]);
      VkBuffer stagingBuffer;
      VkDeviceMemory stagingBufferMemory;

      CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,stagingBufferMemory);

      UpdateBuffer(stagingBufferMemory, indices.data(), bufferSize);

      CreateBuffer(bufferSize,VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer,indexBufferMemory);

      CopyBuffer(stagingBuffer, indexBuffer, bufferSize);

      vkDestroyBuffer(device, stagingBuffer, nullptr);
      vkFreeMemory(device, stagingBufferMemory, nullptr);

    }

    void CreateUniformBuffers()
    {
      VkDeviceSize bufferSize = sizeof(UniformBufferObject);

      uniformBuffers.resize(swapChains->GetCount());
      uniformBuffersMemory.resize(swapChains->GetCount());
      for(size_t i = 0; i < swapChains->GetCount(); i++)
      {
        CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);
      }
    }

    void CreateDescriptorPool()
    {
      std::array<VkDescriptorPoolSize,2> poolSizes = {};
      poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      poolSizes[0].descriptorCount = swapChains->GetCount();
      poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      poolSizes[1].descriptorCount = swapChains->GetCount();

      VkDescriptorPoolCreateInfo poolInfo = {};
      poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
      poolInfo.poolSizeCount = poolSizes.size();
      poolInfo.pPoolSizes = poolSizes.data();
      poolInfo.maxSets = static_cast<uint32_t>(swapChains->GetCount());

      if(vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
        throw std::runtime_error("Failed to create descriptor pool!");
    }

    void CreateDescriptorSets()
    {
      std::vector<VkDescriptorSetLayout> layouts(swapChains->GetCount(), descriptorSetLayout);
      VkDescriptorSetAllocateInfo allocInfo = {};
      allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
      allocInfo.descriptorPool = descriptorPool;
      allocInfo.descriptorSetCount = static_cast<uint32_t>(swapChains->GetCount());
      allocInfo.pSetLayouts = layouts.data(); 

      descriptorSets.resize(swapChains->GetCount());
      if(vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
        throw std::runtime_error("Failed to allocate descriptor sets");

      for(size_t i = 0; i < swapChains->GetCount(); i++)
      {
        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        VkDescriptorImageInfo imageInfo = {};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = textureImageView;
        imageInfo.sampler = textureSampler;

        std::array<VkWriteDescriptorSet, 2> descriptorWrite = {};
        descriptorWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite[0].dstSet = descriptorSets[i];
        descriptorWrite[0].dstBinding = 0;
        descriptorWrite[0].dstArrayElement = 0;
        descriptorWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite[0].descriptorCount = 1;
        descriptorWrite[0].pBufferInfo = &bufferInfo;

        descriptorWrite[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite[1].dstSet = descriptorSets[i];
        descriptorWrite[1].dstBinding = 1;
        descriptorWrite[1].dstArrayElement = 0;
        descriptorWrite[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrite[1].descriptorCount = 1;
        descriptorWrite[1].pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(device, descriptorWrite.size(), descriptorWrite.data(), 0, nullptr);
      }
    }

    void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
    {
      VkBufferCreateInfo bufferInfo = {};
      bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
      bufferInfo.size = size;
      bufferInfo.usage = usage;
      bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

      if(vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
        throw std::runtime_error("Failed to create vertex buffer");

      VkMemoryRequirements memRequirements;
      vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

      VkMemoryAllocateInfo allocInfo = {};
      allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
      allocInfo.allocationSize = memRequirements.size;
      allocInfo.memoryTypeIndex = VulkanHandle::FindMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);

      if(vkAllocateMemory(device,&allocInfo,nullptr, &bufferMemory) != VK_SUCCESS)
        throw std::runtime_error("Failed to allocate vertex buffer memory");

      vkBindBufferMemory(device, buffer, bufferMemory, 0);
    }

    void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
    {
      VkCommandBuffer commandBuffer = VulkanHandle::BeginSingleTimeCommand(device, swapChains->GetCommandPool());
      {
        VkBufferCopy copyRegion = {};
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
      }
      VulkanHandle::EndSingleTimeCommand(device, swapChains->GetCommandPool(), graphicsQueue, commandBuffer);
    }

    void CreateCommandBuffers()
    {
      commandBuffers.resize(swapChains->GetCount());
      VkCommandBufferAllocateInfo allocInfo = {};
      allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
      allocInfo.commandPool  = swapChains->GetCommandPool();
      allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
      allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

      if(vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
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
        renderPassInfo.renderPass = swapChains->GetRenderPass();
        renderPassInfo.framebuffer = swapChains->GetFrameBuffer(i);
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = swapChains->GetExtent();

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
        if(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS || 
            vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
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
      if(vkCreateShaderModule(device, &createInfo,nullptr, &shaderModule) != VK_SUCCESS)
        throw std::runtime_error("Failed to create shader module!");
      return shaderModule;
    }

    std::vector<const char*> GetRequiredExtensions()
    {
      uint32_t glfwExtensionCount = 0;
      const char** glfwExtensions;

      glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

      std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
      if(enableValidationLayers)
      {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
      }
      return extensions;
    }

    bool CheckValidationLayerSupport()
    {
      uint32_t layerCount;
      vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

      std::vector<VkLayerProperties> availableLayers(layerCount);
      vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

      for (const char* layerName : validationLayers) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
          if (strcmp(layerName, layerProperties.layerName) == 0) {
            layerFound = true;
            break;
          }
        }

        if (!layerFound) {
          return false;
        }
      }

      return true;
    }

    void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
    {
      VkCommandBuffer commandBuffer = VulkanHandle::BeginSingleTimeCommand(device, swapChains->GetCommandPool());

      VkBufferImageCopy region = {};
      region.bufferOffset = 0;
      region.bufferRowLength = 0;
      region.bufferImageHeight = 0;
      region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      region.imageSubresource.mipLevel = 0;
      region.imageSubresource.baseArrayLayer = 0;
      region.imageSubresource.layerCount = 1;

      region.imageOffset = {0,0,0};
      region.imageExtent = {width,height,1};

      vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

      VulkanHandle::EndSingleTimeCommand(device, swapChains->GetCommandPool(), graphicsQueue, commandBuffer);
    }

    void UpdateBuffer(VkDeviceMemory buffer, const void* data, uint32_t size)
    {
      void* dataTemp;
      vkMapMemory(device, buffer, 0, size, 0, &dataTemp);
      memcpy(dataTemp, data, size);
      vkUnmapMemory(device, buffer);
    }

    void MainLoop()
    {
      while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        DrawFrame();
      }

      vkDeviceWaitIdle(device);
    }

    void DrawFrame()
    {

      vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());

      uint32_t imageIndex;
      VkResult result = vkAcquireNextImageKHR(device, swapChains->GetSwapChain(), std::numeric_limits<uint64_t>::max(), imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

      if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        RecreateSwapChain();
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

      vkResetFences(device, 1, &inFlightFences[currentFrame]);

      if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
      }

      VkPresentInfoKHR presentInfo = {};
      presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

      presentInfo.waitSemaphoreCount = 1;
      presentInfo.pWaitSemaphores = signalSemaphores;

      VkSwapchainKHR chains[] = {swapChains->GetSwapChain()};
      presentInfo.swapchainCount = 1;
      presentInfo.pSwapchains = chains;

      presentInfo.pImageIndices = &imageIndex;

      result = vkQueuePresentKHR(presentQueue, &presentInfo);

      if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
        framebufferResized = false;
        RecreateSwapChain();
      } else if (result != VK_SUCCESS) {
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
      ubo.proj = Greet::Mat4::ProjectionMatrix(swapChains->GetWidth() / (float) swapChains->GetHeight(), 90, 0.1f, 10.0f);

      UpdateBuffer(uniformBuffersMemory[currentImage], &ubo, sizeof(ubo));
    }

    void Cleanup()
    {
      CleanupSwapChain();

      delete swapChains;

      vkDestroySampler(device, textureSampler, nullptr);
      vkDestroyImageView(device, textureImageView, nullptr);
      vkDestroyImage(device, textureImage, nullptr);
      vkFreeMemory(device, textureImageMemory, nullptr);
      vkDestroyDescriptorPool(device, descriptorPool, nullptr);

      vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
      for(size_t i = 0; i < swapChains->GetCount(); i++)
      {
        vkDestroyBuffer(device, uniformBuffers[i], nullptr);
        vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
      }

      vkDestroyBuffer(device, indexBuffer, nullptr);
      vkFreeMemory(device, indexBufferMemory, nullptr);

      vkDestroyBuffer(device, vertexBuffer, nullptr);
      vkFreeMemory(device, vertexBufferMemory, nullptr);

      for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
      {
        vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(device, inFlightFences[i], nullptr);
      }

      if(enableValidationLayers)
        VulkanHandle::DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);

      vkDestroyDevice(device, nullptr);
      vkDestroySurfaceKHR(instance,surface, nullptr);
      vkDestroyInstance(instance, nullptr);
      glfwDestroyWindow(window);
      glfwTerminate();
    }

    void CleanupSwapChain()
    {
      vkDestroyPipeline(device, graphicsPipeline, nullptr);
      vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
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
