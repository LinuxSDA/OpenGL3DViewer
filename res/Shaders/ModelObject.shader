#shader vertex
#version 330 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 normal;
layout(location = 2) in vec2 texCoord;

out vec2 v_TexCoord;
out vec3 fragmentNormal;
out vec3 fragmetPosition;

uniform mat4 u_MVP;
uniform mat4 u_Model;

void main()
{
    gl_Position = u_MVP * position;
    fragmetPosition = vec3(u_Model * position);
    fragmentNormal = vec3(u_Model * normal);
    v_TexCoord = texCoord;
}

#shader fragment
#version 330 core

struct Material {
    sampler2D diffuseTex;
    sampler2D specularTex;
        float shininess;
};

struct DirectionalLight {
    bool enable;
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct Light {
    bool enable;
    vec3 lightPos;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

layout(location = 0) out vec4 color;

in vec2 v_TexCoord;

in vec3 fragmentNormal;
in vec3 fragmetPosition;

uniform Material            u_MaterialProperty;
uniform Light               u_LightProperty;
uniform DirectionalLight    u_DirectionalLight;

uniform vec3 u_ViewPos;

void main()
{
    color = vec4(0.0f);
    //Direction light
    {
        if(u_DirectionalLight.enable)
        {
            vec3 lightDirectionVec = normalize(u_DirectionalLight.direction);
            vec3 normalisedNormal = normalize(fragmentNormal);
            float diffuseComponent = max(dot(normalisedNormal, lightDirectionVec), 0.0);

            vec3 eyeDirectionVec = normalize(u_ViewPos - fragmetPosition);
            vec3 reflectionVec   = reflect(-lightDirectionVec, normalisedNormal);
            float specularComponent = pow(max(dot(eyeDirectionVec, reflectionVec), 0.0), u_MaterialProperty.shininess);

            vec3 outAmbient =   u_DirectionalLight.ambient * vec3(texture(u_MaterialProperty.diffuseTex, v_TexCoord));
            vec3 outDiffuse =   u_DirectionalLight.diffuse  * vec3(texture(u_MaterialProperty.diffuseTex, v_TexCoord)) * diffuseComponent;
            vec3 outSpecular =  u_DirectionalLight.specular * vec3(texture(u_MaterialProperty.specularTex, v_TexCoord)) * specularComponent;
            vec3 result = (outAmbient + outDiffuse + outSpecular);
            
            color += vec4(result, 1.0);
        }
    }

    //phong shading model
    {
        const float kc = 1.0f;
        const float kl = 0.0022f;
        const float kq = 0.000018f;
        
        float distance = length(u_LightProperty.lightPos - fragmetPosition);
        float attenuation = 1.0 / (kc + kl * distance + kq * (distance * distance));

        vec3 normalisedNormal = normalize(fragmentNormal);
        vec3 lightDirectionVec = normalize(u_LightProperty.lightPos - fragmetPosition);
        float diffuseComponent = max(dot(normalisedNormal, lightDirectionVec), 0.0);
        
        vec3 eyeDirectionVec = normalize(u_ViewPos - fragmetPosition);
        vec3 reflectionVec   = reflect(-lightDirectionVec, normalisedNormal);
        float specularComponent = pow(max(dot(eyeDirectionVec, reflectionVec), 0.0), u_MaterialProperty.shininess);
        
        vec3 outAmbient =   u_LightProperty.ambient * vec3(texture(u_MaterialProperty.diffuseTex, v_TexCoord));
        vec3 outDiffuse =   u_LightProperty.diffuse  * vec3(texture(u_MaterialProperty.diffuseTex, v_TexCoord)) * diffuseComponent;
        vec3 outSpecular =  u_LightProperty.specular * vec3(texture(u_MaterialProperty.specularTex, v_TexCoord)) * specularComponent;

        vec3 result = (outAmbient + outDiffuse + outSpecular) * attenuation;
        color += vec4(result, 1.0);
    }

}
