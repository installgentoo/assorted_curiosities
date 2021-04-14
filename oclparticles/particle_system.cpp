#include <tools.cpp>
cl_command_queue cqQueue0, cqQueue1;
cl_program cpParticles;
cl_kernel ckGrav, ckCollide;
cl_mem params;
cl_context cxGPUContext;
cl_device_id *cdDevices;

void initPartSys(cl_context cxGPUContext) {
  std::string cParticles;
  loadProgSource("../oclparticles/Particles.c", &cParticles);
  cpParticles = clCreateProgramWithSource(
      cxGPUContext, 1, (const char **)&cParticles, NULL, NULL);
  clBuildProgram(cpParticles, 0, NULL, "-cl-fast-relaxed-math", NULL, NULL);
  ckGrav = clCreateKernel(cpParticles, "gravity", NULL);
  ckCollide = clCreateKernel(cpParticles, "collide", NULL);
  char buildlog[16384];
  int err =
      clGetProgramBuildInfo(cpParticles, cdDevices[0], CL_PROGRAM_BUILD_LOG,
                            sizeof(buildlog), buildlog, 0);
  oclCheckError(err, CL_SUCCESS);
  std::cerr << buildlog << "\n";
  params =
      clCreateBuffer(cxGPUContext, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR,
                     sizeof(tSimParams), NULL, NULL);
  free(cdDevices);
}

void startupOpenCL() {
  cl_platform_id cpPlatform;
  cl_uint uiNumDevices;
  oclGetPlatformID(&cpPlatform);
  clGetDeviceIDs(cpPlatform, CL_DEVICE_TYPE_GPU, 0, NULL, &uiNumDevices);
  cdDevices = (cl_device_id *)malloc(uiNumDevices * sizeof(cl_device_id));
  clGetDeviceIDs(cpPlatform, CL_DEVICE_TYPE_GPU, uiNumDevices, cdDevices, NULL);
  cxGPUContext = clCreateContext(0, 1, &cdDevices[0], NULL, NULL, NULL);
  cqQueue0 = clCreateCommandQueue(cxGPUContext, cdDevices[0], 0, NULL);
  cqQueue1 = clCreateCommandQueue(cxGPUContext, cdDevices[0], 0, NULL);

  initPartSys(cxGPUContext);
}
