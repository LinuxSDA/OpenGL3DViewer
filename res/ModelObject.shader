#shader vertex
#version 330 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 normal;
//layout(location = 2) in vec2 texCoord;

//out vec2 v_TexCoord;
out vec3 fragmentNormal;
out vec3 fragmetPosition;

uniform mat4 u_MVP;
uniform mat4 u_Model;

void main()
{
    gl_Position = u_MVP * position;
    fragmetPosition = vec3(u_Model * position);
    fragmentNormal = vec3(u_Model * normal);
//    v_TexCoord = texCoord;
}

#shader fragment
#version 330 core

layout(location = 0) out vec4 color;

//in vec2 v_TexCoord;
//uniform sampler2D u_Texture;

in vec3 fragmentNormal;
in vec3 fragmetPosition;

uniform vec3 u_ObjectColor;
uniform vec3 u_LightColor;
uniform vec3 u_LightPos;

void main()
{
    const float ambientFactor = 0.2;

    vec3 normalisedNormal = normalize(fragmentNormal);
    vec3 lightDirectionVec = normalize(u_LightPos - fragmetPosition);

    float diff = max(dot(normalisedNormal, lightDirectionVec), 0.0);
    
    vec3 diffuseFactor = diff * u_LightColor;

    vec3 result = (ambientFactor + diffuseFactor) * u_ObjectColor;
    color = vec4(result, 1.0);
    
//    color = texture(u_Texture, v_TexCoord);
}
