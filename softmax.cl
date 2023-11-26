__kernel void softmax(__global const float *input,
					  __global const int *length,
					  __global float *output)
{
	if (*length == 0)
		return;
	
	float max_value = input[0];
	for (int i = 1; i < *length; i++)
	{
		max_value = fmax(max_value, input[i]);
	}

	float sum = 0.0f;
	for (int i = 0; i < *length; i++)
	{
		sum = sum + exp(input[i] - max_value); 
	}

	int gid = get_global_id(0);
	output[gid] = exp(input[gid] - max_value) / sum;
}
