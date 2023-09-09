#shader compute
#version 430 core


layout(local_size_x = 1024, local_size_y = 1, local_size_z = 1) in;
layout(std430, binding = 2) coherent buffer InputVertices
{
    float vertices[];
} inputVertices;

layout(std430, binding = 4) coherent buffer OutputSumX
{
    int SumX[];
} inputSumX;

layout(std430, binding = 5) coherent buffer OutputSumN
{
    uint SumN[];
} inputSumN;

layout(std430, binding = 6) coherent buffer OutputVerticesV
{
    float VerticesV[];
} outputVerticesV;

uniform float dt;
uniform int max;

void main()
{
    uint global_index = gl_GlobalInvocationID.x;
    if (global_index >= max) return;

    if (global_index == 0 || global_index == 20) return;

    vec3 vertices = vec3(inputVertices.vertices[global_index * 3 + 0], inputVertices.vertices[global_index * 3 + 1],inputVertices.vertices[global_index * 3 + 2]);
    vec3 sumx = vec3(float(inputSumX.SumX[global_index * 3 + 0])/10000000, float(inputSumX.SumX[global_index * 3 + 1])/10000000,float(inputSumX.SumX[global_index * 3 + 2])/10000000);
    //vec3 sumx = vec3(0, 0, 0);
    uint sumn = inputSumN.SumN[global_index];
    vec3 verticesv = 1/dt * ((0.2f * vertices+sumx)/(0.2+sumn)-vertices);
    outputVerticesV.VerticesV[global_index * 3 + 0] += verticesv.x;
    outputVerticesV.VerticesV[global_index * 3 + 1] += verticesv.y;
    outputVerticesV.VerticesV[global_index * 3 + 2] += verticesv.z;
    //outputVerticesV.VerticesV[global_index] += 1 / dt * ((0.2f * inputVertices.vertices[global_index] + inputSumX.SumX[global_index]) / (0.2f + inputSumN.SumN[global_index]) - inputVertices.vertices[global_index]);
    vec3 newv = (0.2f * vertices + sumx) / (0.2 + sumn);
    inputVertices.vertices[global_index * 3 + 0] = newv.x;
    inputVertices.vertices[global_index * 3 + 1] = newv.y;
    inputVertices.vertices[global_index * 3 + 2] = newv.z;
    //inputVertices.vertices[global_index] = (0.2f * inputVertices.vertices[global_index] + inputSumX.SumX[global_index]) / (0.2f + inputSumN.SumN[global_index]);
}
