#shader vertex
#version 430 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;
uniform mat4 scale = mat4(1.0);
uniform float u_PointSize = 1.0;

void main()
{
    FragPos = (scale * vec4(position, 1.0)).xyz;
    Normal = normalize(mat3(transpose(inverse(model))) * normal);
    TexCoords = texCoord;
    gl_PointSize = u_PointSize;
    gl_Position = proj * view * vec4(FragPos, 1.0);
};

#shader fragment
#version 430 core
out vec4 color;
uniform vec4 u_Color;
void main() { 
    color = u_Color;
} 