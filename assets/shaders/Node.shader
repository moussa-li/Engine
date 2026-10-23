#shader vertex
#version 330 core
layout(location = 0) in vec3 position;

uniform mat4 proj;
uniform mat4 view;
uniform float u_PointSize;

void main()
{
    vec4 viewPosition = view * vec4(position, 1.0);
    gl_Position = proj * viewPosition;
    gl_PointSize = clamp(u_PointSize * 10.0 / max(-viewPosition.z, 0.1), 1.0, 3.0);
};

#shader fragment
#version 330 core
out vec4 color;

uniform vec4 u_Color;

void main()
{
    color = u_Color;
}