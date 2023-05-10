#ifndef MAIN
#define MAIN

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define GLEW_STATIC
#include "GL/glew.h"
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/string_cast.hpp>

#include "SDL2/SDL.h"
#include "SDL2/SDL_opengl.h"

// window constants
#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 650
#define FRAME_RATE 144.f
#define FRAME_PERIOD (1000.f / FRAME_RATE)

// input constants
#define INPUT_QUIT -1
#define INPUT_RESIZE 1

// maths constants
#define PI 3.1415

// event input
struct Input{
	int get(){
		SDL_Event event;
		while(SDL_PollEvent(&event)){
			switch(event.type){
				case SDL_QUIT:
					return INPUT_QUIT;
					break;
				case SDL_WINDOWEVENT:
					return INPUT_RESIZE;
					break;
				default:
					break;
			}
		}
		return 0;
	}
};

#endif