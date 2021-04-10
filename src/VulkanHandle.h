#pragma once

#include <vulkan/vulkan.h>
#include <iostream>

class VulkanHandle
{
  public:
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
