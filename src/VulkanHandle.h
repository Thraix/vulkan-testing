#pragma once

#include <vulkan/vulkan.h>

#include <vector>
#include <iostream>
#include <optional>

struct QueueFamilyIndices
{
  std::optional<uint32_t> graphicsFamily;
  std::optional<uint32_t> presentFamily;

  bool IsComplete()
  {
    return graphicsFamily.has_value() && presentFamily.has_value();
  }
};

struct VulkanHandle
{
  static uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
  {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
    for(uint32_t i = 0;i<memProperties.memoryTypeCount; i++)
    {
      if(typeFilter & (1  << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        return i;
    }
    throw std::runtime_error("Failed to find suitable memory type");
  }

  static QueueFamilyIndices FindQueueFamilies(VkSurfaceKHR surface, VkPhysicalDevice device)
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

  static VkCommandBuffer BeginSingleTimeCommand(VkDevice device, VkCommandPool commandPool)
  {
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    return commandBuffer;
  }

  static void EndSingleTimeCommand(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkCommandBuffer commandBuffer)
  {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);
    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
  }


  static void VkSubmitDebugUtilsMessageEXT(
      VkInstance instance,
      VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
      VkDebugUtilsMessageTypeFlagsEXT messageTypes,
      const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData)
  {
    auto func = (PFN_vkSubmitDebugUtilsMessageEXT) 
      vkGetInstanceProcAddr(instance, "vkSubmitDebugUtilsMessageEXT");

    if(func)
      func(instance, messageSeverity, messageTypes, pCallbackData);
  }

  static VkResult CreateDebugUtilsMessengerEXT(
      VkInstance instance, 
      const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, 
      const VkAllocationCallbacks* pAllocator, 
      VkDebugUtilsMessengerEXT* pDebugMessenger)
  {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) 
      vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

    if(func)
      return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    else
      return VK_ERROR_EXTENSION_NOT_PRESENT;
  }

  static void DestroyDebugUtilsMessengerEXT(
      VkInstance instance, 
      const VkDebugUtilsMessengerEXT debugMessenger, 
      const VkAllocationCallbacks* pAllocator)
  {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) 
      vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

    if(func)
      func(instance, debugMessenger, pAllocator);
  }

};
