#include <GLUT/glut.h>
#include <iostream>
#include <stdio.h>
#include <vector>
#include <math.h>
#include <string>

#include "DLASystem.h"
#include "Window.h" 

#include <iostream>
#include <chrono>
#include <thread>

using namespace std;

// functions which are needed for openGL go into a namespace so that we can identify them
namespace drawFuncs {
  void handleKeypress(unsigned char key, int x, int y);
  void display(void);
  void update(int val);
  void introMessage();
  void Particle_and_rad(int count,int radius, double prob);
}

// this is a global pointer, which is how we access the system itself
DLASystem *sys;

int main(int argc, char **argv) {
  // turn on glut
	glutInit(&argc, argv);

  int window_size[] = { 480,480 };
  string window_title("simple DLA simulation");

  // create a window
  Window *win = new Window(window_size,window_title);

  // create the system
  sys = new DLASystem(win);
  
  // this is the seed for the random numbers
  int seed = 6;
  cout << "setting seed " << seed << endl;
  sys->setSeed(seed);
  
  // print the "help" message to the console
  drawFuncs::introMessage();
  
  // tell openGL how to redraw the screen and respond to the keyboard
	glutDisplayFunc(  drawFuncs::display );
	glutKeyboardFunc( drawFuncs::handleKeypress );
  
  // tell openGL to do its first update after waiting 10ms
  int wait = 10;
  int val = 0;
	glutTimerFunc(wait, drawFuncs::update, val);

  // start the openGL stuff
 	glutMainLoop();

  return 0;
}

// this is just a help message
void drawFuncs::introMessage() {
        cout << "Keys (while in graphics window):" << endl << "  q or e to quit (or exit)" << endl;
        cout << "  h to print this message (help)" << endl;
        cout << "  u for a single update" << endl;
        cout << "  g to start running (go)" << endl;
        cout << "  p to pause running" << endl;
        cout << "  s to run in slow-mode" << endl;
        cout << "  f to run in fast-mode" << endl;
        cout << "  r to clear everything (reset)" << endl;
        cout << "  z to pause and zoom in" << endl;
        cout << "  w or b to change background colour to white or black" << endl;
        cout << "  O to print the num particles and cluster size" << endl;
        cout << "  A to print results to a csv file" << endl;
        cout << "  R to automated run" << endl;
}

void drawFuncs::Particle_and_rad(int count,int radius, double prob) {
        cout << "  Number of particles: " << count << endl;
        cout << "  Cluster radius (size): " << radius << endl;
        cout << "  Probability of sticking: " << prob << endl;
}

void automateRun(DLASystem* sys, int N_runs) {
    // Seed the random number generator
    srand(time(NULL));

    int endNum = sys->getEndNum();
    // EX 1
    // run 1: 1-985, 100 runs per thing, jump ever 1
    // run 2: 985-10000, 3 runs per thing, jump every 100
    // run 3: 1000-28000, 2 runs per thing, jump every 1000
    // run 4: 28000-100000, 2 runs per thing, jump every 1000
    // run 5: 500-100000, 3 runs per thing, jump every 200

    // EX 2
    // run 1: 1-1000, 10 runs per thing, jump every 0.01, prob 1-0 -> killed it somewhere in between
    // run 2: 10-10000, 2 runs per thing, jump every 0.1, prob 1-0 -> only ran prob 1
    // run 3: attempted run 2 again and made a chage probability function to change by k -> either broken or way too slow
    // run 4: 1-100, 1 run per thing, jump every 0.1, prob 1-0 -> only ran prob 1
    // run 5: 1-100, 1 run per thing, jump every 0.1, prob 0.9-0 -> it ran but laptop crashed on the way
    // run 6: running the same as run 5 and praying
    // Given up on loops - running manually
    // run 1: 0.9 prob, 1 run per thing, 0-98
    // run 2: 0.9 prob, 3 runs per thing, 0-100
    // next runs: ran everything from 0.8-0.1 with 0.1 skips 3 times
    // ran 0.5 3 times
    // ran 0.05 to 0.95 3 times with 0.1 skips (0.05,0.15,...)

    double prob = sys->getProbability();

    for (int j = 0; j < endNum; j++) {
      for (int i = 0; i < N_runs; i++) {

          // random seed
          int s = rand(); 
          cout << "Size: "<< j << " Iteration: " << i+1 << " Seed:" << s << "Prob: " << prob << endl;
          sys->setSeed(s);
          sys->setEndNum(j);
          sys->setRunning();
          glutTimerFunc(0, drawFuncs::update, 0);
          sys->setFast();

          while (sys->Update() == 0) {
            // Do nothing, just wait for update
          }
          
          // Get results
          int count = sys->getNumParticles();
          int radius = sys->getClusterRadius();
          // Print and exit
          // sys->printocsv(count, radius, prob);
          cout << "end system" << endl;

          // Reset the system
          sys->Reset();
        
      }
    }
    cout << "Automated run complete" << endl;
}


// openGL function deals with the keyboard
void drawFuncs::handleKeypress(unsigned char key, int x, int y) {
	
  //number of runs
  int N_runs = 10;

  // results
  int count = sys->getNumParticles();
  int radius = sys->getClusterRadius();

  double prob = sys->getProbability();

  switch (key) {
  case 'h':
    drawFuncs::introMessage();
    break;
	case 'q':
  case 'e':
    cout << "Exiting..." << endl;
    // delete the system
    delete sys;
    exit(0);
    break;
  case 'p':
    cout << "pause" << endl;
    sys->pauseRunning();
    break;
  case 'g':
    cout << "go" << endl;
    sys->setRunning();
    glutTimerFunc(0, drawFuncs::update, 0);
    break;
  case 's':
    cout << "slow" << endl;
    sys->setSlow();
    break;
  case 'w':
    cout << "white" << endl;
    sys->setWinBackgroundWhite();
    break;
  case 'b':
    cout << "black" << endl;
    sys->setWinBackgroundBlack();
    break;
  case 'f':
    cout << "fast" << endl;
    sys->setFast();
    break;
  case 'r':
    cout << "reset" << endl;
    sys->Reset();
    break;
  case 'z':
    cout << "zoom" << endl;
    sys->pauseRunning();
    sys->viewAddCircle();
    break;
  case 'u':
    cout << "upd" << endl;
    sys->Update();
    break;
  case 'O':
    drawFuncs::Particle_and_rad(count, radius, prob);
    break;
  case 'A':  
    // sys->printocsv(count, radius, prob); 
    break;
  case 'R':
    automateRun(sys,N_runs);
    sys->Reset();
    exit(0);
    break;
      
  }
  // tell openGL to redraw the window
	glutPostRedisplay();
}

// this function gets called whenever the algorithm should do its update
void drawFuncs::update(int val) {
  int wait;  // time to wait between updates (milliseconds)
  
  if ( sys->running ) {
    if ( sys->slowNotFast == 1)
      wait = 10;
    else
      wait = 0;
    
    sys->Update();
  
    // tell openGL to call this funtion again after "wait" milliseconds
    glutTimerFunc(wait, drawFuncs::update, 0);
  }
  
}

// this function redraws the window when necessary
void drawFuncs::display() {
    	//  Clear the window or more specifically the frame buffer...
      //  This happens by replacing all the contents of the frame
      //  buffer by the clear color (black in our case)
      glClear(GL_COLOR_BUFFER_BIT);

      // this puts the camera at the origin (not sure why) with (I think) z axis out of page and y axis up
      // there is also the question of the GL perspective which is not set up in any clear way at the moment
      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();
      gluLookAt(0.0, 0.0, 1.0,   /* camera position */
        0.0, 0.0, -1.0,        /* point to look at */
        0.0, 1.0, 0.0);		   /* up direction */

      //sys->DrawSpheres();
      //sys->DrawSquares();

      //  Swap contents of backward and forward frame buffers
      glutSwapBuffers();
}

