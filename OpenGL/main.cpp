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
#include "TriangleMesh.hpp"
#include "CommonUtils.hpp"

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

/**************************** TODO **********************************/
/* 1. Major recfactor, classes, common utils to seperate file, all inside my namespaces. */
/* 2. Handle light on object scale and rotation. */
/* 3. Normal Matrix. */
/* 4. Make phong shading ka kb ks params instead of light members. */
/* 5. Move strings to a header with macro. */
/* 6. Convert mesh attributes to shared_ptr. */
/* 7. Mesh may either have a color or texture or both or none. Handle all cases. */
/* 8. Some issues because of which light direction got flipped. Need to investogate. Hardcoded the -1 for now in shader.*/
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
    
    /****************************************/
    TriangleMesh Model("../../../res/Models/Ivysaur_OBJ/Pokemon.obj");
    const auto& modelMeshes = Model.GetModelMesh();
    
    std::deque<VertexArray> vaModels;
    vaModels.resize(modelMeshes.size());
    /* WARNING: careful not to reallocate any entry! */
    for (unsigned int index = 0; index < modelMeshes.size(); index++)
    {
        const auto& modelPositions = modelMeshes.at(index).mPositions;
        const auto& modelNormals   = modelMeshes.at(index).mNormals;
        const auto& modelUVCoords  = modelMeshes.at(index).mUVCoords;
        const auto& modelIndicies  = modelMeshes.at(index).mIndices;

        vaModels[index].CreateVBuffer3f(modelPositions);
        vaModels[index].CreateVBuffer3f(modelNormals);
        ASSERT(!modelUVCoords.empty());      /* UV's might be optional. Put a check! */
        vaModels[index].CreateVBuffer2f(modelUVCoords);
        vaModels[index].CreateIBuffer(modelIndicies);
    }
    
    const auto& texturePaths = Model.GetTexturePaths();
    
    /* WARNING: careful not to reallocate any entry! */
    std::deque<Texture> modelTextures;
    for(const auto& path: texturePaths)
        modelTextures.emplace_back(path);

    int slot = 0;
    
    Shader modelShader("../../../res/Shaders/ModelObject.shader");
    modelShader.Bind();


    /* ToDo: extract these values from model itself. */
    modelShader.SetUniform1f("u_MaterialProperty.shininess", 32.0f);
    modelShader.SetUniform3f("u_DirectionalLight.direction", 0.0f, 1.0f, 0.0f);
    modelShader.SetUniform3f("u_DirectionalLight.ambient",  0.3f, 0.3f, 0.3f);
    modelShader.SetUniform3f("u_DirectionalLight.diffuse",  0.7f, 0.7f, 0.7f);
    modelShader.SetUniform3f("u_DirectionalLight.specular", 1.0f, 1.0f, 1.0f);

    /****************************************/

    TriangleMesh lightObjectMesh("../../../res/Models/Sphere.obj");
    
    const auto& lightPositions = lightObjectMesh.GetModelMesh().at(0).mPositions;
    const auto& lightIndicies  = lightObjectMesh.GetModelMesh().at(0).mIndices;
    
    VertexArray vaLight;
    vaLight.CreateVBuffer3f(lightPositions);
    vaLight.CreateIBuffer(lightIndicies);

    Shader lightShader("../../../res/Shaders/LightObject.shader");
    lightShader.Bind();
    lightShader.SetUniform3f("u_LightColor", 1.0f, 1.0f, 1.0f);
    
    /****************************************/

    Renderer renderer;
    renderer.EnableDepth(GL_LESS);
    
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 150");

    /* Todo: abstract all these GetBBox calls and put them in relevent class. SERIOUSLY! */
    CommonUtils::BBCoord lightObjectBB = CommonUtils::GetBBox(lightPositions);
    CommonUtils::BBCoord modelObjectBB = CommonUtils::GetBBox(Model);
    CommonUtils::BBCoord unionizedBB = CommonUtils::GetBBox({lightObjectBB, modelObjectBB}); /* ToDo: Ugh... Copy... */

    /* Translation, Scale, Rotation to object, Model Matrix. Local to World coordinates. */ /* Todo: abstract to class*/
    CommonUtils::Model objectModel(unionizedBB, modelObjectBB);
    CommonUtils::Model lightModel(unionizedBB, lightObjectBB);

    glm::vec3 initialLightPosition = CommonUtils::GetBBoxCenter(lightObjectBB);
    glm::vec3 lightColorPicker{1.0f, 1.0f, 1.0f};

    glm::vec3 lookAtCenter = CommonUtils::GetBBoxCenter(unionizedBB);
    
    {
        lightModel.fTranslation.x = -25.0f;
        lightModel.fScale.x = lightModel.fScale.y = lightModel.fScale.z = 0.1f;
    }
    

    float fieldOfView = 45.0f;
    bool enableDirectionalLight = true;

    /* Todo: Abstract view matrix into a struct */
    glm::vec3 viewTranslate{};
    
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        
        /* Render here */
        renderer.Clear();
        
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        /* Projection matrix */
        glm::mat4 proj = glm::perspective(glm::radians(fieldOfView), (float)ScreenWidth / (float)ScreenHeight, 0.1f, 100.0f);

        /*************************************/
        const float radius = 60.0;
        float xdist = sin(0/*glfwGetTime()*/) * radius;
        float ydist = cos(0/*glfwGetTime()*/) * radius;
        glm::vec3 cameraPosition(xdist, 0.0, ydist);
        glm::mat4 view = glm::translate(glm::identity<glm::mat4>(), viewTranslate) *
                         glm::lookAt(cameraPosition, lookAtCenter, glm::vec3(0.0, 1.0, 0.0));
        /*************************************/

        /*************************************/
        float yAngle = glfwGetTime();
        objectModel.fAngle = glm::vec3(0.0f, yAngle, 0.0f);
        /*************************************/

        auto finalLightPosition = glm::vec3(lightModel.GetMatrix() * glm::vec4(initialLightPosition, 1.0f));
        
        modelShader.Bind();
        modelShader.SetUniform3f("u_ViewPos", cameraPosition.x, cameraPosition.y, cameraPosition.z);
        
        {
            modelShader.SetUniform1i("u_DirectionalLight.enable", enableDirectionalLight);
        }
        
        {
            modelShader.SetUniform3f("u_LightProperty.lightPos", finalLightPosition.x, finalLightPosition.y, finalLightPosition.z);
            modelShader.SetUniform3f("u_LightProperty.ambient",  0.2f*lightColorPicker.x, 0.2f*lightColorPicker.y, 0.2f*lightColorPicker.z);
            modelShader.SetUniform3f("u_LightProperty.diffuse",  lightColorPicker.x/2, lightColorPicker.y/2, lightColorPicker.z/2);
            modelShader.SetUniform3f("u_LightProperty.specular", lightColorPicker.x, lightColorPicker.y, lightColorPicker.z);
        }
        
        /* Todo: cache Get matrix output */
        modelShader.SetUniformMat4f("u_Model", objectModel.GetMatrix()); /* Todo: pass Normal matrix here. */
        modelShader.SetUniformMat4f("u_MVP", proj * view * objectModel.GetMatrix());
        
        for(int index = 0; index < vaModels.size(); index++)
        {
            /*
             * ToDo: Few issues here:
             * 1) modelMeshes.at() is not very performant.
             * 2) Multiple texture maps of same type for single mesh doesn't make much sense to me right now.
             *    So, I'm just dealing with one map for now.
             * 3) Abstract this out, if else is ughh...
             */
            const auto& vaModel = vaModels[index];
            const auto& meshTextures = modelMeshes.at(index).mTextures;
            
            int slot = 0;
            for (const auto& tex: meshTextures)
            {
                if(tex.type == TriangleMesh::Texture::Diffuse)
                {
                    modelTextures[tex.indices[0]].Bind(slot);
                    modelShader.SetUniform1i("u_MaterialProperty.diffuseTex", slot);
                    ++slot;
                }
                else if(tex.type == TriangleMesh::Texture::Specular)
                {
                    modelTextures[tex.indices[0]].Bind(slot++);
                    modelShader.SetUniform1i("u_MaterialProperty.specularTex", slot);
                    ++slot;
                }
            }
            
            renderer.Draw(vaModel, modelShader);
        }

        
        lightShader.Bind();
        lightShader.SetUniformMat4f("u_MVP", proj * view * lightModel.GetMatrix());
        lightShader.SetUniform3f("u_LightColor", lightColorPicker.x, lightColorPicker.y, lightColorPicker.z);
        renderer.Draw(vaLight, lightShader);
        
        {
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            
            ImGui::SliderFloat("FOV", &fieldOfView, 1.0f, 120.0f);
            ImGui::SliderFloat3("View Translate", glm::value_ptr(viewTranslate), -100.0f, 100.0f);

            ImGui::Checkbox("Sky Light", &enableDirectionalLight);
            ImGui::SliderFloat3("Light Translate", glm::value_ptr(lightModel.fTranslation), -100.0f, 100.0f);
            //ImGui::SliderFloat3("ModelRotate", glm::value_ptr(lightModel.fAngle), glm::radians(0.0f), glm::radians(360.0f));
            ImGui::SliderFloat("Model Scale", glm::value_ptr(objectModel.fScale), 0.1f, 10.0f);
            objectModel.fScale.y = objectModel.fScale.z = objectModel.fScale.x;
            
            ImGui::ColorPicker3("Light Picker", glm::value_ptr(lightColorPicker), ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview);
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
