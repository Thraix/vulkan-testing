#include <string.h>
#include <fstream>
#include <math/Maths.h>

namespace Mesh
{
  struct MeshData
  {
    Greet::Vec3* vertices;
    uint32_t* indices;
    uint32_t vertexCount;
    uint32_t indexCount;

    MeshData(Greet::Vec3* vertices, uint32_t vertexCount, uint32_t* indices, uint32_t indexCount)
      : vertices{vertices}, vertexCount{vertexCount}, indices{indices}, indexCount{indexCount}
    {
    
    }
  };

  static MeshData ReadFromFile(const std::string& filename)
  {
    using namespace Greet;
    // Small buffer for initial stuffs
    char buffer[1024];
    std::ifstream fin(filename);

    // Determine size of the file and reset to beginning.
    int fileSize = fin.tellg();
    fin.seekg(0,std::ios::end);
    fileSize = (uint32_t)fin.tellg() - fileSize;
    fin.seekg(0,std::ios::beg);

    // Check if the file is big enough to contain signature, vertex count, index count and attribute count
    fileSize -= 4 * sizeof(char) + 2 * sizeof(uint32_t) + sizeof(size_t);
    if(fileSize < 0)
    {
      throw std::runtime_error("Could not read MESH file, file is too small to contain signature.");
    }

    // Read signature
    fin.read(buffer, 4 * sizeof(char));

    if(std::string(buffer, 4) != "MESH")
    {
      throw std::runtime_error("Could not read MESH file, signature invalid.");
    }
    // Read how many attributes we have. 
    uint32_t vertexCount;
    fin.read((char*)&vertexCount,sizeof(uint32_t));
    fileSize -= vertexCount * sizeof(Vec3);
    if(fileSize < 0)
    {
      throw std::runtime_error("Could not read MESH file, file is too small to contain vertex data");
    }
    if(vertexCount == 0)
      throw std::runtime_error("VertexCount is 0");

    // Read how many attributes we have. 
    uint32_t indexCount;
    fin.read((char*)&indexCount,sizeof(uint32_t));
    fileSize -= indexCount * sizeof(uint32_t);
    if(fileSize < 0)
    {
      throw std::runtime_error("Could not read MESH file, file is too small to contain vertex data");
    }
    if(indexCount == 0)
      throw std::runtime_error("IndexCount is 0");

    // Read how many attributes we have. 
    size_t attributeCount;
    fin.read((char*)&attributeCount,sizeof(size_t));

    // Check if the file is big enough to contain attribute parameters.
    int attribLength = (sizeof(uint32_t) * 4 + sizeof(bool));
    int attribsLength = attributeCount * attribLength; 

    // Remove attribLength from fileSize to easier check sizes later.
    fileSize -= attribsLength;

    if(fileSize < 0)
    {
      throw std::runtime_error("Could not read MESH file, file is too small to contain attribute parameters");
    }

    if(attribLength > 1024)
    {
      throw std::runtime_error("Could not read MESH file, too many attributes.");
    }
    fin.read(buffer,attribsLength);

    const char* pointer = buffer;
    for(size_t i = 0;i<attributeCount;i++)
    {
      // Read uints
      uint32_t location = ((uint*)pointer)[0];
      uint32_t vertexValueSize = ((uint*)pointer)[1];
      uint32_t memoryValueSize = ((uint*)pointer)[2];
      uint32_t glType = ((uint*)pointer)[3];

      // Move pointer
      pointer += sizeof(uint32_t)*4;

      // Read bool
      bool normalized = ((bool*)buffer)[0];

      // Move pointer
      pointer += sizeof(bool);

      fileSize -= memoryValueSize * vertexCount;
      if(fileSize < 0)
      {
        throw std::runtime_error("Could not read MESH file, file is too small to contain attribute data");
      }
    }

    // Read vertices
    Vec3* vertices = new Vec3[vertexCount];
    fin.read((char*)vertices,vertexCount*sizeof(Vec3));

    // Read indices 
    uint32_t* indices = new uint32_t[indexCount];
    fin.read((char*)indices,indexCount*sizeof(uint32_t));

    return {vertices, vertexCount, indices, indexCount};
  }

}
