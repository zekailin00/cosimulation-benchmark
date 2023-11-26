#pragma once

#include <CL/opencl.h>

struct Context
{
public:
    cl_platform_id platform_id = NULL;
    cl_device_id device_id = NULL;
    cl_context context = NULL;
    cl_command_queue commandQueue = NULL;

    static Context* GetContext();
    void Cleanup();
};

struct Vecadd
{
public:
    uint32_t length = -1;
    float* input1 = nullptr;
    float* input2 = nullptr;
    float* output = nullptr;

    static Vecadd* GetVecadd(Context* ctx, uint32_t length);
    void Execute(Context* ctx);
    void Cleanup();

private:
    cl_mem input1_memobj = NULL;
    cl_mem input2_memobj = NULL;
    cl_mem output_memobj = NULL;
    cl_program program = NULL;
    cl_kernel kernel = NULL;
};

struct Softmax
{
public:
    uint32_t length = -1;
    float* input = nullptr;
    float* output = nullptr;

    static Softmax* GetSoftmax(Context* ctx, uint32_t length);
    void Execute(Context* ctx);
    void Cleanup();

private:
    cl_mem input_memobj = NULL;
    cl_mem length_memobj = NULL;
    cl_mem output_memobj = NULL;
    cl_program program = NULL;
    cl_kernel kernel = NULL;
};

struct Layernorm
{
public:
    uint32_t length = -1;
    float* input = nullptr;
    float* output = nullptr;

    static Layernorm* GetLayernorm(Context* ctx, uint32_t length);
    void Execute(Context* ctx);
    void Cleanup();

private:
    cl_mem input_memobj = NULL;
    cl_mem length_memobj = NULL;
    cl_mem output_memobj = NULL;
    cl_program program = NULL;
    cl_kernel kernel = NULL;
};