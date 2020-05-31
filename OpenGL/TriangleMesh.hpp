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
#include <map>

class TriangleMesh
{
public:

    struct Attributes
    {
        std::vector<float>                  mPositions;
        std::vector<float>                  mNormals;
        std::vector<unsigned int>           mIndices;
        std::vector<float>                  mUVCoords;
    };
    
    using MeshID = unsigned int;

    TriangleMesh(const std::string& path);
    
    void Import3DModel(const std::string& path);
    void CleanModel();
    
    const Attributes& GetMeshAttributes(MeshID) const;
    const std::map<MeshID, Attributes>& GetModelMesh() const;
    unsigned int GetNumberOfMeshes() const;

    
private:

    static constexpr unsigned kCoordinates = 3;
    static constexpr unsigned kTriangleVertices = 3;
    static constexpr unsigned kTextureCoordinates = 2;

    void ProcessModel(const aiScene* scene);
    void ProcessPositions(const aiMesh& mesh, MeshID);
    void ProcessIndices(const aiMesh& mesh, MeshID);
    void ProcessNormals(const aiMesh& mesh, MeshID);
    void ProcessUVCoords(const aiMesh& mesh, MeshID);
    
    std::map<MeshID, Attributes> mMeshes;
};

#endif
