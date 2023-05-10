#include "sph.h"

int Hydrodynamics::setup(glm::vec2 *rp, glm::vec2 *vp, float *rhop, float *mp, int pnmax, 
		glm::vec2 *ro, float *rhoo, float mo, int on, 
		float h, float k, float mu, float rhoWin, float rhoMin, float rhoMax){
	
	// simulation
	particlePos = rp;
	particleVel = vp;
	obstaclePos = ro;
	particleRho = rhop;
	obstacleRho = rhoo;
	particleMass = mp;
	obstacleMass = mo;
	obstacles = on;
	
	// input
	smoothingLength = h;
	gasConstant =  k;
	viscosity = mu;
	restDensityWindow = rhoWin;
	minRestDensity = rhoMin;
	maxRestDensity = rhoMax;
	
	// constants
	smoothDensityConst = 4.f / (PI * pow(smoothingLength, 8));
	smoothPressureConst = 15.f / (PI * pow(smoothingLength, 5));
	smoothViscosityConst = 360.f / (29.f * PI * pow(smoothingLength, 5));
	
	// memory allocation
	if(	(particleP = (float*)malloc(sizeof(float) * pnmax)) == NULL ||  
		(obstacleP = (float*)malloc(sizeof(float) * obstacles)) == NULL || 
		(particleRhoAvg = (float*)malloc(sizeof(float) * pnmax)) == NULL || 
		(obstacleRhoAvg = (float*)malloc(sizeof(float) * obstacles)) == NULL || 
		(particleRho0 = (float*)malloc(sizeof(float) * pnmax)) == NULL || 
		(obstacleRho0 = (float*)malloc(sizeof(float) * obstacles)) == NULL){
		
		printf("SPH allocation failed\n");
		return -1;
	}
	
	// particle initial values
	for(int i = 0; i < pnmax; i++) particleRhoAvg[i] = minRestDensity;
	for(int i = 0; i < obstacles; i++) obstacleRhoAvg[i] = minRestDensity;
	
	return 0;
}

// setup new particle
void Hydrodynamics::add(int index){
	particleRhoAvg[index] = minRestDensity;
}

// clear destroyed particle
void Hydrodynamics::remove(int index, int n){
	particleRhoAvg[index] = particleRhoAvg[n];
}

void Hydrodynamics::updateDensity(int n, float timeStep){
	
	// pedestrian particles
	for(int i = 0; i < n; i++){
		
		// density
		particleRho[i] = 0;
		for(int j = 0; j < n; j++){
			glm::vec2 dist = particlePos[i] - particlePos[j];
			if(length(dist) > smoothingLength) continue;
			particleRho[i] += particleMass[j] * kernel_density(dist);
		}
		for(int j = 0; j < obstacles; j++){
			glm::vec2 dist = particlePos[i] - obstaclePos[j];
			if(length(dist) > smoothingLength) continue;
			particleRho[i] += obstacleMass * kernel_density(dist);
		}
		
		// rest density
		particleRhoAvg[i] = (1.f - timeStep / restDensityWindow) * particleRhoAvg[i] + particleRho[i] * timeStep / restDensityWindow;
		particleRho0[i] = (particleRhoAvg[i] > maxRestDensity ? maxRestDensity : (particleRhoAvg[i] < minRestDensity ? minRestDensity : particleRhoAvg[i]));
		
		// pressure
		particleP[i] = gasConstant * (particleRho[i] - particleRho0[i]);
	}
	
	// obstacle particles
	for(int i = 0; i < obstacles; i++){
		
		// density
		obstacleRho[i] = 0;
		for(int j = 0; j < n; j++){
			glm::vec2 dist = obstaclePos[i] - particlePos[j];
			if(length(dist) > smoothingLength) continue;
			obstacleRho[i] += particleMass[j] * kernel_density(dist);
		}
		for(int j = 0; j < obstacles; j++){
			glm::vec2 dist = obstaclePos[i] - obstaclePos[j];
			if(length(dist) > smoothingLength) continue;
			obstacleRho[i] += obstacleMass * kernel_density(dist);
		}
		
		// rest density
		obstacleRhoAvg[i] = (1.f - timeStep / restDensityWindow) * obstacleRhoAvg[i] + obstacleRho[i] * timeStep / restDensityWindow;
		obstacleRho0[i] = (obstacleRhoAvg[i] > maxRestDensity ? maxRestDensity : (obstacleRhoAvg[i] < minRestDensity ? minRestDensity : obstacleRhoAvg[i]));
		
		// pressure
		obstacleP[i] = gasConstant * (obstacleRho[i] - obstacleRho0[i]);
	}
}

void Hydrodynamics::updateAcceleration(glm::vec2 *ap, int n, float timeStep){
	
	// acceleration terms for pedestrian particles
	for(int i = 0; i < n; i++){
		if(particleRho[i] == 0) continue;
		glm::vec2 pterm, vterm;
		pterm = vterm = glm::vec2(0);
		
		// pressure
		for(int j = 0; j < n; j++){
			glm::vec2 dist = particlePos[i] - particlePos[j];
			if(i == j || length(dist) > smoothingLength || particleP[j] < 0) continue;
			pterm += particleMass[j] * (particleP[i] + particleP[j]) * kernel_pressure(dist) / particleRho[j];
		}
		for(int j = 0; j < obstacles; j++){
			glm::vec2 dist = particlePos[i] - obstaclePos[j];
			if(length(dist) > smoothingLength || obstacleP[j] < 0) continue;
			pterm += obstacleMass * (particleP[i] + obstacleP[j]) * kernel_pressure(dist) / obstacleRho[j];
		}
		
		// viscosity
		for(int j = 0; j < n; j++){
			glm::vec2 dist = particlePos[i] - particlePos[j];
			if(i == j || length(dist) > smoothingLength) continue;
			vterm += particleMass[j] * (particleVel[j] - particleVel[i]) * kernel_viscosity(dist) / particleRho[j];
		}
		for(int j = 0; j < obstacles; j++){
			glm::vec2 dist = particlePos[i] - obstaclePos[j];
			if(length(dist) > smoothingLength) continue;
			vterm += obstacleMass * -particleVel[i] * kernel_viscosity(dist) / obstacleRho[j];
		}
		vterm *= viscosity;
		
		// acceleration
		ap[i] = (pterm + vterm) / particleRho[i];
	}
}

// smoothing kernels
float Hydrodynamics::kernel_density(glm::vec2 r){
	if(length(r) < smoothingLength) return smoothDensityConst * (float)pow(pow(smoothingLength, 2) - (float)pow(length(r), 2), 3);
	return 0;
}
glm::vec2 Hydrodynamics::kernel_pressure(glm::vec2 r){
	if(length(r) < smoothingLength && length(r) != 0) return smoothPressureConst * normalize(r) * (float)pow(smoothingLength - length(r), 2);
	return glm::vec3(0);
}
float Hydrodynamics::kernel_viscosity(glm::vec2 r){
	if(length(r) < smoothingLength) return smoothViscosityConst * (smoothingLength - length(r));
	return 0;
}

void Hydrodynamics::cleanup(){
	free(particleP);
	free(obstacleP);
	free(particleRhoAvg);
	free(obstacleRhoAvg);
	free(particleRho0);
	free(obstacleRho0);
}