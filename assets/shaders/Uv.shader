#shader vertex
#version 430 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 triCenter;
layout(location = 3) in vec2 uv;

out vec2 FragUv;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 scale = mat4(1.0);
uniform float faceShrink = 0.0;

void main()
{
    vec3 shrunk = triCenter + (position - triCenter) * (1.0 - faceShrink);
    FragUv = uv;
    gl_Position = proj * view * scale * vec4(shrunk, 1.0);
};



#shader fragment
#version 430 core
out vec4 FragColor;

in vec2 FragUv;

uniform bool isPureColor;
uniform vec4 u_Color;

void main()
{
    // Selection and locate highlights draw in a single pure color over the UV faces.
    if (isPureColor)
    {
        FragColor = u_Color;
        return;
    }

    // The surface UV is interpolated across the triangle, so the colour ramp follows the
    // parameterisation of the B-Rep face instead of its shaded appearance.
    FragColor = vec4(FragUv.x, FragUv.y, 0.0, 1.0);
};
