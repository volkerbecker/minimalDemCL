//============================================================================
// Name        : minimalDemCL.cpp
// Author      : Volker Becker
// Version     :
// Copyright   : (c) 2015 by Volker Becker
// Description : Hello World in C, Ansi-style
//============================================================================
#include "Visualizer.h"
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <CL/cl.hpp>
#include "clhelpers.h"
#include "initialisation.h"
#include <fstream>
#include "evaluation.h"

#define pi 3.14159265359

void saveSnapShot(std::string name, const cl_float2 *positions,
		const cl_float2 *velocities, const cl_float2 *accelaration,
		const int &N) {
	std::ofstream savefile(name.c_str());
	for (int i = 0; i < N; ++i) {
		savefile << positions[i].s[0] << "\t";
		savefile << positions[i].s[1] << "\t";
		savefile << velocities[i].s[0] << "\t";
		savefile << velocities[i].s[1] << "\t";
		savefile << accelaration[i].s[0] << "\t";
		savefile << accelaration[i].s[1] << "\t";
		//savefile << positionSaveX[i] << "\t";
		//savefile << positionSaveY[i] << "\t";
		savefile << "\n";
	}
	savefile.close();
}

struct kernelParameters{
	cl_float2 lxly;
	cl_int2 cellnxny;
	cl_float timestep;
	cl_float timestepsq;
	cl_float radius;
	cl_float radiussq;
	cl_float mass;
	cl_float springConstant;
	cl_float verletLength;
	cl_float verletLengthSq;
	cl_float verletDistance;
	cl_float verletDistanceSq;
	cl_float2 cellLengthXY;
	cl_int maxparticlePerCell;
	cl_int verletlistLength;
	cl_int maxTimesteps;
	cl_int snapshot;
	cl_int Nparticle;
	cl_float damping;
};

int main(int argc, char* argv[]) {
	// initialize parameters -> todo spaeter in struktur stecken
	if(argc!=4) {
		std::cout << "wrong paramter number, use minimalDemCl particleNumber boxlenghtX boxlengthY"
				<< std::endl;
		std::cout << "parameter number:" << argc << std::endl;
		for(int i=0;i<argc;i++) {
			std::cout << "parameter "<< i <<": " << argv[i] << std::endl;
		}
		exit(0);
	} else {
		std::cout << "parameter number:" << argc << std::endl;
				for(int i=0;i<argc;i++) {
					std::cout << "parameter "<< i <<": " << argv[i] << std::endl;
				}
	}

	//Parameters relevant for the host only
	const int particleNumber = atoi(argv[1]);
	const float maxvelocity = 10; // Maximal velocity
	const int maxTimesteps = 100000;
    const int snapshot = 1000;

	//Kernel Parameters
	struct kernelParameters kernelParams;

	kernelParams.Nparticle=particleNumber;
	kernelParams.lxly = (cl_float2){(float)atof(argv[2]), (float)atof(argv[3]) };
	kernelParams.timestep=0.001;
	kernelParams.timestepsq=kernelParams.timestep*kernelParams.timestep;
	kernelParams.radius=1;
	kernelParams.radiussq=kernelParams.radius*kernelParams.radius;
	kernelParams.mass=1;
	kernelParams.springConstant = 1000;
	kernelParams.verletLength=4*kernelParams.radius;
	kernelParams.verletLengthSq=kernelParams.verletLength*kernelParams.verletLength;
	kernelParams.verletDistance=(kernelParams.verletLength-2*kernelParams.radius) /2;
	kernelParams.verletDistanceSq=kernelParams.verletDistance*kernelParams.verletDistance;
	kernelParams.cellnxny.s[0] = (int)(kernelParams.lxly.s[0] / kernelParams.verletLength);
	kernelParams.cellnxny.s[1] = (int) (kernelParams.lxly.s[1]/ kernelParams.verletLength);
	kernelParams.cellLengthXY.s[0] = kernelParams.lxly.s[0] / kernelParams.cellnxny.s[0];
	kernelParams.cellLengthXY.s[1] = kernelParams.lxly.s[1] / kernelParams.cellnxny.s[1];
	kernelParams.maxparticlePerCell=((kernelParams.cellLengthXY.s[0] + 2 * kernelParams.radius)
			* (kernelParams.cellLengthXY.s[1] + 2 * kernelParams.radius)
			/ pi / kernelParams.radiussq + 0.5);

	kernelParams.verletlistLength = (int) ((pi * (kernelParams.verletLength + kernelParams.radius)
				* (kernelParams.verletLength + kernelParams.radius) - pi * kernelParams.radiussq)
			/ pi * kernelParams.radiussq + 0.5);
	kernelParams.damping=5;

	//////////////////////////////////////////////////////////////////////////
	// Host Datenfelder intialisieren
	/////////////////////////////////////////////////////////////////

	cl_int *verletNeedsUpdate = new cl_int[1] { true };
	const size_t verletNeedsUpdateSize = sizeof(cl_int) * 1;

	const size_t datasize = sizeof(cl_float2) * particleNumber;

	const size_t cellBufferSize = kernelParams.cellnxny.s[0] * kernelParams.cellnxny.s[1]
				* kernelParams.maxparticlePerCell * sizeof(cl_int);

	cl_float2 *positions = new cl_float2[particleNumber];
	cl_float2 *velocities = new cl_float2[particleNumber];
	cl_float2 *accelerations = new cl_float2[particleNumber];
	size_t verletlistSize = (particleNumber + 1) * kernelParams.verletlistLength
			* sizeof(cl_int);
	cl_int *verletlist = new cl_int[verletlistSize];

    printf("Initialize regular grid of %i particles in a %f %f box \n",particleNumber,
    		kernelParams.lxly.s[0],kernelParams.lxly.s[1]);
	initializeParticles(REGULARGRID, positions, velocities, accelerations, kernelParams.lxly,
			kernelParams.radius, particleNumber, maxvelocity);
	cl_float2 korrektur = momentum(velocities, kernelParams.Nparticle,
			kernelParams.mass);
	korrektur.s[0]/= (kernelParams.mass *kernelParams.Nparticle);
	korrektur.s[1]/= (kernelParams.mass *kernelParams.Nparticle);
	for(int i=0;i<kernelParams.Nparticle;i++) {
		velocities[i].s[0]-=korrektur.s[0];
		velocities[i].s[1]-=korrektur.s[1];
	}


	//initalize visualisation
	Visualizer visoutput;
	visoutput.initializeWindow(500, 500);
	visoutput.initializeSystem((float*) positions, kernelParams.Nparticle,
			kernelParams.radius, kernelParams.lxly.s[0], kernelParams.lxly.s[1],0,0,
			15);

	Visualizer visoutputcenter;
	visoutputcenter.initializeWindow(500, 500);
	visoutputcenter.initializeSystem((float*) positions, kernelParams.Nparticle,
				kernelParams.radius, kernelParams.lxly.s[0]/5, kernelParams.lxly.s[1]/5,
				kernelParams.lxly.s[0]/5*2, kernelParams.lxly.s[1]/5*2,
				15);


	try {
		std::vector<cl::Platform> plattforms;
		cl::Platform::get(&plattforms);
		//query plattforms

		std::vector<cl::Device> devices;
		plattforms[0].getDevices(CL_DEVICE_TYPE_GPU, &devices);
		//query gpu-devices

		cl::Context context(devices);
		// create a context

		cl::CommandQueue queue = cl::CommandQueue(context, devices[0]);
		//create a command queue
		cl::Buffer parameterBuffer = cl::Buffer(context, CL_MEM_READ_ONLY,
				sizeof(kernelParameters));
		cl::Buffer bufferPositions = cl::Buffer(context, CL_MEM_READ_WRITE,
				datasize); //Partikel Posotionen
		cl::Buffer bufferOldPositions = cl::Buffer(context, CL_MEM_READ_WRITE,
				datasize); //Partikel Positionen beim
		//bau der letzen Verlet liste
		cl::Buffer bufferVelocities = cl::Buffer(context, CL_MEM_READ_WRITE,
				datasize); //Geschwinfigkeiten
		cl::Buffer bufferAccelarations = cl::Buffer(context, CL_MEM_READ_WRITE,
				datasize); // Beschleunigungen
		//create buffers
		cl::Buffer bufferVerletList = cl::Buffer(context, CL_MEM_READ_WRITE,
				verletlistSize); // Verliste
		cl::Buffer bufferVerletNeedsUpdate = cl::Buffer(context,
		CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, verletNeedsUpdateSize,verletNeedsUpdate);

		cl::Buffer bufferCells = cl::Buffer(context, CL_MEM_READ_WRITE,
				cellBufferSize);

		queue.enqueueWriteBuffer(parameterBuffer,CL_TRUE,0,sizeof(kernelParameters),
				&kernelParams);

		queue.enqueueWriteBuffer(bufferPositions, CL_TRUE, 0, datasize,
				positions);
		queue.enqueueWriteBuffer(bufferOldPositions, CL_TRUE, 0, datasize,
				positions);
		queue.enqueueWriteBuffer(bufferVelocities, CL_TRUE, 0, datasize,
				velocities);
		queue.enqueueWriteBuffer(bufferAccelarations, CL_TRUE, 0, datasize,
				accelerations);
		//queue.enqueueWriteBuffer(bufferVerletNeedsUpdate, CL_TRUE, 0,
		//		verletNeedsUpdateSize, verletNeedsUpdate);
		//send buffers to the device

		cl::Program program = loadCLSource("demKernels.cl", context);

		try {
			program.build(devices);
		} catch (cl::Error &error) {
			std::cerr << error.what() << "(" << error.err() << ")" << std::endl;
			std::cerr << "Build Log:\t "
					<< program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[0])
					<< std::endl;
			exit(-1);
		}
		std::cout << "Build Log:\t "
				<< program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[0])
				<< std::endl;

		cl::Kernel verletStep1Kernel(program, "verletStep1");

		verletStep1Kernel.setArg(0, bufferPositions);
		verletStep1Kernel.setArg(1, bufferVelocities);
		verletStep1Kernel.setArg(2, bufferAccelarations);
		verletStep1Kernel.setArg(3, bufferOldPositions);
		verletStep1Kernel.setArg(4, parameterBuffer);
		verletStep1Kernel.setArg(5, bufferVerletNeedsUpdate);

		cl::Kernel ereaseCellKernel(program, "ereaseCells");

		ereaseCellKernel.setArg(0, bufferCells);
		ereaseCellKernel.setArg(1, bufferVerletNeedsUpdate);
		ereaseCellKernel.setArg(2, parameterBuffer);

		cl::Kernel buildVerlet1(program, "buildVerlet1");

		buildVerlet1.setArg(0, bufferPositions);
		buildVerlet1.setArg(1, bufferOldPositions);
		buildVerlet1.setArg(2, bufferCells);
		buildVerlet1.setArg(3, bufferVerletNeedsUpdate);
		buildVerlet1.setArg(4, parameterBuffer);


		cl::Kernel buildVerlet2(program, "buildVerlet2");

		buildVerlet2.setArg(0, bufferPositions);
		buildVerlet2.setArg(1, bufferCells);
		buildVerlet2.setArg(2, bufferVerletList);
		buildVerlet2.setArg(3, bufferVerletNeedsUpdate);
		buildVerlet2.setArg(4, parameterBuffer);


		cl::Kernel verletStep2(program, "verletStep2");

		verletStep2.setArg(0, bufferPositions);
		verletStep2.setArg(1, bufferVelocities);
		verletStep2.setArg(2, bufferAccelarations);
		verletStep2.setArg(3, bufferVerletList);
		verletStep2.setArg(4, bufferVerletNeedsUpdate);
		verletStep2.setArg(5, parameterBuffer);


		cl::NDRange globalp(particleNumber);
		cl::NDRange localp=cl::NullRange;

		cl::NDRange globalc(kernelParams.cellnxny.s[0], kernelParams.cellnxny.s[1]);
		cl::NDRange localc=cl::NullRange;

		int snapnumber = 0;
		std::cout << "build inital verlet" << std::endl;

		//initiale verletliste bauen
		queue.enqueueNDRangeKernel(ereaseCellKernel, cl::NullRange, globalc,
							localc);
		queue.enqueueNDRangeKernel(buildVerlet1, cl::NullRange, globalp,
							localp);
		queue.enqueueNDRangeKernel(buildVerlet2, cl::NullRange, globalp,
							localp);


		std::cout << "begin main loop" << std::endl;

		// Maion loop
		int verletupdates=0;
		for (int i = 0; i < maxTimesteps; ++i) {
			queue.enqueueNDRangeKernel(verletStep1Kernel, cl::NullRange,
					globalp, localp);
			queue.enqueueMapBuffer(bufferVerletNeedsUpdate,CL_TRUE,CL_MAP_READ,0,verletNeedsUpdateSize);
			if (*verletNeedsUpdate) {
				++verletupdates;
				queue.enqueueNDRangeKernel(ereaseCellKernel, cl::NullRange,
						globalc, localc);
				queue.enqueueNDRangeKernel(buildVerlet1, cl::NullRange, globalp,
						localp);
				queue.enqueueNDRangeKernel(buildVerlet2, cl::NullRange, globalp,
						localp);
			}
			queue.enqueueNDRangeKernel(verletStep2, cl::NullRange, globalp,
					localp);
			if (i % snapshot == 0) {
				std::cout << "Verletlistupdates: " << verletupdates << std::endl;
				verletupdates=0;
				if (i > 0) {
					visoutput.updateimage();
					visoutputcenter.updateimage();
					std::cout << "time " << kernelParams.timestep * i << "snapshot "
							<< *verletNeedsUpdate << std::endl;
					std::cout << "kinetic energy:" << kineticEnergy(velocities,
							kernelParams.Nparticle,kernelParams.mass) << "\n";
					cl_float2 p=momentum(velocities,kernelParams.Nparticle,kernelParams.mass);
					std::cout << "momentum: " << p.s[0] << " " << p.s[1] << std::endl;
 					char number[64]; // string which will contain the number
					sprintf(number, "./data/snap%04d.png", snapnumber++);
					//visoutput.snapshot(number);
					sprintf(number, "./data/center%04d.png", snapnumber++);
					//visoutputcenter.snapshot(number);
				//	saveSnapShot(number, positions, velocities, accelerations,
				//			particleNumber);
				} // Write Data from last Snapshot to HD
				  // then read the momentary data
				queue.enqueueReadBuffer(bufferPositions, CL_TRUE, 0, datasize,
						positions);

				queue.enqueueReadBuffer(bufferVelocities, CL_TRUE, 0, datasize,
						velocities);
				queue.enqueueReadBuffer(bufferAccelarations, CL_TRUE, 0,
						datasize, accelerations);
				queue.enqueueReadBuffer(bufferVerletNeedsUpdate, CL_TRUE, 0,
						verletNeedsUpdateSize, verletNeedsUpdate);
			}
		}
	} catch (cl::Error &error) {
		std::cerr << error.what() << "(" << error.err() << ")" << std::endl;
	}
	/////////////// Release CL Resources
	/////////////// free host ram
	visoutput.close();
	visoutputcenter.close();
	delete positions;
	delete velocities;
	delete accelerations;
	delete verletlist;
}
