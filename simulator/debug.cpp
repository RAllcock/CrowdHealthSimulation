#include "debug.h"

int Debug::setup(int tAvg, bool vuse, double vs, double vp, float *v, float vf, int pn, char *outFile){
	
	// input
	timespanAverageValue = tAvg;
	outputFile = outFile;
	
	// simulation
	particlesLeft = pn;
	
	// initial values
	t = 0;
	pdensityAverage = pvelocityAverage = pmaxVelocity = 0;
	cmaxPenetration = 0;
	cpressure = 0;
	averagesDisplayed = false;
	
	// variable test
	vusage = vuse;
	vstart = vs;
	vperiod = vp;
	vchange = v;
	vinitial = *v;
	vfinal = vf;
	
	// output
	fp = fopen(outputFile, "a");
	fputs("rhop\tvp\tbwall\tac\tvpmax\tnp\tt\n", fp);
	
	return 0;
}

int Debug::checkTermination(float srcDelay){
	if(pmaxVelocity < .001f || (srcDelay < 0 && particlesLeft == 0))
		return -1;
	return 0;
}

void Debug::updateVariable(double curTime){
	if(!vusage) return;
	
	// initial value
	if(curTime < vstart) *vchange = vinitial;
	
	// final value
	else if(vstart + vperiod < curTime) *vchange = vfinal;
	
	// transitioning value
	else{
		float progress = (curTime - vstart) / vperiod;
		*vchange = vinitial + (vfinal - vinitial) * progress;
	}
}

int Debug::updateAverages(glm::vec2 *rp, glm::vec2 *vp, float *pdensity, glm::vec2 *bpos, glm::vec2 *bsize, int pn, int bn, float srcDelay, double tTotal){
	
	// current averages
	float prhoAvg = 0, pvAvg = 0;
	for(int i = 0; i < pn; i++){
		prhoAvg += pdensity[i];
		pvAvg += length(vp[i]);
	}
	prhoAvg /= pn;
	pvAvg /= pn;
	
	// epoch averages
	pdensityAverage += prhoAvg;
	pvelocityAverage += pvAvg;
	
	// maximum velocity across all particles
	getMaximumParticleVelocity(vp, pn);
	
	// display epoch statistics
	t++;
	if(t >= timespanAverageValue){
		pdensityAverage /= t;
		pvelocityAverage /= t;
		cpressure /= t;
		
		// remaining particles
		particlesLeft = pn;
		
		// display
		printAverages(tTotal);
		
		// check for simulation termination
		if(checkTermination(srcDelay) == -1)
			return -1;
		
		pdensityAverage = 0;
		pvelocityAverage = 0;
		pmaxVelocity = 0;
		cpressure = 0;
		cmaxPenetration = 0;
		t = 0;
	}
	return 0;
}

void Debug::updateBoundaryPenetration(float p){
	
	// keep largest historical value
	if(p > cmaxPenetration)
		cmaxPenetration = p;
}

void Debug::updateContactPressure(double value){
	cpressure += value;
}

void Debug::printAverages(double tTotal){
	
	// reset display
	if(averagesDisplayed)
		printf("\r                                                                                                                                                                        \r");
	
	// display values
	printf("avg. particle density:%.3f\tavg. particle velocity:%.3f\tmax. obs. penetration:%.3f\tavg. con. acceleration:%.3f\tmax. velocity:%.3f\tparticles left:%i\tsim. time elapsed:%.3f", 
		pdensityAverage, pvelocityAverage, 
		cmaxPenetration, 
		(float)cpressure, 
		pmaxVelocity, 
		particlesLeft, 
		(float)tTotal);
	averagesDisplayed = true;
	
	// output values
	fprintf(fp, "%.3f \t%.3f \t%.3f \t%.3f \t%.3f \t%i \t%.3f\n", 
		pdensityAverage, pvelocityAverage, 
		cmaxPenetration, 
		(float)cpressure, 
		pmaxVelocity, 
		particlesLeft, 
		(float)tTotal);
}

void Debug::printOverall(glm::vec2 *rp, glm::vec2 *bpos, glm::vec2 *bsize, int pn, int bn, double tTotal){
	
	// final simulation time
	totalTime = tTotal;
	
	// display
	printf("\ntotal running time:%.3f\n", totalTime);
}

void Debug::getMaximumParticleVelocity(glm::vec2 *v, int n){
	for(int i = 0; i < n; i++)
		if(length(v[i]) > pmaxVelocity) pmaxVelocity = length(v[i]);
}

void Debug::cleanup(){
	fclose(fp);
}