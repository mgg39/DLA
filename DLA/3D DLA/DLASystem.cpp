//
//  DLASystem.cpp
//

#include "DLASystem.h"
#include <fstream> //work with csv
#include <cmath> 

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
	// delete particles and the particle list
	for (int i = 0; i < numParticles; i++) {
		delete particleList[i];
	}
	particleList.clear();
	numParticles = 0;
}

// remove any existing particles and setup initial condition
void DLASystem::Reset() {
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
	int halfGrid = gridSize / 2;
	grid[(int)(pos[0] + halfGrid)][(int)(pos[1] + halfGrid)][(int)(pos[2] + halfGrid)] = val;
}

// read the grid cell for a given position
int DLASystem::readGrid(double pos[]) {
	int halfGrid = gridSize / 2;
	return grid[(int)(pos[0] + halfGrid)][(int)(pos[1] + halfGrid)][(int)(pos[2] + halfGrid)];
}

// check if the cluster is big enough and we should stop:
// to be safe, we need the killCircle to be at least 2 less than the edge of the grid
int DLASystem::checkStop() {
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
	double pos[2];
	double theta = rgen.random01() * 2 * M_PI;
	double phi = rgen.random01() * 1 * M_PI; // 1 is the max value for phi
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
}

// if the view is smaller than the kill circle then increase the view area (zoom out)
void DLASystem::updateViewSize() {
	double mult = 1.2;
	if (viewSize < 2.0*killCircle) {
		setViewSize(viewSize * mult);
	}
}

// set the view to be the size of the add circle (ie zoom in on the cluster)
void DLASystem::viewAddCircle() {
	setViewSize(2.0*addCircle);  // factor of 2 is to go from radius to diameter
}

// when we add a particle to the cluster, we should update the cluster radius
// and the sizes of the addCircle and the killCircle
void DLASystem::updateClusterRadius(double pos[]) {

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
	int rr = rgen.randomInt(6);  // pick a random number in the range 0-5, which direction do we hop? 
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
			printpositions3d(n_p, newpos[0], newpos[1], newpos[2], prob, clusterRadius);
			setParticleInactive();  // make the particle inactive (stuck)
			updateClusterRadius(lastP->pos);  // update the cluster radius, addCircle, etc.

			if (numParticles % 100 == 0 && logfile.is_open()) {
				logfile << numParticles << " " << clusterRadius << endl;
			}

		}
	}
	else {
		// if we get to here then we are trying to move to an occupied site
		// (this should never happen as long as the sticking probability is 1.0)
		cout << "reject " << rr << endl;
		cout << lastP->pos[0] << " " << lastP->pos[1] << endl;
		//cout << newpos[0] << " " << newpos[1] << " " << (int)newpos[0] << endl;
		//printOccupied();
	}
}




void DLASystem::printpositions3d(int particle_n, double x, double y, double z,  double prob, double radius) { 

	cout << "saving position" << endl;

    // Open CSV file in append mode
    std::ofstream outFile("3D.csv", std::ios_base::app);

    if (outFile.is_open()) {
		//dimension
		double fractal_dim = log(particle_n)/log(radius);;

        // Write the values 
        outFile << particle_n << "," << x << "," << y << "," << z << "," << prob << "," << radius << "," << fractal_dim << std::endl;
        
        // Close file
        outFile.close();

    } else {
        // Failed to open the file
        std::cerr << "Error: Unable to open the file for writing." << std::endl;
    }

	cout << "position saved" << endl;
}

// check if the last particle should stick (to a neighbour)
int DLASystem::checkStick() {
	Particle *lastP = particleList[numParticles - 1];
	int result = 0;
	int x; // random number
	// loop over neighbours
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
	cout << "creating system, gridSize " << gridSize << endl;
	win = set_win;
	numParticles = 0;
	// TESTs: 1000
	// TODO: dimensions calc python file
	endNum = 1000;
	prob = 0.005; //make - 5 times up to 0.6, 0.6 - 0.05 2 times

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



// this draws the system
void DLASystem::DrawSquares() {

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
	str << "num " << numParticles << " size " << clusterRadius;;

	// print the string
	win->displayString(str, -0.9, 0.9, colours::red);

	// if we are paused then print this (at bottom left)
	if (running == 0) {
		ostringstream pauseStr;
		pauseStr << "paused";
		win->displayString(pauseStr, -0.9, -0.9, colours::red);
	}

}