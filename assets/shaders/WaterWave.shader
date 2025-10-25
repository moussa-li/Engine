#shader compute
#version 430 core

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(std430, binding = 31) coherent buffer Old_H
{
    float data[];
} old_h;

layout(std430, binding = 32) coherent buffer H
{
    float data[];
} h;

layout(std430, binding = 33) coherent buffer New_H
{
    float data[];
} new_h;

uniform float damping;
//uniform float smoothing;
uniform float rate;

uniform int size;

void main()
{
    uint i = gl_GlobalInvocationID.x;
    uint j = gl_GlobalInvocationID.y;

    if (i >= size)return;
    if (j >= size)return;

    uint lower = max(0, j - 1);
    uint upper = min(size-1, j + 1);
    uint left = max(0, i - 1);
    uint right = min(size - 1, i + 1);

    float smoothing = h.data[lower * size + i] + h.data[upper * size + i] + h.data[j * size + right] + h.data[j * size + left] - 4 * h.data[j * size + i];

    new_h.data[j * size + i] = h.data[j * size + i] + (h.data[j * size + i] - old_h.data[j * size + i]) * damping + smoothing * rate;

    old_h.data[j * size + i] = h.data[j * size + i];
    h.data[j * size + i] = new_h.data[j * size + i];

}
