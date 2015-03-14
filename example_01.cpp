
#include <vector>
#include <iostream>
#include <fstream>
#include <cmath>
#include <stdlib.h>
#include <stdio.h>
#include "Eigen/Dense"

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

#ifdef OSX
#include <GLUT/glut.h>
#include <OpenGL/glu.h>
#else
#include <GL/glut.h>
#include <GL/glu.h>
#endif

#include <time.h>
#include <math.h>

using namespace Eigen;
using namespace std;


#define PI 3.14159265  // Should be used from mathlib
inline float sqr(float x) { return x*x; }

using namespace std;

//****************************************************
// Some Classes
//****************************************************

class Viewport;

class Viewport {
  public:
    int w, h; // width and height
};


//****************************************************
// Global Variables
//****************************************************
Viewport	viewport;

struct point_light {
  float x, y, z, R, G, B;
};
struct directional_light {
  float x, y, z, R, G, B;
};

int point_light_count = 0;
int directional_light_count = 0;

float ka_R;
float ka_G;
float ka_B;
Vector3f ambient_term(0, 0, 0);

float kd_R;
float kd_G;
float kd_B;
Vector3f diffuse_term(0, 0, 0);

float ks_R;
float ks_G;
float ks_B;
Vector3f specular_term(0, 0, 0);

float sp_V;

float pl_X;
float pl_Y;
float pl_Z;
float pl_R;
float pl_G;
float pl_B;

float dl_X;
float dl_Y;
float dl_Z;
float dl_R;
float dl_G;
float dl_B;

point_light pl_array[5];
directional_light dl_array[5];




//****************************************************
// Simple init function
//****************************************************
void initScene(){

  // Nothing to do here for this simple example.

}


//****************************************************
// reshape viewport if the window is resized
//****************************************************
void myReshape(int w, int h) {
  viewport.w = w;
  viewport.h = h;

  glViewport (0,0,viewport.w,viewport.h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0, viewport.w, 0, viewport.h);

}


//****************************************************
// A routine to set a pixel by drawing a GL point.  This is not a
// general purpose routine as it assumes a lot of stuff specific to
// this example.
//****************************************************

// Returns a random float between 0 and 1
float random_float()
{
  float the_number = (float)rand()/RAND_MAX;
  printf("rand: %f\n",the_number);
  return the_number;
}

void setPixel(int x, int y, GLfloat r, GLfloat g, GLfloat b) {
  glColor3f(r, g, b);
  glVertex2f(x + 0.5, y + 0.5);   // The 0.5 is to target pixel
  // centers 
  // Note: Need to check for gap
  // bug on inst machines.
}

//****************************************************
// Draw a filled circle.  
//****************************************************


void circle(float centerX, float centerY, float radius) {
  // Draw inner circle
  glBegin(GL_POINTS);

  // We could eliminate wasted work by only looping over the pixels
  // inside the sphere's radius.  But the example is more clear this
  // way.  In general drawing an object by loopig over the whole
  // screen is wasteful.

  int i,j;  // Pixel indices

  int minI = max(0,(int)floor(centerX-radius));
  int maxI = min(viewport.w-1,(int)ceil(centerX+radius));

  int minJ = max(0,(int)floor(centerY-radius));
  int maxJ = min(viewport.h-1,(int)ceil(centerY+radius));


  for (i=0;i<viewport.w;i++) {
    for (j=0;j<viewport.h;j++) {

      // Location of the center of pixel relative to center of sphere
      float x = (i+0.5-centerX);
      float y = (j+0.5-centerY);

      float dist = sqrt(sqr(x) + sqr(y));

      if (dist<=radius) {

        // This is the front-facing Z coordinate
        float z = sqrt(radius*radius-dist*dist);

        Vector3f pixel(x, y, z);
        pixel.normalize();
        Vector3f surfaceNormal(x, y, z);
        surfaceNormal.normalize();
        Vector3f R(0.0, 0.0, 0.0);

        for (int p = 0; p < point_light_count; p++) {
          Vector3f lightloc(pl_array[p].x, pl_array[p].y, pl_array[p].z);
          Vector3f lightcolor(pl_array[p].R,pl_array[p].G,pl_array[p].B);
          //printf("Light stuff: %f %f %f %f %f %f\n",pl_array[p].x, pl_array[p].y, pl_array[p].z,pl_array[p].R,pl_array[p].G,pl_array[p].B);
          lightloc = lightloc - pixel;
          lightloc.normalize();
          //std::cout << "Light: " << lightloc << "\n";

          Vector3f new_ambient(ambient_term(0)*lightcolor(0),ambient_term(1)*lightcolor(1),ambient_term(2)*lightcolor(2));
          R += new_ambient; // Add ambient term for each point light

          float d = lightloc.dot(surfaceNormal);
          float color = fmax(d, 0);
          color *= d;

          Vector3f new_diffuse(diffuse_term(0)*lightcolor(0),diffuse_term(1)*lightcolor(1),diffuse_term(2)*lightcolor(2));

          R += color * new_diffuse;

          Vector3f rd = (-1 * lightloc) + (2 * lightloc.dot(surfaceNormal) * surfaceNormal);
          rd.normalize();
          Vector3f v_vec(0, 0, 1);
          float rvec = rd.dot(v_vec);
          rvec = fmax(rvec, 0);
          Vector3f new_specular(specular_term(0)*lightcolor(0),specular_term(1)*lightcolor(1),specular_term(2)*lightcolor(2));

          R += pow(rvec, sp_V) * new_specular;

        }

        for (int p = 0; p < directional_light_count; p++) {
          Vector3f lightloc(dl_array[p].x, dl_array[p].y, dl_array[p].z);
          Vector3f lightcolor(dl_array[p].R,dl_array[p].G,dl_array[p].B);
          lightloc = -1*lightloc;
          lightloc.normalize();

          Vector3f new_ambient(ambient_term(0)*lightcolor(0),ambient_term(1)*lightcolor(1),ambient_term(2)*lightcolor(2));
          R += new_ambient; // Add ambient term for each point light

          float d = lightloc.dot(surfaceNormal);
          float color = fmax(d, 0);
          color *= d;

          Vector3f new_diffuse(diffuse_term(0)*lightcolor(0),diffuse_term(1)*lightcolor(1),diffuse_term(2)*lightcolor(2));

          R += color * new_diffuse;

          Vector3f rd = (-1 * lightloc) + (2 * lightloc.dot(surfaceNormal) * surfaceNormal);
          rd.normalize();
          Vector3f v_vec(0, 0, 1);
          float rvec = rd.dot(v_vec);
          rvec = fmax(rvec, 0);
          Vector3f new_specular(specular_term(0)*lightcolor(0),specular_term(1)*lightcolor(1),specular_term(2)*lightcolor(2));

          R += pow(rvec, sp_V) * new_specular;

        }

        setPixel(i, j, R(0), R(1), R(2));

        // This is amusing, but it assumes negative color values are treated reasonably.
        // setPixel(i,j, x/radius, y/radius, z/radius );
      }


    }
  }


  glEnd();
}

void keyPressed(char key, int x, int y) {
  if (key == ' ') {
    exit(1);
  }
}
//****************************************************
// function that does the actual drawing of stuff
//***************************************************
void myDisplay() {
//glutKeyboardFunc(keyPressed);
  glClear(GL_COLOR_BUFFER_BIT);				// clear the color buffer

  glMatrixMode(GL_MODELVIEW);			        // indicate we are specifying camera transformations
  glLoadIdentity();				        // make sure transformation is "zero'd"


  // Start drawing
  circle(viewport.w / 2.0 , viewport.h / 2.0 , min(viewport.w, viewport.h) / 3.0);

  glFlush();
  glutSwapBuffers();					// swap buffers (we earlier set double buffer)
  
}



//****************************************************
// the usual stuff, nothing exciting here
//****************************************************
int main(int argc, char *argv[]) {
  //This initializes glut
  srand(time(NULL));
  glutInit(&argc, argv);
    
  //Grab the command line arguments
  if (argc == 1) {
    printf("No arguments given, printing a black sphere with no lights.\n");
  }
  int i = 1;
  int pl_array_index = 0;
  int dl_array_index = 0;
  while (i < argc-1) {
    if (strcmp(argv[i], "-ka") == 0) {
      ka_R = atof(argv[i+1]);
      ka_G = atof(argv[i+2]);
      ka_B = atof(argv[i+3]);
      ambient_term << ka_R, ka_G, ka_B;
      i += 4;
    }  
    else if (strcmp(argv[i], "-kd") == 0) {
      kd_R = atof(argv[i+1]);
      kd_G = atof(argv[i+2]);
      kd_B = atof(argv[i+3]);
      diffuse_term << kd_R, kd_G, kd_B;
      i += 4;
    }
    else if (strcmp(argv[i], "-ks") == 0) {
      ks_R = atof(argv[i+1]);
      ks_G = atof(argv[i+2]);
      ks_B = atof(argv[i+3]);
      specular_term << ks_R, ks_G, ks_B;
      i += 4;
    }
    else if (strcmp(argv[i], "-sp") == 0) {
      sp_V = atof(argv[i+1]);
      i += 2;
    }
    else if (strcmp(argv[i], "-pl") == 0) {
      point_light temp;
      temp.x = atof(argv[i+1]);
      temp.y = atof(argv[i+2]);
      temp.z = atof(argv[i+3]);
      temp.R = atof(argv[i+4]);
      temp.G = atof(argv[i+5]);
      temp.B = atof(argv[i+6]);
      pl_array[pl_array_index] = temp;
      i += 7;
      pl_array_index += 1;
      point_light_count += 1;
    }
    else if (strcmp(argv[i], "-dl") == 0) {
      directional_light temp;
      temp.x = atof(argv[i+1]);
      temp.y = atof(argv[i+2]);
      temp.z = atof(argv[i+3]);
      temp.R = atof(argv[i+4]);
      temp.G = atof(argv[i+5]);
      temp.B = atof(argv[i+6]);
      dl_array[dl_array_index] = temp;
      i += 7;
      dl_array_index += 1;
      directional_light_count += 1;
    }
    else {
      printf("ERROR! Unknown argument '%s'\n",argv[i]);
      i += 100;
    }   
  }

  //This tells glut to use a double-buffered window with red, green, and blue channels 
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

  // Initalize theviewport size
  viewport.w = 400;
  viewport.h = 400;

  //The size and position of the window
  glutInitWindowSize(viewport.w, viewport.h);
  glutInitWindowPosition(0,0);
  glutCreateWindow(argv[0]);

  initScene();							// quick function to set up scene

  glutDisplayFunc(myDisplay);				// function to run when its time to draw something
  glutReshapeFunc(myReshape);				// function to run when the window gets resized

  glutMainLoop();							// infinite loop that will keep drawing and resizing
  // and whatever else

  return 0;
}








