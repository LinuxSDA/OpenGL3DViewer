//
//  ModelRenderer.hpp
//  OpenGL
//
//  Created by Sumit Dhingra on 11/06/20.
//  Copyright © 2020 LinuxSDA. All rights reserved.
//

#ifndef ModelRenderer_hpp
#define ModelRenderer_hpp

#include "TriangleMesh.hpp"
#include "VertexArray.hpp"
#include "Texture.hpp"
#include "Renderer.hpp"
#include "Shader.hpp"

#include <deque>
#include <memory>

namespace Helper
{
    /* ToDo: Support is hard coded, type of coord to read and shader values. Will make this class flexible. */
    class ModelRenderer
    {
    public:
        ModelRenderer(const std::string& filepath);
        ~ModelRenderer();
        void Clear();
        void Import(const std::string& filepath);
        void Draw(const Renderer& renderer, Shader& shader) const;
        const TriangleMesh& GetTriangleMesh() const;
    private:
        void Import();
        std::unique_ptr<TriangleMesh> fModel;
        std::deque<VertexArray> fModelVA;
        std::deque<Texture> fModelTextures;
    };
}

#endif /* ModelRenderer_hpp */
