#ifndef OBSTACLES
#define OBSTACLES

#include "main.h"

struct Obstacles{
	
	// basic properties
	glm::vec2 *boxPos, *boxSize;
	
	// particle properties
	glm::vec2 *position;
	float *density;
	
	// simulation properties
	int n, boxes, oboxes;
	glm::vec2 *mapScale, *obstacleScale;
	
	// input
	float disc, mass;
	
	// shader
	glm::mat4 projection;
	
	// obstacle shader
	int odatan;
	GLfloat *odata;
	GLuint ovbo, ovao, oprogram;
	glm::mat4 oscale;
	
	// box shader
	int bdatan;
	GLfloat *bdata;
	GLuint bvbo, bvao, bprogram;
	
	// input
	int setup(glm::vec2 *bpos, glm::vec2 *bsize, float d, float m, int ob, float eb);
	void setScaling(glm::vec2 *ms, glm::vec2 *os, float sf);
	
	// data
	void set();
	void setBoxData();
	void updateObstacleData();
	
	// visualisation
	void getObstacleShaderSources(const char **vShader, const char **gShader, const char **fShader);
	void setObstacleShader();
	void getBoxShaderSources(const char **vShader, const char **fShader);
	void setBoxShader();
	void display();
	
	void cleanup();
};

#endif