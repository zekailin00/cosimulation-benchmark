#include <stdio.h>
#include <math.h>
#include "kernel.h"

void vecadd1(int length, Vecadd* vecadd, Context* ctx);
void vecadd2(int length, Vecadd* vecadd, Context* ctx);
void softmax1(int length, Softmax* softmax, Context* ctx);
void layernorm1(int length, Layernorm* layernorm, Context* ctx);

int main (int argc, char **argv)
{
  Context* ctx = Context::GetContext();

  int length = 8;
  Vecadd* vecadd;
  Softmax* softmax;
  Layernorm* layernorm;

  printf("\n\nSoftmax 1:\n");
  softmax = Softmax::GetSoftmax(ctx, length);
  softmax1(length, softmax, ctx);
  softmax->Cleanup();
  free(softmax);

  // printf("\n\nVecadd 1:\n");
  // vecadd = Vecadd::GetVecadd(ctx, length);
  // vecadd2(length, vecadd, ctx);
  // vecadd->Cleanup();
  // free(vecadd);

  // printf("\n\nVecadd 2:\n");
  // length = 16;
  // vecadd = Vecadd::GetVecadd(ctx, length);
  // vecadd1(length, vecadd, ctx);
  // vecadd->Cleanup();
  // free(vecadd);

  // printf("\n\nLayernorm 1:\n");
  // layernorm = Layernorm::GetLayernorm(ctx, length);
  // layernorm1(length, layernorm, ctx);
  // layernorm->Cleanup();
  // free(layernorm);
  
  ctx->Cleanup();
  free(ctx);
}

void vecadd1(int length, Vecadd* vecadd, Context* ctx)
{
  for (int i = 0; i < length; i++)
  {
    printf("store at %d\n", i); fflush(stdout);
    vecadd->input1[i] = i;
    vecadd->input2[i] = i + 0.5;
    vecadd->output[i] = 3.33f;
  }

  vecadd->Execute(ctx);

  for (int i = 0; i < length; i++)
  {
    printf("%f ", vecadd->output[i]);
  }
  printf("\n");

}

void vecadd2(int length, Vecadd* vecadd, Context* ctx)
{
  for (int i = 0; i < length; i++)
  {
    printf("store at %d\n", i); fflush(stdout);
    vecadd->input1[i] = i;
    vecadd->input2[i] = length - i;
  }

  vecadd->Execute(ctx);

  for (int i = 0; i < length; i++)
  {
    printf("%f ", vecadd->output[i]);
  }
  printf("\n");

}

void softmax1(int length, Softmax* softmax, Context* ctx)
{
  for (int i = 0; i < length; i++)
  {
    printf("store at %d\n", i); fflush(stdout);
    softmax->input[i] = i;
    softmax->output[i] = 3.33f;
  }

  softmax->Execute(ctx);

  printf("Values from GPU:\n");
  for (int i = 0; i < length; i++)
  {
    printf("%f ", softmax->output[i]);
  }
  printf("\n");

  const float *input = softmax->input;
  float max_value = input[0];
	for (int i = 1; i < length; i++)
	{
		max_value = fmax(max_value, input[i]);
	}

	float sum = 0.0f;
	for (int i = 0; i < length; i++)
	{
		sum = sum + exp(input[i] - max_value); 
	}

  printf("Values from CPU:\n");
  for (int i = 0; i < length; i++)
  {
    printf("%f ", exp(input[i] - max_value) / sum);
  }
  printf("\n");
}

void layernorm1(int length, Layernorm* layernorm, Context* ctx)
{
  for (int i = 0; i < length; i++)
  {
    printf("store at %d\n", i); fflush(stdout);
    layernorm->input[i] = i;
    layernorm->output[i] = 3.33f;
  }

  layernorm->Execute(ctx);

  printf("Values from GPU:\n");
  for (int i = 0; i < length; i++)
  {
    printf("%f ", layernorm->output[i]);
  }
  printf("\n");

  const float *input = layernorm->input;
  float sum = 0.0f;
  for (int i = 0; i < length; i++)
  {
      sum += input[i];
  }

  float mean = sum / length;
  
  float square_diff_sum = 0.0f;
  for (int i = 0; i < length; i++)
  {
      square_diff_sum += (input[i] - mean) * (input[i] - mean);
  }

  float stddev = sqrt(square_diff_sum / length);

  printf("Values from CPU:\n");
  for (int i = 0; i < length; i++)
  {
    printf("%f ", (input[i] - mean) / stddev);
  }
  printf("\n");
}