//
//  ModelRenderer.cpp
//  OpenGL
//
//  Created by Sumit Dhingra on 11/06/20.
//  Copyright © 2020 LinuxSDA. All rights reserved.
//

#include "ModelRenderer.hpp"

namespace Helper
{
    
    ModelRenderer::ModelRenderer(const std::string& filepath):fModel(std::make_unique<TriangleMesh>(filepath))
    {
        Import();
    }

    ModelRenderer::~ModelRenderer()
    {
        
    }
    
    void ModelRenderer::Import()
    {
        const auto& modelMeshes = fModel->GetModelMesh();
        fModelVA.resize(modelMeshes.size());
        /* WARNING: careful not to reallocate any entry! */
        for (unsigned int index = 0; index < modelMeshes.size(); index++)
        {
            const auto& meshPositions = modelMeshes.at(index).mPositions;
            const auto& meshNormals   = modelMeshes.at(index).mNormals;
            const auto& meshUVCoords  = modelMeshes.at(index).mUVCoords;
            const auto& meshIndicies  = modelMeshes.at(index).mIndices;
            
            fModelVA[index].CreateVBuffer3f(meshPositions);
            fModelVA[index].CreateVBuffer3f(meshNormals);
            ASSERT(!meshUVCoords.empty());      /* UV's might be optional. Put a check! */
            fModelVA[index].CreateVBuffer2f(meshUVCoords);
            fModelVA[index].CreateIBuffer(meshIndicies);
        }
        
        const auto& texturePaths = fModel->GetTexturePaths();
        
        /* WARNING: careful not to reallocate any entry! */
        for(const auto& path: texturePaths)
            fModelTextures.emplace_back(path);
    }

    void ModelRenderer::Clear()
    {
        fModelTextures.clear();
        fModelVA.clear();
        fModel.reset();
    }
    
    void ModelRenderer::Import(const std::string& filepath)
    {
        Clear();
        fModel = std::make_unique<TriangleMesh>(filepath);
        Import();
    }
    
    void ModelRenderer::Draw(const Renderer& renderer, Shader& shader) const
    {
        /*
         * ToDo: Few issues here:
         */
        const auto& modelMeshes = fModel->GetModelMesh();

        for(int index = 0; index < fModelVA.size(); index++)
        {
            const auto& meshVA = fModelVA[index];
            /* 1) modelMeshes.at() is not very performant. */
            const auto& meshTextures = modelMeshes.at(index).mTextures;
            
            int slot = 0;
            for (const auto& tex: meshTextures)
            {
                /* 2) if else is ughh... */
                if(tex.type == TriangleMesh::Texture::Diffuse)
                {
                    /* 3) Multiple texture maps of same type for single mesh doesn't make much sense to me right now.
                          So, I'm just dealing with one map for now. */
                    fModelTextures[tex.indices[0]].Bind(slot);
                    shader.SetUniform1i("u_MaterialProperty.diffuseTex", slot);
                    ++slot;
                }
                else if(tex.type == TriangleMesh::Texture::Specular)
                {
                    fModelTextures[tex.indices[0]].Bind(slot++);
                    shader.SetUniform1i("u_MaterialProperty.specularTex", slot);
                    ++slot;
                }
            }
            
            renderer.Draw(meshVA, shader);
        }
    }
    
    const TriangleMesh& ModelRenderer::GetTriangleMesh() const
    {
        return *fModel;
    }

}
