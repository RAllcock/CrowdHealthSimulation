#ifndef PARTICLES
#define PARTICLES

#include "main.h"

struct Particles{
	
	// basic properties
	glm::vec2 *position, *velocity, *acceleration;
	
	// extra properties
	glm::vec2 *acc1, *acc2, *acc3, *goalPos, defaultGoalPos;
	float *disc, *mass, *density;
	
	// simulation properties
	int n, nmax;
	glm::vec2 *mapScale, *particleScale;
	
	// input
	float maxSpeed, maxAcceleration;
	
	// shader
	int datan, datanmax;
	GLfloat *data;
	GLuint vbo, vao, program;
	glm::mat4 scale, projection;
	
	// input
	int setup(int pn, int pnmax, glm::vec2 pb, glm::ivec2 pbs, glm::vec2 rg, float maxSpd, float maxAcc, float sdist);
	void setScaling(glm::vec2 *sm, glm::vec2 *sp);
	
	// visualisation
	void set(glm::vec2 p, glm::ivec2 s, glm::vec2 rg, float spawnDist);
	void updateData();
	
	// shader
	void getShaderSources(const char **vShader, const char **gShader, const char **fShader);
	void setShader();
	void display();
	
	// simulation
	void move(float timeStep);
	
	// generators
	int add(glm::vec2 p);
	int remove(int index);
	
	void cleanup();
};

#endif