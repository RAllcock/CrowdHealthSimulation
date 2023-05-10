#ifndef SPH
#define SPH

#include "main.h"

struct Hydrodynamics{
	
	// simulation properties
	int obstacles;
	glm::vec2 *particlePos, *particleVel, *obstaclePos;
	float *particleMass, obstacleMass, *particleRho, *particleRhoAvg, *obstacleRhoAvg;
	
	// temporary properties
	float *particleP, *particleRho0, *obstacleRho0;
	float *obstacleRho, *obstacleP;
	
	// input
	float smoothingLength;
	float gasConstant, viscosity;
	float restDensityWindow, minRestDensity, maxRestDensity;
	
	// constants
	float smoothDensityConst, smoothPressureConst, smoothViscosityConst;
	
	// input
	int setup(glm::vec2 *rp, glm::vec2 *vp, float *rhop, float *mp, int pnmax, 
		glm::vec2 *ro, float *rhoo, float mo, int on, 
		float h, float k, float mu, float rhoWin, float rhoMin, float rhoMax);
	
	// generators
	void add(int index);
	void remove(int index, int n);
	
	// update
	void updateDensity(int n, float timeStep);
	void updateAcceleration(glm::vec2 *ap, int n, float timeStep);
	
	// smoothing kernels
	float kernel_density(glm::vec2 r);
	glm::vec2 kernel_pressure(glm::vec2 r);
	float kernel_viscosity(glm::vec2 r);
	
	void cleanup();
};

#endif