//
//  DLASystem.cpp
//

#include "DLASystem.h"
#include <fstream> //work with csv
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <random>

// colors
namespace colours {
	GLfloat blue[] = { 0.1, 0.3, 0.9, 1.0 };   // blue
	GLfloat red[] = { 1.0, 0.2, 0.1, 0.2 };   // red
	GLfloat green[] = { 0.3, 0.6, 0.3, 1.0 };     // green
	GLfloat paleGrey[] = { 0.7, 0.7, 0.7, 1.0 };     // green
	GLfloat darkGrey[] = { 0.2, 0.2, 0.2, 1.0 };     // green
}


// this function gets called every step,
//   if there is an active particle then it gets moved,
//   if not then add a particle
int DLASystem::Update() {

	cout << "Update get's called" << endl;

	bool end = 0; //allows me to know when simulation stopped

	if (lastParticleIsActive == 1)
		moveLastParticle();
	else if (numParticles < endNum) {
		addParticleOnAddCircle();
		setParticleActive();
	}
	else if (numParticles == endNum)
	{
		end = 1;
	}
	if (lastParticleIsActive == 0 || slowNotFast == 1)
		glutPostRedisplay(); //Tell GLUT that the display has changed

	return end;
}



void DLASystem::clearParticles() {

	cout << "clearParticles get's called" << endl;
	// delete particles and the particle list
	for (int i = 0; i < numParticles; i++) {
		delete particleList[i];
	}
	particleList.clear();
	numParticles = 0;
}

// remove any existing particles and setup initial condition
void DLASystem::Reset() {

	cout << "Reset get's called" << endl;
	// stop running
	running = 0;

	clearParticles();

	lastParticleIsActive = 0;

	// set the grid to zero
	for (int i = 0; i < gridSize; i++) {
		for (int j = 0; j < gridSize; j++) {
			for (int k = 0; k < gridSize; k++)
				grid[i][j][k] = 0;
		}
	}

	// setup initial condition and parameters
	addCircle = 10;
	killCircle = 2.0*addCircle;
	clusterRadius = 0.0;
	// add a single particle at the origin
	double pos[] = { 0.0, 0.0, 0.0 };
	addParticle(pos);

	// set the view
	int InitialViewSize = 40;
	setViewSize(InitialViewSize);

}

// set the value of a grid cell for a particular position
// note the position has the initial particle at (0,0,0)
// but this corresponds to the middle of the grid array ie grid[ halfGrid ][ halfGrid ][ halfGrid ]
void DLASystem::setGrid(double pos[], int val) {
	cout << "setGrid get's called" << endl;
	int halfGrid = gridSize / 2;
	grid[(int)(pos[0] + halfGrid)][(int)(pos[1] + halfGrid)][(int)(pos[2] + halfGrid)] = val;
}

// read the grid cell for a given position
int DLASystem::readGrid(double pos[]) {
	cout << "readGrid get's called" << endl;
	int halfGrid = gridSize / 2;
	return grid[(int)(pos[0] + halfGrid)][(int)(pos[1] + halfGrid)][(int)(pos[2] + halfGrid)];
}

// check if the cluster is big enough and we should stop:
// to be safe, we need the killCircle to be at least 2 less than the edge of the grid
int DLASystem::checkStop() {
	cout << "checkStop get's called"  << endl;
	if (killCircle + 2 >= gridSize / 2) {
		pauseRunning();
		cout << "STOP" << endl;
		glutPostRedisplay(); // update display
		return 1;
	}
	else return 0;
}

// add a particle to the system at a specific position
void DLASystem::addParticle(double pos[]) {
	cout << "addParticle get's called" << endl;
	// create a new particle
	Particle * p = new Particle(pos);
	// push_back means "add this to the end of the list"
	particleList.push_back(p);
	numParticles++;

	// pos coordinates should be -gridSize/2 < x < gridSize/2
	setGrid(pos, 1);
}

// add a particle to the system at a random position on the addCircle
// if we hit an occupied site then we do nothing except print a message
// (this should never happen)
void DLASystem::addParticleOnAddCircle() {
	cout << "addParticleOnAddCircle get's called" << endl;
	double pos[3];
	double theta = random_max(0,1) * 2 * M_PI;
	double phi = random_max(0,1) * M_PI; // 1 is the max value for phi
	pos[0] = ceil(addCircle * cos(theta));
	pos[1] = ceil(addCircle * sin(theta));
	pos[2] = ceil(addCircle * sin(phi));
	if (readGrid(pos) == 0)
		addParticle(pos);
	else
		cout << "FAIL " << pos[0] << " " << pos[1] << endl;
}

// send back the position of a neighbour of a given grid cell
// NOTE: there is no check that the neighbour is inside the grid,
// this has to be done separately...
void DLASystem::setPosNeighbour(double setpos[], double pos[], int val) {
		
	int x = random_max(0,100);

	cout << "setPosNeighbour get's called:" << pos[0] << "," << pos[1] << "," << pos[2] << endl;
	
	// Gravity version
	/*
	switch (val) {
		case 0:
			if ((x < prob) && (pos[0] < 0)){
				setpos[0] = pos[0] + 1.0;
				setpos[1] = pos[1];
				setpos[2] = pos[2];
			}
			else if ((x < prob) && (pos[0] > 0)){
				setpos[0] = pos[0] - 1.0;
				setpos[1] = pos[1];
				setpos[2] = pos[2];
			}
			else { //no gravity effect -  we just randomly generate here 
				int randomValue = rand() % 2; // Generate a random value of either 0 or 1
				if (randomValue == 0) {
					setpos[0] = pos[0] - 1.0;
				} else {
					setpos[0] = pos[0] + 1.0;
				}
				setpos[1] = pos[1];
				setpos[2] = pos[2];
			}
			break;
		case 1:
			if ((x < prob) && (pos[1] < 0)){
				setpos[0] = pos[0];
				setpos[1] = pos[1] + 1.0;
				setpos[2] = pos[2];
			}
			else if ((x < prob) && (pos[1] > 0)){
				setpos[0] = pos[0];
				setpos[1] = pos[1] - 1.0;
				setpos[2] = pos[2];
			}
			else { //no gravity effect -  we just randomly generate here 
				int randomValue = rand() % 2; // Generate a random value of either 0 or 1
				if (randomValue == 0) {
					setpos[1] = pos[1] - 1.0;
				} else {
					setpos[1] = pos[1] + 1.0;
				}
				setpos[0] = pos[0];
				setpos[2] = pos[2];
			}
			break;
		case 2:
			if ((x < prob) && (pos[2] < 0)){
				setpos[0] = pos[0];
				setpos[1] = pos[1];
				setpos[2] = pos[2] + 1.0;
			}
			else if ((x < prob) && (pos[2] > 0)){
				setpos[0] = pos[0];
				setpos[1] = pos[1];
				setpos[2] = pos[2] - 1.0;
			}
			else {
				int randomValue = rand() % 2; // Generate a random value of either 0 or 1
				if (randomValue == 0) {
					setpos[2] = pos[2] - 1.0;
				} else {
					setpos[2] = pos[2] + 1.0;
				}
				setpos[0] = pos[0];
				setpos[1] = pos[1];
			}
			break;
		}
		*/

		//Probability, but no gravity version
		
			switch (val) {
				case 0:
					setpos[0] = pos[0] + 1.0;
					setpos[1] = pos[1];
					setpos[2] = pos[2];
					break;
				case 1:
					setpos[0] = pos[0] - 1.0;
					setpos[1] = pos[1];
					setpos[2] = pos[2];
					break;
				case 2:
					setpos[0] = pos[0];
					setpos[1] = pos[1] + 1.0;
					setpos[2] = pos[2];
					break;
				case 3:
					setpos[0] = pos[0];
					setpos[1] = pos[1] - 1.0;
					setpos[2] = pos[2];
					break;
				case 4:
					setpos[0] = pos[0];
					setpos[1] = pos[1];
					setpos[2] = pos[2] + 1.0;
					break;
				case 5:
					setpos[0] = pos[0];
					setpos[1] = pos[1];
					setpos[2] = pos[2] - 1.0;
					break;
				}

			cout << "Pos:" << pos[0] << "," << pos[1] << "," << pos[2] << endl;
			cout << "SetPos:" << pos[0] << "," << pos[1] << "," << pos[2] << endl;
		
	}

// if the view is smaller than the kill circle then increase the view area (zoom out)
void DLASystem::updateViewSize() {
	cout << "updateViewSize get's called" << endl;
	double mult = 1.2;
	if (viewSize < 2.0*killCircle) {
		setViewSize(viewSize * mult);
	}
}

// set the view to be the size of the add circle (ie zoom in on the cluster)
void DLASystem::viewAddCircle() {
	cout << "viewAddCircle get's called" << endl;
	setViewSize(2.0*addCircle);  // factor of 2 is to go from radius to diameter
}

// when we add a particle to the cluster, we should update the cluster radius
// and the sizes of the addCircle and the killCircle
void DLASystem::updateClusterRadius(double pos[]) {

	cout << "updateClusterRadius get's called" << endl;

	double rr = distanceFromOrigin(pos);
	if (rr > clusterRadius) {
		clusterRadius = rr;
		// this is how big addCircle is supposed to be:
		//   either 20% more than cluster radius, or at least 5 bigger.
		double check = clusterRadius * addRatio;
		if (check < clusterRadius + 5)
			check = clusterRadius + 5;
		// if it is smaller then update everything...
		if (addCircle < check) {
			addCircle = check;
			killCircle = killRatio * addCircle;
			updateViewSize();
		}
		checkStop();
	}
}

// make a random move of the last particle in the particleList
void DLASystem::moveLastParticle() {
	cout << "moveLastParticle get's called" << endl;

	int rr = random_max(0,5);  // pick a random number in the range 0-5, which direction do we hop? 
	double newpos[3];

	Particle *lastP = particleList[numParticles - 1];

	setPosNeighbour(newpos, lastP->pos, rr);

	if (distanceFromOrigin(newpos) > killCircle) {
		//cout << "#deleting particle" << endl;
		setGrid(lastP->pos, 0);
		particleList.pop_back();  // remove particle from particleList
		numParticles--;
		setParticleInactive();
	}
	
	// check if destination is empty
	else if (readGrid(newpos) == 0) {
		
		setGrid(lastP->pos, 0);  // set the old grid site to empty
		
		// update the position
		particleList[numParticles - 1]->pos[0] = newpos[0];
		particleList[numParticles - 1]->pos[1] = newpos[1];
		particleList[numParticles - 1]->pos[2] = newpos[2];
		setGrid(lastP->pos, 1);  // set the new grid site to be occupied

		// check if we stick
		if (checkStick()) {
			//cout << "stick" << endl;
			int n_p = numParticles-1;

			printpositions3d(n_p, newpos[0], newpos[1], newpos[2], clusterRadius, prob,G);
			
			setParticleInactive();  // make the particle inactive (stuck)
			updateClusterRadius(lastP->pos);  // update the cluster radius, addCircle, etc.

			if (numParticles % 100 == 0 && logfile.is_open()) {
				logfile << numParticles << " " << clusterRadius << endl;
			}

			cout << "working:" << lastP->pos[0] << "," << lastP->pos[1] << "," << lastP->pos[2] << endl;

		}
	}
	else {
		// if we get to here then we are trying to move to an occupied site
		// (this should never happen as long as the sticking probability is 1.0)
		cout << "reject " << rr << endl;
		cout << lastP->pos[0] << " " << lastP->pos[1] << " " << lastP->pos[2] << endl;
		//cout << newpos[0] << " " << newpos[1] << " " << (int)newpos[0] << endl;
		//printOccupied();
	}
}


void DLASystem::printpositions3d(int particle_n, double x, double y, double z, double rad, double prob, double G) { // TODO: add these properly
	cout << "printpositions3d get's called" << endl;
	cout << "saving position" << endl;

    // Open CSV file in append mode
    std::ofstream outFile("3d_no_gravity.csv", std::ios_base::app);

    if (outFile.is_open()) {

		float fractal_dim = (log(particle_n) / log(rad));

		// TODO: run tests with gravity
		// TODO: run tests with different probabilities but no gravity - requires change moving particle conditions

        // Write the values 
        outFile << particle_n << "," << x << "," << y << "," << z << "," << rad << "," << fractal_dim << "," << prob << "," << G << std::endl;
        
        // Close file
        outFile.close();

    } else {
        // Failed to open the file
        std::cerr << "Error: Unable to open the file for writing." << std::endl;
    }

	std::cout << "position saved" << std::endl;
}

// check if the last particle should stick (to a neighbour)
int DLASystem::checkStick() {
	cout << "checkStick get's called" << endl;
	Particle *lastP = particleList[numParticles - 1];
	int result = 0;
	int x; // random number
	// loop over neighbours

	// Implement force field
	double particle_position = *lastP->pos;
	// 3000 particles test: 10000,
	G = 0; // gravitational constant
	double F = (G)* numParticles / (particle_position * 100); //force // mases are "equivalent" = 1 
	prob = 1; //change prob manually here if needed 

	for (int i = 0; i < 6; i++) {
		double checkpos[3];
		setPosNeighbour(checkpos, lastP->pos, i);
		// if the neighbour is occupied...
		if (readGrid(checkpos) == 1) {
			x = rand() % 1000 + 1; // assuming m = 1000
			// if x/m is less than the sticking probability p, the particle should stick
			if ((double)x / 1000 < prob) {
				result = 1;
				cout << "Stick: Particle " << numParticles << endl;
			} else {
				cout << "Not Stick: Particle " << numParticles << ": " << i << endl;
			}
		}
	}
	return result;

	/*
	The updated algorithm now incorporates a stochastic element into the growth of the cluster.
	Upon contact with existing particles, each new particle has a chance of adhering to the cluster,
	determined by a specified sticking probability, p. 
	If the random ratio generated from 1 to an upper limit (m) is less than p, 
	the particle becomes part of the cluster. Otherwise, it proceeds independently, 
	following the original trajectory rules. This adjustment introduces variability into the 
	cluster's expansion,as particles may or may not attach depending on the probabilistic outcome. 
	Those that do not stick continue their movement without contributing to cluster 
	growth but still interact with existing particles according to system rules.
	*/
}

// constructor
DLASystem::DLASystem(Window *set_win) {
	cout << "DLASystem get's called" << endl;
	cout << "creating system, gridSize " << gridSize << endl;
	win = set_win;
	numParticles = 0;
	endNum = 10;

	// allocate memory for the grid, remember to free the memory in destructor
	grid = new int**[gridSize];
	for (int i = 0; i < gridSize; i++) {
		grid[i] = new int*[gridSize];
		for (int j = 0; j < gridSize; j++) {
			grid[i][j] = new int[gridSize];
		}
	}
	slowNotFast = 1;
	// reset initial parameters
	Reset();

	addRatio = 1.2;   // how much bigger the addCircle should be, compared to cluster radius
	killRatio = 1.7;   // how much bigger is the killCircle, compared to the addCircle

	// this opens a logfile, if we want to...
	//logfile.open("opfile.txt");
}

// destructor
DLASystem::~DLASystem() {
	// strictly we should not print inside the destructor but never mind...
	cout << "deleting system" << endl;
	// delete the particles
	clearParticles();
	// delete the grid
	for (int i = 0; i < gridSize; i++)
		delete[] grid[i];
	delete[] grid;

	if (logfile.is_open())
		logfile.close();
}


//Runs faster without this
// this draws the system
void DLASystem::DrawSquares() {
	cout << "DrawSquares get's called" << endl;
	// draw the particles
	double halfSize = 0.5;
	for (int p = 0; p < numParticles; p++) {
		double *vec = particleList[p]->pos;
		glPushMatrix();
		if (p == numParticles - 1 && lastParticleIsActive == 1)
			glColor4fv(colours::red);
		else if (p == 0)
			glColor4fv(colours::green);
		else
			glColor4fv(colours::blue);
		glRectd(drawScale*(vec[0] - halfSize),
			drawScale*(vec[1] - halfSize),
			drawScale*(vec[0] + halfSize),
			drawScale*(vec[1] + halfSize));
		glPopMatrix();
	}

	// print some information (at top left)
	// this ostringstream is a way to create a string with numbers and words (similar to cout << ... )
	ostringstream str;
	str << "num " << numParticles << " size " << clusterRadius;

	// print the string
	win->displayString(str, -0.9, 0.9, colours::red);

	// if we are paused then print this (at bottom left)
	if (running == 0) {
		ostringstream pauseStr;
		pauseStr << "paused";
		win->displayString(pauseStr, -0.9, -0.9, colours::red);
	}

}


