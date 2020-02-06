#pragma once

#include <vulkan/vulkan.h>
#include <set>
#include <vector>


struct DeviceSetup
{
  std::vector<const char*> deviceExtensions;
  std::vector<const char*> validationLayers;

  VkInstance instance;
  VkSurfaceKHR surface;
};

class Device
{
  private:
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;

    VkQueue graphicsQueue;
    VkQueue presentQueue;
  public:
    Device(std::initializer_list<const char*> deviceExtensions, std::initializer_list<const char*> validationLayers, VkInstance instance, VkSurfaceKHR surface);

    VkDevice GetDevice() const { return device; }
    VkPhysicalDevice GetPhysicalDevice() const { return physicalDevice; }
    VkQueue GetGraphicsQueue() const { return graphicsQueue; }
    VkQueue GetPresentQueue() const { return presentQueue; }

  private:
    void PickPhysicalDevice(DeviceSetup& setup);

    // Check if the GPU has support for the wanted operations
    bool IsDeviceSuitable(DeviceSetup& setup, VkPhysicalDevice device);
    bool CheckDeviceExtensionSupport(DeviceSetup& setup, VkPhysicalDevice device);
    void PrintPhysicalDeviceName(VkPhysicalDevice device);

    void CreateLogicalDevice(DeviceSetup& setup);
};
