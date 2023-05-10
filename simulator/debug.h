#ifndef DEBUG
#define DEBUG

#include "main.h"

struct Debug{
	
	// parameters
	int timespanAverageValue;
	char *outputFile; FILE *fp;
	
	// tracked averages
	bool averagesDisplayed;
	int t;
	float pdensityAverage, pvelocityAverage;
	double cpressure;
	
	// tracked properties
	float totalTime;
	float cmaxPenetration;
	float pmaxVelocity;
	int particlesLeft;
	
	// variable parameter
	bool vusage;
	double vstart, vperiod;
	float *vchange;
	float vinitial, vfinal;
	
	int setup(int tAvg, bool vuse, double vs, double vp, float *v, float vf, int pn, char *outFile);
	int checkTermination(float srcDelay);
	
	// update
	void updateVariable(double curTime);
	int updateAverages(glm::vec2 *rp, glm::vec2 *vp, float *pdensity, glm::vec2 *bpos, glm::vec2 *bsize, int pn, int bn, float srcDelay, double tTotal);
	void updateBoundaryPenetration(float p);
	void updateContactPressure(double value);
	
	// display
	void printAverages(double tTotal);
	void printOverall(glm::vec2 *rp, glm::vec2 *bpos, glm::vec2 *bsize, int pn, int bn, double tTotal);
	
	// calculations
	void getMaximumParticleVelocity(glm::vec2 *v, int n);
	
	void cleanup();
};

#endif