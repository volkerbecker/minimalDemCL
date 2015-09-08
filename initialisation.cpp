#include "initialisation.h"
#include <CL/cl.hpp>
#include <cmath>
#include <iostream>
#include <stdlib.h>

//Geschwindikeiten und Beschleunigung initialisieren
void initialVelAndAcc(cl_float2 &velocity, cl_float2 &acceleretion,
		const float & maxvelocity) {
	velocity.s[0] = (float) 2.0*((drand48()-0.5) * maxvelocity);
	velocity.s[1] = (float) 2.0*((drand48()-0.5) * maxvelocity);
	//velocity.s[1] = velocity.s[0];
	acceleretion.s[0] = 0;
	acceleretion.s[1] = 0;
}

void initializeParticles(int initialOrder, cl_float2 *positions,
		cl_float2 *velocities, cl_float2 *accelerations, const cl_float2 &LXLY,
		const cl_float radius, const cl_int & N, const int &maxvel) {
	cl_float radiussq = radius * radius;
	srand48(5); // to make the initialization reproducible
	switch (initialOrder) {
	case RANDOM:
		bool positionvalid;
		for (int i = 0; i < N; i++) {
			do {
				positionvalid = true;
				positions[i].s[0] = (float) rand() / (float) RAND_MAX
						* LXLY.s[0];
				positions[i].s[1] = (float) rand() / (float) RAND_MAX
						* LXLY.s[1]; //choose a random position

				for (int j = 0; j < i; ++j) {
					if (distancesq(positions[i], positions[j], LXLY)
							<= 4 * radiussq) {
						positionvalid = false;
						break;
					}
				} // check whether or not position is valid
			} while (!positionvalid);
			initialVelAndAcc(velocities[i], accelerations[i], 10);
		}
		break;
	case REGULARGRID: {
		float lx = LXLY.s[0] - 4 * radius;
		float ly = LXLY.s[1] - 4 * radius;
		int nnx = round(sqrt(lx / ly * N) + 0.499999); //nächstgroeßere ganze Zahl, teilchen je zeile
		int nny = round(sqrt(ly / lx * N) + 0.499999); //nächstgrößere ganzer Zahl, teilchen je spalte
		float dx0 = lx / (float) nnx;
		float dy0 = ly / (float) nny;
		if (dx0 < 2 * radius || dy0 < 2 * radius) {
			std::cerr
					<< "Error, not enough room to place all particles on a grid without overlap"
					<< std::endl;
			exit(0);
		}
		float downleftx = 4 * radius;
		float downlefty = 4 * radius;
		int j = 0;
		for (int i = 0; i < N; ++i) {
			if ((i % nnx == 0) && (i != 0)) {
							++j;
						}
			positions[i].s[0] = downleftx + (i % nnx) * dx0;
			positions[i].s[1] = downlefty + j * dy0;
			if (positions[i].s[0] >= LXLY.s[0]
					|| positions[i].s[1] >= LXLY.s[1]) {
				std::cerr << "intialisation failed, particle " << i
						<< " out of box" << positions[i].s[0] << " "
						<< positions[i].s[1] << std::endl;
			}
			initialVelAndAcc(velocities[i], accelerations[i], maxvel);
		}
	}
	}
}
