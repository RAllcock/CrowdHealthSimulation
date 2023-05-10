#include "main.h"
#include "window.h"
#include "simulation.h"

int main(int argc, char *argv[]){
	
	// arguments
	char *outputFile = NULL;
	if(argc == 2)
		outputFile = argv[1];
	
	// window
	Window window;
	if(window.setup() == -1) return -1;
	
	// simulation
	Simulation simulation;
	if(simulation.setup(window.size, outputFile) == -1) return -1;
	
	// loop
	bool running = true;
	while(running){
		
		// input
		switch(window.getInput()){
			case INPUT_QUIT:
				running = false;
				break;
			default:
				break;
		}
		
		// simulation
		if(simulation.update() == -1)
			running = false;
		
		// display
		window.clear();
		simulation.display();
		window.update();
	}
	
	// cleanup
	simulation.cleanup();
	window.cleanup();
	return 0;
}