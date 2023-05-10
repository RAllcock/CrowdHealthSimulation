#include "window.h"

int Window::setup(){
	
	// SDL window
	size = glm::ivec2(WINDOW_WIDTH, WINDOW_HEIGHT);
	updateAspectRatio();
	if(SDL_Init(SDL_INIT_EVERYTHING) < 0){
		printf("SDL initialisation failed: %s\n", SDL_GetError());
		return -1;
	}
	if((window = SDL_CreateWindow("Project Basic Window", 10, 50, size.x, size.y, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE)) == NULL){
		printf("SDL window creation failed: %s\n", SDL_GetError());
		return -1;
	}
	
	// SDL OpenGL
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	if((context = SDL_GL_CreateContext(window)) == NULL){
		printf("GL context creation failed: %s\n", SDL_GetError());
		return -1;
	}
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	
	// OpenGL
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	
	// GLEW
	glewExperimental = GL_TRUE;
	GLenum glewStatus = glewInit();
	if(glewStatus != GLEW_OK){
		printf("GLEW initialisation failed: %s\n", glewGetErrorString(glewStatus));
		return -1;
	}
	
	// background
	glClearColor(.2f, .2f, .2f, 1.f);
	
	return 0;
}

// update window
int Window::getInput(){
	int out = input.get();
	switch(out){
		case INPUT_RESIZE:
			resize();
			break;
		default:
			break;
	}
	return out;
}

void Window::updateAspectRatio(){
	ratio = (float)size.x / (float)size.y;
}

// resize window
void Window::resize(){
	SDL_GetWindowSize(window, &size.x, &size.y);
	glViewport(0, 0, size.x, size.y);
	updateAspectRatio();
}

// clear buffer
void Window::clear(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

// update visualisation
void Window::update(){
	SDL_GL_SwapWindow(window);
	SDL_Delay(FRAME_PERIOD);
}

// destroy window
void Window::cleanup(){
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();
}