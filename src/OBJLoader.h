#pragma once

#include <vector>
#include <string>
#include <fstream>

#include "Mesh.h"
#include "math/Vec3.h"

class OBJLoader
{
  private:
    static inline void ProcessVertex(
        const std::vector<std::string>& vertexData,
        std::vector<uint>& indicesArray,
        const std::vector<Greet::Vec2>& texCoords,
        const std::vector<Greet::Vec3>& normals,
        Greet::Vec2* texCoordsArray,
        Greet::Vec3* normalsArray);
  public:
    static MeshData LoadObj(const std::string& filename);
};
