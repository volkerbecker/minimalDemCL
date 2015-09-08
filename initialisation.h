/*
 * initialisation.h
 *
 *  Created on: 06.08.2015
 *      Author: sax
 */

#ifndef INITIALISATION_H_
#define INITIALISATION_H_

#define RANDOM 1
#define REGULARGRID 2

#include <cmath>
#include <CL/cl.hpp>

void initialVelAndAcc(cl_float2 &velocity, cl_float2 &acceleretion,
		const float & maxvelocity);
//Geschwindigkeiten und Beschleunugung zufaellig initialisieren

void initializeParticles(int initialOrder, // Random oder Gitter
		cl_float2 *positions, // positionen
		cl_float2 *velocities, // geschwindigkeiten
		cl_float2 *accelerations, // beschleunigungen
		const cl_float2 &LXLY, //Boxgroesse
		const cl_float radius, //radius
		const cl_int & N, const int &);  //partikelanzahl

inline int sgn(const float &value) {
	return copysign(1, value);
}

inline float min(const float &a, const float&b) {
	return a <= b ? a : b;
}
inline float max(const float &a, const float&b) {
	return a >= b ? a : b;
}

inline void wrap(cl_float2 &position, cl_float2 LXLY) {
	position.s[0] = fmod(position.s[0] + LXLY.s[0], LXLY.s[0]);
	position.s[1] = fmod(position.s[0] + LXLY.s[1], LXLY.s[1]); // Periodische Randbedinungen
}

inline float wrappedDifferenz(const float & p1, const float & p2,
		const float & L) {

	int a = sgn((p1 - p2));
	float b = fabs(p1 - p2);
	if (L - b < b)
		return -1.0f * (float) a * (L - b);
	else
		return (p1 - p2);
} //todo kann optimiert werden

inline float distancesq(const cl_float2 & p1, const cl_float2 & p2,
		const cl_float2 & LXLY) {
	float dx = wrappedDifferenz(p1.s[0], p2.s[0], LXLY.s[0]);
	float dy = wrappedDifferenz(p1.s[1], p2.s[1], LXLY.s[1]);
	return dx * dx + dy * dy;
}

#endif /* INITIALISATION_H_ */
