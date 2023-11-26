__kernel void vecadd(__global const float *input1,
	                   __global const float *input2,
	                   __global float *output)
{
  int gid = get_global_id(0);
  output[gid] = input1[gid] + input2[gid];
}