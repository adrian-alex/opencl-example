__kernel void square(__global const int *inputArray, __global int *outputArray, const size_t size)
{
    int threadIdx = get_global_id(0);
    
    if(threadIdx >= size)
        return;
    
    outputArray[threadIdx] = inputArray[threadIdx] * inputArray[threadIdx];

}
