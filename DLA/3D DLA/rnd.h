#pragma once
#include <random>

// ... don't worry how this all works
// ... member functions that you may want to use:
//       random01() returns a random double between 0 and 1
//       randomInt(max) returns a random int between 0 and max-1 (inclusive)

class rnd {
private:
	// nuts and bolts.. should not need to touch this.
	std::default_random_engine generator;
	int genMax;
	std::uniform_int_distribution<int> *intmax;
	std::uniform_real_distribution<double> *real01;

public:
	// constructor
	rnd() {
		genMax = 0x7fffffff;
		//cout << "genMax is " << generator.max() << endl;
		intmax = new std::uniform_int_distribution<int>(0, genMax);
		real01 = new std::uniform_real_distribution<double>(0.0, 1.0);
	}
	// destructor
	~rnd() { delete intmax; delete real01; }

	// set the random seed
	void   setSeed(int seed) { generator.seed(seed); }
	// member functions for generating random double in [0,1] and random integer in [0,max-1]
	double random01() { return (*real01)(generator); }
	int    randomInt(int max) { return (*intmax)(generator) % max; }
};
