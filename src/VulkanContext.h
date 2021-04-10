#pragma once

#include "VulkanHandle.h"
#include "VulkanUtils.h"

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <vector>
#include <set>
#include <cstring>
#include <optional>
#include <limits>
#include <array>

#ifdef _DEBUG
const bool enableValidationLayers = true;
#else
const bool enableValidationLayers = false;
#endif

class VulkanContext
{

  private:
    struct QueueFamilyIndices
    {
      std::optional<uint32_t> graphicsFamily;
      std::optional<uint32_t> presentFamily;

      bool IsComplete()
      {
        return graphicsFamily.has_value() && presentFamily.has_value();
      }
    };

    struct SwapChainSupportDetails
    {
      VkSurfaceCapabilitiesKHR capabilities;
      std::vector<VkSurfaceFormatKHR> formats;
      std::vector<VkPresentModeKHR> presentModes;
    };

  private:

    static VulkanContext context;

    GLFWwindow* window;

    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkSurfaceKHR surface;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkSwapchainKHR swapChain;
    VkRenderPass renderPass;

    VkCommandPool commandPool;

    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;

    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;



    std::vector<const char*> layers =
    {
      "VK_LAYER_KHRONOS_validation",
    };

    const std::vector<const char*> deviceExtensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };

    VulkanContext() = default;

  public:
    static void Init()
    {
      CreateGlfwWindow();
      CreateInstance();
      SetupDebugMessenger();
      CreateSurface();
      PickPhysicalDevice();
      CreateLogicalDevice();
      CreateSwapChain();
      CreateRenderPass();
      CreateSwapChainImageViews();
      CreateCommandPool();
      CreateDepthResource();
      CreateFramebuffers();
    }

    static void CleanupSwapChain()
    {

      for(auto&& framebuffer : context.swapChainFramebuffers)
        vkDestroyFramebuffer(VulkanContext::GetDevice(), framebuffer, nullptr);

      vkDestroyImageView(VulkanContext::GetDevice(), context.depthImageView, nullptr);
      vkDestroyImage(VulkanContext::GetDevice(), context.depthImage, nullptr);
      vkFreeMemory(VulkanContext::GetDevice(), context.depthImageMemory, nullptr);


      for(auto&& imageView : context.swapChainImageViews)
        vkDestroyImageView(VulkanContext::GetDevice(),imageView, nullptr);
      vkDestroyRenderPass(VulkanContext::GetDevice(), context.renderPass, nullptr);
      vkDestroySwapchainKHR(context.device, context.swapChain,nullptr);
    }

    static void Cleanup()
    {
      CleanupSwapChain();
      vkDestroyCommandPool(VulkanContext::GetDevice(), VulkanContext::GetCommandPool(), nullptr);

      if(enableValidationLayers)
        VulkanHandle::DestroyDebugUtilsMessengerEXT(context.instance, context.debugMessenger, nullptr);

      vkDestroyDevice(VulkanContext::GetDevice(), nullptr);
      vkDestroySurfaceKHR(context.instance, context.surface, nullptr);
      vkDestroyInstance(context.instance, nullptr);
      glfwDestroyWindow(context.window);
      glfwTerminate();
    }

    static void RecreateSwapChain()
    {
      CleanupSwapChain();

      CreateSwapChain();
      CreateRenderPass();
      CreateSwapChainImageViews();
      CreateDepthResource();
      CreateFramebuffers();
    }

    static VkDevice GetDevice()
    {
      return context.device;
    }

    static VkPhysicalDevice GetPhysicalDevice()
    {
      return context.physicalDevice;
    }

    static GLFWwindow* GetWindow()
    {
      return context.window;
    }

    static size_t GetSwapChainSize()
    {
      return context.swapChainImageViews.size();
    }

    static VkCommandPool GetCommandPool()
    {
      return context.commandPool;
    }

    static VkRenderPass GetRenderPass()
    {
      return context.renderPass;
    }

    static VkSwapchainKHR GetSwapChain()
    {
      return context.swapChain;
    }

    static VkFramebuffer GetSwapChainFramebuffer(int index)
    {
      return context.swapChainFramebuffers[index];
    }

    static VkExtent2D GetSwapChainExtent()
    {
      return context.swapChainExtent;
    }

    static uint32_t GetSwapChainWidth()
    {
      return context.swapChainExtent.width;
    }

    static uint32_t GetSwapChainHeight()
    {
      return context.swapChainExtent.height;
    }

    static VkQueue GetGraphicsQueue()
    {
      return context.graphicsQueue;
    }

    static VkQueue GetPresentQueue()
    {
      return context.presentQueue;
    }

  private:
    static void CreateGlfwWindow();
    static void CreateInstance();
    static void SetupDebugMessenger();
    static void CreateSurface();
    static void PickPhysicalDevice();
    static void CreateLogicalDevice();
    static void CreateSwapChain();
    static void CreateCommandPool();
    static void CreateRenderPass();
    static void CreateDepthResource();
    static void CreateFramebuffers();
    static void CreateSwapChainImageViews();

    static bool CheckValidationLayerSupport();
    static std::vector<const char*> GetRequiredExtensions();
    static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);
    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
      VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
      VkDebugUtilsMessageTypeFlagsEXT messageType,
      const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
      void* pUserData);
    // Check if the GPU has support for the wanted operations
    static bool IsDeviceSuitable(VkPhysicalDevice device);
    static bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
    static QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
    static SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
    static void PrintPhysicalDeviceName(VkPhysicalDevice device);
    static VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    static VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    static VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    static VkFormat FindDepthFormat();
    static VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

};
