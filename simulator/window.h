#ifndef WINDOW
#define WINDOW

#include "main.h"

struct Window{
	
	// SDL window
	SDL_Window *window = NULL;
	SDL_GLContext context = NULL;
	glm::ivec2 size;
	GLfloat ratio;
	Input input;
	
	// creation and destruction
	int setup();
	void cleanup();
	
	// update operations
	int getInput();
	void updateAspectRatio();
	void resize();
	
	// visualisation
	void clear();
	void update();
};

#endif