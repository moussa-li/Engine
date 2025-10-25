#shader vertex
#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;

out vec2 v_TexCoord;

uniform mat4 u_MVP;
uniform mat4 view;
uniform mat4 model;
       
void main()
{
    gl_Position =  u_MVP * view * model * vec4(position,1.0);
    v_TexCoord = texCoord;
};

#shader fragment
#version 330 core

layout(location = 0) out vec4 color;

in vec2 v_TexCoord;

uniform vec4 u_Color;
uniform sampler2D u_Texture;
uniform sampler2D u_Texture2;

void main()
{
    color = u_Color;
};