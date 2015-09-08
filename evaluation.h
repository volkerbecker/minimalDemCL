#ifndef EVALUATION_H_
#define EVALUATION_H_

#include <CL/cl.hpp>

float kineticEnergy(cl_float2* velocities,int Nparticles,float mass);
cl_float2 momentum(cl_float2* velocities,int Nparticles,float mass);

#endif
