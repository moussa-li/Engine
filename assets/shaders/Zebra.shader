#shader vertex
#version 430 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 triCenter;

out vec3 Normal;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 scale = mat4(1.0);
uniform float faceShrink = 0.0;

void main()
{
    vec3 shrunk = triCenter + (position - triCenter) * (1.0 - faceShrink);
    Normal = normal;
    gl_Position = proj * view * scale * vec4(shrunk, 1.0);
};



#shader fragment
#version 430 core
out vec4 FragColor;

in vec3 Normal;

uniform bool isPureColor;
uniform vec4 u_Color;

// Zebra (isophote) display owns its own lighting setup instead of inheriting the scene light
// or the camera: the stripes mirror a user-configured light direction, so orbiting or zooming
// the camera never changes the pattern.
uniform vec3 zebraLightDirection = vec3(0.0, 0.0, 1.0); // direction towards the virtual light
uniform float zebraFrequency = 8.0; // stripe density over the reflected latitude range
uniform vec4 zebraColor = vec4(0.05, 0.05, 0.05, 1.0); // dark bands; light bands stay white

// SSBO for per-triangle colors. Binding must match the one used by the C++ code.
layout(std430, binding = 2) buffer ColorTable {
    vec4 colors[];
};

void main()
{
    // Selection and locate highlights draw in a single pure color over the zebra faces.
    if (isPureColor)
    {
        FragColor = u_Color;
        return;
    }

    vec4 baseColor = vec4(1.0);
    int pid = int(gl_PrimitiveID);
    if (pid >= 0)
    {
        baseColor = colors[pid];
    }

    // Mirror the configured light ray about the surface normal in model space and slice the
    // reflection's latitude into bands. A flat region keeps one band, while steep normal changes
    // bend the bands, exposing curvature discontinuities across and inside the B-Rep faces.
    vec3 lightDir = zebraLightDirection;
    if (dot(lightDir, lightDir) < 1e-6)
    {
        lightDir = vec3(0.0, 0.0, 1.0);
    }
    lightDir = normalize(lightDir);
    vec3 mirrorDir = reflect(-lightDir, normalize(Normal));
    float latitude = asin(clamp(mirrorDir.y, -1.0, 1.0));
    vec3 stripe = sin(latitude * zebraFrequency) > 0.0 ? zebraColor.rgb : vec3(1.0);
    FragColor = vec4(stripe, baseColor.a);
};
