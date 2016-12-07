#include <iostream>
#include <fstream>
//#include <OpenCL/OpenCL.h>
#include <OpenCL/cl.hpp>

void loadInput(int *arr, size_t size);

void writeOutput(int *arr, size_t size);

int main(int argc, const char * argv[]) {
    
    
    const char* SOURCE;
    int *inArr, *outArr;
    size_t size = 100000;
    inArr = new int[size];
    outArr = new int[size];
    
    loadInput(inArr, size);
    
    // OpenCL variables
    
    cl_context  context; //help to comunicate with opencl device //cpu gpu or other
    cl_device_id devices[5];//
    
    cl_platform_id platformID;
    cl_int err;
    cl_event stop;
    cl_ulong startTime, endTime;
    cl_program program;//
    cl_command_queue queue;//
    cl_mem inputArray;
    cl_mem outputArray;
    size_t inputSize = size;//
    cl_uint num;
    
    int deviceID = 0;//device you want to use
    
    cl_kernel squareKernel;//function which will be executed, will be linked to the program
    
    // Create Context
    
    cl_uint ok = 1;
    err = clGetPlatformIDs(ok, &platformID, &num);
    
    //clGetPlatformIDs(NULL, &platformID, NULL);
    
    cl_uint numOfDevices = 0;
    
    clGetDeviceIDs(platformID, CL_DEVICE_TYPE_GPU, 2, devices, &numOfDevices);
    
    if (numOfDevices > 0) {
        
        char deviceName[5][256];
        char deviceVendor[5][256];
        char deviceVersion[5][256];
        cl_int deviceComputeUnits[5];
        cl_int deviceClockFreq[5];
        cl_ulong deviceLocMemSize[5];
        cl_ulong deviceGlobMemSize[5];
        size_t valSize = sizeof deviceName[0];
        
        for (int i = 0; i < numOfDevices; ++i) {
            valSize = sizeof deviceName[i];
            clGetDeviceInfo (devices[i], CL_DEVICE_NAME, valSize, &deviceName[i], &valSize);
            printf ("Device Name: %s\n", deviceName[i]);
            valSize = sizeof deviceVendor[i];
            clGetDeviceInfo (devices[i], CL_DEVICE_VENDOR, valSize, &deviceVendor[i], &valSize);
            printf ("Device Vendor: %s\n", deviceVendor[i]);
            valSize = sizeof deviceVersion[i];
            clGetDeviceInfo (devices[i], CL_DEVICE_VERSION, valSize, &deviceVersion[i], &valSize);
            printf ("Device Version: %s\n", deviceVersion[i]);
            valSize = sizeof deviceComputeUnits[i];
            clGetDeviceInfo (devices[i], CL_DEVICE_MAX_COMPUTE_UNITS, valSize, &deviceComputeUnits[i], &valSize);
            printf ("Num Of Compute Units: %d\n", deviceComputeUnits[i]);
            int numOfSPs = deviceComputeUnits[i];
            if (strcmp(deviceVendor[i], "AMD") == 0) {
                numOfSPs *= 64;
            }
            printf("Num Of processors: %d\n", numOfSPs);
//            size_t maxWorkGroupSize;
//            valSize = sizeof maxWorkGroupSize;
//            clGetDeviceInfo (devices[i], CL_DEVICE_MAX_WORK_GROUP_SIZE, valSize, &maxWorkGroupSize, &valSize);
//            printf ("Max Work Group Size: %zu\n", maxWorkGroupSize);
//            
//            size_t maxWorkDims;
//            valSize = sizeof maxWorkDims;
//            clGetDeviceInfo (devices[i], CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, valSize, &maxWorkDims, &valSize);
//            printf ("Max Work Item dims: %zu\n", maxWorkDims);
//            
//            size_t maxWorkItemSize;
//            valSize = sizeof maxWorkItemSize;
//            clGetDeviceInfo (devices[i], CL_DEVICE_MAX_WORK_ITEM_SIZES, valSize, &maxWorkItemSize, &valSize);
//            printf ("Max Work Item Size: %zu\n", maxWorkItemSize);
            
            valSize = sizeof deviceClockFreq[i];
            clGetDeviceInfo (devices[i], CL_DEVICE_MAX_CLOCK_FREQUENCY, valSize, &deviceClockFreq[i], &valSize);
            printf ("Max Clock Frequency: %d MHz\n", deviceClockFreq[i]);
            valSize = sizeof deviceLocMemSize[i];
            clGetDeviceInfo (devices[i], CL_DEVICE_LOCAL_MEM_SIZE, valSize, &deviceLocMemSize[i], &valSize);
            printf ("Size of Local memory: %llu KBytes\n", deviceLocMemSize[i]/1024);
            valSize = sizeof deviceGlobMemSize[i];
            clGetDeviceInfo (devices[i], CL_DEVICE_GLOBAL_MEM_SIZE, valSize, &deviceGlobMemSize[i], &valSize);
            printf ("Size of Global memory: %llu MBytes\n\n", deviceGlobMemSize[i]/1024/1024);
        }
    }
    
    context = clCreateContext(0, numOfDevices, devices, NULL, NULL, &err);
    
    // Compile GPU Code
    std::ifstream sourceFile("/Users/adrianalex/Desktop/gpu/GPUTest/GPUTest/kernels.cl");
    std::string sourceCode( std::istreambuf_iterator<char>(sourceFile), (std::istreambuf_iterator<char>()));
    
    SOURCE = sourceCode.c_str ();
    program = clCreateProgramWithSource(context, 1, &SOURCE, NULL, &err);
    err = clBuildProgram (program, 1, &devices[deviceID], NULL,NULL,NULL);
    //
    if (err) {
        char log[10240] = "";
        err = clGetProgramBuildInfo(program, devices[deviceID], CL_PROGRAM_BUILD_LOG,
                                    sizeof log, log, NULL);
        printf ("Build log:\n%s\n", log);
    }
    
    //printf("Source: %s\n", SOURCE);
    
    squareKernel = clCreateKernel (program, "square", &err);
    
    queue = clCreateCommandQueue (context, devices[deviceID], CL_QUEUE_PROFILING_ENABLE, &err);
    
    // Set Kernel Arguments
    
    inputArray = clCreateBuffer (context, CL_MEM_READ_WRITE,  size * sizeof (cl_int), NULL, &err);
    
    outputArray = clCreateBuffer (context, CL_MEM_READ_WRITE,  size * sizeof (cl_int), NULL, &err);
    
    err = clSetKernelArg (squareKernel, 0, sizeof (cl_mem), (void*) &inputArray);
    err = clSetKernelArg (squareKernel, 1, sizeof (cl_mem), (void*) &outputArray);
    err = clSetKernelArg (squareKernel, 2, sizeof (inputSize), (void*) &inputSize);
    
    // Copy Data from Host to Device
    
    err = clEnqueueWriteBuffer (queue, inputArray, CL_TRUE, 0, size * sizeof (cl_int), inArr, 0, NULL, NULL);
    
    err = clEnqueueWriteBuffer (queue, outputArray, CL_TRUE, 0, size * sizeof (cl_int), outArr, 0, NULL, NULL);
    
    size_t localSize;
    
    err = clGetKernelWorkGroupInfo(squareKernel, devices[deviceID], CL_KERNEL_WORK_GROUP_SIZE, sizeof(localSize), &localSize, NULL);
    
    size_t blockSize = localSize;//group of threads, ask from the hardware, upper limit
    size_t localWorkSize = blockSize;//hardware supports
    size_t globalWorkSize =  (size/blockSize + 1) * blockSize;//how many total threads
    
    size_t offset = 0;
    
    // Dispatch Computation
    
    //this dispaches the threas you need//how many threads
    err = clEnqueueNDRangeKernel (queue, squareKernel, 1, &offset, &globalWorkSize, &localWorkSize, 0, NULL, &stop);
    
    clWaitForEvents (1, &stop); //wait until kerner execution returns
    
    clGetEventProfilingInfo(stop, CL_PROFILING_COMMAND_START, sizeof (cl_ulong), &startTime, NULL);
    clGetEventProfilingInfo(stop, CL_PROFILING_COMMAND_END, sizeof (cl_ulong), &endTime, NULL);
    
    double actTime = endTime - startTime;
    
    printf("Computation Time: %lf nanoseconds\n", actTime);
    
    // Copy Data from Device to Host
    
    err = clEnqueueReadBuffer(queue, outputArray, CL_TRUE, 0, size * sizeof (cl_float), outArr, 0, NULL, NULL);
    
    // Print Results;
    
   // writeOutput(outArr, size);
    
    // Clean up
    clReleaseMemObject(inputArray);
    clReleaseMemObject(outputArray);
    clReleaseKernel(squareKernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);
    
    for (int i = 0; i < numOfDevices; i++) {
        clReleaseDevice(devices[i]);
        
    }
    
    delete inArr;
    delete outArr;
    
    return 0;
}

void loadInput(int *arr, size_t size)
{
    for (int i = 0; i < size; ++i) {
        arr[i] = i;
    }
}


void writeOutput(int *arr, size_t size)
{
    for (int i = 0; i < size; ++i) {
        printf("%d element: %d\n", i, arr[i]);
    }
}
