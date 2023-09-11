#shader compute
#version 430 core

layout(local_size_x = 1024, local_size_y = 1, local_size_z = 1) in;

layout(std430, binding = 21) coherent buffer Indices
{
    uint data[];
} indices;


layout(std430, binding = 22) coherent buffer Vertices
{
    float data[];
} vertices;


layout(std430, binding = 23) coherent buffer Normals
{
    float data[];
} normals;

uniform int max;

void main()
{
    uint global_index = gl_GlobalInvocationID.x;
    if (global_index >= max) return;
    vec3 p1 = vec3(vertices.data[(indices.data[global_index*3+0])*3+0],vertices.data[(indices.data[global_index*3+0])*3+1],vertices.data[(indices.data[global_index*3+0])*3+2]);
    vec3 p2 = vec3(vertices.data[(indices.data[global_index*3+1])*3+0],vertices.data[(indices.data[global_index*3+1])*3+1],vertices.data[(indices.data[global_index*3+1])*3+2]);
    vec3 p3 = vec3(vertices.data[(indices.data[global_index*3+2])*3+0],vertices.data[(indices.data[global_index*3+2])*3+1],vertices.data[(indices.data[global_index*3+2])*3+2]);

    vec3 n = cross(p3 - p1, p2 - p1);
    normals.data[indices.data[global_index*3 + 0]*3+0] = n.x;
    normals.data[indices.data[global_index*3 + 0]*3+1] = n.y;
    normals.data[indices.data[global_index*3 + 0]*3+2] = n.z;

    normals.data[indices.data[global_index*3 + 1]*3+0] = n.x;
    normals.data[indices.data[global_index*3 + 1]*3+1] = n.y;
    normals.data[indices.data[global_index*3 + 1]*3+2] = n.z;

    normals.data[indices.data[global_index*3 + 2]*3+0] = n.x;
    normals.data[indices.data[global_index*3 + 2]*3+1] = n.y;
    normals.data[indices.data[global_index*3 + 2]*3+2] = n.z;
}
