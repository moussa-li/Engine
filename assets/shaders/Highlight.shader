#shader vertex
#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;

void main()
{
    Normal = mat3(transpose(inverse(model))) * normal;
    FragPos = vec3(vec4(position, 1.0));
    //FragPos = FragPos + Normal * 0.02f;
    TexCoords = texCoord;
    gl_Position = proj * view * vec4(FragPos, 1.0);
};

#shader fragment
#version 330 core
out vec4 color;
uniform vec4 u_Color;
void main() { 
    color = u_Color;
} 