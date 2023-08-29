#shader vertex
#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

out vec3 FragPos;
out vec3 Normal;
out vec3 TexCoords;

uniform mat4 u_MVP;
uniform mat4 view;
uniform mat4 model;
       
void main()
{
    FragPos = vec3(model * vec4(position, 1.0));
    Normal = mat3(transpose(inverse(model))) * normal;
    TexCoords = vec3(position.x,-position.y,position.z);
    gl_Position = u_MVP * view * vec4(FragPos, 1.0);
};

#shader fragment
#version 330 core
out vec4 FragColor;

uniform samplerCube skybox;

in vec3 FragPos;
in vec3 Normal;
in vec3 TexCoords;

void main()
{    
    FragColor = texture(skybox, TexCoords);
}
