#ifndef INPUT
#define INPUT

#include "main.h"
#include "simulation.h"
#include "particles.h"
#include "sph.h"
#include "path.h"
#include "contact.h"
#include "obstacles.h"
#include "generators.h"
#include "debug.h"

struct SimInput{
	
	// simulation
	unsigned int seed = time(NULL);
	int particles = 400;
	int maxParticles = 400;
	glm::vec2 mapSize = glm::vec2(22, 20);
	float fineTimeStep = .02f;
	float coarseTimeStep = .1f;
	
	// particles
	glm::vec2 pboxPos = glm::vec2(0, 0);
	glm::ivec2 pboxSize = glm::ivec2(20, 20);
	float pmaxSpeed = 1.8f;
	float pmaxAccel = 5;
	float pscaleFactor = .48f;
	float pdistance = 1.f;
	
	// sph
	float hsmoothingLength = 1;
	float hgasConstant = 200;
	float hrestDensityWindow = .1f;
	float hminRestDensity = 0;
	float hmaxRestDensity = 5;
	float hviscosity = 3;
	
	// pathing
	glm::vec3 goal = glm::vec3(22, 10, 0);
	float gforce = 1.f;
	float gpreferredSpeed = 1.4f;
	float grelaxationTime = .5f;
	
	// contact forces
	float cpforceConst = 50; // 50-500 for cf, 50 for cf+sph
	float coforceConst = 200; // 500 for cf, 200 for cf+sph
	
	// obstacles
	float odisc = .24f;
	float omass = 1;
	float oscaleFactor = .48f;
	int obstacleBoxes = 2;
	glm::vec2 opos[2] = {
		glm::vec2(20, 10.4), 
		glm::vec2(20, 9.6)};
	glm::vec2 osize[2] = {
		glm::vec2(1, 9.6), 
		glm::vec2(1, -9.6)};
	int oemptyBoxes = 0;
	
	// generators
	glm::vec2 srcpos = glm::vec2(0, 0);
	glm::vec2 srcsize = glm::vec2(1, mapSize.y);
	int srcdelay = -1;
	glm::vec2 snkpos = glm::vec2(22, 10);
	float snkradius = .5f;
	
	// debug
	int debugAverageTick = 10;
	const char *outputFile = "output.txt";
	bool vusage = false;
	double vstart = 40;
	double vperiod = 60;
	float *vchange; // set with modules
	
	int setupSimulation(glm::ivec2 bounds, glm::vec2 *sm, glm::vec2 *sw, glm::vec2 *pscale, Particles *p, Obstacles *o, float *fineT, float *coarseT){
		
		// window size
		*sw = glm::vec2((float)bounds.x, (float)bounds.y);
		
		// simulation map size
		*sm = mapSize;
		
		// particle scale
		if(sw->x / sw->y > mapSize.x / mapSize.y) *pscale = glm::vec2(sw->x/sw->y, 1);
		else *pscale = glm::vec2(1, sw->y/sw->x);
		p->setScaling(&mapSize, pscale);
		o->setScaling(&mapSize, pscale, oscaleFactor);
		
		// input
		*fineT = fineTimeStep;
		*coarseT = coarseTimeStep;
		
		return 0;
	}
	
	int setupModules(Particles *p, Hydrodynamics *h, Pathing *g, Contact *c, Obstacles *o, Source *src, Sink *snk, Debug *debug, char *outFile){
		
		// seed
		srand(seed);
		
		// particles
		if(p->setup(particles, maxParticles, pboxPos, pboxSize, goal, pmaxSpeed, pmaxAccel, pdistance) == -1) return -1;
		if(o->setup(opos, osize, odisc, omass, obstacleBoxes, oemptyBoxes) == -1) return -1;
		
		// forces
		if(h->setup(p->position, p->velocity, p->density, p->mass, p->nmax, 
			o->position, o->density, o->mass, o->n, 
			hsmoothingLength, hgasConstant, hviscosity, hrestDensityWindow, hminRestDensity, hmaxRestDensity) == -1) return -1;
		if(g->setup(p->position, p->velocity, p->mass, p->goalPos, gpreferredSpeed, grelaxationTime, gforce) == -1) return -1;
		if(c->setup(p->position, p->velocity, p->disc, p->mass, o->boxPos, o->boxSize, o->boxes, cpforceConst, coforceConst, debug) == -1) return -1;
		
		// generators
		if(src->setup(srcpos, srcsize, srcdelay) == -1) return -1;
		if(snk->setup(snkpos, snkradius) == -1) return -1;
		
		// extra
		if(debug->setup(debugAverageTick, vusage, vstart, vperiod, &g->forceStrength, 2.f, p->n, outFile == NULL ? (char*)outputFile : outFile) == -1) return -1;
		
		return 0;
	}
	
};

#endif