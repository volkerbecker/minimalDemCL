/*
 * clhelpers.cpp

 *
 *  Created on: 04.08.2015
 *      Author: sax
 */
#include <iostream>
#include "clhelpers.h"
#include <iostream>
#include <fstream>
#include <string>

cl::Program loadCLSource(const char *filename, const cl::Context &context) {
	try {
		std::ifstream sourceFile(filename); //open file
		std::string sourceCode((std::istreambuf_iterator<char>(sourceFile)),
				std::istreambuf_iterator<char>()); //copy file to c_string

		//size_t ewa_programstart=sourceCode.find("//ECLIPSE_WA_START");
		//sourceCode=sourceCode.substr(ewa_programstart);
		//neglet prepocesser code included to avoid eclipse parsing errors
		cl::Program::Sources source(1,
				std::make_pair(sourceCode.c_str(), sourceCode.length() + 1));
		cl::Program program = cl::Program(context, source);
		return program;
	} catch (std::ifstream::failure &e) {
		std::cerr << "Exception opening/reading/closing file\n";
		exit(0);
	} catch (cl::Error &error) {
		std::cerr << "CL Error occured \n";
		exit(0);
	}

}

