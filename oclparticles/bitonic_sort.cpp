#include <cl.h>
#include <fstream>
#include <iostream>

void loadProgSource(const char *filename, std::string *data) {
  std::ifstream file(filename);
  data->assign(std::istreambuf_iterator<char>(file),
               (std::istreambuf_iterator<char>()));
  file.close();
}

const unsigned int wgSizeLim = 1024;
cl_kernel ckBitSortLoc0, ckBitSortLoc1, ckBitMergeGlb, ckBitMergeLoc;
cl_command_queue bitonic_cq;

void initBitSort(cl_context cxGPUContext) {
  static cl_program cpBitSort;
  std::string cBitonicSort;
  loadProgSource("../oclparticles/BitonicSort_b.c", &cBitonicSort);
  cpBitSort = clCreateProgramWithSource(
      cxGPUContext, 1, (const char **)&cBitonicSort, NULL, NULL);
  clBuildProgram(cpBitSort, 0, NULL, NULL, NULL, NULL);
  ckBitSortLoc0 = clCreateKernel(cpBitSort, "bitSortLoc0", NULL);
  ckBitSortLoc1 = clCreateKernel(cpBitSort, "bitSortLoc1", NULL);
  ckBitMergeGlb = clCreateKernel(cpBitSort, "bitMergeGlb", NULL);
  ckBitMergeLoc = clCreateKernel(cpBitSort, "bitMergeLoc", NULL);
}

void bitonicSort(cl_command_queue cqQueue0, cl_mem g_Key, cl_mem g_Val,
                 unsigned int arrSize) {
  // Only power-of-two array lengths are supported so far
  size_t wgSize, glSize0;
  if (arrSize <= wgSizeLim) {
    clSetKernelArg(ckBitSortLoc0, 0, cmSize, (void *)&g_Key);
    clSetKernelArg(ckBitSortLoc0, 1, cmSize, (void *)&g_Val);
    clSetKernelArg(ckBitSortLoc0, 2, sizeof(cl_uint), (void *)&arrSize);
    wgSize = wgSizeLim / 2;
    glSize0 = arrSize / 2;
    clEnqueueNDRangeKernel(cqQueue0, ckBitSortLoc0, 1, NULL, &glSize0, &wgSize,
                           0, NULL, NULL);
  } else {
    clSetKernelArg(ckBitSortLoc1, 0, cmSize, (void *)&g_Key);
    clSetKernelArg(ckBitSortLoc1, 1, cmSize, (void *)&g_Val);
    wgSize = wgSizeLim / 2;
    glSize0 = arrSize / 2;
    clEnqueueNDRangeKernel(cqQueue0, ckBitSortLoc1, 1, NULL, &glSize0, &wgSize,
                           0, NULL, NULL);
    for (unsigned int size = 2 * wgSizeLim; size <= arrSize; size <<= 1)
      for (unsigned int stride = size / 2; stride > 0; stride >>= 1)
        if (stride >= wgSizeLim) {
          clSetKernelArg(ckBitMergeGlb, 0, cmSize, (void *)&g_Key);
          clSetKernelArg(ckBitMergeGlb, 1, cmSize, (void *)&g_Val);
          clSetKernelArg(ckBitMergeGlb, 2, sizeof(cl_uint), (void *)&arrSize);
          clSetKernelArg(ckBitMergeGlb, 3, sizeof(cl_uint), (void *)&size);
          clSetKernelArg(ckBitMergeGlb, 4, sizeof(cl_uint), (void *)&stride);
          wgSize = wgSizeLim / 4;
          glSize0 = arrSize / 2;
          clEnqueueNDRangeKernel(cqQueue0, ckBitMergeGlb, 1, NULL, &glSize0,
                                 &wgSize, 0, NULL, NULL);
        } else {
          clSetKernelArg(ckBitMergeLoc, 0, cmSize, (void *)&g_Key);
          clSetKernelArg(ckBitMergeLoc, 1, cmSize, (void *)&g_Val);
          clSetKernelArg(ckBitMergeLoc, 2, sizeof(cl_uint), (void *)&arrSize);
          clSetKernelArg(ckBitMergeLoc, 3, sizeof(cl_uint), (void *)&stride);
          clSetKernelArg(ckBitMergeLoc, 4, sizeof(cl_uint), (void *)&size);
          wgSize = wgSizeLim / 2;
          glSize0 = arrSize / 2;
          clEnqueueNDRangeKernel(cqQueue0, ckBitMergeLoc, 1, NULL, &glSize0,
                                 &wgSize, 0, NULL, NULL);
          break;
        }
  }
}
