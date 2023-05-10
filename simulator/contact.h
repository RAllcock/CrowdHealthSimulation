#ifndef CONTACT
#define CONTACT

#include "main.h"
#include "debug.h"

struct Contact{
	
	// simulation properties
	glm::vec2 *particlePos, *particleVel;
	float *disc, *mass;
	glm::vec2 *boxPos, *boxSize;
	int boxes;
	
	// constants
	glm::vec2 *midPoint;
	
	// input
	float pforceConst, oforceConst;
	
	// debug
	Debug *debug;
	
	int setup(glm::vec2 *rp, glm::vec2 *vp, float *dp, float *mp, glm::vec2 *rb, glm::vec2 *sb, int bn, float pK, float oK, Debug *deb){
		
		// particles
		particlePos = rp;
		particleVel = vp;
		disc = dp;
		mass = mp;
		
		// memory allocation
		if((midPoint = (glm::vec2*)malloc(sizeof(glm::vec2) * bn)) == NULL){
			printf("Contact allocation failed\n");
			return -1;
		}
		
		// boxes
		boxPos = rb;
		boxSize = sb;
		boxes = bn;
		for(int i = 0; i < boxes; i++)
			midPoint[i] = boxPos[i] + .5f * boxSize[i];
		
		// input
		pforceConst = pK;
		oforceConst = oK;
		
		debug = deb;
		
		return 0;
	}
	
	void update(glm::vec2 *a, int n){
		for(int i = 0; i < n; i++){
			double pressure = 0;
			a[i] = glm::vec2(0);
			for(int j = 0; j < n; j++){
				if(i == j) continue;
				
				// basic contact force
				glm::vec2 distance = particlePos[i] - particlePos[j];
				if(length(distance) == 0) continue;
				float contact = disc[i] + disc[j] - length(distance);
				a[i] += pforceConst * (contact < 0 ? 0.f : contact) * normalize(distance) / mass[i];
				pressure += pforceConst * (contact < 0 ? 0 : contact) / mass[i];
			}
			
			// soft wall collisions
			for(int j = 0; j < boxes; j++){
				glm::vec2 la[4], lb[4], distance[4];
				la[0] = boxPos[j]; lb[0] = la[0] + glm::vec2(boxSize[j].x, 0);
				la[1] = lb[0]; lb[1] = lb[0] + glm::vec2(0, boxSize[j].y);
				la[2] = lb[1]; lb[2] = lb[1] + glm::vec2(-boxSize[j].x, 0);
				la[3] = lb[2]; lb[3] = la[0];
				float contact[4];
				bool adjacentWall[4], insideRange[4], done = true;
				
				// distances to walls
				for(int k = 0; k < 4; k++){
					glm::vec2 near = nearestPointOnLine(particlePos[i], la[k], lb[k]);
					adjacentWall[k] = 
						!((near.x == la[k].x && near.y == la[k].y) || 
						(near.x == lb[k].x && near.y == lb[k].y));
					distance[k] = particlePos[i] - near;
					contact[k] = disc[i] - length(distance[k]);
					insideRange[k] = contact[k] > 0;
					done = done && !insideRange[k];
					
					// penetration into wall
					if(length(near - midPoint[j]) > length(particlePos[i] - midPoint[j])){
						distance[k] *= -1.f;
						contact[k] = 2.f * disc[i] - contact[k];
					}
				}
				if(done) continue;
				
				// adjacent wall
				bool outsideWall = false;
				for(int k = 0; k < 4; k++){
					if(insideRange[k] && contact[k] < disc[i]){
						outsideWall = true;
						if(adjacentWall[k]){
							a[i] += oforceConst * contact[k] * normalize(distance[k]) / mass[i];
							pressure += oforceConst * contact[k] / mass[i];
							debug->updateBoundaryPenetration(contact[k]);
							done = true;
							break;
						}
					}
				}
				if(done) continue;
				
				// non-adjacent wall
				if(outsideWall){
					for(int k = 0; k < 4; k++){
						if(insideRange[k] && contact[k] < disc[i]){
							a[i] += oforceConst * contact[k] * normalize(distance[k]) / mass[i];
							pressure += oforceConst * contact[k] / mass[i];
							debug->updateBoundaryPenetration(contact[k]);
							done = true;
							break;
						}
					}
				}
				if(done) continue;
				
				// closest entered wall
				float minContact = contact[0];
				int index = 0;
				for(int k = 1; k < 4; k++){
					if(insideRange[k] && minContact > contact[k]){
						minContact = contact[k];
						index = k;
					}
				}
				a[i] += oforceConst * contact[index] * normalize(distance[index]) / mass[i];
				pressure += oforceConst * contact[index] / mass[i];
				debug->updateBoundaryPenetration(contact[index]);
			}
			debug->updateContactPressure(pressure / (double)n);
		}
	}
	
	glm::vec2 nearestPointOnLine(glm::vec2 r, glm::vec2 a, glm::vec2 b){
		if(a == b) return a;
		glm::vec2 l = a - b;
		float k = dot((r-b), l)/(l.x*l.x + l.y*l.y);
		k = (k < 0 ? 0 : (k > 1 ? 1 : k));
		return (a*k) + (b*(1.f-k));
	}
	
	void cleanup(){}
};

#endif