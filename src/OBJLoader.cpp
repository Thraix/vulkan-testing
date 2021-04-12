#include "OBJLoader.h"

#include <map>
#include <sstream>

static std::vector<std::string> SplitString(const std::string &s, const std::string& delimiter)
{
  size_t start = 0;
  size_t end = s.find_first_of(delimiter);

  std::vector<std::string> output;

  while (end <= std::string::npos)
  {
    output.emplace_back(s.substr(start, end - start));

    if (end == std::string::npos)
      break;

    start = end + 1;
    end = s.find_first_of(delimiter, start);
  }

  return output;
}

MeshData OBJLoader::LoadObj(const std::string& filename)
{
  std::vector<std::string> dataLine;

  std::vector<Greet::Vec3> positions;
  std::vector<Greet::Vec2> texCoords;
  std::vector<Greet::Vec3> normals;
  std::vector<uint> indices;

  std::ifstream input(filename);
  std::map<std::string, uint> verticesMap;

  // These are the same length.
  std::vector<Greet::Vec3> vertexPos;
  std::vector<Greet::Vec2> vertexTexCoords;
  std::vector<Greet::Vec3> vertexNormals;

  for (std::string line; getline(input, line); )
  {
    std::istringstream iss{line};
    std::string s;
    iss >> s;
    if (s == "v")
    {
      Greet::Vec3 vector{};
      iss >> vector.x;
      iss >> vector.y;
      iss >> vector.z;
      positions.push_back(vector);
    }
    else if (s == "vt")
    {
      Greet::Vec2 vector{};
      iss >> vector.x;
      iss >> vector.y;
      texCoords.push_back(vector);
    }
    else if (s == "vn")
    {
      Greet::Vec3 vector{};
      iss >> vector.x;
      iss >> vector.y;
      iss >> vector.z;
      normals.push_back(vector);
    }
    else if (s == "f")
    {
      dataLine = SplitString(line, " ");
      for(size_t i = 1; i< dataLine.size(); i++)
      {
        if(dataLine[i] == "" || dataLine[i] == "\r")
          continue;
        auto it = verticesMap.find(dataLine[i]);
        if(it != verticesMap.end())
        {
          indices.push_back(it->second);
        }
        else
        {
          indices.push_back(vertexPos.size());
          verticesMap.emplace(dataLine[i], vertexPos.size());
          std::vector<std::string> vertex = SplitString(dataLine[i], "/");

          // Only position defined
          if(vertex.size() == 1)
          {
            vertexPos.push_back(positions[atoi(vertex[0].c_str())-1]);
          }
          else if(vertex.size() == 2) // Position and texcoord defined
          {
            vertexPos.push_back(positions[atoi(vertex[0].c_str())-1]);
            if(vertex[1] != "")
              vertexTexCoords.push_back(texCoords[atoi(vertex[1].c_str())-1]);
          }
          else if(vertex.size() == 3)
          {
            vertexPos.push_back(positions[atoi(vertex[0].c_str())-1]);
            if(vertex[1] != "")
              vertexTexCoords.push_back(texCoords[atoi(vertex[1].c_str())-1]);
            if(vertex[2] != "")
              vertexNormals.push_back(normals[atoi(vertex[2].c_str())-1]);
          }
        }
      }
    }
  }

  input.close();

  return MeshData{vertexPos, indices, vertexNormals, vertexTexCoords};
}
