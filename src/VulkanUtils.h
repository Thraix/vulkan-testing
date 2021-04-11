#pragma once

#include <vulkan/vulkan.h>

class VulkanUtils
{
  public:
    static void CreateImage(
        uint32_t width,
        uint32_t height,
        VkFormat format,
        VkImageTiling tiling,
        VkImageUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkImage& image,
        VkDeviceMemory& imageMemory);

    static VkImageView CreateImageView(
        VkImage image,
        VkFormat format,
        VkImageAspectFlags aspectFlags);

    static void TransitionImageLayout(
        VkImage image,
        VkFormat format,
        VkImageLayout oldLayout,
        VkImageLayout newLayout);

    static bool HasStencilComponent(
        VkFormat format);

    static VkCommandBuffer BeginSingleTimeCommand();

    static void EndSingleTimeCommand(
        VkCommandBuffer commandBuffer);

    static uint32_t FindMemoryType(
        uint32_t typeFilter,
        VkMemoryPropertyFlags properties);

    static void CreateBuffer(
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkBuffer& buffer,
        VkDeviceMemory& bufferMemory);

    static void DestroyBuffer(
        VkBuffer& buffer,
        VkDeviceMemory& bufferMemory);

    static void UpdateBuffer(
        VkDeviceMemory buffer,
        const void* data,
        uint32_t size);

    static void CopyBuffer(
        VkBuffer srcBuffer,
        VkBuffer dstBuffer,
        VkDeviceSize size);
};
