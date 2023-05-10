#ifndef PATH
#define PATH

#include "main.h"

struct Pathing{
	
	// simulation properties
	glm::vec2 *position, *velocity, *goalPos;
	float *mass;
	
	// input
	float preferredSpeed, relaxationTime, forceStrength;
	
	int setup(glm::vec2 *r, glm::vec2 *v, float *m, glm::vec2 *rg, float u, float tau, float Kg){
		position = r;
		velocity = v;
		mass = m;
		goalPos = rg;
		preferredSpeed = u;
		relaxationTime = tau;
		forceStrength = Kg;
		return 0;
	}
	
	void update(glm::vec2 *a, int n){
		
		// calculate goal forces for particles
		for(int i = 0; i < n; i++){
			a[i] = glm::vec2(0);
			glm::vec2 goalDistance = goalPos[i] - position[i];
			if(length(goalDistance) == 0) continue;
			glm::vec2 preferredVelocity = preferredSpeed * normalize(goalDistance);
			a[i] = forceStrength * (preferredVelocity - velocity[i]) / (relaxationTime * mass[i]);
		}
	}
	
	void cleanup(){}
};

#endif