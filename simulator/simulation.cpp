#include "simulation.h"

int Simulation::setup(glm::ivec2 bounds, char *outFile){
	
	// input
	input.setupSimulation(bounds, &mapSize, &size, &pscale, &p, &o, &fineTimeStep, &coarseTimeStep);
	input.setupModules(&p, &h, &g, &c, &o, &src, &snk, &debug, outFile);
	
	// simulation time
	timeNextFineUpdate = timeNextCoarseUpdate = timeCurrent = timeLastUpdate = timeStart = (double)clock() / CLOCKS_PER_SEC;
	timeNextFineUpdate += (double)fineTimeStep;
	timeNextCoarseUpdate += (double)coarseTimeStep;
	
	return 0;
}

int Simulation::update(){
	
	// current time
	timeCurrent = (double)clock() / CLOCKS_PER_SEC;
	
	// update
	if(timeCurrent > timeNextFineUpdate){
		
		// coarse time update
		if(timeCurrent > timeNextCoarseUpdate){
			
			// generate particles
			int pnprev = p.n;
			if(p.add(src.update()) != pnprev) h.add(pnprev);
			
			// force particles
			h.updateDensity(p.n, fineTimeStep);
			h.updateAcceleration(p.acc1, p.n, fineTimeStep);
			g.update(p.acc2, p.n);
			c.update(p.acc3, p.n);
			
			// last update time
			timeLastUpdate = timeNextCoarseUpdate;
			
			// calculate next coarse epoch
			timeNextCoarseUpdate += (double)coarseTimeStep;
			
			// move particles
			p.move(fineTimeStep);
			
			// update statistics
			if(debug.updateAverages(p.position, p.velocity, p.density, o.boxPos, o.boxSize, p.n, o.boxes, src.delay, timeLastUpdate - timeStart) == -1)
				return -1;
			debug.updateVariable(timeLastUpdate - timeStart);
		}
		
		// fine time update
		else if(timeCurrent > timeNextFineUpdate){
			
			// force particles
			h.updateDensity(p.n, fineTimeStep);
			h.updateAcceleration(p.acc1, p.n, fineTimeStep);
			c.update(p.acc3, p.n);
			
			// last update time
			timeLastUpdate = timeNextFineUpdate;
			
			// move particles
			p.move(fineTimeStep);
		}
		
		// calculate next fine epoch
		timeNextFineUpdate += (double)fineTimeStep;
		
		// remove particles
		int rem;
		while((rem = snk.update(p.position, p.n)) > -1)
			h.remove(rem, p.remove(rem));
		
		// update visualisation
		p.updateData();
		o.updateObstacleData();
	}
	
	return 0;
}

void Simulation::display(){
	p.display();
	o.display();
}

void Simulation::cleanup(){
	
	// overall statistics
	debug.printOverall(p.position, o.boxPos, o.boxSize, p.n, o.boxes, timeLastUpdate - timeStart);
	
	p.cleanup();
	o.cleanup();
	h.cleanup();
	g.cleanup();
	c.cleanup();
	src.cleanup();
	snk.cleanup();
	debug.cleanup();
}