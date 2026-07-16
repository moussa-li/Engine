#shader vertex
#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;
        
uniform mat4 mvp;
out vec3 ourColor;

void main() {
    gl_Position = mvp * vec4(aPos, 1.0);
    ourColor = vec3(1.0, 0.0, 0.0);
}

#shader fragment
#version 330 core
in vec3 ourColor;
out vec4 FragColor;

void main() {
    FragColor = vec4(ourColor, 1.0);
}