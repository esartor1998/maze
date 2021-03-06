#ifdef __APPLE__
#include <GLUT/glut.h>
#include <OpenGL/glu.h>
#include <OpenGL/gl.h>
#elif _WIN32
#include <windows.h>
#include <GL/glut.h>
#include <GL/glu.h>
#include <GL/gl.h>
#else
#include <GL/glut.h>
#include <GL/glu.h>
#include <GL/gl.h>
#endif

#include <time.h>
#include <sys/time.h>

/* world size and storage array */
#define WORLDX 100
#define WORLDY 50
#define WORLDZ 100
GLubyte world[WORLDX][WORLDY][WORLDZ];

/* list of cubes to draw with each screen update */
#define MAX_DISPLAY_LIST 500000

/* maximum number of user defined colours */
#define NUMBERCOLOURS 100

	/* maximum number of textures which can be loaded */
#define NUMBERTEXTURES 100

	/* maximum number of meshes which can be loaded from ~/models dir */
#define NUMBERMESH 100

	/* maximum number of mesh the user can instantiate at one time */
#define MAXMESH 100

	/* maximum texture width and height */
#define TEXTURESIZE 256
