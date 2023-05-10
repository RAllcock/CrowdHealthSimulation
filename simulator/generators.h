#ifndef GENERATORS
#define GENERATORS

#include "main.h"

struct Source{
	
	// basic properties
	int counter;
	
	// input
	glm::vec2 pos, size;
	int delay;
	
	int setup(glm::vec2 p, glm::vec2 s, int d){
		pos = p;
		size = s;
		delay = d;
		counter = delay;
		return 0;
	}
	
	glm::vec2 update(){
		
		// count down to next generation
		if(delay == -1) return glm::vec2(-1);
		else if(counter > 0){
			counter--;
			return glm::vec2(-1);
		}
		
		// generate new particle position in source area
		float x = fmod((float)rand() / 100.f, size.x);
		float y = fmod((float)rand() / 100.f, size.y);
		counter = delay;
		return glm::vec2(x, y);
	}
	
	void cleanup(){}
};

struct Sink{
	
	// input
	glm::vec2 pos;
	float radius;
	
	int setup(glm::vec2 p, float r){
		pos = p;
		radius = r;
		return 0;
	}
	
	int update(glm::vec2 *p, int n){
		
		// check if particles within sink area
		for(int i = 0; i < n; i++){
			glm::vec2 distance = p[i] - pos;
			if(length(distance) < radius)
				return i;
		}
		return -1;
	}
	
	void cleanup(){}
};

#endif
