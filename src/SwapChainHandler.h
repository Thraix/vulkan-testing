#pragma once

#include "VulkanHandle.h"
#include <GLFW/glfw3.h>
#include "ImageView.h"

#include <vector>
#include <array>

class Device;

struct SwapChainSupportDetails
{
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> presentModes;
};

struct SwapChainContainer
{
  VkImage image;
  VkImageView imageView;
  VkFramebuffer framebuffer;
};

class SwapChainHandler
{
  private:
    VkSwapchainKHR swapChain;
    VkQueue graphicsQueue;
    VkFormat imageFormat;
    VkExtent2D extent;
    VkCommandPool commandPool;
    VkRenderPass renderPass;

    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;

    std::vector<VkImage> images;
    std::vector<VkImageView> imageViews;
    std::vector<VkFramebuffer> framebuffers;

    GLFWwindow* window;
    Device* device;
    VkSurfaceKHR surface;

  public:
    SwapChainHandler(GLFWwindow* window, VkSurfaceKHR surface, Device* device);

    ~SwapChainHandler();

    // Getters
    VkExtent2D GetExtent();
    uint32_t GetCount();
    uint32_t GetWidth();
    uint32_t GetHeight();
    VkFormat GetImageFormat();
    VkRenderPass GetRenderPass();
    VkCommandPool GetCommandPool();
    VkFramebuffer GetFrameBuffer(uint32_t index);
    VkImageView GetImageView(uint32_t index);
    VkSwapchainKHR GetSwapChain();

  private:
    void CreateSwapChain();
    void CreateImageViews();
    void CreateRenderPass();
    void CreateCommandPool();
    void CreateDepthResource();
    void CreateFrameBuffers();

    void CleanupSwapChain();

    // Helper functions
    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    VkFormat FindDepthFormat();
    VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

  public:
    static SwapChainSupportDetails QuerySwapChainSupport(VkSurfaceKHR surface, VkPhysicalDevice physicalDevice);
};
