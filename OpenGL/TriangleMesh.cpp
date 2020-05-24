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
    // Create an instance of the Importer class
    Assimp::Importer importer;
    
    // And have it read the given file with some example postprocessing
    // Usually - if speed is not the most important aspect for you - you'll
    // probably to request more postprocessing than we do in this example.
    const aiScene* scene = importer.ReadFile( path,
                                             aiProcess_CalcTangentSpace       |
                                             aiProcess_Triangulate            |
                                             aiProcess_JoinIdenticalVertices  |
                                             aiProcess_SortByPType            |
                                             aiProcess_GenNormals);
    
    // If the import failed, report it
    if( !scene)
        throw std::runtime_error(importer.GetErrorString());
    
    ProcessModel(scene);
    
}

void TriangleMesh::ProcessModel(const aiScene* scene)
{
    ASSERT(scene != nullptr);
    
    if(scene->HasMeshes())
    {
        aiMesh& mesh = *(*(scene->mMeshes));
        
        ASSERT(mesh.mPrimitiveTypes == aiPrimitiveType_TRIANGLE);
     
        ProcessPositions(mesh);
        ProcessIndices(mesh);
        ProcessNormals(mesh);
        ProcessUVCoords(mesh);
    }
}

void TriangleMesh::ProcessPositions(const aiMesh& mesh)
{
    if(!mesh.HasPositions())
        throw std::runtime_error("Need positions to draw!");

    mPositions.resize(mesh.mNumVertices * kCoordinates);

    unsigned int pIndex = 0;
    for(int vIndex = 0; vIndex < mesh.mNumVertices; vIndex++)
    {
        const auto& vertex = mesh.mVertices[vIndex];
        mPositions[pIndex++] = vertex.x;
        mPositions[pIndex++] = vertex.y;
        mPositions[pIndex++] = vertex.z;        
    }
}

void TriangleMesh::ProcessIndices(const aiMesh& mesh)
{
    if(!mesh.HasFaces())
        throw std::runtime_error("Need Faces to draw!");
    
    mIndices.resize(mesh.mNumFaces * kTriangleVertices);
    
    for(int fIndex = 0; fIndex < mesh.mNumFaces; fIndex++)
        for(int position = 0; position < mesh.mFaces[fIndex].mNumIndices; position++)
            mIndices[fIndex * kTriangleVertices + position] = mesh.mFaces[fIndex].mIndices[position];
}

void TriangleMesh::ProcessNormals(const aiMesh& mesh)
{
    /* ToDo: Replace runntime throw with msg assert.*/

    if(!mesh.HasNormals())
        throw std::runtime_error("Normal not found!");
    
    mNormals.resize(mesh.mNumVertices * kCoordinates);
    
    unsigned int nIndex = 0;
    for(int vIndex = 0; vIndex < mesh.mNumVertices; vIndex++)
    {
        const auto& vertex = mesh.mNormals[vIndex].Normalize();
        mNormals[nIndex++] = vertex.x;
        mNormals[nIndex++] = vertex.y;
        mNormals[nIndex++] = vertex.z;
    }
}

void TriangleMesh::ProcessUVCoords(const aiMesh& mesh)
{
    /* WARNING: I ONLY SUPPORT SINGLE CHANNEL 2D UV COORDS FOR NOW. */
    /* ToDo: Support for 3D texture. */
    
    mUVCoords.clear();      /* Optional processing code ahead, clear this */

    const unsigned int channelIndex = 0;
    
    if(!mesh.HasTextureCoords(channelIndex))
        return;
    
    mUVCoords.resize(mesh.mNumVertices * kTextureCoordinates);

    unsigned int uvIndex = 0;
    for(int vIndex = 0; vIndex < mesh.mNumVertices; vIndex++)
    {
        mUVCoords[uvIndex++] = mesh.mTextureCoords[channelIndex][vIndex].x;
        mUVCoords[uvIndex++] = mesh.mTextureCoords[channelIndex][vIndex].y;
    }

    /*
     unsigned int numChannel = mesh.GetNumUVChannels();
     
     if(numChannel)
     mUVCoords.resize(numChannel);

     for(unsigned int channel = 0; channel < numChannel; channel++)
    {
        if(!mesh.HasTextureCoords(channel))
            throw std::runtime_error("Texture for channel not found!");

        mUVCoords[channel].resize(mesh.mNumVertices * kTextureCoordinates);

        unsigned int uvIndex = 0;
        for(int vIndex = 0; vIndex < mesh.mNumVertices; vIndex++)
        {
            mUVCoords[channel][uvIndex++] = mesh.mTextureCoords[channel][vIndex].x;
            mUVCoords[channel][uvIndex++] = mesh.mTextureCoords[channel][vIndex].y;
            mUVCoords[channel][uvIndex++] = mesh.mTextureCoords[channel][vIndex].z;
        }
    }
    */
}

const std::vector<float>& TriangleMesh::GetPositions() const
{
    return mPositions;
}

const std::vector<unsigned int>& TriangleMesh::GetIndices() const
{
    return mIndices;
}

const std::vector<float>& TriangleMesh::GetNormals() const
{
    return mNormals;
}

const std::vector<float>& TriangleMesh::GetUVCoords() const
{
    return mUVCoords;
}
