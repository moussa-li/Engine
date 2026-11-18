#shader vertex
#version 330 core
layout(location = 0) in vec3 position;
// Hidden nodes keep their place in the vertex buffers so every external index stays stable.
// The attribute defaults to 0 for primitives that do not provide it (for example lines).
layout(location = 1) in float nodeHidden;

uniform mat4 proj;
uniform mat4 view;
uniform float u_PointSize;

void main()
{
    if (nodeHidden > 0.5)
    {
        gl_Position = vec4(2.0, 2.0, 2.0, 1.0);
        gl_PointSize = 0.0;
        return;
    }

    vec4 viewPosition = view * vec4(position, 1.0);
    gl_Position = proj * viewPosition;
    gl_PointSize = clamp(u_PointSize, 1.0, 8.0);
};

#shader fragment
#version 330 core
out vec4 color;

uniform vec4 u_Color;

void main()
{
    color = u_Color;
}