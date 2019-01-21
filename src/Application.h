#pragma once

#include "VulkanHandle.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <set>
#include <cstring>
#include <optional>

const std::vector<const char*> validationLayers = {
  "VK_LAYER_LUNARG_standard_validation",
};

const std::vector<const char*> deviceExtensions = {
  VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

const uint32_t DEFAULT_WIDTH = 1280;
const uint32_t DEFAULT_HEIGHT = 720;

#ifdef _DEBUG
const bool enableValidationLayers = true;
#else
const bool enableValidationLayers = false;
#endif

struct SwapChainSupportDetails
{
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> presentModes;
};

struct QueueFamilyIndices
{
  std::optional<uint32_t> graphicsFamily;
  std::optional<uint32_t> presentFamily;

  bool IsComplete()
  {
    return graphicsFamily.has_value() && presentFamily.has_value();
  }
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
    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews;
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
    }

    void InitVulkan()
    {
      CreateInstance();
      SetupDebugMessenger();
      CreateSurface();
      PickPhysicalDevice();
      CreateLogicalDevice();
      CreateSwapChain();
      CreateImageViews();
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

      std::cout << "Setting up Debug messenger" << std::endl;
      VkDebugUtilsMessengerCreateInfoEXT createInfo = {};

      createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
      createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | 
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | 
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

      createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | 
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
      QueueFamilyIndices indices = FindQueueFamilies(device);
      bool extensionSupported = CheckDeviceExtensionSupport(device);
      bool swapChainAdequate = false;
      if(extensionSupported )
      {
        SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
      }
      return indices.IsComplete() && extensionSupported && swapChainAdequate;
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

    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
    {
      if(availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED)
        return {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};

      for(auto&& availableFormat : availableFormats)
      {
        if(availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
          return availableFormat;
      }
      return availableFormats[0];
    }

    VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
    {
      VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;
      for(auto&& availablePresentMode : availablePresentModes)
      {
        if(availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
          return availablePresentMode;
        else if(availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
          bestMode = availablePresentMode;
      }
      // Guaranteed to be available
      return bestMode;
    }

    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
    {
      if(capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        return capabilities.currentExtent;
      else
      {
        VkExtent2D actualExtent = {DEFAULT_WIDTH, DEFAULT_HEIGHT};

        actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height= std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));
        return actualExtent;
      }

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
      QueueFamilyIndices indices = FindQueueFamilies(physicalDevice);
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
        queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
      }

      VkPhysicalDeviceFeatures deviceFeatures = {};
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

    void CreateSwapChain()
    {
      SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(physicalDevice);

      VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
      VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
      VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

      uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
      if(swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
      {
        imageCount = swapChainSupport.capabilities.maxImageCount;
      }

      VkSwapchainCreateInfoKHR createInfo = {};
      createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
      createInfo.surface = surface;
      createInfo.minImageCount = imageCount;
      createInfo.imageFormat = surfaceFormat.format;
      createInfo.imageColorSpace = surfaceFormat.colorSpace;
      createInfo.imageExtent = extent;
      createInfo.imageArrayLayers = 1;
      createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

      QueueFamilyIndices indices = FindQueueFamilies(physicalDevice);
      uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(),indices.presentFamily.value()};

      if(indices.graphicsFamily != indices.presentFamily)
      {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
      }
      else
      {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
      }
      createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
      createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
      createInfo.presentMode = presentMode;
      createInfo.clipped = VK_TRUE;
      createInfo.oldSwapchain = VK_NULL_HANDLE;

      if(vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
        throw std::runtime_error("Failed to create swap chain");

      vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
      swapChainImages.resize(imageCount);
      vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());
      swapChainImageFormat = surfaceFormat.format;
      swapChainExtent = extent;
    }

    void CreateImageViews()
    {
      swapChainImageViews.resize(swapChainImages.size());
      for(size_t i = 0;i< swapChainImages.size();++i)
      {
        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = swapChainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = swapChainImageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if(vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS)
          throw std::runtime_error("Failed to create image views");
      }
    }

    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device)
    {
      QueueFamilyIndices indices;
      uint32_t queueFamilyCount;
      vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
      std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
      vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
      int i = 0;
      for(auto&& queueFamily : queueFamilies)
      {
        if(queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
          indices.graphicsFamily = i;

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
        if(queueFamily.queueCount > 0 && presentSupport)
          indices.presentFamily = i;

        if(indices.IsComplete())
          break;

        i++;
      }
      return indices;
    }

    SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device)
    {
      SwapChainSupportDetails details;
      vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

      uint32_t formatCount;;
      vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr); 
      if(formatCount != 0)
      {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
      }

      uint32_t presentModeCount;
      vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr); 
      if(presentModeCount != 0)
      {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
      }

      return details;
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

    void MainLoop()
    {
      while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();
      }
    }

    void Cleanup()
    {
      for(auto&& imageView : swapChainImageViews)
        vkDestroyImageView(device,imageView, nullptr);

      vkDestroySwapchainKHR(device,swapChain,nullptr);
      if(enableValidationLayers)
        VulkanHandle::DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);

      vkDestroyDevice(device, nullptr);
      vkDestroySurfaceKHR(instance,surface, nullptr);
      vkDestroyInstance(instance, nullptr);
      glfwDestroyWindow(window);
      glfwTerminate();
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

};
