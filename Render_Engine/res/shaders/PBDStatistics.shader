#shader compute
#version 430 core

layout(local_size_x = 1024, local_size_y = 1, local_size_z = 1) in;
layout(std430, binding = 1) coherent buffer InputEdges
{
    uint edges[];
} inputEdges;

layout(std430, binding = 2) coherent buffer InputVertices
{
    float vertices[];
} inputVertices;

layout(std430, binding = 3) coherent buffer InputL
{
    float l[];
} inputL;

layout(std430, binding = 4) coherent buffer OutputSumX
{
    int SumX[];
} outputSumX;

layout(std430, binding = 5) coherent buffer OutputSumN
{
    uint SumN[];
} outputSumN;

float length(vec3 v) {
    return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

uniform int max;

void main()
{
    uint index = gl_LocalInvocationID.x;
    uint global_index = gl_GlobalInvocationID.x;
    if (global_index > max)return;
    outputSumX.SumX[global_index*2+0] = 0;
    outputSumX.SumX[global_index*2+1] = 0;
    outputSumN.SumN[global_index*2+0] = 0;
    outputSumN.SumN[global_index*2+1] = 0;
    barrier();

    uint p1 = inputEdges.edges[global_index*2+0];
    uint p2 = inputEdges.edges[global_index*2+1];

    vec3 x1 = vec3(inputVertices.vertices[p1 * 3 + 0],inputVertices.vertices[p1 * 3 + 1],inputVertices.vertices[p1 * 3 + 2]);
    vec3 x2 = vec3(inputVertices.vertices[p2 * 3 + 0],inputVertices.vertices[p2 * 3 + 1],inputVertices.vertices[p2 * 3 + 2]);

    //vec3 X1 = inputL.l[global_index]  / length(x1 - x2)* (x1 - x2);
    vec3 X1 = (0.5f * (x1 + x2 + (inputL.l[global_index]  / length(x1 - x2))* (x1 - x2)));
    vec3 X2 = (0.5f * (x1 + x2 - (inputL.l[global_index]  / length(x1 - x2))* (x1 - x2)));
    atomicAdd(outputSumX.SumX[p1*3+0],int(X1.x*10000000));
    atomicAdd(outputSumX.SumX[p1*3+1],int(X1.y*10000000));
    atomicAdd(outputSumX.SumX[p1*3+2],int(X1.z*10000000));
    atomicAdd(outputSumX.SumX[p2*3+0],int(X2.x*10000000));
    atomicAdd(outputSumX.SumX[p2*3+1],int(X2.y*10000000));
    atomicAdd(outputSumX.SumX[p2*3+2],int(X2.z*10000000));
    //if ((p1) == 1)
    //{
    //    /*outputSumX.SumX[0] = x1.x;
    //    outputSumX.SumX[1] = x1.y;
    //    outputSumX.SumX[2] = x1.z;
    //    outputSumX.SumX[3] = x2.x;
    //    outputSumX.SumX[4] = x2.y;
    //    outputSumX.SumX[5] = x2.z;
    //    outputSumX.SumX[6] = length(x1 - x2);*/
    //    outputSumX.SumX[global_index*2+0] = int(X1.z*100000000);
    //    outputSumX.SumX[global_index*2+1] = int(global_index);
    //}
    //outputSumX.SumX[p2] += ;

    
    atomicAdd(outputSumN.SumN[p1], 1);
    atomicAdd(outputSumN.SumN[p2], 1);
}
