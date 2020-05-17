//
//  main.cpp
//  OpenGL
//
//  Created by Sumit Dhingra on 27/04/20.
//  Copyright © 2020 LinuxSDA. All rights reserved.
//
#include "GL/glew.h"
#include "GLFW/glfw3.h"


#include "Renderer.hpp"
#include "VertexBuffer.hpp"
#include "IndexBuffer.hpp"
#include "VertexArray.hpp"
#include "Shader.hpp"
#include "Texture.hpp"

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>

#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtx/euler_angles.hpp"
#include "gtc/type_ptr.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "TriangleMesh.hpp"

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
    
    BBCoord GetBBox(const TriangleMesh& mesh)
    {
        const std::vector<float>& serializedVertices = mesh.GetPositions();
        return GetBBox(serializedVertices);
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
    
    struct Model
    {
        Model(const BBCoord& completeSpan, const BBCoord& localSpan)
        {
            fLocalCenter.x = ( completeSpan.Min.x + completeSpan.Max.x ) / 2.0f;
            fLocalCenter.y = ( completeSpan.Min.y + completeSpan.Max.y ) / 2.0f;
            fLocalCenter.z = ( completeSpan.Min.z + completeSpan.Max.z ) / 2.0f;
            
            fMeshCenter.x = ( localSpan.Min.x + localSpan.Max.x ) / 2.0f;
            fMeshCenter.y = ( localSpan.Min.y + localSpan.Max.y ) / 2.0f;
            fMeshCenter.z = ( localSpan.Min.z + localSpan.Max.z ) / 2.0f;
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


/********************************************************************/
/* 1. Major recfactor, classes, common utils to seperate file, all inside my namespaces. */
/* 2. Handle light on object scale and rotation. */
/* 3. Normal Matrix. */
/*******************************************************************/

int main(void)
{
    GLFWwindow* window;
    
    /* Initialize the library */
    if (!glfwInit())
        return -1;
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    
    const int ScreenWidth = 1280;
    const int ScreenHeight = 720;
    
    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(ScreenWidth, ScreenHeight, "OpenGL", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }
    
    /* Make the window's context current */
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    
    GLenum err = glewInit();
    if (GLEW_OK != err)
        std::cout << "Glew Not Okay" << std::endl;

    #pragma message("Use ErrorLogger instead of couts")
    std::cout << "OpenGL Version: => " << glGetString(GL_VERSION) << std::endl;
    
    
    TriangleMesh modelObjectMesh("../../../res/Sphere.obj");

    const auto& modelPositions = modelObjectMesh.GetPositions();
    const auto& modelNormals   = modelObjectMesh.GetNormals();
    const auto& modelIndicies  = modelObjectMesh.GetIndices();

    VertexArray vaModel;
    vaModel.CreateVBuffer3f(modelPositions);
    vaModel.CreateVBuffer3f(modelNormals);
    vaModel.CreateIBuffer(modelIndicies);

    glm::vec3 modelObjectColor{0.1f, 0.5f, 0.8f};

    Shader modelShader("../../../res/ModelObject.shader");
    modelShader.Bind();
    modelShader.SetUniform3f("u_ObjectColor", modelObjectColor.x, modelObjectColor.y, modelObjectColor.z);
    
    TriangleMesh lightObjectMesh("../../../res/Sphere.obj");
    
    const auto& lightPositions = lightObjectMesh.GetPositions();
    const auto& lightIndicies  = lightObjectMesh.GetIndices();
    
    VertexArray vaLight;
    vaLight.CreateVBuffer3f(lightPositions);
    vaLight.CreateIBuffer(lightIndicies);

    Shader lightShader("../../../res/LightObject.shader");
    lightShader.Bind();
    lightShader.SetUniform3f("u_LightColor", 1.0f, 1.0f, 1.0f);
    
//    int slot = 0;
//    Texture texture("../../res/logo.png");
//    texture.Bind(slot);
//
//    shader.SetUniform1i("u_Texture", slot);


    Renderer renderer;
    renderer.EnableDepth(GL_LESS);
    
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 150");

    /* Todo: abstract all these GetBBox calls and put them in relevent class. SERIOUSLY! */
    CommonUtils::BBCoord lightObjectBB = CommonUtils::GetBBox(lightPositions);
    CommonUtils::BBCoord modelObjectBB = CommonUtils::GetBBox(modelPositions);
    CommonUtils::BBCoord unionizedBB = CommonUtils::GetBBox({lightObjectBB, modelObjectBB});  /* Todo: Uggh.. Copy.. */


    /* Translation, Scale, Rotation to object, Model Matrix. Local to World coordinates. */ /* Todo: abstract to class*/
    CommonUtils::Model objectModel(unionizedBB, modelObjectBB);
    CommonUtils::Model lightModel(unionizedBB, lightObjectBB);

    glm::vec3 initialLightPosition = CommonUtils::GetBBoxCenter(lightObjectBB);
    glm::vec3 lightColorPicker{1.0f, 1.0f, 1.0f};

    {
        lightModel.fTranslation.x = -30.0f;
        lightModel.fScale.x = lightModel.fScale.y = lightModel.fScale.z = 0.3f;
    }
    
    /* Projection matrix */
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)ScreenWidth / (float)ScreenHeight, 0.1f, 100.0f);


    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        
        /* Render here */
        renderer.Clear();
        
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        /*************************************/
        const float radius = 60.0;
        float xdist = sin(glfwGetTime()) * radius;
        float ydist = cos(glfwGetTime()) * radius;
        glm::mat4 view = glm::lookAt(glm::vec3(xdist, 0.0, ydist), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
        /*************************************/

        /* Binding before rendering*/
        auto finalLightPosition = glm::vec3(lightModel.GetMatrix() * glm::vec4(initialLightPosition, 1.0f));
        
        modelShader.Bind();
        modelShader.SetUniform3f("u_LightPos", finalLightPosition.x, finalLightPosition.y, finalLightPosition.z);
        modelShader.SetUniform3f("u_LightColor", lightColorPicker.x, lightColorPicker.y, lightColorPicker.z);
        modelShader.SetUniform3f("u_ObjectColor", modelObjectColor.x, modelObjectColor.y, modelObjectColor.z);
        modelShader.SetUniformMat4f("u_Model", objectModel.GetMatrix());
        modelShader.SetUniformMat4f("u_MVP", proj * view * objectModel.GetMatrix());
        renderer.Draw(vaModel, modelShader);

        
        lightShader.Bind();
        lightShader.SetUniformMat4f("u_MVP", proj * view * lightModel.GetMatrix());
        lightShader.SetUniform3f("u_LightColor", lightColorPicker.x, lightColorPicker.y, lightColorPicker.z);
        renderer.Draw(vaLight, lightShader);
        
        {
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

            ImGui::SliderFloat3("ModelTranslate", glm::value_ptr(lightModel.fTranslation), -100.0f, 100.0f);
            ImGui::SliderFloat3("ModelRotate", glm::value_ptr(lightModel.fAngle), glm::radians(0.0f), glm::radians(360.0f));
            ImGui::SliderFloat("ModelScale", glm::value_ptr(lightModel.fScale), 0.1f, 1.0f);
            lightModel.fScale.y = lightModel.fScale.z = lightModel.fScale.x;

            ImGui::ColorPicker3("LightPicker", (float*)&lightColorPicker, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview);
        }
        
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        /* Swap front and back buffers */
        glfwSwapBuffers(window);
        
        /* Poll for and process events */
        glfwPollEvents();
    }
    
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}
