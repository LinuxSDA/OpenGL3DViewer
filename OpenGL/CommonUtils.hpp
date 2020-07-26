//
//  CommonUtils.hpp
//  OpenGL
//
//  Created by Sumit Dhingra on 23/05/20.
//  Copyright © 2020 LinuxSDA. All rights reserved.
//

#ifndef CommonUtils_h
#define CommonUtils_h

#include "gtx/euler_angles.hpp"

namespace CommonUtils
{
    struct BBCoord
    {
        glm::vec3 Min;
        glm::vec3 Max;
    };
    
    BBCoord GetBBox(const std::vector<BBCoord>& vertices)
    {
        BBCoord boundingBox = vertices[0];
        
        for (size_t i = 1; i < vertices.size(); i++)
        {
            boundingBox.Min.x = std::min(boundingBox.Min.x, vertices[i].Min.x);
            boundingBox.Min.y = std::min(boundingBox.Min.y, vertices[i].Min.y);
            boundingBox.Min.z = std::min(boundingBox.Min.z, vertices[i].Min.z);
            
            boundingBox.Max.x = std::max(boundingBox.Max.x, vertices[i].Max.x);
            boundingBox.Max.y = std::max(boundingBox.Max.y, vertices[i].Max.y);
            boundingBox.Max.z = std::max(boundingBox.Max.z, vertices[i].Max.z);
        }
        
        return boundingBox;
    }
    
    BBCoord GetBBox(const std::vector<float>& serializedVertices)
    {
        BBCoord boundingBox{};
        
        boundingBox.Min.x = serializedVertices[0];
        boundingBox.Min.y = serializedVertices[1];
        boundingBox.Min.z = serializedVertices[2];
        
        boundingBox.Max.x = serializedVertices[0];
        boundingBox.Max.y = serializedVertices[1];
        boundingBox.Max.z = serializedVertices[2];
        
        for (size_t i = 3; i < serializedVertices.size(); i += 3)
        {
            boundingBox.Min.x = std::min(boundingBox.Min.x, serializedVertices[i + 0]);
            boundingBox.Min.y = std::min(boundingBox.Min.y, serializedVertices[i + 1]);
            boundingBox.Min.z = std::min(boundingBox.Min.z, serializedVertices[i + 2]);
            
            boundingBox.Max.x = std::max(boundingBox.Max.x, serializedVertices[i + 0]);
            boundingBox.Max.y = std::max(boundingBox.Max.y, serializedVertices[i + 1]);
            boundingBox.Max.z = std::max(boundingBox.Max.z, serializedVertices[i + 2]);
        }
        
        return boundingBox;
    }
    
    BBCoord GetBBox(const TriangleMesh& model)
    {
        std::vector<CommonUtils::BBCoord> objectBBs;
        auto modelMeshes = model.GetModelMesh();
        objectBBs.reserve(modelMeshes.size());
        
        for (const auto& mesh: modelMeshes)
            objectBBs.emplace_back(CommonUtils::GetBBox(mesh.second.mPositions));
        
        return CommonUtils::GetBBox(objectBBs);
    }
    
    BBCoord GetBBox(const std::vector<TriangleMesh>& meshes)
    {
        std::vector<BBCoord> boundingBoxes;
        boundingBoxes.reserve(meshes.size());
        
        for(auto& mesh: meshes)
        {
            boundingBoxes.emplace_back(GetBBox(mesh));
        }
        
        return GetBBox(boundingBoxes);
    }
    
    BBCoord GetBBox(const std::vector<std::vector<float>>& serializedVerticesVec)
    {
        std::vector<BBCoord> boundingBoxes;
        boundingBoxes.reserve(serializedVerticesVec.size());
        
        for(auto& vertices: serializedVerticesVec)
        {
            boundingBoxes.emplace_back(GetBBox(vertices));
        }
        
        return GetBBox(boundingBoxes);
    }
    
    glm::vec3 GetBBoxCenter(const BBCoord& bbox)
    {
        return glm::vec3 {
            (bbox.Min.x + bbox.Max.x)/2.0f,
            (bbox.Min.y + bbox.Max.y)/2.0f,
            (bbox.Min.z + bbox.Max.z)/2.0f
        };
    }

    float GetBBoxHeight(const BBCoord& bbox)
    {
        return (bbox.Max.y - bbox.Min.y);
    }

    float GetBBoxWidth(const BBCoord& bbox)
    {
        return (bbox.Max.x - bbox.Min.x);
    }

    glm::mat4 GenerateOrthoMatrix(BBCoord span, float aspectRatio)
    {
        float width  = std::fabs(span.Min.x - span.Max.x);
        float height = std::fabs(span.Min.y - span.Max.y);
        float depth  = std::fabs(span.Min.z - span.Max.z);
        
        float currentAspectRatio = width/height;
        
        float factor = aspectRatio/currentAspectRatio;
        
        if(factor > 1.0f)
            width *= factor;
        else
            height /= factor;
        
        /* ToDo: choosing buffer depth */
        span.Min.z -= depth - 100; span.Max.z += depth + 100;
        
        return glm::ortho(-width/2.0f, width/2.0f, -height/2.0f, height/2.0f, span.Min.z, span.Max.z);
    }
    
    struct ModelMatrix
    {
        ModelMatrix(const BBCoord& completeSpan, const BBCoord& meshSpan)
        {
            fLocalCenter.x = ( completeSpan.Min.x + completeSpan.Max.x ) / 2.0f;
            fLocalCenter.y = ( completeSpan.Min.y + completeSpan.Max.y ) / 2.0f;
            fLocalCenter.z = ( completeSpan.Min.z + completeSpan.Max.z ) / 2.0f;
            
            fMeshCenter.x = ( meshSpan.Min.x + meshSpan.Max.x ) / 2.0f;
            fMeshCenter.y = ( meshSpan.Min.y + meshSpan.Max.y ) / 2.0f;
            fMeshCenter.z = ( meshSpan.Min.z + meshSpan.Max.z ) / 2.0f;
        }
        
        glm::vec3 fAngle       {0.0f, 0.0f, 0.0f};
        glm::vec3 fTranslation {0.0f, 0.0f, 0.0f};
        glm::vec3 fScale       {1.0f, 1.0f, 1.0f};
        
        glm::mat4 GetMatrix()
        {
            /* TODO: Optimise and organise*/
            auto TranslateToOrigin  = glm::translate(glm::identity<glm::mat4>(), -fMeshCenter);
            auto Scale              = glm::scale(glm::identity<glm::mat4>(), glm::vec3(fScale.x, fScale.y, fScale.z));
            auto Rotate             = glm::eulerAngleXYZ(fAngle.x, fAngle.y, fAngle.z);
            auto Translate          = glm::translate(glm::identity<glm::mat4>(), fTranslation);
            auto TranslateToInitial = glm::translate(glm::identity<glm::mat4>(), fMeshCenter);
            
            return TranslateToInitial * Translate * Rotate * Scale * TranslateToOrigin;
        }
        
    private:
        glm::vec3 fMeshCenter{};
        glm::vec3 fLocalCenter{};
        glm::vec3 fWorldCenter{0.0f, 0.0f, 0.0f};
    };
    
}

#endif /* CommonUtils_h */
