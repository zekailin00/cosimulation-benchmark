__kernel void layernorm(__global const float *input,
                        __global const int *pLength,
						__global float *output)
{
	if (*pLength == 0)
		return;
    
    float sum = 0.0;
    for (int i = 0; i < *pLength; i++)
    {
        sum += input[i];
    }

    float mean = sum / *pLength;
    
    float square_diff_sum = 0.0f;
    for (int i = 0; i < *pLength; i++)
    {
        square_diff_sum += (input[i] - mean) * (input[i] - mean);
    }

    float stddev = sqrt(square_diff_sum / *pLength);

	int gid = get_global_id(0);
	output[gid] = (input[gid] - mean) / stddev;
}