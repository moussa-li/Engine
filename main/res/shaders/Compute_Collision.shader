#shader compute
#version 430 core

layout(local_size_x = 1024, local_size_y = 1, local_size_z = 1) in;

layout(std430, binding = 1) volatile buffer Collision
{
    int collision_number;
} collision;

layout(std430, binding = 2) volatile buffer Sum
{
    int sum_x;
    int sum_y;
    int sum_z;
} sum;

layout(std430, binding = 3) coherent buffer  Input_param
{
    vec3 position;
    vec3 P;
    vec3 V;
    vec3 W;
    vec3 N;
    //mat4 R;

} input_param;

layout(std430, binding = 4) coherent buffer Vertices
{
    float position[];
} vertices;

layout(std430, binding = 5) buffer R_Matrix
{
    mat4 R;
} r_Matrix;

layout(std430, binding = 6) buffer Debug
{
    vec4 index[177152];
    //uint global_index[1024];
    //float phi_x[1024];
} debug;

shared int sharedData[1024];
shared float sharedData_sum0[1024];
shared float sharedData_sum1[1024];
shared float sharedData_sum2[1024];

void main(void)
{
    uint index = gl_LocalInvocationID.x;
    uint global_index = gl_GlobalInvocationID.x;

    sharedData[index] = 0;
    sharedData_sum0[index] = 0;
    sharedData_sum1[index] = 0;
    sharedData_sum2[index] = 0;
    barrier();

    vec3 position = vec3(vertices.position[global_index * 3 + 0], vertices.position[global_index * 3 + 1], vertices.position[global_index * 3 + 2]);
    vec3 Xi = input_param.position + vec3(r_Matrix.R * vec4(position,0.0));
    float phi_x = dot((Xi - input_param.P), input_param.N);
    if (phi_x < 0)
    {
        vec3 Vi = input_param.V + cross(input_param.W , vec3(r_Matrix.R * vec4(position, 0.0)));
        if(dot(Vi,input_param.N) < 0)
        { 
            sharedData[index] = 1;
            sharedData_sum0[index] = position.x;
            sharedData_sum1[index] = position.y;
            sharedData_sum2[index] = position.z;
            
        }
    }
    barrier();

    for (uint stride = 1024 / 2; stride > 0; stride >>= 1)
    {
        if (index < stride)
        {
            sharedData[index] += sharedData[index + stride];
            sharedData_sum0[index] += sharedData_sum0[index + stride];
            sharedData_sum1[index] += sharedData_sum1[index + stride];
            sharedData_sum2[index] += sharedData_sum2[index + stride];
        }
        barrier();
    }
    if (index == 0)
    {
        atomicAdd(collision.collision_number , sharedData[0]);
        
        int s0 = int(sharedData_sum0[0] * 100000);
        int s1 = int(sharedData_sum1[0] * 100000);
        int s2 = int(sharedData_sum2[0] * 100000);

        atomicAdd(sum.sum_x , s0);
        atomicAdd(sum.sum_y , s1);
        atomicAdd(sum.sum_z , s2);
    }

}


