/*
 * evaluation.cpp
 *
 *  Created on: Aug 29, 2015
 *      Author: becker
 */

#include "evaluation.h"


float kineticEnergy(cl_float2* velocities,int Nparticles,float mass){
	float energy=0;
	for(int i=0;i<Nparticles;i++) {
		energy+=(velocities[i].s[0]*velocities[i].s[0]
	           +velocities[i].s[1]*velocities[i].s[1])/2/mass;
	}
	return energy;
}

cl_float2 momentum(cl_float2* velocities,int Nparticles,float mass) {
	cl_float2 momentum {0,0};
	for(int i=0;i<Nparticles;i++) {
			momentum.s[0]+=velocities[i].s[0]*mass;
			momentum.s[1]+=velocities[i].s[1]*mass;
		}
	return momentum;
}


