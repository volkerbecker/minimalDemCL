/*
 * clhelpers.h
 *
 *  Created on: 04.08.2015
 *      Author: sax
 */
// iclude helping fuctions for intialising cl and compiling cl files

#ifndef CLHELPERS_H_
#define CLHELPERS_H_

#include <string>
#include <CL/cl.hpp>

cl::Program loadCLSource(const char *, const cl::Context &);

#endif /* CLHELPERS_H_ */
