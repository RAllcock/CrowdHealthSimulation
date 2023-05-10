#ifndef SIM
#define SIM

#include "main.h"
#include "particles.h"
#include "sph.h"
#include "path.h"
#include "contact.h"
#include "obstacles.h"
#include "generators.h"
#include "input.h"
#include "debug.h"

struct Simulation{
	
	// general
	double timeNextFineUpdate, timeNextCoarseUpdate, timeCurrent, timeLastUpdate, timeStart;
	float fineTimeStep;
	float coarseTimeStep;
	
	// input
	glm::vec2 size, pscale;
	glm::vec2 mapSize;
	SimInput input;
	
	// modules
	Particles p;
	Hydrodynamics h;
	Pathing g;
	Contact c;
	Obstacles o;
	Source src;
	Sink snk;
	Debug debug;
	
	int setup(glm::ivec2 bounds, char *outFile);
	int update();
	void display();
	void cleanup();
};

#endif