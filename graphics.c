
/* Derived from scene.c in the The OpenGL Programming Guide */
/* Keyboard and mouse rotation taken from Swiftless Tutorials #23 Part 2 */
/* http://www.swiftless.com/tutorials/opengl/camera2.html */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <float.h>
#include <dirent.h>

#include "graphics.h"
#include "mesh.h"

GLubyte world[WORLDX][WORLDY][WORLDZ];

#define MOB_COUNT 10
#define PLAYER_COUNT 10
#define TUBE_COUNT 10

extern void update();
extern void collisionResponse();
extern void buildDisplayList();
extern void mouse(int, int, int, int);
extern void draw2D();

	// stores mesh structures read from .obj files
struct meshStruct *meshobj;
	// number of stored mesh structures in meshobj
int meshcount;
	/* user information for drawing mesh */
	/* flag indicates if mesh has been instantiated 0 == no, 1 == yes */
int meshUsed[MAXMESH];

	/* struct stores user mesh configuration information */
struct uMesh {
	/* number corresponding to file number in ./models - what to draw */
   int meshNumber; 
	/* flag indicates if mesh should be drawn, 0 == no, 1 == yes */
   int drawMesh; 
	/* position of mesh */
   float xpos, ypos, zpos;
	/* rotation of mesh */
   float xrot, yrot, zrot;
	/* scale mesh */
   float scale;
};
	/* holds all user mesh instances */
struct uMesh userMesh[MAXMESH];

	// load mesh from .obj file
extern int readObjFile(char *, struct meshStruct *);

/* flags used to control the appearance of the image */
int lineDrawing = 0;	// draw polygons as solid or lines
int lighting = 1;	// use diffuse and specular lighting
int smoothShading = 1;  // smooth or flat shading
int textures = 0;

/* texture data */ 
//this was in his a1 and never used so i'm not removing it lawl
GLubyte  Image[64][64][4];
	/* OpenGL id for each textue */
GLuint   textureID[NUMBERTEXTURES];
	/* number of loaded textures */
int textureCount = 0;
	/* flags indicate which textures are allocated in texture array */
	/* used textures marked with 1, empty textures marked with 0 */
int textureUsed[NUMBERTEXTURES];
	/* list of textures associated with colour ids */
	/* one texture for each user defined colour */
int textureAssigned[NUMBERCOLOURS];
	/* texture offset, shifts u,v coordinates for textures */
	/* one set of offsets for each user defined colour/texture pair */
	/* [0] is u offset, [1] is v offset */
float tOffset[NUMBERCOLOURS][2];

	/* texture information for meshes, mirrors previous texture variables */
GLuint   meshtextureID[NUMBERMESH];
int meshtextureCount = 0;
int meshtextureUsed[NUMBERMESH];


/* viewpoint coordinates */
float vpx = -50.0, vpy = -50.0, vpz = -50.0;
float oldvpx, oldvpy, oldvpz;

/* mouse direction coordiates */
float mvx = 0.0, mvy = 45.0, mvz = 0.0;

/* stores current mouse position value */
float oldx, oldy;

/* location for the light source (the sun), the first three
	values are the x,y,z coordinates */
GLfloat lightPosition[] = {0.0, 49.0, 0.0, 0.0};
/* location for light source that is kept at viewpoint location */
GLfloat viewpointLight[] = {-50.0, -50.0, -50.0, 1.0};

/* sky cube size */
float skySize;

/* screen dimensions */
int screenWidth = 1024;
int screenHeight = 768;

/* command line flags */
int flycontrol = 0;		 // allow viewpoint to move in y axis when 1
int displayAllCubes = 0; // draw all of the cubes in the world when 1
int testWorld = 0;		 // sample world for timing tests
int fps = 0;			    // turn on frame per second output
int netClient = 0;		 // network client flag, is client when = 1
int netServer = 0;		 // network server flag, is server when = 1

/* list of cubes to display */
int displayList[MAX_DISPLAY_LIST][3];
int displayCount = 0;		// count of cubes in displayList[][]

/* list of mobs - number of mobs, xyz values and rotation about y */
float mobPosition[MOB_COUNT][4];
/* visibility of mobs, 0 not drawn, 1 drawn */
short mobVisible[MOB_COUNT];

/* list of players - number of mobs, xyz values and rotation about y */
float playerPosition[PLAYER_COUNT][4];
/* visibility of players, 0 not drawn, 1 drawn */
short playerVisible[PLAYER_COUNT];

/* list of tubes - number of tubes, staring x, y, z position, ending  */
/*  x, y, z position */
float tubeData[TUBE_COUNT][6];
/* tube colour for each tube */
int   tubeColour[TUBE_COUNT];
/* visibility of tubes, 0 not drawn, 1 drawn */
short tubeVisible[TUBE_COUNT];

/* flag indicating the user wants the cube in front of them removed */
int space = 0;
/* flag indicates if map is to be printed */
int displayMap = 1;
/* flag indicating a fixed viewpoint - not updated by mouse/keyboard */
int fixedVP = 0;

/* list of user defined colours */
/* ambient (RGBA) followed by diffuse (RGBA) */
GLfloat uAmbColour[NUMBERCOLOURS][4];
GLfloat uDifColour[NUMBERCOLOURS][4];
/* flag indicating user defined colour has been allocated */
/* initialized to 0, set to 1 when colour stored in uColour[][] */
int uColourUsed[NUMBERCOLOURS];

bool usegravity = true;
bool collisions = true;
/* functions draw 2D images */
void  draw2Dline(int, int, int, int, int);
void  draw2Dbox(int, int, int, int);
void  draw2Dtriangle(int, int, int, int, int, int);
void  set2Dcolour(float []);


	/* texture functions attach textures to cubes */
int setAssignedTexture(int, int);
void unsetAssignedTexture(int);
int getAssignedTexture(int);
void setTextureOffset(int, float, float);

	/* user mesh funcitons - create and position loaded mesh objects */
void setMeshID(int, int, float, float, float);
void unsetMeshID(int);
void setTranslateMesh(int, float, float, float);
void setRotateMesh(int, float, float, float);
void setScaleMesh(int, float);
/***************/



/* player control functions */
/* set all player location, rotation, and visibility values to zero */
void initPlayerArray() {
int i;
	for (i=0; i<MOB_COUNT; i++) {
		playerPosition[i][0] = 0.0;
		playerPosition[i][1] = 0.0;
		playerPosition[i][2] = 0.0;
		playerPosition[i][3] = 0.0;
		playerVisible[i] = 0;
	}
}

/*** create player with identifier "number" at x,y,z with
 heading of rotx, roty, rotz. num x y z roty***/
void createPlayer(int number, float x, float y, float z, float playerroty) {
	if (number >= PLAYER_COUNT) {
		printf("ERROR: player number greater than %d\n", PLAYER_COUNT);
		exit(1);
	}
	playerPosition[number][0] = x;
	playerPosition[number][1] = y;
	playerPosition[number][2] = z;
	playerPosition[number][3] = playerroty;
	playerVisible[number] = 1;
}

/* move player to a new position xyz with rotation roty */
void setPlayerPosition(int number, float x, float y, float z, float playerroty){
	if (number >= PLAYER_COUNT) {
		printf("ERROR: player number greater than %d\n", PLAYER_COUNT);
		exit(1);
	}
	playerPosition[number][0] = x;
	playerPosition[number][1] = y;
	playerPosition[number][2] = z;
	playerPosition[number][3] = playerroty;
}

/* turn off drawing for player number */
void hidePlayer(int number) {
	if (number >= PLAYER_COUNT) {
		printf("ERROR: player number greater than %d\n", PLAYER_COUNT);
		exit(1);
	}
	playerVisible[number] = 0;
}

/* turn on drawing for player number */
void showPlayer(int number) {
	if (number >= PLAYER_COUNT) {
		printf("ERROR: player number greater than %d\n", PLAYER_COUNT);
		exit(1);
	}
	playerVisible[number] = 1;
}



/* mob control functions */
/* set all mob location, rotation, and visibility values to zero */
void initMobArray() {
int i;
	for (i=0; i<MOB_COUNT; i++) {
		mobPosition[i][0] = 0.0;
		mobPosition[i][1] = 0.0;
		mobPosition[i][2] = 0.0;
		mobPosition[i][3] = 0.0;
		mobVisible[i] = 0;
	}
}

/* create mob with identifier "number" at x,y,z with */
/* heading of rotx, roty, rotz */
void createMob(int number, float x, float y, float z, float mobroty) {
	if (number >= MOB_COUNT) {
		printf("ERROR: mob number greater than %d\n", MOB_COUNT);
		exit(1);
	}
	mobPosition[number][0] = x;
	mobPosition[number][1] = y;
	mobPosition[number][2] = z;
	mobPosition[number][3] = mobroty;
	mobVisible[number] = 1;
}

/* move mob to a new position xyz with rotation rotx,roty,rotz */
void setMobPosition(int number, float x, float y, float z, float mobroty){
	if (number >= MOB_COUNT) {
		printf("ERROR: mob number greater than %d\n", MOB_COUNT);
		exit(1);
	}
	mobPosition[number][0] = x;
	mobPosition[number][1] = y;
	mobPosition[number][2] = z;
	mobPosition[number][3] = mobroty;
}

/* turn off drawing for mob number */
void hideMob(int number) {
	if (number >= MOB_COUNT) {
		printf("ERROR: mob number greater than %d\n", MOB_COUNT);
		exit(1);
	}
	mobVisible[number] = 0;
}

/* turn on drawing for mob number */
void showMob(int number) {
	if (number >= MOB_COUNT) {
		printf("ERROR: mob number greater than %d\n", MOB_COUNT);
		exit(1);
	}
	mobVisible[number] = 1;
}


/* initialize all tubes as not visible */
void initTubeArray(){
int i;
	for (i=0; i<TUBE_COUNT; i++) {
		tubeVisible[i] = 0;
	}
}

void createTube(int number, float bx, float by, float bz,
	float ex, float ey, float ez, int colour) {
	tubeData[number][0] = bx;
	tubeData[number][1] = by;
	tubeData[number][2] = bz;
	tubeData[number][3] = ex;
	tubeData[number][4] = ey;
	tubeData[number][5] = ez;
	tubeColour[number] = colour;
	tubeVisible[number] = 1;
}

/* turn off drawing for tube number */
void hideTube(int number) {
	if (number >= TUBE_COUNT) {
		printf("ERROR: tube number greater than %d\n", TUBE_COUNT);
		exit(1);
	}
	tubeVisible[number] = 0;
}

/* turn on drawing for tube number */
void showTube(int number) {
	if (number >= TUBE_COUNT) {
		printf("ERROR: tube number greater than %d\n", TUBE_COUNT);
		exit(1);
	}
	tubeVisible[number] = 1;
}


/* allows user to set position of the light */
void setLightPosition(GLfloat x, GLfloat y, GLfloat z) {
	lightPosition[0] = x;
	lightPosition[1] = y;
	lightPosition[2] = z;
	glLightfv (GL_LIGHT0, GL_POSITION, lightPosition);
}

/* returns current position of the light */
GLfloat* getLightPosition() {
	return(lightPosition);
}

/* functions store and return the current location of the viewpoint */
void getViewPosition(float *x, float *y, float *z) {
	*x = vpx;
	*y = vpy;
	*z = vpz;
}

void setViewPosition(float x, float y, float z) {
	vpx = x;
	vpy = y;
	vpz = z;
}

/* returns the previous location of the viewpoint */
void getOldViewPosition(float *x, float *y, float *z) {
	*x = oldvpx;
	*y = oldvpy;
	*z = oldvpz;
}

/* sets the previous location of the viewpoint */
void setOldViewPosition(float x, float y, float z) {
	oldvpx = x;
	oldvpy = y;
	oldvpz = z;
}

/* sets the current orientation of the viewpoint */
void setViewOrientation(float xaxis, float yaxis, float zaxis) {
	mvx = xaxis;
	mvy = yaxis;
	mvz = zaxis;
}

/* returns the current orientation of the viewpoint */
void getViewOrientation(float *xaxis, float *yaxis, float *zaxis) {
	*xaxis = mvx;
	*yaxis = mvy;
	*zaxis = mvz;
}

/* add the cube at world[x][y][z] to the display list and */
/* increment displayCount */
void addDisplayList(int x, int y, int z) {
	displayList[displayCount][0] = x;
	displayList[displayCount][1] = y;
	displayList[displayCount][2] = z;
	displayCount++;
	if (displayCount > MAX_DISPLAY_LIST) {
		printf("You have put more items in the display list then there are\n");
		printf("cubes in the world. Set displayCount = 0 at some point.\n");
		exit(1);
	}
}



/*  Initialize material property and light source.  */
void init (void)
{

	GLfloat light_ambient[] = { 0.5, 0.5, 0.5, 1.0 };
	GLfloat light_diffuse[] = { 0.8, 0.8, 0.8, 1.0 };
	GLfloat light_specular[] = { 0.5, 0.5, 0.5, 1.0 };
	GLfloat light_full_off[] = {0.0, 0.0, 0.0, 1.0};
	GLfloat light_full_on[] = {1.0, 1.0, 1.0, 1.0};

	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);

	/* if lighting is turned on then use ambient, diffuse and specular
		lights, otherwise use ambient lighting only */
	if (lighting == 1) {
		/* sun light */
		glLightfv (GL_LIGHT0, GL_AMBIENT, light_ambient);
		glLightfv (GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	/* no specular reflection from sun, it is too distracting */
		glLightfv (GL_LIGHT0, GL_SPECULAR, light_full_off);
	} else {
		glLightfv (GL_LIGHT0, GL_AMBIENT, light_full_on);
		glLightfv (GL_LIGHT0, GL_DIFFUSE, light_full_off);
		glLightfv (GL_LIGHT0, GL_SPECULAR, light_full_off);
	}
	glLightfv (GL_LIGHT0, GL_POSITION, lightPosition);

	/* viewpoint light */
	glLightfv (GL_LIGHT1, GL_POSITION, viewpointLight);
	glLightfv (GL_LIGHT1, GL_AMBIENT, light_ambient);
	glLightfv (GL_LIGHT1, GL_DIFFUSE, light_diffuse);
	glLightfv (GL_LIGHT1, GL_SPECULAR, light_specular);
	glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.5);

	
	glEnable (GL_LIGHTING);
	glEnable (GL_LIGHT0);
	glEnable (GL_LIGHT1);

	glEnable(GL_DEPTH_TEST);

}

/* pass in the number representing the colour, sets OpenGL materials */
void setObjectColour(int colourID) {
	/* predefined colours */
	GLfloat blue[]  = {0.0, 0.0, 1.0, 1.0};
	GLfloat red[]   = {1.0, 0.0, 0.0, 1.0};
	GLfloat green[] = {0.0, 1.0, 0.0, 1.0};
	GLfloat yellow[]   = {1.0, 1.0, 0.0, 1.0};
	GLfloat purple[]   = {1.0, 0.0, 1.0, 1.0};
	GLfloat orange[]   = {1.0, 0.64, 0.0, 1.0};
	GLfloat white[] = {1.0, 1.0, 1.0, 1.0};
	GLfloat black[] = {0.0, 0.0, 0.0, 1.0};
	GLfloat dblue[]  = {0.0, 0.0, 0.5, 1.0};
	GLfloat dred[]   = {0.5, 0.0, 0.0, 1.0};
	GLfloat dgreen[] = {0.0, 0.5, 0.0, 1.0};
	GLfloat dyellow[]   = {0.5, 0.5, 0.0, 1.0};
	GLfloat dpurple[]   = {0.5, 0.0, 0.5, 1.0};
	GLfloat dorange[]   = {0.5, 0.32, 0.0, 1.0};
	GLfloat brown[]     = {0.5, 0.37, 0.05, 1.0}; //?
	GLfloat grey[]      = {0.5, 0.5, 0.5, 1.0};


	/* system defined colours are numbers 1 to 8 */
	/* user defined colours are 9-99 */
	if (colourID == 1) {
		glMaterialfv(GL_FRONT, GL_AMBIENT, dgreen);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, green);
	}
	else if (colourID == 2) { 
		glMaterialfv(GL_FRONT, GL_AMBIENT, dblue);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, blue);
	}
	else if (colourID == 3) {
		glMaterialfv(GL_FRONT, GL_AMBIENT, dred);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, red);
	}
	else if (colourID == 4) {
		glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, black);
	}
	else if (colourID == 5) {
		glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);
	}
	else if (colourID == 6) {
		glMaterialfv(GL_FRONT, GL_AMBIENT, dpurple);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, purple);
	}
	else if (colourID == 7) { 
		glMaterialfv(GL_FRONT, GL_AMBIENT, dorange);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, orange);
	}
	else if (colourID == 8) { 
		glMaterialfv(GL_FRONT, GL_AMBIENT, dyellow);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, yellow);
	}
	else if (colourID == 9) {
		glMaterialfv(GL_FRONT, GL_AMBIENT, brown);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, brown);
	}
	else if (colourID == 10) {
		glMaterialfv(GL_FRONT, GL_AMBIENT, grey);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, grey);
	}
		else {
		/* user define colour */
		if (uColourUsed[ colourID ] != 1) {
			printf("ERROR, attempt to access colour which is not allocated.\n");
		}
		/* user defined colours, look up the RGBA colour values */
		/* for the world value in the user defined colour array */
		glMaterialfv(GL_FRONT, GL_AMBIENT, uAmbColour[ colourID ]);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, uDifColour[ colourID ]);
	}
}

	/* activate texture using textureid stored in colourID */
void setObjectTexture(int colourID) {
	GLfloat white[] = {1.0, 1.0, 1.0, 1.0};
		/* if a texture is bound to that colour then enable texturing */
	if (textureAssigned[colourID] != -1) {
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, textureID[textureAssigned[colourID]]);
		/* if textured, then use white as base colour */
	//      glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);
	}

}


void unsetObjectTexture(int colourID) {
		/* if a texture is bound to that colour then disable texturing */
	if (textureAssigned[colourID] != -1) {
		glDisable(GL_TEXTURE_2D);
	}
}


/* draw cube in world[i][j][k] */
void drawCube(int i, int j, int k) {
		// colour/texture number for this cube
	int colourId;
		// texture coordinates
	float umin, umax, vmin, vmax;


	GLfloat white[] = {1.0, 1.0, 1.0, 1.0};
	//glMaterialfv(GL_FRONT, GL_SPECULAR, white);
	colourId = world[i][j][k];
		/* select colour based on value in the world array */
   setObjectColour(colourId);
		/* set texture */
   setObjectTexture(colourId);

   glPushMatrix ();
	/* offset cubes by 0.5 so the centre of the */
	/* cube falls in the centre of the world array */ 
   glTranslatef(i + 0.5, j + 0.5, k + 0.5);


	/* calculate the texture offsets, used for shifted/animating textures */
	/* modulus 1.0 to prevent texture coordinates from becoming to large */
   umin = fmodf(0.0 + tOffset[colourId][0], 1.0);
   umax = fmodf(1.0 + tOffset[colourId][0], 1.0);
   vmin = fmodf(0.0 + tOffset[colourId][1], 1.0);
   vmax = fmodf(1.0 + tOffset[colourId][1], 1.0);

		// draw cube
		// side 1
   glNormal3f(1.0, 0.0, 0.0);
   glBegin(GL_TRIANGLES);
	  glTexCoord2f(0.0 + umin, 0.0 + vmax);
	  glVertex3f(0.5, 0.5, 0.5);
	  glTexCoord2f(1.0 + umax, 1.0 + vmin);
	  glVertex3f(0.5, -0.5, -0.5);
	  glTexCoord2f(0.0 + umin, 1.0 + vmin);
	  glVertex3f(0.5, -0.5, 0.5);
   glEnd();

   glBegin(GL_TRIANGLES);
	  glTexCoord2f(0.0 + umin, 0.0 + vmax);
	  glVertex3f(0.5, 0.5, 0.5);
	  glTexCoord2f(1.0 + umax, 0.0 + vmax);
	  glVertex3f(0.5, 0.5, -0.5);
	  glTexCoord2f(1.0 + umax, 1.0 + vmin);
	  glVertex3f(0.5, -0.5, -0.5);
   glEnd();

		// side 2
   glNormal3f(0.0, 0.0, 1.0);
   glBegin(GL_TRIANGLES);
	  glTexCoord2f(0.0 + umin, 0.0 + vmax);
	  glVertex3f(-0.5, 0.5, 0.5);
	  glTexCoord2f(1.0 + umax, 1.0 + vmin);
	  glVertex3f(0.5, -0.5, 0.5);
	  glTexCoord2f(0.0 + umin, 1.0 + vmin);
	  glVertex3f(-0.5, -0.5, 0.5);
   glEnd();

   glBegin(GL_TRIANGLES);
	  glTexCoord2f(0.0 + umin, 0.0 + vmax);
	  glVertex3f(-0.5, 0.5, 0.5);
	  glTexCoord2f(1.0 + umax, 0.0 + vmax);
	  glVertex3f(0.5, 0.5, 0.5);
	  glTexCoord2f(1.0 + umax, 1.0 + vmin);
	  glVertex3f(0.5, -0.5, 0.5);
   glEnd();

		// side 3
   glNormal3f(-1.0, 0.0, 0.0);
   glBegin(GL_TRIANGLES);
	  glTexCoord2f(0.0 + umin, 0.0 + vmax);
	  glVertex3f(-0.5, 0.5, -0.5);
	  glTexCoord2f(1.0 + umax, 1.0 + vmin);
	  glVertex3f(-0.5, -0.5, 0.5);
	  glTexCoord2f(0.0 + umin, 1.0 + vmin);
	  glVertex3f(-0.5, -0.5, -0.5);
   glEnd();

   glBegin(GL_TRIANGLES);
	  glTexCoord2f(0.0 + umin, 0.0 + vmax);
	  glVertex3f(-0.5, 0.5, -0.5);
	  glTexCoord2f(1.0 + umax, 0.0 + vmax);
	  glVertex3f(-0.5, 0.5, 0.5);
	  glTexCoord2f(1.0 + umax, 1.0 + vmin);
	  glVertex3f(-0.5, -0.5, 0.5);
   glEnd();

		// side 4
   glNormal3f(0.0, 0.0, -1.0);
   glBegin(GL_TRIANGLES);
	  glTexCoord2f(0.0 + umin, 0.0 + vmax);
	  glVertex3f(0.5, 0.5, -0.5);
	  glTexCoord2f(1.0 + umax, 1.0 + vmin);
	  glVertex3f(-0.5, -0.5, -0.5);
	  glTexCoord2f(0.0 + umin, 1.0 + vmin);
	  glVertex3f(0.5, -0.5, -0.5);
   glEnd();

   glBegin(GL_TRIANGLES);
	  glTexCoord2f(0.0 + umin, 0.0 + vmax);
	  glVertex3f(0.5, 0.5, -0.5);
	  glTexCoord2f(1.0 + umax, 0.0 + vmax);
	  glVertex3f(-0.5, 0.5, -0.5);
	  glTexCoord2f(1.0 + umax, 1.0 + vmin);
	  glVertex3f(-0.5, -0.5, -0.5);
   glEnd();

		// side 5 - top
   glNormal3f(0.0, 1.0, 0.0);
   glBegin(GL_TRIANGLES);
	  glTexCoord2f(0.0 + umin, 0.0 + vmax);
	  glVertex3f(-0.5, 0.5, -0.5);
	  glTexCoord2f(1.0 + umax, 1.0 + vmin);
	  glVertex3f(0.5, 0.5, 0.5);
	  glTexCoord2f(0.0 + umin, 1.0 + vmin);
	  glVertex3f(-0.5, 0.5, 0.5);
   glEnd();

   glBegin(GL_TRIANGLES);
	  glTexCoord2f(0.0 + umin, 0.0 + vmax);
	  glVertex3f(-0.5, 0.5, -0.5);
	  glTexCoord2f(1.0 + umax, 0.0 + vmax);
	  glVertex3f(0.5, 0.5, -0.5);
	  glTexCoord2f(1.0 + umax, 1.0 + vmin);
	  glVertex3f(0.5, 0.5, 0.5);
   glEnd();

		// side 6 - bottom
   glNormal3f(0.0, -1.0, 0.0);
   glBegin(GL_TRIANGLES);
	  glTexCoord2f(0.0 + umin, 0.0 + vmax);
	  glVertex3f(-0.5, -0.5, 0.5);
	  glTexCoord2f(1.0 + umax, 1.0 + vmin);
	  glVertex3f(0.5, -0.5, -0.5);
	  glTexCoord2f(0.0 + umin, 1.0 + vmin);
	  glVertex3f(-0.5, -0.5, -0.5);
   glEnd();

   glBegin(GL_TRIANGLES);
	  glTexCoord2f(0.0 + umin, 0.0 + vmax);
	  glVertex3f(-0.5, -0.5, 0.5);
	  glTexCoord2f(1.0 + umax, 0.0 + vmax);
	  glVertex3f(0.5, -0.5, 0.5);
	  glTexCoord2f(1.0 + umax, 1.0 + vmin);
	  glVertex3f(0.5, -0.5, -0.5);
   glEnd();

   unsetObjectTexture(world[i][j][k]);

   glPopMatrix ();
}



/* called each time the world is redrawn */
void display (void)
{
	GLfloat skyblue[]  = {0.52, 0.74, 0.84, 1.0};
	GLfloat black[] = {0.0, 0.0, 0.0, 1.0};
	GLfloat red[] = {1.0, 0.0, 0.0, 1.0};
	GLfloat gray[] = {0.3, 0.3, 0.3, 1.0};
	GLfloat white[] = {1.0, 1.0, 1.0, 1.0};
	int i, j, k;
	int meshNumber;

	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* position viewpoint based on mouse rotation and keyboard 
		translation */
	glLoadIdentity();

	if (fixedVP) {
	// Fixed position - mvx =90; mvy =0; mvz =0;
	// vpx =-50; vpy =-98; vpz =-50;
		glRotatef(90.0, 1.0, 0.0, 0.0);
		glRotatef(0.0, 0.0, 1.0, 0.0);
		glRotatef(0.0, 0.0, 0.0, 1.0);
		glTranslatef(-50.0, -98.0, -50.0);
	} else {
		glRotatef(mvx, 1.0, 0.0, 0.0);
		glRotatef(mvy, 0.0, 1.0, 0.0);
		glRotatef(mvz, 0.0, 0.0, 1.0);
	/* Subtract 0.5 to raise viewpoint slightly above objects. */
	/* Gives the impression of a head on top of a body. */
		glTranslatef(vpx, vpy - 0.5, vpz);
	//   glTranslatef(vpx, vpy, vpz);
	}

	buildDisplayList();


	/* set viewpoint light position */
	viewpointLight[0] = -vpx;
	viewpointLight[1] = -vpy;
	viewpointLight[2] = -vpz;
	glLightfv (GL_LIGHT1, GL_POSITION, viewpointLight);

	/* draw surfaces as either smooth or flat shaded */
	if (smoothShading == 1)
		glShadeModel(GL_SMOOTH);
	else
		glShadeModel(GL_FLAT);

	/* draw polygons as either solid or outlines */
	if (lineDrawing == 1)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else 
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


	/* give all objects the same shininess value and specular colour */
	glMaterialf(GL_FRONT, GL_SHININESS, 90.0);

	/* set starting location of objects */
	glPushMatrix ();

	/* make a blue sky cube */
	glShadeModel(GL_SMOOTH);
	/* turn off all reflection from sky so it is a solid colour */
	glMaterialfv(GL_FRONT, GL_AMBIENT, black);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, black);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, skyblue);
	glPushMatrix ();
	/* move the sky cube center to middle of world space */
	glTranslatef((float)WORLDX/2.0, (float)WORLDY/2.0, (float)WORLDZ/2.0);
	//glutSolidCube(150.0);
	glutSolidCube(skySize);
	glPopMatrix ();
	glShadeModel(GL_SMOOTH);
	/* turn off emision lighting, use only for sky */
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, black);

	/* draw mobs in the world */
	for(i=0; i<MOB_COUNT; i++) {
		if (mobVisible[i] == 1) {
			glPushMatrix();
		/* black body */
			glTranslatef(mobPosition[i][0]+0.5, mobPosition[i][1]+0.5,
					mobPosition[i][2]+0.5);
			glMaterialfv(GL_FRONT, GL_AMBIENT, black);
			glMaterialfv(GL_FRONT, GL_DIFFUSE, gray);
			glutSolidSphere(0.5, 8, 8);
		/* white eyes */
			glRotatef(mobPosition[i][3], 0.0, 1.0, 0.0);
			glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);
			glTranslatef(0.3, 0.1, 0.3);
			glutSolidSphere(0.1, 4, 4);
			glTranslatef(-0.6, 0.0, 0.0);
			glutSolidSphere(0.1, 4, 4);
			glPopMatrix();
		}
	}

		/* draw mesh objects */
   for(i=0; i<MAXMESH; i++ ) {
		/* if mesh instantiated the draw */
	  if((meshUsed[i]  == 1) && (userMesh[i].drawMesh == 1)) {
		/* for each user instantiated mesh, draw in the world */
		 glPushMatrix();
		 glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, white);
		 glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, white);


		 glEnableClientState(GL_VERTEX_ARRAY);
		 if (meshobj[1].ncount > 1)
			glEnableClientState(GL_NORMAL_ARRAY);
		 if (meshobj[1].tcount > 1)
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		 glTranslatef(userMesh[i].xpos, userMesh[i].ypos, userMesh[i].zpos);
		 glRotatef(userMesh[i].xrot, 1.0, 0.0, 0.0);
		 glRotatef(userMesh[i].yrot, 0.0, 1.0, 0.0);
		 glRotatef(userMesh[i].zrot, 0.0, 0.0, 1.0);
		 glScalef(userMesh[i].scale, userMesh[i].scale, userMesh[i].scale);

		 meshNumber = userMesh[i].meshNumber;
		 glVertexPointer(3, GL_FLOAT, 0, meshobj[meshNumber].svdata);

		 if (meshobj[0].ncount > 1)
			glNormalPointer(GL_FLOAT, 0, meshobj[meshNumber].sndata);

		 if (meshobj[0].tcount > 1) {
			glTexCoordPointer(2, GL_FLOAT, 0, meshobj[meshNumber].stdata);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, meshtextureID[meshNumber]);
		 }

		 glDrawArrays(GL_TRIANGLES, 0, meshobj[meshNumber].icount * 3);

		 glDisableClientState(GL_VERTEX_ARRAY);
		 if (meshobj[0].ncount > 1)
			glDisableClientState(GL_NORMAL_ARRAY);
		 if (meshobj[0].tcount > 1) {
			glDisable(GL_TEXTURE_2D);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		 }
		 glPopMatrix();
	  }  
   }


	/* draw players in the world */
	for(i=0; i<PLAYER_COUNT; i++) {
		if (playerVisible[i] == 1) {
			glPushMatrix();
		/* white body */
			glTranslatef(playerPosition[i][0]+0.5, playerPosition[i][1]+0.5,
					playerPosition[i][2]+0.5);
			glMaterialfv(GL_FRONT, GL_AMBIENT, white);
			glMaterialfv(GL_FRONT, GL_DIFFUSE, gray);
			glutSolidSphere(0.5, 8, 8);
		/* white eyes */
			glRotatef(playerPosition[i][3], 0.0, 1.0, 0.0);
			glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, red);
			glTranslatef(0.3, 0.1, 0.3);
			glutSolidSphere(0.1, 4, 4);
			glTranslatef(-0.6, 0.0, 0.0);
			glutSolidSphere(0.1, 4, 4);
			glPopMatrix();
		}
	}


	/* draw tubes in the world */
	for(i=0; i<TUBE_COUNT; i++) {
		if (tubeVisible[i] == 1) {
			glPushMatrix();
			setObjectColour(tubeColour[i]);
			glBegin(GL_QUADS);
			glVertex3f(tubeData[i][0]-0.1, tubeData[i][1], tubeData[i][2]);
			glVertex3f(tubeData[i][0]+0.1, tubeData[i][1], tubeData[i][2]);
			glVertex3f(tubeData[i][3]+0.1, tubeData[i][4], tubeData[i][5]);
			glVertex3f(tubeData[i][3]-0.1, tubeData[i][4], tubeData[i][5]);

			glVertex3f(tubeData[i][0], tubeData[i][1]-0.1, tubeData[i][2]);
			glVertex3f(tubeData[i][0], tubeData[i][1]+0.1, tubeData[i][2]);
			glVertex3f(tubeData[i][3], tubeData[i][4]+0.1, tubeData[i][5]);
			glVertex3f(tubeData[i][3], tubeData[i][4]-0.1, tubeData[i][5]);

			glVertex3f(tubeData[i][0], tubeData[i][1], tubeData[i][2]-0.1);
			glVertex3f(tubeData[i][0], tubeData[i][1], tubeData[i][2]+0.1);
			glVertex3f(tubeData[i][3], tubeData[i][4], tubeData[i][5]+0.1);
			glVertex3f(tubeData[i][3], tubeData[i][4], tubeData[i][5]-0.1);
			glEnd();
			glPopMatrix();
		}
	}

	/* draw all cubes in the world array */
	if (displayAllCubes == 1) {
	/* draw all cubes */
		for(i=0; i<WORLDX; i++) {
			for(j=0; j<WORLDY; j++) {
				for(k=0; k<WORLDZ; k++) {
					if (world[i][j][k] != 0) {
						drawCube(i, j, k);
					}
				}
			}
		}
	} else {
		/* draw only the cubes in the displayList */
		/* these should have been selected in the update function */

		for(i=0; i<displayCount; i++) {
			drawCube(displayList[i][0],
					displayList[i][1],
					displayList[i][2]);
		}
	}



	/* 2D drawing section used to create interface components */
	/* change to orthographic mode to display 2D images */
	glMatrixMode (GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity ();
	gluOrtho2D(0, screenWidth, 0, screenHeight);
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();

	/* turn on alpha blending for 2D */
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glShadeModel(GL_FLAT);
	glNormal3f(0.0, 0.0, -1.0);

	/* call user's 2D drawing function */
	draw2D();

	/* reset graphics for 3D drawing */
	glDisable(GL_BLEND);

	glMatrixMode (GL_PROJECTION);
	glPopMatrix();

	glMatrixMode (GL_MODELVIEW);
	glPopMatrix();
	/* end 2d display code */

	glutSwapBuffers();
}

/* sets viewport information */
void reshape(int w, int h)
{
	glViewport (0, 0, (GLsizei) w, (GLsizei) h);
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	/* use skySize for far clipping plane */
	gluPerspective(45.0, (GLfloat)w/(GLfloat)h, 0.1, skySize*1.5);
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();
	/* set global screen width and height */
	screenWidth = w;
	screenHeight = h;

}

/* respond to keyboard events */
void keyboard(unsigned char key, int x, int y)
{
	float rotx, roty;
	float a, b, c = 0.0;
	static int lighton = 1;

	switch (key) {
		case 'p':
			getViewPosition(&a, &b, &c);
			printf("viewpos: %d %d %d\n", (int)a,(int)b,(int)c);
			break;
		case 27:
		case 'q':
			exit(0);
			break;
		case '1':		// draw polygons as outlines
			lineDrawing = 1;
			lighting = 0;
			smoothShading = 0;
			textures = 0;
			init();
			glutPostRedisplay();
			break;
		case '2':		// draw polygons as filled
			lineDrawing = 0;
			lighting = 0;
			smoothShading = 0;
			textures = 0;
			init();
			glutPostRedisplay();
			break;
		case '3':		// diffuse and specular lighting, flat shading
			lineDrawing = 0;
			lighting = 1;
			smoothShading = 0;
			textures = 0;
			init();
			glutPostRedisplay();
			break;
		case '4':		// diffuse and specular lighting, smooth shading
			lineDrawing = 0;
			lighting = 1;
			smoothShading = 1;
			textures = 0;
			init();
			glutPostRedisplay();
			break;
		case '5':		// texture with  smooth shading
			lineDrawing = 0;
			lighting = 1;
			smoothShading = 1;
			textures = 1;
			init();
			glutPostRedisplay();
			break;
		case 'W':
		case 'w':		// forward motion
			oldvpx = vpx;
			oldvpy = vpy;
			oldvpz = vpz;
			rotx = (mvx / 180.0 * 3.141592);
			roty = (mvy / 180.0 * 3.141592);
			vpx -= sin(roty) * 0.3;
			// turn off y motion so you can't fly
			if (flycontrol == 1)
				vpy += sin(rotx) * 0.3;
			vpz += cos(roty) * 0.3;
	 		collisionResponse();
			glutPostRedisplay();
			break;
		case 'S':
		case 's':		// backward motion
			oldvpx = vpx;
			oldvpy = vpy;
			oldvpz = vpz;
			rotx = (mvx / 180.0 * 3.141592);
			roty = (mvy / 180.0 * 3.141592);
			vpx += sin(roty) * 0.3;
			// turn off y motion so you can't fly
			if (flycontrol == 1)
				vpy -= sin(rotx) * 0.3;
			vpz -= cos(roty) * 0.3;
	 		collisionResponse();
			glutPostRedisplay();
			break;
		case 'A':
		case 'a':		// strafe left motion
			oldvpx = vpx;
			oldvpy = vpy;
			oldvpz = vpz;
			roty = (mvy / 180.0 * 3.141592);
			vpx += cos(roty) * 0.3;
			vpz += sin(roty) * 0.3;
	 		collisionResponse();
			glutPostRedisplay();
			break;
		case 'D':
		case 'd':		// strafe right motion
			oldvpx = vpx;
			oldvpy = vpy;
			oldvpz = vpz;
			roty = (mvy / 180.0 * 3.141592);
			vpx -= cos(roty) * 0.3;
			vpz -= sin(roty) * 0.3;
	 		collisionResponse();
			glutPostRedisplay();
			break;
		case 'f':		// toggle flying controls
			if (flycontrol == 0) flycontrol = 1;
			else flycontrol = 0;
			break;
		case 'c':
			collisions = !collisions;
			break;
		case ' ':		// toggle space flag
			space = 1;
			break;
		case 'm':		// toggle map display, 0=none, 1=small, 2=large
			displayMap++;
			if (displayMap > 2)
				displayMap = 0;
			break;
		case 'g':
			usegravity = !usegravity;
			break;
		case '0':		// toggle viewpoint motion, 0=on, 1=off
			if (fixedVP == 0)
				fixedVP = 1;
			else
				fixedVP = 0;
			break;
	}
}
	/* load mesh from obj file */
void loadMesh() {
		// structures directory operations and reading files
DIR *dp;
struct dirent * fname;
	// obj file name
char objName[128];
	// path/objectfilename
char pathName[128];
	// length of obj file name
int length;
	// count number of loaded mesh
int count;
	// return value from reading obj file, readObjFile()
int retValue;
	// used to find . in obj file name
char *ptr;
	// number from file name
int mNumber;

	// find obj files in the ~/models directory 
   dp = opendir("./models");
	// model count
   count = 0;
   while((fname = readdir(dp)) != NULL) {
	  length = strlen(fname->d_name);
	  strcpy(objName, fname->d_name);
		// file needes to be at least five characters long, e.g. x.obj
		// suffix must be .obj or .OBJ
	  if ((length > 4)
			&& (strncmp(objName, "._", 2) != 0) 
			&& ((strcmp(&(objName[length-4]), ".obj") == 0)
			|| (strcmp(&(objName[length-4]), ".OBJ") == 0))  ) {
		// mesh file found, name stored in fname->d_name
		// create path and filename in pathName
		 pathName[0] = '\0';
		 strcpy(pathName, "./models/");
		 strcat(pathName, objName);

		// extract mesh number from file name
		 ptr = strchr(objName, '.');
		 *ptr = '\0';
		 sscanf(objName, "%d", &mNumber);

		// load mesh data from obj file
		 retValue = readObjFile(pathName, &(meshobj[mNumber]));
		 if (retValue == 1) {
			printf("ERROR, failed to load mesh, %s\n", pathName);
		 }
		 count++;

	  }
   }  // while readdir

   closedir(dp);
   meshcount = count;
}

	/* load a textures from a file */
	// dirname, the directory containing the textures
	// tID, the array that stores the opengl texture ids
	// texcount, number of loaded textures
void loadTexture(char *dirname, GLuint tID[], int *texcount, int tUsed[]) {
FILE *fp;
int  i, j;
int  red, green, blue;

	// structures directory operations and reading files
DIR *dp;
struct dirent * fname;
int length;
	// name of file and pathname to file of texture
char ppmName[128], pathName[128];
	// input line from file, maximum length is 256 characters
char line[256];
	// points to the . in the string containing the file name
char *ptr;
	// number of loaded textures
int count = 0;
	// texture number, taken from file name
int tNumber;
	// if image depth not 255 then use this to make it 255
int divisor;
	// image size values
int wd, ht, depth;
	// text/binary file flag
int binFile;
	// counts bytes in timage array
int tcount;
	// holds texture image read from file
GLubyte  *timage;

		// allocate space to store texture image
		// size = width * height * 4 colours
   timage = (GLubyte *) malloc(sizeof(GLubyte) * TEXTURESIZE * TEXTURESIZE * 4);

		// find files in the textures directory ending with
		// either .ppm or .PPM
   dp = opendir(dirname);
   while((fname = readdir(dp)) != NULL) {
	  length = strlen(fname->d_name);
	  strcpy(ppmName, fname->d_name);
		// file needes to be at least five characters long, e.g. x.ppm
		// suffix must be .ppm or .PPM
	  if ((length > 4)
			&& (strncmp(ppmName, "._", 2) != 0) 
			&& ((strcmp(&(ppmName[length-4]), ".ppm") == 0)
			|| (strcmp(&(ppmName[length-4]), ".PPM") == 0))  ) {
		// texture file found, name stored in fname->d_name
		// create path and filename in pathName
		 pathName[0] = '\0';
		 strcpy(pathName, dirname);
		 strcat(pathName, ppmName);

		// open texture file and load texture
		 if ((fp = fopen(pathName, "r")) == 0) {
			printf("Error, failed to find the file named %s.\n", pathName);
			exit(0);
		 } 
		// read header file type P3
		 fgets(line, 256, fp);
		 if ((strncmp(line, "P3", 2) != 0) && (strncmp(line, "P6", 2) != 0)) {
			 printf("File %s not P3 or P6 format ppm file\n", pathName);
			 exit(1);
		 }
		 if (strncmp(line, "P3", 2) == 0) 
			binFile = 0; 
		 else
			binFile = 1;

		// read comment line
		 fgets(line, 256, fp);
		// read past comment lines starting with #
		 while (line[0] == '#') fgets(line, 256, fp);
		// read width, height, and depth of pixels
		 sscanf(line, "%d %d", &wd, &ht);
		 fgets(line, 256, fp);
		 if ((wd != ht) || (wd > TEXTURESIZE)) {
			printf("Texture %s too large. Increase TEXTURESIZE to %d.\n", 
			   pathName, wd);
			exit(1);
		 }
		// read past comment lines starting with #
		 while (line[0] == '#') fgets(line, 256, fp);
		 sscanf(line, "%d", &depth);
		 divisor = 1;
		 if (depth != 255) {
			divisor = depth / 256;
		 }

		// set byte counter for image to zero
		 tcount = 0;
		// load texture data into image array
		 if (binFile ==0) {
		// texture file
			for(i=0; i<ht; i++) {
			   for(j=0; j<wd; j++) {
				  fgets(line, 256, fp);
				  sscanf(line, "%d", &red);
				  fgets(line, 256, fp);
				  sscanf(line, "%d", &green);
				  fgets(line, 256, fp);
				  sscanf(line, "%d", &blue);

				  red /= divisor;
				  blue /= divisor;
				  green /= divisor;

				  timage[tcount] = red;
				  timage[tcount+ 1] = green;
				  timage[tcount + 2] = blue;
				  timage[tcount + 3] = 255;

				  tcount+=4;
			   }
			}
		 } else {
		// binary file
			for(i=0; i<ht; i++) {
			   for(j=0; j<wd; j++) {
				  fread(&red, sizeof(char), 1, fp);
				  fread(&green, sizeof(char), 1, fp);
				  fread(&blue, sizeof(char), 1, fp);

				  red /= divisor;
				  blue /= divisor;
				  green /= divisor;

				  timage[tcount] = red;
				  timage[tcount+ 1] = green;
				  timage[tcount + 2] = blue;
				  timage[tcount + 3] = 255;

				  tcount+=4;
			   }
			}
		 }

		 fclose(fp);

		// extract texture number from file name
		 ptr = strchr(ppmName, '.');
		 *ptr = '\0';
		 sscanf(ppmName, "%d", &tNumber);
		 tUsed[tNumber] = 1;

		// create OpenGL texture
		 glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		 glGenTextures(1, &(tID[tNumber]));
		 glBindTexture(GL_TEXTURE_2D, tID[tNumber]);
		 count++;
		 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// filtering of textures is turned off, 
		 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		 glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, wd, ht, 0, GL_RGBA,
			GL_UNSIGNED_BYTE, timage);
		 glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
//         glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	  } // load a texture

   }  // while readdir

   closedir(dp);
   free(timage);
   *texcount = count;

}

/* responds to mouse movement when a button is pressed */
void motion(int x, int y) {
	/* update current mouse movement but don't use to change the viewpoint*/
	oldx = x;
	oldy = y;
}

/* responds to mouse movement when a button is not pressed */
void passivemotion(int x, int y) {
	mvx += (float) y - oldy;
	mvy += (float) x - oldx;
	oldx = x;
	oldy = y;
	glutPostRedisplay();
}



/* initilize graphics information and mob data structure */
void graphicsInit(int *argc, char **argv) {
	int i, fullscreen;
	// directory for textures
	char dirName[128];
	/* set GL window information */
	glutInit(argc, argv);
	glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);

	/* parse command line args */
	fullscreen = 0;
	for(i=1; i<*argc; i++) { //i could use my systems programming knowledge to improve this..
		if (strcmp(argv[i], "-nofly") == 0)
			flycontrol = 0;
		if (strcmp(argv[i],"-full") == 0)
			fullscreen = 1;
		if (strcmp(argv[i],"-drawall") == 0)
			displayAllCubes = 1;
		if (strcmp(argv[i],"-testworld") == 0)
			testWorld = 1;
		if (strcmp(argv[i],"-fps") == 0)
			fps = 1;
		if (strcmp(argv[i],"-client") == 0)
			netClient = 1;
		if (strcmp(argv[i],"-server") == 0)
			netServer = 1;
		if (strcmp(argv[i], "-displayMap")) {
			if (argv[i + 1]) { //check it's valid first
				displayMap = atoi(argv[i + 1]) % 3; //idc what it does if its not in the range lol
			} //dM @ 0: no map, @ 1: no fow, @ 2: fow
			else {
				displayMap = 1; //defaulting to show the whole map
			}
		}
		if (strcmp(argv[i],"-help") == 0) {
			printf("Usage: a4 [-full] [-drawall] [-testworld] [-fps] [-client] [-server] [-displayMap [0-2]]\n");
			exit(0);
		}
	}

	if (fullscreen == 1) {
		glutGameModeString("1024x768:32@75");
		glutEnterGameMode();
	} else {
		glutInitWindowSize (screenWidth, screenHeight);
		glutCreateWindow (argv[0]);
	}

	init();

		/* initialize all textures as empty == 0 */
   for (i=0; i<NUMBERTEXTURES; i++)
	  textureUsed[i] = 0;
	/* initialize assigned textures to -1, unassigned */
	/* initialize texture offsets to 0.0, no offset */
   for (i=0; i<NUMBERCOLOURS; i++) {
	  textureAssigned[i] = -1;
	  tOffset[i][0] = 0.0;
	  tOffset[i][1] = 0.0;
	}

	/* load textures for cubes */
   strcpy(dirName, "./textures/");
   loadTexture(dirName, textureID, &textureCount, textureUsed);

	/* load textures for mesh */
   strcpy(dirName, "./models/");
   loadTexture(dirName, meshtextureID, &meshtextureCount, meshtextureUsed);

	/* load mesh objects from files in ./models dir */
   meshobj = (struct meshStruct *) malloc(sizeof(struct meshStruct) * NUMBERMESH);
   loadMesh();
	/* initialize user mesh information */
	/* set all user mesh as unusued == 0 */
   for(i=0; i<MAXMESH; i++)
	  meshUsed[i] = 0;


	/* attach functions to GL events */
	glutReshapeFunc (reshape);
	glutDisplayFunc(display);
	glutKeyboardFunc (keyboard);
	glutPassiveMotionFunc(passivemotion);
	glutMotionFunc(motion);
	glutMouseFunc(mouse);
	glutIdleFunc(update);


	/* initialize mob and player array to empty */
	initMobArray();
	initPlayerArray();
	initTubeArray();

	/* initialize all user defined colours as unused == 0  */
	for (i=0; i<NUMBERCOLOURS; i++)
		uColourUsed[i] = 0;

	/* set the size of the sky */
	if (WORLDX > WORLDY)
		skySize = (float) WORLDX;
	else
		skySize = (float) WORLDY;
	if (WORLDZ > skySize)
		skySize = (float) WORLDZ;
	skySize *= 2.0;
}

	/* functions to draw 2d images on screen */
void draw2Dline(int x1, int y1, int x2, int y2, int lineWidth) {
	glLineWidth(lineWidth);
	glBegin(GL_LINES);
	glVertex2i(x1, y1);
	glVertex2i(x2, y2);
	glEnd();
	glLineWidth(1);
}

void draw2Dbox(int x1, int y1, int x2, int y2) {
	glBegin(GL_QUADS);
	glVertex2i(x1, y1);
	glVertex2i(x1, y2);
	glVertex2i(x2, y2);
	glVertex2i(x2, y1);
	glEnd();
}

void  draw2Dtriangle(int x1, int y1, int x2, int y2, int x3, int y3) {
	glBegin(GL_TRIANGLES);
	glVertex2i(x1, y1);
	glVertex2i(x2, y2);
	glVertex2i(x3, y3);
	glEnd();
}

void  set2Dcolour(float colourv[]) {
	glMaterialfv(GL_FRONT, GL_EMISSION, colourv);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, colourv);
}



/* Functions for user defined colours */
int setUserColour(int id, GLfloat ambRed, GLfloat ambGreen, GLfloat ambBlue,
  GLfloat ambAlpha, GLfloat difRed, GLfloat difGreen, GLfloat difBlue,
  GLfloat difAlpha) {

	if ((id >= 0) && (id <=8)) {
		printf("ERROR, user defined colours must be greater than 8.\n");
		printf("Colours 0 to 8 are reserved by the system.\n");
		return(1);
	}
	if (id >= NUMBERCOLOURS) {
		printf("ERROR, attempt to setUserColour() with a colour number of %d which is greater than the maximum user colour number %d.\n", id, NUMBERCOLOURS-1);
		return(1);
	}

	/* set flag which indicates colour id has been defined by the user */
	uColourUsed[id] = 1;

	/* store user defined values */
	uAmbColour[id][0] = ambRed;
	uAmbColour[id][1] = ambGreen;
	uAmbColour[id][2] = ambBlue;
	uAmbColour[id][3] = ambAlpha;
	uDifColour[id][0] = difRed;
	uDifColour[id][1] = difGreen;
	uDifColour[id][2] = difBlue;
	uDifColour[id][3] = difAlpha;

	return(0);
}

	/* releases user defined colour at location id */
void unsetUserColour(int id) {
	uColourUsed[id] = 0;
}

void getUserColour(int id, GLfloat *ambRed, GLfloat *ambGreen, GLfloat *ambBlue,
  GLfloat *ambAlpha, GLfloat *difRed, GLfloat *difGreen, GLfloat *difBlue,
  GLfloat *difAlpha) {

	*ambRed = uAmbColour[id][0];
	*ambGreen = uAmbColour[id][1];
	*ambBlue = uAmbColour[id][2];
	*ambAlpha = uAmbColour[id][3];
	*difRed = uDifColour[id][0];
	*difGreen = uDifColour[id][1];
	*difBlue = uDifColour[id][2];
	*difAlpha = uDifColour[id][3];
}

	/* functions to assign a texture to a colour */
	/* colourid is the colour number to assign a texture, textureid
	   is the number of the texture to attach to the colour */
int setAssignedTexture(int colourid, int textureid) {
   if (colourid >= NUMBERCOLOURS) {
	  printf("ERROR, attempt to setTexture() and assign a texture to a colour number of %d which is greater than the maximum user colour number %d.\n", colourid, NUMBERCOLOURS-1);
	  return(1);
   }
   if (textureUsed[textureid] != 1) {
	  printf("ERROR, attempt to setTexture() with a texture id number of %d has not been assigned a texture.\n", textureid);
	  return(1);
   }

	/* set flag which indicates colour id has been defined by the user */
	/* textures are asociated with a colour id */
   uColourUsed[colourid] = 1;

   textureAssigned[colourid] = textureid;
   return(0);
}

	/* turn texture off for an associated colour id */
void unsetAssignedTexture(int id) {
   textureAssigned[id] = -1;
}

	/* returns texture number for an associated colour id */
	/* returns -1 if no texture assigned */
int getAssignedTexture(int id) {
   return(textureAssigned[id]);
}

	/* sets offsets to texture coordinatex u,v for a given colour id */
void setTextureOffset(int id, float uoffset, float voffset) {
   tOffset[id][0] = uoffset;
   tOffset[id][1] = voffset;
}

	/* id is the user defined number for that instance of a mesh, the
			  same mesh can have different instances, each with its own
			  position, rotatio, scale */
	/* meshNumber is the number of the loaded mesh to draw, it
		  corresponds to the file number of the mesh in ~/models/ dir. */
	/* (xpos, ypos, zpos) is the position of the mesh in the world */
void setMeshID(int id, int meshNumber, float xpos, float ypos, float zpos) {

   if (id > MAXMESH-1) {
	  printf("ERROR, setMeshID(), the id must be less than %d\n", MAXMESH);
	  exit(1);
   }
	// set that mesh id as active
   meshUsed[id] = 1;
	// store mesh information 
   userMesh[id].meshNumber = meshNumber;
   userMesh[id].xpos = xpos;
   userMesh[id].ypos = ypos;
   userMesh[id].zpos = zpos;

   userMesh[id].xrot = 0.0;
   userMesh[id].yrot = 0.0;
   userMesh[id].zrot = 0.0;
   userMesh[id].scale = 1.0;
	// set mesh as visible
   userMesh[id].drawMesh = 1;

}

void unsetMeshID(int id) {
   meshUsed[id] = 0;
}

float getx(int id) {return userMesh[id].xpos;}
float gety(int id) {return userMesh[id].ypos;}
float getz(int id) {return userMesh[id].zpos;}

int getVisible(int id) {return userMesh[id].drawMesh;}

void setTranslateMesh(int id, float xpos, float ypos, float zpos) {
   userMesh[id].xpos = xpos;
   userMesh[id].ypos = ypos;
   userMesh[id].zpos = zpos;
}

void setRotateMesh(int id, float xrot, float yrot, float zrot) {
   userMesh[id].xrot = xrot;
   userMesh[id].yrot = yrot;
   userMesh[id].zrot = zrot;
}

void setScaleMesh(int id, float scale) {
   userMesh[id].scale = scale;
}

void drawMesh(int id) {
	// set mesh as visible
   userMesh[id].drawMesh = 1;
}

void hideMesh(int id) {
	// set mesh as invisible - it wont be drawn but it remains in the
	// mesh data structures
   userMesh[id].drawMesh = 0;
}
