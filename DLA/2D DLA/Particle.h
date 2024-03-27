#pragma once

class Particle {
  public:
    static const int dim = 2;  // we are in two dimensions
    double *pos;  // pointer to an array of size dim, to store the position
  
    // default constructor
    Particle() {
      pos = new double[dim];
    }
    // constructor, with a specified initial position
    Particle(double set_pos[]) {
      pos = new double[dim];
      for (int d=0;d<dim;d++)
        pos[d]=set_pos[d];
    }
    // destructor
    ~Particle() { delete[] pos; }
};
