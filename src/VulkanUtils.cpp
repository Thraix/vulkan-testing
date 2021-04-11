#include "VulkanUtils.h"

#include "VulkanContext.h"

void VulkanUtils::CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
{
  VkImageCreateInfo imageInfo = {};
  imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.imageType = VK_IMAGE_TYPE_2D;
  imageInfo.extent.width = width;
  imageInfo.extent.height = height;
  imageInfo.extent.depth = 1;
  imageInfo.mipLevels = 1;
  imageInfo.arrayLayers = 1;
  imageInfo.format = format;
  imageInfo.tiling = tiling;
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageInfo.usage = usage;
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imageInfo.flags = 0;

  if(vkCreateImage(VulkanContext::GetDevice(), &imageInfo, nullptr, &image) != VK_SUCCESS)
    throw std::runtime_error("Failed to create image");

  VkMemoryRequirements memRequirements;
  vkGetImageMemoryRequirements(VulkanContext::GetDevice(), image, &memRequirements);

  VkMemoryAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

  if(vkAllocateMemory(VulkanContext::GetDevice(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
    throw std::runtime_error("Failed to allocate image memory");
  vkBindImageMemory(VulkanContext::GetDevice(), image, imageMemory, 0);
}

VkImageView VulkanUtils::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
{
  VkImageViewCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  createInfo.image = image;
  createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  createInfo.format = format;
  createInfo.subresourceRange.aspectMask = aspectFlags;
  createInfo.subresourceRange.baseMipLevel = 0;
  createInfo.subresourceRange.levelCount = 1;
  createInfo.subresourceRange.baseArrayLayer = 0;
  createInfo.subresourceRange.layerCount = 1;
  VkImageView imageView;
  if(vkCreateImageView(VulkanContext::GetDevice(), &createInfo, nullptr, &imageView) != VK_SUCCESS)
    throw std::runtime_error("Failed to create image view");
  return imageView;
}

void VulkanUtils::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
  VkCommandBuffer commandBuffer = BeginSingleTimeCommand();

  VkImageMemoryBarrier barrier = {};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = oldLayout;
  barrier.newLayout = newLayout;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image = image;
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;

  if(newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
  {
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    if(HasStencilComponent(format))
      barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
  }
  else
  {
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  }
  VkPipelineStageFlags srcStage;
  VkPipelineStageFlags dstStage;

  if(oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
  {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  }
  else if(oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
  {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  }
  else if(oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
  {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  }
  else
    throw std::runtime_error("Unsupported layout transition");

  vkCmdPipelineBarrier(commandBuffer, srcStage, dstStage, 0, 0, nullptr, 0, nullptr,1, &barrier);

  EndSingleTimeCommand(commandBuffer);
}

bool VulkanUtils::HasStencilComponent(VkFormat format)
{
  return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}


VkCommandBuffer VulkanUtils::BeginSingleTimeCommand()
{
  VkCommandBufferAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = VulkanContext::GetCommandPool();
  allocInfo.commandBufferCount = 1;

  VkCommandBuffer commandBuffer;
  vkAllocateCommandBuffers(VulkanContext::GetDevice(), &allocInfo, &commandBuffer);

  VkCommandBufferBeginInfo beginInfo = {};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(commandBuffer, &beginInfo);
  return commandBuffer;
}

void VulkanUtils::EndSingleTimeCommand(VkCommandBuffer commandBuffer)
{
  vkEndCommandBuffer(commandBuffer);

  VkSubmitInfo submitInfo = {};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;

  vkQueueSubmit(VulkanContext::GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(VulkanContext::GetGraphicsQueue());
  vkFreeCommandBuffers(VulkanContext::GetDevice(), VulkanContext::GetCommandPool(), 1, &commandBuffer);
}

uint32_t VulkanUtils::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
  VkPhysicalDeviceMemoryProperties memProperties;
  vkGetPhysicalDeviceMemoryProperties(VulkanContext::GetPhysicalDevice(), &memProperties);
  for(uint32_t i = 0;i<memProperties.memoryTypeCount; i++)
  {
    if(typeFilter & (1  << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
      return i;
  }
  throw std::runtime_error("Failed to find suitable memory type");

}

void VulkanUtils::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
  VkBufferCreateInfo bufferInfo = {};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = size;
  bufferInfo.usage = usage;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if(vkCreateBuffer(VulkanContext::GetDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
    throw std::runtime_error("Failed to create vertex buffer");

  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(VulkanContext::GetDevice(), buffer, &memRequirements);

  VkMemoryAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = VulkanUtils::FindMemoryType(memRequirements.memoryTypeBits, properties);

  if(vkAllocateMemory(VulkanContext::GetDevice(),&allocInfo,nullptr, &bufferMemory) != VK_SUCCESS)
    throw std::runtime_error("Failed to allocate vertex buffer memory");

  vkBindBufferMemory(VulkanContext::GetDevice(), buffer, bufferMemory, 0);
}

void VulkanUtils::DestroyBuffer(VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
  vkDestroyBuffer(VulkanContext::GetDevice(), buffer, nullptr);
  vkFreeMemory(VulkanContext::GetDevice(), bufferMemory, nullptr);
}

void VulkanUtils::UpdateBuffer(VkDeviceMemory buffer, const void* data, uint32_t size)
{
  void* dataTemp;
  vkMapMemory(VulkanContext::GetDevice(), buffer, 0, size, 0, &dataTemp);
  memcpy(dataTemp, data, size);
  vkUnmapMemory(VulkanContext::GetDevice(), buffer);
}

void VulkanUtils::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
  VkCommandBuffer commandBuffer = VulkanUtils::BeginSingleTimeCommand();
  {
    VkBufferCopy copyRegion = {};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
  }
  VulkanUtils::EndSingleTimeCommand(commandBuffer);
}
