//
//  TriangleMesh.cpp
//  OpenGL
//
//  Created by Sumit Dhingra on 05/05/20.
//  Copyright © 2020 LinuxSDA. All rights reserved.
//

#include "TriangleMesh.hpp"
#include "assimp/Importer.hpp"      // C++ importer interface
#include "assimp/scene.h"           // Output data structure
#include "assimp/postprocess.h"     // Post processing flags
#include "ErrorHandler.hpp"
#include <algorithm>

TriangleMesh::TriangleMesh(const std::string& path)
{
    Import3DModel(path);
}

void TriangleMesh::Import3DModel(const std::string& path)
{
    CleanModel();
    // Create an instance of the Importer class
    Assimp::Importer importer;
    
    // And have it read the given file with some example postprocessing
    // Usually - if speed is not the most important aspect for you - you'll
    // probably to request more postprocessing than we do in this example.
    const aiScene* scene = importer.ReadFile( path,
                                             aiProcess_Triangulate            |
                                             aiProcess_JoinIdenticalVertices  |
                                             aiProcess_GenNormals             |
                                             aiProcess_OptimizeMeshes         |
                                             aiProcess_SplitLargeMeshes);
    
    // If the import failed, report it
    if(!scene)
        throw std::runtime_error(importer.GetErrorString());
    
    ProcessModel(scene);
}

void TriangleMesh::CleanModel()
{
    mMeshes.clear();
}

void TriangleMesh::ProcessModel(const aiScene* scene)
{
    ASSERT(scene != nullptr);
    
    if(scene->HasMeshes())
    {
        for(unsigned int num = 0; num < scene->mNumMeshes; num++)
        {
            aiMesh& mesh = *((scene->mMeshes)[num]);
            
            ASSERT(mesh.mPrimitiveTypes == aiPrimitiveType_TRIANGLE);
            
            ProcessPositions(mesh, num);
            ProcessIndices(mesh, num);
            ProcessNormals(mesh, num);
            ProcessUVCoords(mesh, num);
        }
    }
}

void TriangleMesh::ProcessPositions(const aiMesh& mesh, MeshID id)
{
    ASSERT(mesh.HasPositions());

    Attributes& attr = mMeshes[id];
    attr.mPositions.resize(mesh.mNumVertices * kCoordinates);

    unsigned int pIndex = 0;
    for(int vIndex = 0; vIndex < mesh.mNumVertices; vIndex++)
    {
        const auto& vertex = mesh.mVertices[vIndex];
        attr.mPositions[pIndex++] = vertex.x;
        attr.mPositions[pIndex++] = vertex.y;
        attr.mPositions[pIndex++] = vertex.z;
    }
}

void TriangleMesh::ProcessIndices(const aiMesh& mesh, MeshID id)
{
    ASSERT(mesh.HasFaces());
    
    Attributes& attr = mMeshes[id];
    attr.mIndices.resize(mesh.mNumFaces * kTriangleVertices);
    
    for(int fIndex = 0; fIndex < mesh.mNumFaces; fIndex++)
        for(int position = 0; position < mesh.mFaces[fIndex].mNumIndices; position++)
            attr.mIndices[fIndex * kTriangleVertices + position] = mesh.mFaces[fIndex].mIndices[position];
}

void TriangleMesh::ProcessNormals(const aiMesh& mesh, MeshID id)
{
    ASSERT(mesh.HasNormals());

    Attributes& attr = mMeshes[id];
    attr.mNormals.resize(mesh.mNumVertices * kCoordinates);
    
    unsigned int nIndex = 0;
    for(int vIndex = 0; vIndex < mesh.mNumVertices; vIndex++)
    {
        const auto& vertex = mesh.mNormals[vIndex].Normalize();
        attr.mNormals[nIndex++] = vertex.x;
        attr.mNormals[nIndex++] = vertex.y;
        attr.mNormals[nIndex++] = vertex.z;
    }
}

void TriangleMesh::ProcessUVCoords(const aiMesh& mesh, MeshID id)
{
    /* WARNING: I ONLY SUPPORT SINGLE CHANNEL 2D UV COORDS FOR NOW. */
    /* ToDo: Support for 3D texture. */
    const unsigned int channelIndex = 0;
    
    /* WARNING: If any mesh doesn't have UV coordinates then assert for now. Will make UV optional later on. */
    ASSERT(mesh.HasTextureCoords(channelIndex));

    Attributes& attr = mMeshes[id];
    attr.mUVCoords.resize(mesh.mNumVertices * kTextureCoordinates);

    unsigned int uvIndex = 0;
    for(int vIndex = 0; vIndex < mesh.mNumVertices; vIndex++)
    {
        attr.mUVCoords[uvIndex++] = mesh.mTextureCoords[channelIndex][vIndex].x;
        attr.mUVCoords[uvIndex++] = mesh.mTextureCoords[channelIndex][vIndex].y;
    }
}

const TriangleMesh::Attributes& TriangleMesh::GetMeshAttributes(MeshID id) const
{
    return mMeshes.at(id);
}

const std::map<TriangleMesh::MeshID, TriangleMesh::Attributes>& TriangleMesh::GetModelMesh() const
{
    return mMeshes;
}

unsigned int TriangleMesh::GetNumberOfMeshes() const
{
    return mMeshes.size();
}
