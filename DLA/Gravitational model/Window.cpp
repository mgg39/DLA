#include "Window.h"

// constructor
Window::Window(int set_size[],  string &set_title) {
  size[0]=set_size[0]; size[1]=set_size[1];
  title=set_title;

  locateOnScreen();
  glutInitWindowSize(size[0],size[1]);
  glutInitWindowPosition(pos[0],pos[1]);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
  glutCreateWindow(title.c_str());

  // sets the background to black
  glClearColor(0.0, 0.0, 0.0, 0.0);
}

// print a string at a given position, don't worry about how this works...
void Window::displayString( ostringstream &str, double x, double y, GLfloat col[]) {
    string localString = str.str();
    glColor4fv(col);
    glRasterPos2d(x,y);
    for (int i = 0; i < localString.length(); i++) {
      glutBitmapCharacter(GLUT_BITMAP_8_BY_13, localString[i]);
    }
}
