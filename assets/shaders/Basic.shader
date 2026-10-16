#shader vertex
#version 430 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

uniform mat4 proj;
uniform mat4 view;

void main()
{
    FragPos = position;
    TexCoords = texCoord;
    Normal = normal;
    gl_Position = proj * view * vec4(position, 1.0);
};
       


#shader fragment
#version 430 core
out vec4 FragColor;

struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
    sampler2D texture_normal1;
    float shininess;
}; 

struct DirLight {
    vec3 direction;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
  
    float constant;
    float linear;
    float quadratic;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;       
};

#define NR_POINT_LIGHTS 4

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 viewPos;
uniform DirLight dirLight = DirLight(
    vec3(-0.2, -1.0, -0.3),
    vec3(0.15),
    vec3(0.8),
    vec3(0.35)
);
uniform PointLight pointLights;
//uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLight;
uniform Material material;

uniform bool isPureColor;
uniform vec4 u_Color;

// SSBO for per-triangle colors. Binding must match the one used by the C++ code.
layout(std430, binding = 2) buffer ColorTable {
    vec4 colors[];
};

// function prototypes
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec4 baseColor);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec4 baseColor);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec4 baseColor);

void main()
{    
    // properties
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    // determine base color: pure color override, otherwise sample from SSBO by primitive ID
    vec4 baseColor = vec4(1.0);
    if (isPureColor)
    {
        baseColor = u_Color;
    }
    else
    {
        // gl_PrimitiveID is used to index the color table (one entry per triangle).
        // Note: some drivers require a geometry shader for gl_PrimitiveID to be available in the fragment shader.
        int pid = int(gl_PrimitiveID);
        // Safety: if pid is negative, fall back to white
        if (pid >= 0)
        {
            baseColor = colors[pid];
        }
    }
    
    // lighting using baseColor
    vec3 result = CalcPointLight(pointLights, norm, FragPos, viewDir, baseColor);
    // optionally add other lights (commented out as before)
    // for(int i = 0; i < NR_POINT_LIGHTS; i++) result += CalcPointLight(pointLights, norm, FragPos, viewDir, baseColor);
    // result += CalcSpotLight(spotLight, norm, FragPos, viewDir, baseColor);

    FragColor = vec4(result, baseColor.a);
}

// calculates the color when using a directional light.
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec4 baseColor)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // combine results
    vec3 ambient = light.ambient * baseColor.rgb;
    vec3 diffuse = light.diffuse * diff * baseColor.rgb;
    vec3 specular = light.specular * spec;
    //return (ambient + diffuse + specular);
    return (ambient + diffuse + specular);
}

// calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec4 baseColor)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // combine results
    vec3 ambient = light.ambient * vec3(baseColor);
    vec3 diffuse = light.diffuse * diff * vec3(baseColor);
    vec3 specular = light.specular * spec * vec3(baseColor);
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}

// calculates the color when using a spot light.
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec4 baseColor)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    // combine results
    vec3 ambient = light.ambient * vec3(baseColor);
    vec3 diffuse = light.diffuse * diff * vec3(baseColor);
    vec3 specular = light.specular * spec * vec3(baseColor);
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    return (ambient + diffuse + specular);
}