#include "kernel.h"

#include <chrono>
#include <stdio.h>
#include <cstdlib>
#include <sys/stat.h>


#define CL_CHECK(_expr)                                                \
   do {                                                                \
     cl_int _err = _expr;                                              \
     if (_err == CL_SUCCESS)                                           \
       break;                                                          \
     printf("OpenCL Error: '%s' returned %d!\n", #_expr, (int)_err);   \
	 ctx->Cleanup();			                                       \
     exit(-1);                                                         \
   } while (0)

#define CL_CHECK2(_expr)                                               \
   ({                                                                  \
     cl_int _err = CL_INVALID_VALUE;                                   \
     decltype(_expr) _ret = _expr;                                     \
     if (_err != CL_SUCCESS) {                                         \
       printf("OpenCL Error: '%s' returned %d at line %d!\n", #_expr, (int)_err, __LINE__); \
	   ctx->Cleanup();			                                       \
       exit(-1);                                                       \
     }                                                                 \
     _ret;                                                             \
   })


static int read_kernel_file(const char* filename, uint8_t** data, size_t* size) {
  if (nullptr == filename || nullptr == data || 0 == size)
      return -1;
  
  printf("File name: %s\n", filename); fflush(stdout);
  FILE* fp = fopen(filename, "r");

  if (NULL == fp) {

      fprintf(stderr, "Failed to load kernel.");
      return -1;
  }

  long fsize;
  {
      struct stat buffer;
      int         status;
      status = stat(filename, &buffer);
      fsize = buffer.st_size;
  }
  fsize = 100000;

  rewind(fp);

  *data = (uint8_t*)malloc(fsize);
  *size = fread(*data, 1, fsize, fp);

  printf("File size: %d\n", *size); fflush(stdout);
  fclose(fp);
  
  return 0;
}

static int write_operand_file(const char* filename, void* data, size_t size) {
  if (nullptr == filename || nullptr == data || 0 == size)
    return -1;

  FILE* fp = fopen(filename, "wb");
  if (NULL == fp) {
    fprintf(stderr, "Failed to write operand data.\n");
    return -1;
  }

  size_t wsize = fwrite(data, size, 1, fp);
  if (wsize != 1) {
    fprintf(stderr, "Failed to write operand data.\n");
    return -1;
  }

  return 0;
}

void Context::Cleanup() {
    if (commandQueue) clReleaseCommandQueue(commandQueue);
    if (context) clReleaseContext(context);
    if (device_id) clReleaseDevice(device_id);
}

Context* Context::GetContext()
{
    Context* ctx = new Context();

    CL_CHECK(clGetPlatformIDs(1, &ctx->platform_id, NULL));
    CL_CHECK(clGetDeviceIDs(ctx->platform_id, CL_DEVICE_TYPE_DEFAULT, 1, &ctx->device_id, NULL));

    //////////////////////////////  Platform Info  ////////////////////////////////
    char buffer[256];
    size_t retSize;

    CL_CHECK(clGetPlatformInfo(ctx->platform_id, CL_PLATFORM_NAME, 256, buffer, &retSize));
    printf("CL_PLATFORM_NAME: %s\n", buffer);
    CL_CHECK(clGetPlatformInfo(ctx->platform_id, CL_PLATFORM_VENDOR, 256, buffer, &retSize));
    printf("CL_PLATFORM_VENDOR: %s\n", buffer);
    CL_CHECK(clGetPlatformInfo(ctx->platform_id, CL_PLATFORM_PROFILE, 256, buffer, &retSize));
    printf("CL_PLATFORM_PROFILE: %s\n", buffer);

    CL_CHECK(clGetDeviceInfo(ctx->device_id, CL_DEVICE_VENDOR, 256, buffer, &retSize));
    printf("CL_DEVICE_VENDOR: %s\n", buffer);
    //////////////////////////////////////////////////////////////////////////////

    printf("Create context\n");
    ctx->context = CL_CHECK2(clCreateContext(NULL, 1, &ctx->device_id, NULL, NULL,  &_err));
    ctx->commandQueue = CL_CHECK2(clCreateCommandQueue(ctx->context, ctx->device_id, 0, &_err));  

    fflush(stdout);
    return ctx;
}

Vecadd* Vecadd::GetVecadd(Context* ctx, uint32_t length)
{
    size_t kernel_size;
    cl_int binary_status;
    uint8_t *kernel_bin = NULL;

    // Read kernel binary from file  
    if (0 != read_kernel_file("vecadd.pocl", &kernel_bin, &kernel_size))
        return nullptr;

    size_t nbytes = (int)length * sizeof(float);

    Vecadd *vecadd = new Vecadd();
    vecadd->input1 = (float*)malloc(nbytes);
    vecadd->input2 = (float*)malloc(nbytes);
    vecadd->output = (float*)malloc(nbytes);
    vecadd->length = length;

    printf("Allocate device buffers: %d bytes\n", nbytes); fflush(stdout); 
    vecadd->input1_memobj = CL_CHECK2(clCreateBuffer(ctx->context, CL_MEM_READ_ONLY, nbytes, NULL, &_err));
    vecadd->input2_memobj = CL_CHECK2(clCreateBuffer(ctx->context, CL_MEM_READ_ONLY, nbytes, NULL, &_err));
    vecadd->output_memobj = CL_CHECK2(clCreateBuffer(ctx->context, CL_MEM_WRITE_ONLY, nbytes, NULL, &_err));

    printf("Create program with binary\n");
    vecadd->program = CL_CHECK2(clCreateProgramWithBinary(
        ctx->context, 1, &ctx->device_id, &kernel_size,
        (const uint8_t**)&kernel_bin, &binary_status, &_err
    ));

    if (vecadd->program == NULL)
    {
        vecadd->Cleanup();
        delete vecadd;
        return nullptr;
    }

    printf("clBuildProgram\n"); fflush(stdout);

    // Build program
    CL_CHECK(clBuildProgram(vecadd->program, 1, &ctx->device_id, NULL, NULL, NULL));

    // Create kernel
    vecadd->kernel = CL_CHECK2(clCreateKernel(vecadd->program, "vecadd", &_err));

    // // Set kernel arguments
    CL_CHECK(clSetKernelArg(vecadd->kernel, 0, sizeof(cl_mem), (void *)&(vecadd->input1_memobj)));fflush(stdout);
    CL_CHECK(clSetKernelArg(vecadd->kernel, 1, sizeof(cl_mem), (void *)&(vecadd->input2_memobj)));fflush(stdout);
    CL_CHECK(clSetKernelArg(vecadd->kernel, 2, sizeof(cl_mem), (void *)&(vecadd->output_memobj)));fflush(stdout);

    return vecadd;
}

void Vecadd::Execute(Context* ctx)
{
    Vecadd* vecadd  = this;

    size_t nbytes = vecadd->length * sizeof(float);

    if (write_operand_file("vecadd.input.a.bin", vecadd->input1, nbytes) != 0 ||
        write_operand_file("vecadd.input.b.bin", vecadd->input2, nbytes) != 0)
    {
        printf("ERROR. Write operants failed.");
        exit(-1);
    }

    printf("---------Upload source buffers with size: %d\n", nbytes); fflush(stdout);
    CL_CHECK(clEnqueueWriteBuffer(ctx->commandQueue, vecadd->input1_memobj,
        CL_TRUE, 0, nbytes, vecadd->input1, 0, NULL, NULL));
    CL_CHECK(clEnqueueWriteBuffer(ctx->commandQueue, vecadd->input2_memobj,
        CL_TRUE, 0, nbytes, vecadd->input2, 0, NULL, NULL));

    printf("Execute the kernel\n");
    size_t global_work_size[1] = {vecadd->length};
    size_t local_work_size[1] = {1};
    auto time_start = std::chrono::high_resolution_clock::now();
    CL_CHECK(clEnqueueNDRangeKernel(ctx->commandQueue, vecadd->kernel, 1, NULL,
        global_work_size, local_work_size, 0, NULL, NULL));

    CL_CHECK(clFinish(ctx->commandQueue));

    printf("Download destination buffer\n"); fflush(stdout);
    CL_CHECK(clEnqueueReadBuffer(ctx->commandQueue,
        vecadd->output_memobj, CL_TRUE, 0, nbytes, vecadd->output, 0, NULL, NULL));
    
    auto time_end = std::chrono::high_resolution_clock::now();
    double elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(time_end - time_start).count();
    printf("Elapsed time: %lg ms\n", elapsed);
    fflush(stdout);
}

void Vecadd::Cleanup()
{
    if (input1) free(input1);
    if (input2) free(input2);
    if (output) free(output);

    if (kernel) clReleaseKernel(kernel);
    if (program) clReleaseProgram(program);

    if (input1_memobj) clReleaseMemObject(input1_memobj);
    if (input2_memobj) clReleaseMemObject(input2_memobj);
    if (output_memobj) clReleaseMemObject(output_memobj);

    input1 = nullptr;
    input2 = nullptr;
    output = nullptr;

    input1_memobj = NULL;
    input2_memobj = NULL;
    output_memobj = NULL;
    program = NULL;
    kernel = NULL;
}

Softmax* Softmax::GetSoftmax(Context* ctx, uint32_t length)
{
    size_t kernel_size;
    cl_int binary_status;
    uint8_t *kernel_bin = NULL;

    // Read kernel binary from file  
    if (0 != read_kernel_file("/scratch/zekailin00/cosimulation-benchmark/softmax.pocl", &kernel_bin, &kernel_size))
        return nullptr;

    size_t nbytes = (int)length * sizeof(float);

    Softmax *softmax = new Softmax();
    softmax->input = (float*)malloc(nbytes);
    softmax->output = (float*)malloc(nbytes);
    softmax->length = length;

    printf("Allocate device buffers: %d bytes\n", nbytes); fflush(stdout); 
    softmax->input_memobj = CL_CHECK2(clCreateBuffer(ctx->context, CL_MEM_READ_ONLY, nbytes, NULL, &_err));
    softmax->length_memobj = CL_CHECK2(clCreateBuffer(ctx->context, CL_MEM_READ_ONLY, sizeof(float), NULL, &_err));
    softmax->output_memobj = CL_CHECK2(clCreateBuffer(ctx->context, CL_MEM_WRITE_ONLY, nbytes, NULL, &_err));

    printf("Create program with binary\n");
    softmax->program = CL_CHECK2(clCreateProgramWithBinary(
        ctx->context, 1, &ctx->device_id, &kernel_size,
        (const uint8_t**)&kernel_bin, &binary_status, &_err
    ));

    if (softmax->program == NULL)
    {
        softmax->Cleanup();
        delete softmax;
        return nullptr;
    }

    printf("clBuildProgram\n"); fflush(stdout);

    // Build program
    CL_CHECK(clBuildProgram(softmax->program, 1, &ctx->device_id, NULL, NULL, NULL));

    // Create kernel
    softmax->kernel = CL_CHECK2(clCreateKernel(softmax->program, "softmax", &_err));

    // // Set kernel arguments
    CL_CHECK(clSetKernelArg(softmax->kernel, 0, sizeof(cl_mem), (void *)&(softmax->input_memobj)));fflush(stdout);
    CL_CHECK(clSetKernelArg(softmax->kernel, 1, sizeof(cl_mem), (void *)&(softmax->length_memobj)));fflush(stdout);
    CL_CHECK(clSetKernelArg(softmax->kernel, 2, sizeof(cl_mem), (void *)&(softmax->output_memobj)));fflush(stdout);

    return softmax;
}

void Softmax::Execute(Context* ctx)
{
    Softmax* softmax = this;

    size_t nbytes = softmax->length * sizeof(float);

    if (write_operand_file("softmax.input.a.bin", softmax->input, nbytes) != 0 ||
        write_operand_file("softmax.input.b.bin", &softmax->length, sizeof(float)) != 0)
    {
        printf("ERROR. Write operants failed.");
        exit(-1);
    }

    printf("---------Upload source buffers with size: %d\n", nbytes); fflush(stdout);
    CL_CHECK(clEnqueueWriteBuffer(ctx->commandQueue, softmax->input_memobj,
        CL_TRUE, 0, nbytes, softmax->input, 0, NULL, NULL));
    CL_CHECK(clEnqueueWriteBuffer(ctx->commandQueue, softmax->length_memobj,
        CL_TRUE, 0, sizeof(float), &softmax->length, 0, NULL, NULL));

    printf("Execute the kernel\n");
    size_t global_work_size[1] = {softmax->length};
    size_t local_work_size[1] = {1};
    auto time_start = std::chrono::high_resolution_clock::now();
    CL_CHECK(clEnqueueNDRangeKernel(ctx->commandQueue, softmax->kernel, 1, NULL,
        global_work_size, local_work_size, 0, NULL, NULL));

    CL_CHECK(clFinish(ctx->commandQueue));

    printf("Download destination buffer\n"); fflush(stdout);
    CL_CHECK(clEnqueueReadBuffer(ctx->commandQueue,
        softmax->output_memobj, CL_TRUE, 0, nbytes, softmax->output, 0, NULL, NULL));
    
    auto time_end = std::chrono::high_resolution_clock::now();
    double elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(time_end - time_start).count();
    printf("Elapsed time: %lg ms\n", elapsed);
    fflush(stdout);
}

void Softmax::Cleanup()
{
    if (input) free(input);
    if (output) free(output);

    if (kernel) clReleaseKernel(kernel);
    if (program) clReleaseProgram(program);

    if (input_memobj) clReleaseMemObject(input_memobj);
    if (length_memobj) clReleaseMemObject(length_memobj);
    if (output_memobj) clReleaseMemObject(output_memobj);

    input = nullptr;
    output = nullptr;

    input_memobj = NULL;
    length_memobj = NULL;
    output_memobj = NULL;
    program = NULL;
    kernel = NULL;
}

Layernorm* Layernorm::GetLayernorm(Context* ctx, uint32_t length)
{
    size_t kernel_size;
    cl_int binary_status;
    uint8_t *kernel_bin = NULL;

    // Read kernel binary from file  
    if (0 != read_kernel_file("layernorm.pocl", &kernel_bin, &kernel_size))
        return nullptr;

    size_t nbytes = (int)length * sizeof(float);

    Layernorm *layernorm = new Layernorm();
    layernorm->input = (float*)malloc(nbytes);
    layernorm->output = (float*)malloc(nbytes);
    layernorm->length = length;

    printf("Allocate device buffers: %d bytes\n", nbytes); fflush(stdout); 
    layernorm->input_memobj = CL_CHECK2(clCreateBuffer(ctx->context, CL_MEM_READ_ONLY, nbytes, NULL, &_err));
    layernorm->length_memobj = CL_CHECK2(clCreateBuffer(ctx->context, CL_MEM_READ_ONLY, sizeof(float), NULL, &_err));
    layernorm->output_memobj = CL_CHECK2(clCreateBuffer(ctx->context, CL_MEM_WRITE_ONLY, nbytes, NULL, &_err));

    printf("Create program with binary\n");
    layernorm->program = CL_CHECK2(clCreateProgramWithBinary(
        ctx->context, 1, &ctx->device_id, &kernel_size,
        (const uint8_t**)&kernel_bin, &binary_status, &_err
    ));

    if (layernorm->program == NULL)
    {
        layernorm->Cleanup();
        delete layernorm;
        return nullptr;
    }

    printf("clBuildProgram\n"); fflush(stdout);

    // Build program
    CL_CHECK(clBuildProgram(layernorm->program, 1, &ctx->device_id, NULL, NULL, NULL));

    // Create kernel
    layernorm->kernel = CL_CHECK2(clCreateKernel(layernorm->program, "layernorm", &_err));

    // // Set kernel arguments
    CL_CHECK(clSetKernelArg(layernorm->kernel, 0, sizeof(cl_mem), (void *)&(layernorm->input_memobj)));fflush(stdout);
    CL_CHECK(clSetKernelArg(layernorm->kernel, 1, sizeof(cl_mem), (void *)&(layernorm->length_memobj)));fflush(stdout);
    CL_CHECK(clSetKernelArg(layernorm->kernel, 2, sizeof(cl_mem), (void *)&(layernorm->output_memobj)));fflush(stdout);

    return layernorm;
}

void Layernorm::Execute(Context* ctx)
{
    Layernorm* layernorm = this;

    size_t nbytes = layernorm->length * sizeof(float);

    if (write_operand_file("layernorm.input.a.bin", layernorm->input, nbytes) != 0 ||
        write_operand_file("layernorm.input.b.bin", &layernorm->length, sizeof(float)) != 0)
    {
        printf("ERROR. Write operants failed.");
        exit(-1);
    }

    printf("---------Upload source buffers with size: %d\n", nbytes); fflush(stdout);
    CL_CHECK(clEnqueueWriteBuffer(ctx->commandQueue, layernorm->input_memobj,
        CL_TRUE, 0, nbytes, layernorm->input, 0, NULL, NULL));
    CL_CHECK(clEnqueueWriteBuffer(ctx->commandQueue, layernorm->length_memobj,
        CL_TRUE, 0, sizeof(float), &layernorm->length, 0, NULL, NULL));

    printf("Execute the kernel\n");
    size_t global_work_size[1] = {layernorm->length};
    size_t local_work_size[1] = {1};
    auto time_start = std::chrono::high_resolution_clock::now();
    CL_CHECK(clEnqueueNDRangeKernel(ctx->commandQueue, layernorm->kernel, 1, NULL,
        global_work_size, local_work_size, 0, NULL, NULL));

    CL_CHECK(clFinish(ctx->commandQueue));

    printf("Download destination buffer\n"); fflush(stdout);
    CL_CHECK(clEnqueueReadBuffer(ctx->commandQueue,
        layernorm->output_memobj, CL_TRUE, 0, nbytes, layernorm->output, 0, NULL, NULL));
    
    auto time_end = std::chrono::high_resolution_clock::now();
    double elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(time_end - time_start).count();
    printf("Elapsed time: %lg ms\n", elapsed);
    fflush(stdout);
}

void Layernorm::Cleanup()
{
    if (input) free(input);
    if (output) free(output);

    if (kernel) clReleaseKernel(kernel);
    if (program) clReleaseProgram(program);

    if (input_memobj) clReleaseMemObject(input_memobj);
    if (length_memobj) clReleaseMemObject(length_memobj);
    if (output_memobj) clReleaseMemObject(output_memobj);

    input = nullptr;
    output = nullptr;

    input_memobj = NULL;
    length_memobj = NULL;
    output_memobj = NULL;
    program = NULL;
    kernel = NULL;
}