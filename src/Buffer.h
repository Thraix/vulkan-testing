#pragma once

#include <vulkan/vulkan.h>
#include <vector>

#include "VulkanUtils.h"
#include "VulkanContext.h"

enum class BufferType
{
  VERTEX = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
  INDEX = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
  UNIFORM = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
};

enum class BufferUpdateType
{
  STATIC, DYNAMIC
};

class Buffer
{

  private:
    uint32_t dataSize;
    VkBuffer buffer;
    VkDeviceMemory bufferMemory;

    BufferUpdateType updateType;

  public:
    Buffer(void* data, uint32_t dataSize, BufferType type, BufferUpdateType updateType)
      : dataSize{dataSize}, updateType{updateType}
    {
      CreateBuffer(type);
      UpdateBuffer(data);
    }

    Buffer(uint32_t dataSize, BufferType type, BufferUpdateType updateType)
      : dataSize{dataSize}, updateType{updateType}
    {
      CreateBuffer(type);
    }

    void CreateBuffer(BufferType type)
    {
      if(updateType == BufferUpdateType::STATIC)
      {
        VulkanUtils::CreateBuffer(dataSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | (VkBufferUsageFlagBits)type, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, bufferMemory);
      }
      else
      {
        VulkanUtils::CreateBuffer(dataSize, (VkBufferUsageFlagBits)type, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, buffer, bufferMemory);
      }
    }

    virtual ~Buffer()
    {
      VulkanUtils::DestroyBuffer(buffer, bufferMemory);
    }

    VkBuffer GetBuffer() const
    {
      return buffer;
    }

    void UpdateBuffer(void* data)
    {
      switch(updateType)
      {
        case BufferUpdateType::STATIC:
        {
          VkBuffer stagingBuffer;
          VkDeviceMemory stagingBufferMemory;

          VulkanUtils::CreateBuffer(dataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,stagingBufferMemory);
          VulkanUtils::UpdateBuffer(stagingBufferMemory, data, dataSize);
          VulkanUtils::CopyBuffer(stagingBuffer, buffer, dataSize);
          VulkanUtils::DestroyBuffer(stagingBuffer, stagingBufferMemory);
          break;
        }
        case BufferUpdateType::DYNAMIC:
        {
          VulkanUtils::UpdateBuffer(bufferMemory, data, dataSize);
          break;
        }
      }
    }
};
