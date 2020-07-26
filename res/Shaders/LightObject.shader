#shader vertex
#version 330 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 normal;
layout(location = 2) in vec2 texCoord;

out vec2 v_TexCoord;

uniform mat4 u_MVP;
uniform mat4 u_Model;

void main()
{
    gl_Position = u_MVP * position;
    v_TexCoord = texCoord;
}

#shader fragment
#version 330 core

struct Material {
    sampler2D diffuseTex;
    sampler2D specularTex;
    float shininess;
};

in vec2 v_TexCoord;

uniform Material  u_MaterialProperty;

layout(location = 0) out vec4 color;
uniform vec3 u_LightColor;

void main()
{
    color = vec4( 0.5*u_LightColor + 0.5*vec3(texture(u_MaterialProperty.diffuseTex, v_TexCoord)), 1.0);
}
