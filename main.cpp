#include <stdio.h>
#include <math.h>
#include "kernel.h"

void vecadd1(int length, Vecadd* vecadd, Context* ctx);
void vecadd2(int length, Vecadd* vecadd, Context* ctx);
void softmax1(int length, Softmax* softmax, Context* ctx);
void layernorm1(int length, Layernorm* layernorm, Context* ctx);

inline static uint64_t read_cycles() {
  uint64_t cycles = 0;
#ifdef RISCV_COMPILATION
  asm volatile ("rdcycle %0" : "=r" (cycles));
#endif
  return cycles;
}

void task(int length);

int main (int argc, char **argv)
{
  printf("RecordTime\n");
  task(16);
  printf("RecordTime\n");
  task(32);
  printf("RecordTime\n");
  task(64);
  printf("RecordTime\n");
  task(128);
  printf("RecordTime\n");
  task(256);
  printf("RecordTime\n");
  task(512);
  printf("RecordTime\n");
  task(1024);
  printf("RecordTime\n");
  task(2048);
  printf("RecordTime\n");
  task(4096);
  printf("RecordTime\n");
}

void task(int length)
{

  volatile uint64_t cycle_begin = read_cycles();
  printf("[CPU Cycle Count] begin %llu\n", cycle_begin);

  Context* ctx = Context::GetContext();

  printf("Length is %d\n", length);

  Vecadd* vecadd;
  Softmax* softmax;
  Layernorm* layernorm;

  printf("\n\nSoftmax 1:\n");
  softmax = Softmax::GetSoftmax(ctx, length);
  softmax1(length, softmax, ctx);
  softmax->Cleanup();
  free(softmax);
  
  ctx->Cleanup();
  free(ctx);
  
  volatile uint64_t cycle_end = read_cycles();
  printf("[CPU Cycle Count] end %llu\n", cycle_end);
  printf("[CPU Cycle Count] difference %llu\n", cycle_end-cycle_begin);
}

void vecadd1(int length, Vecadd* vecadd, Context* ctx)
{
  for (int i = 0; i < length; i++)
  {
    printf("store at %d\n", i); fflush(stdout);
    vecadd->input1[i] = i;
    vecadd->input2[i] = i + 0.5;
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

  // const float *input = softmax->input;
  // float max_value = input[0];
	// for (int i = 1; i < length; i++)
	// {
	// 	max_value = fmax(max_value, input[i]);
	// }

	// float sum = 0.0f;
	// for (int i = 0; i < length; i++)
	// {
	// 	sum = sum + exp(input[i] - max_value); 
	// }

  // printf("Values from CPU:\n");
  // for (int i = 0; i < length; i++)
  // {
  //   printf("%f ", exp(input[i] - max_value) / sum);
  // }
  // printf("\n");
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