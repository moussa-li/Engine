#shader vertex
#version 330 core
layout(location = 0) in vec3 position;

uniform mat4 proj;
uniform mat4 view;

void main()
{
    //gl_Position = vec4(position, 1.0);
    gl_Position = proj*view*vec4(position, 1.0);
    gl_PointSize = 3.0;
};