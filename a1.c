
/* Derived from scene.c in the The OpenGL Programming Guide */
/* Keyboard and mouse rotation taken from Swiftless Tutorials #23 Part 2 */
/* http://www.swiftless.com/tutorials/opengl/camera2.html */

/* Frames per second code taken from : */
/* http://www.lighthouse3d.com/opengl/glut/index.php?fps */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include "graphics.h"

extern GLubyte  world[WORLDX][WORLDY][WORLDZ];

/* mouse function called by GLUT when a button is pressed or released */
void mouse(int, int, int, int);

/* initialize graphics library */
extern void graphicsInit(int *, char **);

/* lighting control */
extern void setLightPosition(GLfloat, GLfloat, GLfloat);
extern GLfloat* getLightPosition();

/* viewpoint control */
extern void setViewPosition(float, float, float);
extern void getViewPosition(float *, float *, float *);
extern void getOldViewPosition(float *, float *, float *);
extern void setOldViewPosition(float, float, float);
extern void setViewOrientation(float, float, float);
extern void getViewOrientation(float *, float *, float *);

/* add cube to display list so it will be drawn */
extern void addDisplayList(int, int, int);

extern void createMob(int, float, float, float, float);
extern void setMobPosition(int, float, float, float, float);
extern void hideMob(int);
extern void showMob(int);

extern void createPlayer(int, float, float, float, float);
extern void setPlayerPosition(int, float, float, float, float);
extern void hidePlayer(int);
extern void showPlayer(int);

extern void createTube(int, float, float, float, float, float, float, int);
extern void hideTube(int);
extern void showTube(int);

extern void  draw2Dline(int, int, int, int, int);
extern void  draw2Dbox(int, int, int, int);
extern void  draw2Dtriangle(int, int, int, int, int, int);
extern void  set2Dcolour(float []);


/* flag which is set to 1 when flying behaviour is desired */
extern int flycontrol;
/* flag used to indicate that the test world should be used */
extern int testWorld;
/* flag to print out frames per second */
extern int fps;
/* flag to indicate the space bar has been pressed */
extern int space;
/* flag indicates the program is a client when set = 1 */
extern int netClient;
/* flag indicates the program is a server when set = 1 */
extern int netServer; 
/* size of the window in pixels */
extern int screenWidth, screenHeight;
/* flag indicates if map is to be printed */
extern int displayMap;
/* flag indicates use of a fixed viewpoint */
extern int fixedVP;
extern bool usegravity;
/* frustum corner coordinates, used for visibility determination  */
extern float corners[4][3];

/* determine which cubes are visible e.g. in view frustum */
extern void ExtractFrustum();
extern void tree(float, float, float, float, float, float, int);

/* allows users to define colours */
extern int setUserColour(int, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat,
	GLfloat, GLfloat, GLfloat);
void unsetUserColour(int);
extern void getUserColour(int, GLfloat *, GLfloat *, GLfloat *, GLfloat *,
	GLfloat *, GLfloat *, GLfloat *, GLfloat *); 

/********* end of extern variable declarations **************/

/********* utility functions *********/     
//and temp globals
#define QTHEIGHT 24
bool plsnocrash = false;

//utility function. rand should be seeded before this is called, please! if you do this then you're the nicest
int getRandomNumber(int min, int max) {
	return min + (rand() % (max - min + 1));
}
struct coord {
	int x;
	int y;
	int z;
}; //just to make working with this shit easier although i might not end up using it

//TODO: also miny, maxy
struct coord generatePoint(int minx, int maxx, int minz, int maxz) { //i dont care that its shit you made me do this calvert. three rows
	struct coord point;
	point.x = getRandomNumber(minx, maxx);
	point.y = QTHEIGHT;
	point.z = getRandomNumber(minz, maxz);
	printf("Generated point: %d %d %d\n", point.x, point.y, point.z);
	return point;
}

//offsets a point by some value. negative numbers work, obv
struct coord offsetPoint(struct coord a, int offsetx, int offsety, int offsetz) {
	struct coord b;
	b.x = a.x + offsetx;
	b.y = a.y + offsety;
	b.z = a.z + offsetz;
	return b;
}

struct qtNode {
	struct coord bottomleft;
	struct coord topright;
	struct qtNode* nw;
	struct qtNode* ne;
	struct qtNode* sw;
	struct qtNode* se;
};

/*** collisionResponse() ***/
/* -performs collision detection and response */
/*  sets new xyz  to position of the viewpoint after collision */
/* -can also be used to implement gravity by updating y position of vp*/
/* note that the world coordinates returned from getViewPosition()
	will be the negative value of the array indices */
void collisionResponse() {
	float x, y, z = 0.0;
	getViewPosition(&x, &y, &z);
	const float margin = 0.3;
	//the following could definitely be improved:
	if (world[-(int)(x-margin)][-(int)y][-(int)(z-margin)] || world[-(int)(x+margin)][-(int)y][-(int)(z+margin)]) {
		printf("Collision detected at: x:%f y:%f z:%f", x, y, z);
		//going up the stairs takes confidence mr/ms/other TA, it's just like your first time conquering them as a child
		if(world[-(int)(x-margin)][-(int)y + 1][-(int)(z-margin)] == 0 && world[-(int)(x+margin)][-(int)y + 1][-(int)(z+margin)] == 0) {
			printf("up\n");
			getOldViewPosition(&x, &y, &z);
			setViewPosition(x, y-1.0, z);
		} else 		{
			//then we fix it
			getOldViewPosition(&x, &y, &z);
			setViewPosition(x, y, z);
			printf("proposed solution: %f %f %f\n", x, y, z);
		}

	}
}

/******* draw2D() *******/
/* draws 2D shapes on screen */
/* use the following functions: 			*/
/*	draw2Dline(int, int, int, int, int);		*/
/*	draw2Dbox(int, int, int, int);			*/
/*	draw2Dtriangle(int, int, int, int, int, int);	*/
/*	set2Dcolour(float []); 				*/
/* colour must be set before other functions are called	*/
void draw2D() {
	if (testWorld) {
		/* draw some sample 2d shapes */
		if (displayMap == 1) {
			GLfloat green[] = {0.0, 0.5, 0.0, 0.5};
			set2Dcolour(green);
			draw2Dline(0, 0, 500, 500, 15);
			draw2Dtriangle(0, 0, 200, 200, 0, 200);

			GLfloat black[] = {0.0, 0.0, 0.0, 0.5};
			set2Dcolour(black);
			draw2Dbox(500, 380, 524, 388);
		}
	} else {
		/* your code goes here */
		//TODO: your code goes here?
	}
}


/*** update() ***/
/* background process, it is called when there are no other events */
/* -used to control animations and perform calculations while the  */
/*  system is running */
/* -gravity must also implemented here, duplicate collisionResponse */
void update() {
	int i, j, k = 0;
	float* la = NULL;
	float x, y, z = 0.0;
	float rx, ry, rz = 0.0;
	/* sample animation for the testworld, don't remove this code */
	/* demo of animating mobs */
	if (testWorld) {

		/* update old position so it contains the correct value */
		/* -otherwise view position is only correct after a key is */
		/*  pressed and keyboard() executes. */
		#if 0
		// Fire a ray in the direction of forward motion
		float xx, yy, zz;
		getViewPosition(&x, &y, &z);
		getOldViewPosition(&xx, &yy, &zz);
		printf("%f %f %f %f %f %f\n", xx, yy, zz, x, y, z);
		printf("%f %f %f\n",  -xx+((x-xx)*25.0), -yy+((y-yy)*25.0), -zz+((z-zz)*25.0));
		createTube(2, -xx, -yy, -zz, -xx-((x-xx)*25.0), -yy-((y-yy)*25.0), -zz-((z-zz)*25.0), 5);
		#endif

		getViewPosition(&x, &y, &z);
		setOldViewPosition(x,y,z);

		/* sample of rotation and positioning of mob */
		/* coordinates for mob 0 */
		static float mob0x = 50.0, mob0y = 25.0, mob0z = 52.0;
		static float mob0ry = 0.0;
		static int increasingmob0 = 1;
		/* coordinates for mob 1 */
		static float mob1x = 50.0, mob1y = 25.0, mob1z = 52.0;
		static float mob1ry = 0.0;
		static int increasingmob1 = 1;
		/* counter for user defined colour changes */
		static int colourCount = 0;
		static GLfloat offset = 0.0;

		/* move mob 0 and rotate */
		/* set mob 0 position */
		setMobPosition(0, mob0x, mob0y, mob0z, mob0ry);

		/* move mob 0 in the x axis */
		if (increasingmob0 == 1)
			mob0x += 0.2;
		else 
			mob0x -= 0.2;
		if (mob0x > 50) increasingmob0 = 0;
		if (mob0x < 30) increasingmob0 = 1;

		/* rotate mob 0 around the y axis */
		mob0ry += 1.0;
		if (mob0ry > 360.0) mob0ry -= 360.0;

		/* move mob 1 and rotate */
		setMobPosition(1, mob1x, mob1y, mob1z, mob1ry);

		/* move mob 1 in the z axis */
		/* when mob is moving away it is visible, when moving back it */
		/* is hidden */
		if (increasingmob1 == 1) {
			mob1z += 0.2;
			showMob(1);
		} else {
			mob1z -= 0.2;
			hideMob(1);
		}
		if (mob1z > 72) increasingmob1 = 0;
		if (mob1z < 52) increasingmob1 = 1;

		/* rotate mob 1 around the y axis */
		mob1ry += 1.0;
		if (mob1ry > 360.0) mob1ry -= 360.0;

		/* change user defined colour over time */
		if (colourCount == 1) offset += 0.05;
		else offset -= 0.01;
		if (offset >= 0.5) colourCount = 0;
		if (offset <= 0.0) colourCount = 1;
		setUserColour(9, 0.7, 0.3 + offset, 0.7, 1.0, 0.3, 0.15 + offset, 0.3, 1.0);

		/* sample tube creation  */
		/* draws a purple tube above the other sample objects */
		 createTube(1, 45.0, 30.0, 45.0, 50.0, 30.0, 50.0, 6);

	 	/* end testworld animation */


	} else {
		/* your code goes here */
		//why would i put it here i want gravity to exist on the test world too?
	}
	if(plsnocrash && usegravity){
		setOldViewPosition(x,y,z); //i'm like 90% sure this should be called before gvp(i i i)
		getViewPosition(&x, &y, &z);
		getViewOrientation(&rx, &ry, &rz);
		if(world[-(int)x][-(int)(y+1)][-(int)z] == 0) {
			//printf("grabity\n");
			setViewPosition(x,y+0.1,z);
			//collisionResponse();
		}
		setPlayerPosition(0, -x-1, -y, -z-1, -ry);
	}
}


/* called by GLUT when a mouse button is pressed or released */
/* -button indicates which button was pressed or released */
/* -state indicates a button down or button up event */
/* -x,y are the screen coordinates when the mouse is pressed or */
/*  released */ 
void mouse(int button, int state, int x, int y) {

	if (button == GLUT_LEFT_BUTTON)
		printf("left button - ");
	else if (button == GLUT_MIDDLE_BUTTON)
		printf("middle button - ");
	else
		printf("right button - ");

	if (state == GLUT_UP)
		printf("up - ");
	else
		printf("down - ");

	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		printf("da mouse is been clicked");
	}

	printf("%d %d\n", x, y);
}

/*(all ints pls) origin, l w h, colour.
 the origin coords will point to the bottom left of the room*/
void createRoom(struct coord origin, int l, int w, int h, GLbyte colour) {
	//dx, dy, dz = drwaing x,y,z. the n stands for nega
	for (int dy = origin.y; dy <= origin.y + h; dy++) {
		for (int dx = origin.x; dx <= origin.x + w; dx++) {
			world[dx][dy][origin.z] = colour;
		}
		for (int dz = origin.z; dz <= origin.z + l; dz++) {
			world[origin.x][dy][dz] = colour;
		}
		for (int dnx = origin.x; dnx <= origin.x + w; dnx++) {
			world[dnx][dy][origin.z + l] = colour;
		}
		for (int dnz = origin.z; dnz <= origin.z + l; dnz++) {
			world[origin.x + w][dy][dnz] = colour;
		}
	} //TODO: have this make the room floors too
	for(int floorx = origin.x; floorx <= origin.x + w; floorx++) {
		for (int floorz = origin.z; floorz <= origin.z + l; floorz++) {
			world[floorx][origin.y][floorz] = colour;
		}
	}
	//TODO: this is technically inefficient, i could do this all in way less runtime but idc rlly cuz im timed. if i have time i will fix it
} //i like pot_hole but i guess i'll SnakeStance this shit

struct coord getMidpoint(struct coord a, struct coord b) {
	struct coord midpoint;
	midpoint.x = (a.x + b.x) / 2; //int math
	midpoint.y = QTHEIGHT;
	midpoint.z = (a.z + b.z) / 2;
	return midpoint;
} //FIXME: not deprecated because i could use it :)

void initQuadtree(struct qtNode* node, struct coord bottomleft, struct coord topright) {
	node->bottomleft.x = bottomleft.x;
	node->bottomleft.y = bottomleft.y;
	node->bottomleft.z = bottomleft.z;
	node->topright.x = topright.x;
	node->topright.y = topright.y;
	node->topright.z = topright.z;
	node->nw = NULL;
	node->ne = NULL;
	node->sw = NULL;
	node->se = NULL;
} //FIXME: deprecated due to three rows

/*we need to create the inital quadtree as the whole space
this function only requires a struct qtNode* to split */
void splitQuadtree(struct qtNode* node) {
	struct coord mp = getMidpoint(node->bottomleft, node->topright);
	int midX = mp.x;
	int midZ = mp.z;
	//printf("mp.x %d mp.z %d\n", mp.x, mp.z);
	struct coord a;
	struct coord b;
	node->nw = calloc(1, sizeof(struct qtNode));
	a.x = node->bottomleft.x; a.y = QTHEIGHT; a.z = midZ;
	b.x = midX; b.y = QTHEIGHT; b.z = node->topright.z;
	//printf("a: %d %d %d\nb: %d %d %d\n", a.x, a.y, a.z, b.x, b.y, b.z);
	initQuadtree(node->nw, a, b);
	node->ne = calloc(1, sizeof(struct qtNode));
	a.x = midX; a.y = QTHEIGHT; a.z = midZ;
	b.x = node->topright.x; b.y = QTHEIGHT; b.z = node->topright.z;
	//printf("a: %d %d %d\nb: %d %d %d\n", a.x, a.y, a.z, b.x, b.y, b.z);
	initQuadtree(node->ne, a, b);
	node->sw = calloc(1, sizeof(struct qtNode));
	a.x = node->bottomleft.x; a.y = QTHEIGHT; a.z = node->bottomleft.z;
	b.x = midX; b.y = QTHEIGHT; b.z = midZ;
	//printf("a: %d %d %d\nb: %d %d %d\n", a.x, a.y, a.z, b.x, b.y, b.z);
	initQuadtree(node->sw, a, b);
	node->se = calloc(1, sizeof(struct qtNode));
	a.x = midX; a.y = QTHEIGHT; b.z = node->bottomleft.x;
	b.x = node->topright.x; b.y = QTHEIGHT; b.z = midZ;
	//printf("a: %d %d %d\nb: %d %d %d\n", a.x, a.y, a.z, b.x, b.y, b.z);
	initQuadtree(node->se, a, b);
} //FIXME: deprecated due to three rows
	
/*this function repurposes struct coord to store dimension data:
  x is the width, y is (useless lawl), z is the depth. its purpose is
  to calculate the room dimensions for any given quadtree quadrant.
  min offset is at least 5% (int) of the size of the given area
  struct coord bottomleft, struct coord topright*/
struct coord calculateDimensions(struct coord a, struct coord b, int maxOffset) {
	struct coord dimensions;
	int xoffset = ((int)((float)(b.x - a.x))*(0.1)) + (rand() % (maxOffset+1)); //killme
	int zoffset = ((int)((float)(b.z - a.z))*(0.1)) + (rand() % (maxOffset+1));
	printf("offsets: %d %d\n", xoffset, zoffset);
	dimensions.x = (b.x - a.x) - xoffset; //width
	dimensions.y = 8; //these can be further randomized/improved
	dimensions.z = (b.z - a.z) - zoffset;
	return dimensions;
} //FIXME: deprecated due to three rows

struct coord getRandomDimensions(int min, int max) {
	//this is the brain damaged version of the above function. rooms cant exceed max size in any dimension.
	struct coord dimensions;
	dimensions.x = min + (rand() % (max - min + 1));
	dimensions.y = min + (rand() % (max - min + 1));
	dimensions.z = min + (rand() % (max - min + 1));
	printf("generated dimensions: %d %d %d\n", dimensions.x, dimensions.y, dimensions.z);
	return dimensions; //why has god forsaken me i just wanted to use a quadtree.
} //x = w; y = h; z = l

bool proximityCheck(struct coord dimensions, struct coord points[], int pindex, int margin) { //idc idgaf that its bad this is not my fault i just wanted qtree
	bool result = true; //yes i know this doesnt account for the dimensions of point 2 but dont fuCking worry ab it
	for(int i = 0; i < pindex; i++) {
		struct coord point1_max;
		int longer_side = 0;
		point1_max.x = points[pindex].x + dimensions.x;
		point1_max.y = QTHEIGHT;
		point1_max.z = points[pindex].z + dimensions.z;
		struct coord center = getMidpoint(points[pindex], point1_max); //this is the center of our circle;
		if (dimensions.x > dimensions.z) {
			longer_side = dimensions.x;
		} else {longer_side = dimensions.z;} //the longer side len / 2 will be used as the radius
		if ((((points[i].x - center.x) * (points[i].x - center.x)) + ((points[i].z - center.z) * (points[i].z - center.z))) < (((longer_side + margin)/2) * ((longer_side + margin)/2))) {
			result = false;
			break;
		}
	}
	return result;
} //FIXME: deprecated

bool proximityCheck2(struct coord dimensions[], struct coord points[], int pindex, int margin) { //idc idgaf that its bad this is not my fault i just wanted qtree
	bool result = true; //yes i know this doesnt account for the dimensions of point 2 but dont fuCking worry ab it
	if (pindex == 0) return true;
	for(int i = 0; i < pindex; i++) {
		struct coord point1_max;
		struct coord point2_max;
		float r = 0.0;
		float r2 = 0.0;
		point1_max.x = points[pindex].x + dimensions[pindex].x;
		point1_max.y = QTHEIGHT;
		point1_max.z = points[pindex].z + dimensions[pindex].z;
		point2_max.x = points[i].x + dimensions[i].x;
		point2_max.y = QTHEIGHT;
		point2_max.z = points[i].z + dimensions[i].z; 
		struct coord center1 = getMidpoint(points[pindex], point1_max); //this is the center of our first;
		struct coord center2 = getMidpoint(points[i], point2_max);
		printf("a: %d %d %d b: %d %d %d\n", points[pindex].x, points[pindex].y, points[pindex].z, points[i].x, points[i].y, points[i].z);
		if (dimensions[pindex].x > dimensions[pindex].z) {
			r = ((float)(dimensions[pindex].x + margin)) / 2.0 + (float)margin;
		} else {r = ((float)(dimensions[pindex].z + margin)) / 2.0 + (float)margin;} //the longer side len / 2 will be used as the radius
		if(dimensions[i].x > dimensions[i].z) {
			r2 = ((float)(dimensions[i].x)) / 2.0 + (float)margin;
		} else {r2 = ((float)(dimensions[i].z)) / 2.0 + (float)margin;}
		printf("%f %f %lf\n", r, r2, sqrt((float)(((center2.x - center1.x) * (center2.x - center1.x)) + ((center2.z - center2.z) * (center2.z - center1.z)))));
		//if ((((points[i].x - center.x) * (points[i].x - center.x)) + ((points[i].z - center.z) * (points[i].z - center.z))) < (((longer_side + margin)/2) * ((longer_side +  margin)/2))) {
		if (sqrt((float)(((center2.x - center1.x) * (center2.x - center1.x)) + ((center2.z - center2.z) * (center2.z - center1.z)))) < (r + r2)) {
			//if the distance between the two centers is less than the sum of their radii, they intersect
			result = false;
			printf("pc failed\n");
			break;
		}
	}
	return result;
} //FIXME: deprecated: i dont even care


void createDoor(struct coord a, int w, int h) {
	for (int dw = a.x; dw < a.x + w; dw++) {
		for (int dh = a.y + 1; dh < a.y + h; dh++) {
			world[dw][dh][a.z] = 0;
		}
	}
}

void drawCorridor(struct coord a, int l, int w, int h, GLbyte colour) {
	GLbyte drawcolour = colour;
	for(int floorx = a.x; floorx <= a.x + w; floorx++) {
		for (int floorz = a.z; floorz <= a.z + l; floorz++) {
			world[floorx][a.y][floorz] = colour;
			world[floorx][a.y + h][floorz]; //plus or minus uhhh
		}
	}
	for(int wally = a.y; wally <= a.y + h; wally++) {
		for (int wallz = a.z; wallz <= a.z + l; wallz++) {
			world[a.x][wally][wallz] = colour + 1; //leftwall
			world[a.x + w][wally][wallz] = colour + 1; //right wall
		}
	} //O(2n^2). idk if O is the right y but you know
	createDoor(a, w, h);//intro door
	createDoor(offsetPoint(a, 0, 0, l-1), w, h); //exit door
}

int main(int argc, char** argv)
{
	int i, j, k = 0;
	float x, y, z = 0.0;
	//seed random
	srand(time(NULL));
	/* initialize the graphics system */
	graphicsInit(&argc, argv);

	/* the first part of this if statement builds a sample */
	/* world which will be used for testing */
	/* DO NOT remove this code. */
	/* Put your code in the else statment below */
	/* The testworld is only guaranteed to work with a world of
		with dimensions of 100,50,100. */
	if (testWorld == 1) {
	/* initialize world to empty */
		for(i=0; i<WORLDX; i++)
			for(j=0; j<WORLDY; j++)
				for(k=0; k<WORLDZ; k++)
					world[i][j][k] = 0;

		plsnocrash = true;
		/* some sample objects */
		/* build a red platform */
		for(i=0; i<WORLDX; i++) {
			for(j=0; j<WORLDZ; j++) {
				world[i][24][j] = 3;
			}
		}
		/* create some green and blue cubes */
		world[50][25][50] = 1;
		world[49][25][50] = 1;
		world[49][26][50] = 1;
		world[52][25][52] = 2;
		world[52][26][52] = 2;

		/* create user defined colour and draw cube */
		setUserColour(9, 0.7, 0.3, 0.7, 1.0, 0.3, 0.15, 0.3, 1.0);
		world[54][25][50] = 9;


		/* blue box shows xy bounds of the world */
		for(i=0; i<WORLDX-1; i++) {
			world[i][25][0] = 2;
			world[i][25][WORLDZ-1] = 2;
		}
		for(i=0; i<WORLDZ-1; i++) {
			world[0][25][i] = 2;
			world[WORLDX-1][25][i] = 2;
		}

		/* create two sample mobs */
		/* these are animated in the update() function */
		createMob(0, 50.0, 25.0, 52.0, 0.0);
		createMob(1, 50.0, 25.0, 52.0, 0.0);
		struct coord p1; struct coord p2;
		p1.x = 0; p1.y = 50; p1.z = 10;
		p2.x = 10; p2.y = 53; p2.z = 20; 
		drawCorridor(p1, 10, 6, 8, 3);
		/* create sample player */
		//createPlayer(0, 52.0, 27.0, 52.0, 0.0);
		//no. he's creepy
	} else {
		/* your code to build the world goes here */
		//inits the world to empty at first
		for(i=0; i<WORLDX; i++) {
			for(j=0; j<WORLDY; j++) {
				for(k=0; k<WORLDZ; k++) {
					world[i][j][k] = 0;
				}
			}
		}
		plsnocrash = true;
		struct qtNode* head = calloc(1, sizeof(struct qtNode));
		struct coord origin; origin.x = 0; origin.y = QTHEIGHT; origin.z = 0;
		struct coord limit; limit.x = WORLDX; limit.y = QTHEIGHT; limit.z = WORLDZ;
		initQuadtree(head, origin, limit);
		struct qtNode* target = head;
		//just read i need to generate the maze in three rows
		//three rows
		//three rows
		//three rows
		//three rows
		//three rows
		//three rows
		//three rows
		//three rows
		//three rows
		//three rows
		//three rows
		//three rows
		//three rows
		//three rows
		//three rows
		//three rows
		//three rows
		//three rows
		//three rows
		//three rows
		//three rows
		//imagine making a perfectly fine quadtree then being forced to mutilate it into "three rows" waht a stupid clause
		
		/*in order to generate "three rows", i will do the following instead of just normally using the quadtree as described in lecture
		  (i really do not understand why you'd detail a really nice method like quadtree and then make everyone generate a completely
		  different structure. seems pretty mean-spirited to me and i really wish i could just generate 9 rooms in any way i want): i will
		  generate 9 points in predefined zones, then then generate 9 rooms of random sizes in three three "rows" that you have described.
		  these rooms will also be guaranteed to be at least 1 block away from each other and not intersect. why cant i just quadtree*/
		
		//int attempts = 0;
		//const int max_attempts = 50;
		const int min_d = 8;
		const int max_d = 15;
		const int padding = min_d + (min_d/2);
		const int roomsepMargin = 0; //play w this like u play with my heart
		int borderz1 = WORLDZ/3; //alice in borderland(s 2)
		int borderz2 = 2*(WORLDZ/3);
		int borderz3 = WORLDZ - max_d;
		int borderx1 = WORLDX/3;
		int borderx2 = 2*(WORLDX/3);
		int borderx3 = WORLDX - max_d;
		
		bool valid = false; //✨✨✨ FIXME: deprecated, remnant of quadtree implementation pre-three rows
		
		struct coord points[9];
		struct coord dimensions[9];
		//the following code generates 9 rooms in 9 zones with padding so they don't intersect.
		points[0] = generatePoint(0, borderx1 - padding, 0, borderz1 - padding);
		dimensions[0] = getRandomDimensions(min_d, max_d);
		points[1] = generatePoint(borderx1 - padding, borderx2 - padding, 0, borderz1 - padding);
		dimensions[1] = getRandomDimensions(min_d, max_d);
		points[2] = generatePoint(borderx2 + padding, WORLDX - max_d, 0, borderz1 - padding);
		dimensions[2] = getRandomDimensions(min_d, max_d);
		points[3] = generatePoint(0, borderx1 - padding, borderz1 + padding, borderz2 - padding);
		dimensions[3] = getRandomDimensions(min_d, max_d);
		points[4] = generatePoint(borderx1, borderx2, borderz1, borderz2);
		dimensions[4] = getRandomDimensions(min_d, max_d);
		points[5] = generatePoint(borderx2 + padding, WORLDX - max_d, borderz1 + padding, borderz2 - padding);
		dimensions[5] = getRandomDimensions(min_d, max_d);
		points[6] = generatePoint(0, borderx1 - padding, borderz2 + padding, WORLDZ - max_d);
		dimensions[6] = getRandomDimensions(min_d, max_d);
		points[7] = generatePoint(borderx1 + padding, borderx2 - padding, borderz2 + padding, WORLDZ - max_d);
		dimensions[7] = getRandomDimensions(min_d, max_d);
		points[8] = generatePoint(borderx2 + padding, WORLDX - max_d, borderz2 + padding, WORLDZ - max_d);
		dimensions[8] = getRandomDimensions(min_d, max_d); //"14 if statements" type beat because you hate quadtrees @dcalvert
		for (int i = 0; i < 9; i++) {
			printf("creating room %d @ %d %d %d\n", i, points[i].x, points[i].y, points[i].z);
			createRoom(points[i], dimensions[i].z, dimensions[i].x, dimensions[i].y, ((i+1)%7) + 1); //lwh args are out of order oops
			//we can also create the one random block in the rooms now:
			struct coord randomBlockPos;
			randomBlockPos = generatePoint(points[i].x+1, points[i].x + dimensions[i].x - 1, points[i].z + 1, points[i].z + dimensions[i].z - 1);
			world[randomBlockPos.x][randomBlockPos.y + 1][randomBlockPos.z] = 7;
		}
		/*imagine doing things programmatically... OH WAIT, i cant, because you took away my quatree @dcalvert
		  anyways, now, because @dcalvert is a literal hater and hates people using fitting and appropriate algorithms
		  to procedurally generate mazes as he describes in HIS OWN LECTURES, i will continue this trend of simply basically
		  hard-coding everything. thanks @dcalvert. let me remind you, TA reading this, i really did not want to do this. i 
		  really really wanted to use a quadtree. but @dcalvert forced my hand.*/

		/*okay, i've taken a few days off after i submitted that terrible first landmark
		  i've fully accepted that i wasted a whole lot of time trying to make a different program
		  it's best i move on from that*/
		const int w = 3;
		const int h = 5; //could make this random, min = 2, max = dimensions[i].h; but i dont really care
		GLbyte colour = 3;
		int mid;
		//can generate between two spots
		for(int i = 0; i < 6; i++) {
			//draw the corridors from and to two rooms
			mid = (points[i+3].z - (points[i].z + dimensions[i].z)) / 2;
			drawCorridor(offsetPoint(points[i], 0, 0, dimensions[i].z), mid, w, h, 3);
			drawCorridor(offsetPoint(points[i+3], 0, 0, -mid), mid, w, h, 3);
			//draw the horizontal (connecting) corridor
			//drawCorridor(offsetPoint(points[i+3], 0, 0, -mid), , , , );
			printf("%d/%d's corridor pair generated.\n", i, i+3);
		}
		/*drawCorridorZ();
		drawCorridorZ();
		drawCorridorZ();
		drawCorridorZ();
		drawCorridorZ();
		drawCorridorZ();
		drawCorridorZ();
		drawCorridorZ();
		drawCorridorZ();
		drawCorridorZ();*/
		/*for(int l = points[0].z + dimensions[0].z; l < ((points[3].z - (points[0].z + dimensions[0].z))); l++) { // this will go posz
			printf("a\n");
			for (int dw = points[0].x + (dimensions[0].x / 2) - w; dw < w; dw++) {
				printf("b\n");
				for(int dh = QTHEIGHT; dh < QTHEIGHT + h; dh++) { //there is a more efficient way to do this, i know. tehre's also a better way to procedurally generate a maze
					printf("c\n");
					if (dw == 0 || dh == 0 || l == 0) {colour = 1;}
					else {colour = 2;}
					world[dw][dh][l] = colour;
				}
			}
		} //doesnt draw anything for somer eason*/
		//instead of using functions, i dont want to. i just dont care. this is so borked idc idgaf why three rows dude just why
		//three rows
		//three rows
		//three rows
		//three rows
		//three rows
		//three rows
		//three rows
		//three rows
		//three rows
		//three rows
		//three rows
		//three rows
		//three rows
		//three rows
		//three rows
		//three rows
		//three rows
		//three rows
		//three rows

		//the next two create the player but he is kinda scary so let's just not for now
		//getViewPosition(&x, &y, &z);
		//createPlayer(0, x-5, -y, z-5, 0.0);
	}

	/* starts the graphics processing loop */
	/* code after this will not run until the program exits */

	//do you know what i dont care im not finishing this i am literally too mad. taking the L here and im gonna make up for it later
	glutMainLoop();
	return 0; 
}

