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

/********************************************************************/
/* 1. Major recfactor, classes, common utils to seperate file, all inside my namespaces. */
/* 2. Handle light on object scale and rotation. */
/* 3. Normal Matrix. */
/* 4. Make phong shading ka kb ks params instead of light members. */
/* 5. Move strings to a header with macro*/
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
    TriangleMesh modelObjectMesh("../../../res/Sphere.obj");

    const auto& modelPositions = modelObjectMesh.GetPositions();
    const auto& modelNormals   = modelObjectMesh.GetNormals();
    const auto& modelIndicies  = modelObjectMesh.GetIndices();
    const auto& modelUVCoords  = modelObjectMesh.GetUVCoords();

    VertexArray vaModel;
    vaModel.CreateVBuffer3f(modelPositions);
    vaModel.CreateVBuffer3f(modelNormals);
    ASSERT(!modelUVCoords.empty());      /* UV's are optional. Put a check! */
    vaModel.CreateVBuffer2f(modelUVCoords);
    vaModel.CreateIBuffer(modelIndicies);

    /* Todo: implement a get slot function. */
    int slot = 0;
    
    Shader modelShader("../../../res/ModelObject.shader");
    modelShader.Bind();

    Texture diffuseTexture("../../../res/container.png");
    Texture specularTexture("../../../res/container_specular.png");

    diffuseTexture.Bind(slot);
    modelShader.SetUniform1i("u_MaterialProperty.diffuseTex", slot);
    slot++;
    specularTexture.Bind(slot);
    modelShader.SetUniform1i("u_MaterialProperty.specularTex", slot);

    modelShader.SetUniform1f("u_MaterialProperty.shininess", 32.0f);
    /*Todo: abstract material and light from here and create imgui for changing material.*/

    modelShader.SetUniform3f("u_DirectionalLight.direction", 0.0f, 1.0f, 0.0f);
    modelShader.SetUniform3f("u_DirectionalLight.ambient",  0.1f, 0.1f, 0.1f);
    modelShader.SetUniform3f("u_DirectionalLight.diffuse",  0.5f, 0.5f, 0.5f);
    modelShader.SetUniform3f("u_DirectionalLight.specular", 1.0f, 1.0f, 1.0f);

    /****************************************/

    TriangleMesh lightObjectMesh("../../../res/Sphere.obj");
    
    const auto& lightPositions = lightObjectMesh.GetPositions();
    const auto& lightIndicies  = lightObjectMesh.GetIndices();
    
    VertexArray vaLight;
    vaLight.CreateVBuffer3f(lightPositions);
    vaLight.CreateIBuffer(lightIndicies);

    Shader lightShader("../../../res/LightObject.shader");
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

    bool enableDirectionalLight = true;

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
        float xdist = sin(0/*glfwGetTime()*/) * radius;
        float ydist = cos(0/*glfwGetTime()*/) * radius;
        glm::vec3 cameraPosition(xdist, 0.0, ydist);
        glm::mat4 view = glm::lookAt(cameraPosition, glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
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
            modelShader.SetUniform3f("u_LightProperty.ambient",  0.3f*lightColorPicker.x, 0.3f*lightColorPicker.y, 0.3f*lightColorPicker.z);
            modelShader.SetUniform3f("u_LightProperty.diffuse",  lightColorPicker.x/2, lightColorPicker.y/2, lightColorPicker.z/2);
            modelShader.SetUniform3f("u_LightProperty.specular", lightColorPicker.x, lightColorPicker.y, lightColorPicker.z);
        }
        
        /* Todo: cache Get matrix output */
        modelShader.SetUniformMat4f("u_Model", objectModel.GetMatrix()); /* Todo: pass Normal matrix here. */
        modelShader.SetUniformMat4f("u_MVP", proj * view * objectModel.GetMatrix());
        renderer.Draw(vaModel, modelShader);

        
        lightShader.Bind();
        lightShader.SetUniformMat4f("u_MVP", proj * view * lightModel.GetMatrix());
        lightShader.SetUniform3f("u_LightColor", lightColorPicker.x, lightColorPicker.y, lightColorPicker.z);
        renderer.Draw(vaLight, lightShader);
        
        {
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

            ImGui::Checkbox("Sky Light", &enableDirectionalLight);
            ImGui::SliderFloat3("ModelTranslate", glm::value_ptr(lightModel.fTranslation), -100.0f, 100.0f);
//            ImGui::SliderFloat3("ModelRotate", glm::value_ptr(lightModel.fAngle), glm::radians(0.0f), glm::radians(360.0f));
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
