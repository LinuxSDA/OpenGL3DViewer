//
//  LoadModel.hpp
//  OpenGL
//
//  Created by Sumit Dhingra on 05/05/20.
//  Copyright © 2020 LinuxSDA. All rights reserved.
//

#ifndef TriangleMesh_hpp
#define TriangleMesh_hpp

#include <string>
#include "assimp/scene.h"           // Output data structure
#include <vector>
#include <float.h>
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"

class TriangleMesh
{
public:
    TriangleMesh(const std::string& path);
    void Import3DModel(const std::string& path);
 
    const std::vector<float>&          GetNormals() const;
    const std::vector<float>&          GetPositions() const;
    const std::vector<unsigned int>&   GetIndices() const;
    const std::vector<float>&          GetUVCoords() const;

private:

    static constexpr unsigned kCoordinates = 3;
    static constexpr unsigned kTriangleVertices = 3;
    static constexpr unsigned kTextureCoordinates = 2;

    void ProcessModel(const aiScene* scene);
    void ProcessPositions(const aiMesh& mesh);
    void ProcessIndices(const aiMesh& mesh);
    void ProcessNormals(const aiMesh& mesh);
    void ProcessUVCoords(const aiMesh& mesh);
    
    std::vector<float>                  mPositions;
    std::vector<float>                  mNormals;
    std::vector<unsigned int>           mIndices;
    std::vector<float>                  mUVCoords;
};

#endif
