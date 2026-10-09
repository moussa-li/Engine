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
    FragPos = vec3(vec4(position, 1.0));
    Normal = normalize(mat3(transpose(inverse(model))) * normal);
    TexCoords = texCoord;
    FragPos = FragPos + normal * 1.1; // Offset the position along the normal to avoid z-fighting
    gl_Position = proj * view * vec4(FragPos, 1.0);
};

#shader fragment
#version 330 core
out vec4 color;
uniform vec4 u_Color;
void main() { 
    color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
} 