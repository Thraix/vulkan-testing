#pragma once

#include <vulkan/vulkan.h>

#include "VulkanUtils.h"
#include "ImageUtils.h"
#include "VulkanContext.h"

struct TextureParams
{
  VkFilter minFilter = VK_FILTER_LINEAR;
  VkFilter magFilter = VK_FILTER_LINEAR;
  VkSamplerAddressMode wrap = VK_SAMPLER_ADDRESS_MODE_REPEAT;
};

class Texture
{
  private:
    VkImage textureImage;
    VkImageView textureImageView;
    VkSampler textureSampler;
    VkDeviceMemory textureImageMemory;
    uint32_t width;
    uint32_t height;

  public:
    Texture(const std::string& filename, const TextureParams& params = TextureParams())
    {
      LoadImage(filename);
      CreateTextureImageView();
      CreateTextureSampler(params);
    }

    virtual ~Texture()
    {
      vkDestroySampler(VulkanContext::GetDevice(), textureSampler, nullptr);
      vkDestroyImageView(VulkanContext::GetDevice(), textureImageView, nullptr);
      vkDestroyImage(VulkanContext::GetDevice(), textureImage, nullptr);
      vkFreeMemory(VulkanContext::GetDevice(), textureImageMemory, nullptr);
    }

    VkDescriptorImageInfo GetDescriptor()
    {
      VkDescriptorImageInfo imageInfo = {};
      imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      imageInfo.imageView = textureImageView;
      imageInfo.sampler = textureSampler;
      return imageInfo;
    }

  private:
    void LoadImage(const std::string& filename)
    {
      BYTE* bytes = ImageUtils::LoadImage(filename, &width, &height);

      VkDeviceSize imageSize = width * height * 4;

      VkBuffer stagingBuffer;
      VkDeviceMemory stagingBufferMemory;
      VulkanUtils::CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

      VulkanUtils::UpdateBuffer(stagingBufferMemory, bytes, imageSize);

      delete[] bytes;
      VulkanUtils::CreateImage(width, height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

      VulkanUtils::TransitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
      CopyBufferToImage(stagingBuffer, textureImage, width, height);

      VulkanUtils::TransitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
      vkDestroyBuffer(VulkanContext::GetDevice(), stagingBuffer, nullptr);
      vkFreeMemory(VulkanContext::GetDevice(), stagingBufferMemory, nullptr);
    }

    void CreateTextureImageView()
    {
      textureImageView = VulkanUtils::CreateImageView(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
    }

    void CreateTextureSampler(const TextureParams& params)
    {
      VkSamplerCreateInfo samplerInfo = {};
      samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
      samplerInfo.magFilter = params.magFilter;
      samplerInfo.minFilter = params.minFilter;
      samplerInfo.addressModeU = params.wrap;
      samplerInfo.addressModeV = params.wrap;
      samplerInfo.addressModeW = params.wrap;
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

      if(vkCreateSampler(VulkanContext::GetDevice(), &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS)
        throw std::runtime_error("Failed to create texture sampler");

    }

    void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
    {
      VkCommandBuffer commandBuffer = VulkanUtils::BeginSingleTimeCommand();

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

      VulkanUtils::EndSingleTimeCommand(commandBuffer);
    }
};
