#pragma once

#include <string.h>
#include <fstream>
#include <math/Maths.h>
#include <memory>

#include "Buffer.h"

#include <vector>

struct MeshData
{
  std::vector<Greet::Vec3> vertices;
  std::vector<uint32_t> indices;

  std::vector<Greet::Vec3> normals;
  std::vector<Greet::Vec2> texCoords;

  MeshData(const std::vector<Greet::Vec3>& vertices, const std::vector<uint32_t>& indices)
    : vertices{vertices}, indices{indices}
  {
    normals.resize(vertices.size());
    texCoords.resize(vertices.size());
  }

  MeshData(const std::vector<Greet::Vec3>& vertices, const std::vector<uint32_t>& indices, const std::vector<Greet::Vec3>& normals)
    : vertices{vertices}, indices{indices}, normals{normals}
  {
    texCoords.resize(vertices.size());
  }

  MeshData(const std::vector<Greet::Vec3>& vertices, const std::vector<uint32_t>& indices, const std::vector<Greet::Vec2>& texCoords)
    : vertices{vertices}, indices{indices}, texCoords{texCoords}
  {
    normals.resize(vertices.size());
  }

  MeshData(const std::vector<Greet::Vec3>& vertices, const std::vector<uint32_t>& indices, const std::vector<Greet::Vec3>& normals, const std::vector<Greet::Vec2>& texCoords)
    : vertices{vertices}, indices{indices}, normals{normals}, texCoords{texCoords}
  {}
};

class Mesh
{
  private:
    std::vector<std::shared_ptr<Buffer>> buffers;

    std::shared_ptr<Buffer> indexBuffer;
    uint32_t indices;

  public:
    Mesh(const MeshData& data)
      : indices{static_cast<uint32_t>(data.indices.size())}
    {
      buffers.push_back(std::make_shared<Buffer>((void*)data.vertices.data(), data.vertices.size() * sizeof(Greet::Vec3), BufferType::VERTEX, BufferUpdateType::STATIC));

      buffers.push_back(std::make_shared<Buffer>((void*)data.normals.data(), data.normals.size() * sizeof(Greet::Vec3), BufferType::VERTEX, BufferUpdateType::STATIC));

      buffers.push_back(std::make_shared<Buffer>((void*)data.texCoords.data(), data.texCoords.size() * sizeof(Greet::Vec2), BufferType::VERTEX, BufferUpdateType::STATIC));

      indexBuffer = std::make_shared<Buffer>((void*)data.indices.data(), data.indices.size() * sizeof(uint32_t), BufferType::INDEX, BufferUpdateType::STATIC);
    }

    void Render(VkCommandBuffer cmdBuffer)
    {
      VkDeviceSize offsets[] = {0, 0, 0};

      VkBuffer vkBuffers[] = {buffers[0]->GetBuffer(), buffers[1]->GetBuffer(), buffers[2]->GetBuffer()};

      vkCmdBindVertexBuffers(cmdBuffer, 0, static_cast<uint32_t>(buffers.size()), vkBuffers, offsets);
      vkCmdBindIndexBuffer(cmdBuffer, indexBuffer->GetBuffer(), 0,VK_INDEX_TYPE_UINT32);

      vkCmdDrawIndexed(cmdBuffer, indices, 1, 0, 0, 0);
    }
};
