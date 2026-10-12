#shader vertex
#version 330 core
layout(location = 0) in vec3 position;

uniform mat4 proj;
uniform mat4 view;
uniform float u_PointSize;

void main()
{
    //gl_Position = vec4(position, 1.0);
    gl_Position = proj*view*vec4(position, 1.0);
    gl_PointSize = u_PointSize;
};

#shader fragment
#version 330 core
out vec4 color;

uniform vec4 u_Color;

void main()
{
    color = u_Color;
}