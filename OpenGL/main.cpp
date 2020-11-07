//
//  main.cpp
//  OpenGL
//
//  Created by Sumit Dhingra on 27/04/20.
//  Copyright © 2020 LinuxSDA. All rights reserved.
//

#include "GUIContext.hpp"

#include "TriangleMesh.hpp"
#include "CommonUtils.hpp"
#include "ModelRendererHelper.hpp"

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
/* 9. Load nano suit to check whether model loading works alright.*/
/* 10. Implement export mesh. */
/*******************************************************************/

int main(void)
{
    const int ScreenWidth = 1280;
    const int ScreenHeight = 720;
    const std::string WindowName = "OpenGL";

    GLFWInitWindow window(ScreenWidth, ScreenHeight, WindowName);
    window.HideCursor();

    
    /* Y axis is up. */
    
    Helper::ModelRenderer groundModel("../../../res/Models/GroundPlane/GroundPlane.obj");
    Helper::ModelRenderer objectModel("../../../res/Models/Ivysaur_OBJ/Pokemon.obj");

    Shader modelShader("../../../res/Shaders/ModelObject.shader");
    modelShader.Bind();

    /* ToDo: extract these values from model itself. */
    modelShader.SetUniform3f("u_DirectionalLight.direction", 0.0f, 1.0f, 0.0f);
    modelShader.SetUniform3f("u_DirectionalLight.ambient",  0.3f, 0.3f, 0.3f);
    modelShader.SetUniform3f("u_DirectionalLight.diffuse",  0.7f, 0.7f, 0.7f);
    modelShader.SetUniform3f("u_DirectionalLight.specular", 1.0f, 1.0f, 1.0f);

    /****************************************/

    Helper::ModelRenderer lightModel("../../../res/Models/Light/Light.obj");

    Shader lightShader("../../../res/Shaders/LightObject.shader");
    lightShader.Bind();
    lightShader.SetUniform3f("u_LightColor", 1.0f, 1.0f, 1.0f);
    

    /******* Frame Buffer code here *********/
//    Helper::FramebufferRenderer framebuffer(ScreenWidth, ScreenHeight);
//    Shader framebufferShader("../../../res/Shaders/Framebuffer.shader");
    /****************************************/

    Renderer renderer;
    renderer.EnableDepth(GL_LESS);
//    renderer.EnableBlend();
    
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window.GetWindowContext(), true);
    ImGui_ImplOpenGL3_Init("#version 150");

    /* Todo: abstract all these GetBBox calls and put them in relevent class. SERIOUSLY! */
    CommonUtils::BBCoord lightObjectBB = CommonUtils::GetBBox(lightModel.GetTriangleMesh());
    CommonUtils::BBCoord modelObjectBB = CommonUtils::GetBBox(objectModel.GetTriangleMesh());
    CommonUtils::BBCoord groundObjectBB = CommonUtils::GetBBox(groundModel.GetTriangleMesh()); /* Todo: Add a get bound to renderermodel maybe?*/

    CommonUtils::BBCoord unionizedBB = CommonUtils::GetBBox({lightObjectBB, modelObjectBB, groundObjectBB}); /* ToDo: Ugh... Copy... */

    /* Translation, Scale, Rotation to object, Model Matrix. Local to World coordinates. */ /* Todo: abstract to class*/
    CommonUtils::ModelMatrix objectModelMatrix(unionizedBB, modelObjectBB);
    CommonUtils::ModelMatrix lightModelMatrix(unionizedBB, lightObjectBB);
    CommonUtils::ModelMatrix groundModelMatrix(unionizedBB, groundObjectBB);

    glm::vec3 initialLightPosition = CommonUtils::GetBBoxCenter(lightObjectBB);
    glm::vec3 lightColorPicker{1.0f, 1.0f, 1.0f};

    glm::vec3 lookAtCenter = CommonUtils::GetBBoxCenter(unionizedBB);
    
    {
        lightModelMatrix.fTranslation.x = -15.0f;
        lightModelMatrix.fScale.x = lightModelMatrix.fScale.y = lightModelMatrix.fScale.z = 1.4f;
    }
    
    float fieldOfView = 45.0f;
    bool enableDirectionalLight = true;

    /* Todo: Abstract view matrix into a struct */
    glm::vec3 viewTranslate{};
    
    objectModelMatrix.fScale = glm::vec3(5.0f, 5.0f, 5.0f);
    
//    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    /* Loop until the user closes the window */
    while (!(window.ShouldCloseWindow()))
    {
        /* Render here */
        renderer.Clear();
        
//        framebuffer.Bind();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        /* Projection matrix */
        glm::mat4 proj = glm::perspective(glm::radians(fieldOfView), (float)ScreenWidth / (float)ScreenHeight, 0.1f, 100.0f);

        /*************************************/
        const float radius = 60.0;
        float xdist = sin(glfwGetTime()) * radius;
        float ydist = cos(glfwGetTime()) * radius;
        glm::vec3 cameraPosition(xdist, 0.0, ydist);
        glm::mat4 view = glm::translate(glm::identity<glm::mat4>(), viewTranslate) *
                         glm::lookAt(cameraPosition, lookAtCenter, glm::vec3(0.0, 1.0, 0.0));
        /*************************************/

        /*************************************/
        float yAngle = 0; /* glfwGetTime(); */
        objectModelMatrix.fAngle = glm::vec3(0.0f, yAngle, 0.0f);
        /*************************************/

        auto finalLightPosition = glm::vec3(lightModelMatrix.GetMatrix() * glm::vec4(initialLightPosition, 1.0f));
        
        modelShader.Bind();
        modelShader.SetUniform3f("u_ViewPos", cameraPosition.x, cameraPosition.y, cameraPosition.z);
        
        {
            modelShader.SetUniform1i("u_DirectionalLight.enable", enableDirectionalLight);
        }
        
        {
            modelShader.SetUniform3f("u_LightProperty.lightPos", finalLightPosition.x, finalLightPosition.y, finalLightPosition.z);
            modelShader.SetUniform3f("u_LightProperty.ambient",  0.1f*lightColorPicker.x, 0.1f*lightColorPicker.y, 0.1f*lightColorPicker.z);
            modelShader.SetUniform3f("u_LightProperty.diffuse",  lightColorPicker.x/2, lightColorPicker.y/2, lightColorPicker.z/2);
            modelShader.SetUniform3f("u_LightProperty.specular", lightColorPicker.x, lightColorPicker.y, lightColorPicker.z);
        }
        
        {
            /* Place ground below object */
            auto distance = CommonUtils::GetBBoxCenter(modelObjectBB) - CommonUtils::GetBBoxCenter(groundObjectBB);
            distance -= glm::vec3(0, CommonUtils::GetBBoxHeight(modelObjectBB)/2  * objectModelMatrix.fScale.y, 0);
            groundModelMatrix.fTranslation = distance;
            /* ToDo: I should actually put object on ground instead of other way round. */
        }

        {
            /* Todo: cache Get matrix output */
            modelShader.SetUniformMat4f("u_Model", objectModelMatrix.GetMatrix()); /* Todo: pass Normal matrix here. */
            modelShader.SetUniformMat4f("u_MVP", proj * view * objectModelMatrix.GetMatrix());
            objectModel.Draw(renderer, modelShader);
        }
        
        {
            /* Todo: cache Get matrix output */
            modelShader.SetUniformMat4f("u_Model", groundModelMatrix.GetMatrix()); /* Todo: pass Normal matrix here. */
            modelShader.SetUniformMat4f("u_MVP", proj * view * groundModelMatrix.GetMatrix());
            groundModel.Draw(renderer, modelShader);
        }

        {
            lightShader.Bind();
            lightShader.SetUniformMat4f("u_MVP", proj * view * lightModelMatrix.GetMatrix());
            lightShader.SetUniform3f("u_LightColor", lightColorPicker.x, lightColorPicker.y, lightColorPicker.z);
            
            lightModel.Draw(renderer, lightShader);
        }

//        framebuffer.Unbind();
//        framebuffer.Draw(renderer, framebufferShader);

        /** IM GUI **/
        {
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            
            ImGui::SliderFloat("FOV", &fieldOfView, 1.0f, 120.0f);
            ImGui::SliderFloat3("View Translate", glm::value_ptr(viewTranslate), -100.0f, 100.0f);
//            ImGui::SliderFloat3("View Translate", glm::value_ptr(viewTranslate), -100.0f, 100.0f);

            ImGui::Checkbox("Sky Light", &enableDirectionalLight);
            ImGui::SliderFloat3("Light Translate", glm::value_ptr(lightModelMatrix.fTranslation), -100.0f, 100.0f);
            //ImGui::SliderFloat3("ModelRotate", glm::value_ptr(lightModel.fAngle), glm::radians(0.0f), glm::radians(360.0f));
            ImGui::SliderFloat("Model Scale", glm::value_ptr(objectModelMatrix.fScale), 0.1f, 10.0f);
            objectModelMatrix.fScale.y = objectModelMatrix.fScale.z = objectModelMatrix.fScale.x;
            groundModelMatrix.fScale = objectModelMatrix.fScale;
            ImGui::ColorPicker3("Light Picker", glm::value_ptr(lightColorPicker), ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview);
        }
        
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        window.SwapBuffersAndPollEvents();
    }
    
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    return 0;
}
