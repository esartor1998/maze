
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
	/* texture functions */
extern int setAssignedTexture(int, int);
extern void unsetAssignedTexture(int);
extern int getAssignedTexture(int);
extern void setTextureOffset(int, float, float);

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
extern bool collisions;
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

	/* mesh creation, translatio, rotation functions */
extern void setMeshID(int, int, float, float, float);
extern void unsetMeshID(int);
extern void setTranslateMesh(int, float, float, float);
extern void setRotateMesh(int, float, float, float);
extern void setScaleMesh(int, float);
extern void drawMesh(int);
extern void hideMesh(int);

/********* end of extern variable declarations **************/

/********* utility functions *********/

//and temp globals
#define QTHEIGHT 24
#define NUMROOMS 9
#define STAIRS_UP 0
#define STAIRS_DOWN 1
#define MAX_ROOMS 100

bool plsnocrash = false;
float cloudoffset = 0.0;
int framecount = 0;

//utility function. rand should be seeded before this is called, please! if you do this then you're the nicest
int getRandomNumber(int min, int max) {
	return min + (rand() % (max - min + 1));
}

struct coord {
	int x;
	int y;
	int z;
}; //just to make working with this shit easier although i might not end up using it

struct record {
	GLbyte world[WORLDX][WORLDY][WORLDZ];
	struct coord spawn;
	struct coord stairs[2]; //this is stored because the stairs hold special data + also need to be cross referenced from several scopes
};

//could make this infinite, but currently, i have no time. what a theme
//struct record* floors = calloc(MAX_ROOMS, sizeof(struct record)); //could be a doublke pointer hah maybe that'd be better...
struct record* floors[MAX_ROOMS] = {NULL}; //if only things were different... @dcalvert...
int currfloor = 0;//global memory management for records.the first one will be the outside level,

//TODO: also miny, maxy
struct coord generatePoint(int minx, int maxx, int minz, int maxz) { //i dont care that its shit you made me do this calvert. three rows
	struct coord point = {.x = getRandomNumber(minx, maxx), .y = QTHEIGHT, .z = getRandomNumber(minz, maxz)};
	////printf("Generated point: %d %d %d\n", point.x, point.y, point.z);
	return point;
}

//offsets a point by some value. negative numbers work, obv
struct coord offsetPoint(struct coord a, int offsetx, int offsety, int offsetz) {
	struct coord b = {.x = a.x + offsetx, .y = a.y + offsety, .z = a.z + offsetz};
	return b;
}

void fillVolume(struct coord origin, int l, int w, int h, GLbyte colour) {
	////printf("origin %d %d %d; l=%d, w=%d, h=%d\n", origin.x, origin.y, origin.z, l, w, h);
	for(int x = origin.x; x < origin.x+w; x++) {
		////printf("x: %d\t", x);
		for(int y = origin.y; y < origin.y+h; y++) {
			////printf("y: %d\t", y);
			for(int z = origin.z; z < origin.z+l; z++) {
				////printf("z: %d\n", z);
				world[x][y][z] = colour;
			}
		}
	}
}

void fillVolumeCalvertizedTest(struct coord origin, int l, int w, int h, GLbyte colour) {
	for(int x = origin.x; x > origin.x-w; x--) {
		////printf("x: %d\t", x);
		for(int y = origin.y; y > origin.y-h; y--) {
			////printf("y: %d\t", y);
			for(int z = origin.z; z > origin.z-l; z--) {
				////printf("z: %d\n", z);
				world[x][y][z] = colour;
			}
		}
	}
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
	if (collisions) {
		float x, y, z = 0.0;
		getViewPosition(&x, &y, &z);
		const float margin = 0.3;
		//the following could definitely be improved:
		if (world[-(int)(x-margin)][-(int)y][-(int)(z-margin)] || world[-(int)(x+margin)][-(int)y][-(int)(z+margin)] || -(int)x > WORLDX-1 || -(int)z > WORLDZ-1 || -(int)x < 0 || -(int)z < 0) {
			////printf("Collision detected at: x:%f y:%f z:%f", x, y, z);
			//going up the stairs takes confidence mr/ms/other TA, it's just like your first time conquering them as a child
			if(!(-(int)x > WORLDX-1 || -(int)z > WORLDZ-1 || -(int)x < 0 || -(int)z < 0) && world[-(int)(x-margin)][-(int)y + 1][-(int)(z-margin)] == 0 && world[-(int)(x+margin)][-(int)y + 1][-(int)(z+margin)] == 0) {
				////printf("up\n");
				getOldViewPosition(&x, &y, &z);
				setViewPosition(x, y-1.0, z);
			} else 		{
				//then we fix it
				getOldViewPosition(&x, &y, &z);
				setViewPosition(x, y, z);
				////printf("proposed solution: %f %f %f\n", x, y, z);
			}

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
		; //nop
		//printf("da mouse is been clicked");
	}

	//printf("%d %d\n", x, y);
}

/*(all ints pls) origin, l w h, colour.
 the origin coords will point to the bottom left of the room*/
void createRoom_old(struct coord origin, int l, int w, int h, GLbyte colour) {
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

/*(all ints pls) origin, l w h, colour.
 the origin coords will point to the bottom left of the room*/
void createRoom(struct coord origin, int l, int w, int h, GLbyte colour) {
	//dx, dy, dz = drwaing x,y,z. the n stands for nega
	GLbyte altcolour = colour == 4 ? colour - 1 : colour + 1; //the floor cant be white or some staircases would be "invisible"
	for (int dy = origin.y; dy <= origin.y + h; dy++) {
		for (int dx = origin.x; dx <= origin.x + w; dx++) {
			world[dx][dy][origin.z] = colour;
			world[dx][dy][origin.z + l] = colour;
			for (int floorz = origin.z; floorz <= origin.z + l; floorz++) {
				world[dx][origin.y][floorz] = altcolour;
				world[dx][origin.y + h][floorz] = altcolour;
			}
		}
		for (int dz = origin.z; dz <= origin.z + l; dz++) {
			world[origin.x][dy][dz] = colour;
			world[origin.x + w][dy][dz] = colour;
		}
	}
	fillVolume(offsetPoint(origin, 1, 1, 1), l-1, w-1, h-1, 0); //why didnt i do this before.
	//TODO: this is technically inefficient, i could do this all in way less runtime but idc rlly cuz im timed. if i have time i will fix it
} //i like pot_hole but i guess i'll SnakeStance this shit

struct coord getMidpoint(struct coord a, struct coord b) {
	struct coord midpoint;
	midpoint.x = (a.x + b.x) / 2; //int math
	midpoint.y = QTHEIGHT;
	midpoint.z = (a.z + b.z) / 2;
	return midpoint;
} //FIXME: deprecated: unused, though could be used during generation to clean it up a bit

double getDistance2d(struct coord a, struct coord b) {
	double n_squared = (double)((b.x - a.x) * (b.x - a.x)); //not my usual style but i don't want to mess anything up!!
	double m_squared = (double)((b.z - a.z) * (b.z - a.z));
	return sqrt(n_squared + m_squared);
}

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
	//////printf("mp.x %d mp.z %d\n", mp.x, mp.z);
	struct coord a;
	struct coord b;
	node->nw = calloc(1, sizeof(struct qtNode));
	a.x = node->bottomleft.x; a.y = QTHEIGHT; a.z = midZ;
	b.x = midX; b.y = QTHEIGHT; b.z = node->topright.z;
	//////printf("a: %d %d %d\nb: %d %d %d\n", a.x, a.y, a.z, b.x, b.y, b.z);
	initQuadtree(node->nw, a, b);
	node->ne = calloc(1, sizeof(struct qtNode));
	a.x = midX; a.y = QTHEIGHT; a.z = midZ;
	b.x = node->topright.x; b.y = QTHEIGHT; b.z = node->topright.z;
	//////printf("a: %d %d %d\nb: %d %d %d\n", a.x, a.y, a.z, b.x, b.y, b.z);
	initQuadtree(node->ne, a, b);
	node->sw = calloc(1, sizeof(struct qtNode));
	a.x = node->bottomleft.x; a.y = QTHEIGHT; a.z = node->bottomleft.z;
	b.x = midX; b.y = QTHEIGHT; b.z = midZ;
	//////printf("a: %d %d %d\nb: %d %d %d\n", a.x, a.y, a.z, b.x, b.y, b.z);
	initQuadtree(node->sw, a, b);
	node->se = calloc(1, sizeof(struct qtNode));
	a.x = midX; a.y = QTHEIGHT; b.z = node->bottomleft.x;
	b.x = node->topright.x; b.y = QTHEIGHT; b.z = midZ;
	//////printf("a: %d %d %d\nb: %d %d %d\n", a.x, a.y, a.z, b.x, b.y, b.z);
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
	////printf("offsets: %d %d\n", xoffset, zoffset);
	dimensions.x = (b.x - a.x) - xoffset; //width
	dimensions.y = 8; //these can be further randomized/improved
	dimensions.z = (b.z - a.z) - zoffset;
	return dimensions;
} //FIXME: deprecated due to three rows

struct coord getRandomDimensions(int min, int max) {
	//this is the brain damaged version of the above function. rooms cant exceed max size in any dimension.
	struct coord dimensions;
	dimensions.x = getRandomNumber(min, max);
	dimensions.y = getRandomNumber(min, max);
	dimensions.z = getRandomNumber(min, max);
	////printf("generated dimensions: %d %d %d\n", dimensions.x, dimensions.y, dimensions.z);
	return dimensions; //why has god forsaken me i just wanted to use a quadtree.
} //x = w; y = h; z = l

bool proximityCheck(struct coord dimensions[], struct coord points[], int pindex, int margin) { //idc idgaf that its bad this is not my fault i just wanted qtree
	bool result = true; //yes i know this doesnt account for the dimensions of point 2 but dont fuCking worry ab it
	//int NUMROOMS = sizeof(points)/sizeof(*points); //ONLY PASS AN ARRAY HERE!!!
	////printf("NUMROOMS resolves to %d\n", NUMROOMS);
	struct coord point1_max;
	int longer_side = 0;
	point1_max.x = points[pindex].x + dimensions[pindex].x;
	point1_max.y = QTHEIGHT; //FIXME: QTHEIGHT USE! THIS AND THE ONE IN THE LOOP WILL NEED DEBUGGING IF HEIGHT NEEDS TO BE FACTORED IN
	point1_max.z = points[pindex].z + dimensions[pindex].z;
	struct coord center = getMidpoint(points[pindex], point1_max); //this is the center of our circle;
	if (dimensions[pindex].x > dimensions[pindex].z) {
		longer_side = dimensions[pindex].x;
	} else {longer_side = dimensions[pindex].z;} //the longer side len / 2 will be used as the radius
	int r1 = (longer_side / 2) + margin;
	//TODO: optimally, this would only check adjacents, but i dont have time to do that right now
	for(int i = 0; i < NUMROOMS; i++) {
		if (!(i == pindex)) {
			struct coord point2_max;
			int longer_side_r2 = 0;
			point2_max.x = points[i].x + dimensions[i].x;
			point2_max.y = QTHEIGHT;
			point2_max.z = points[i].z + dimensions[i].z;
			struct coord center2 = getMidpoint(points[i], point2_max); //this is the center of our circle;
			if (dimensions[i].x > dimensions[i].z) {
				longer_side_r2 = dimensions[i].x;
			} else {longer_side_r2 = dimensions[i].z;} //the longer side len / 2 will be used as the radius
			int r2 = longer_side_r2 + margin;
			int d = (int)getDistance2d(center, center2); //INTEGER TRUNCATION BLAH BLAH BLUUUURGH 
			if (d < (r1 + r2)) {
				result = false;
				break;
			}
		}
	}
	return result;
} //FIXME: no longer deprecated :)

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
		////printf("a: %d %d %d b: %d %d %d\n", points[pindex].x, points[pindex].y, points[pindex].z, points[i].x, points[i].y, points[i].z);
		if (dimensions[pindex].x > dimensions[pindex].z) {
			r = ((float)(dimensions[pindex].x + margin)) / 2.0 + (float)margin;
		} else {r = ((float)(dimensions[pindex].z + margin)) / 2.0 + (float)margin;} //the longer side len / 2 will be used as the radius
		if(dimensions[i].x > dimensions[i].z) {
			r2 = ((float)(dimensions[i].x)) / 2.0 + (float)margin;
		} else {r2 = ((float)(dimensions[i].z)) / 2.0 + (float)margin;}
		////printf("%f %f %lf\n", r, r2, sqrt((float)(((center2.x - center1.x) * (center2.x - center1.x)) + ((center2.z - center2.z) * (center2.z - center1.z)))));
		//if ((((points[i].x - center.x) * (points[i].x - center.x)) + ((points[i].z - center.z) * (points[i].z - center.z))) < (((longer_side + margin)/2) * ((longer_side +  margin)/2))) {
		if (sqrt((float)(((center2.x - center1.x) * (center2.x - center1.x)) + ((center2.z - center2.z) * (center2.z - center1.z)))) < (r + r2)) {
			//if the distance between the two centers is less than the sum of their radii, they intersect
			result = false;
			////printf("pc failed\n");
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
} //FIXME: deprecated: bad

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
	//createDoor(a, w, h);//intro door
	//createDoor(offsetPoint(a, 0, 0, l-1), w, h); //exit door
} //FIXME: deprecated: bad

void joinRooms(struct coord points[9], struct coord dimensions[9], int r1_index, int r2_index, const int w, const int h, GLbyte colour) {
	int mid;
	if (r2_index == r1_index + 3) { //mode 0 - x*
		mid = (points[r2_index].z - (points[r1_index].z + dimensions[r1_index].z)) / 2;
		//draw the corridors from and to two rooms
		createRoom(offsetPoint(points[r1_index], 0, 0, dimensions[r1_index].z), mid, w, h, colour);
		createRoom(offsetPoint(points[r2_index], 0, 0, -mid), mid, w, h, colour); //IMPORTANT: keep ratio o:o-1
		//draw the horizontal (connecting) corridor
		/*since we can't draw right to left becuase i dont want to change anything,
		we draw the corridor from the leftmost side first. love to just use abs but 🤷*/
		if (points[r1_index].x < points[r2_index].x) { //x+
			////printf("todoroki %d\n", points[r2_index].x - points[r1_index].x); //w is passed as the l because its horizontal
			createRoom(offsetPoint(points[r1_index], 0, 0, mid + dimensions[r1_index].z), w, points[r2_index].x - points[r1_index].x + w, h, colour); //TODO: + w in w dubious?
			fillVolume(offsetPoint(points[r1_index], 1, 1, mid + dimensions[r1_index].z + 1), w - 1, points[r2_index].x - points[r1_index].x + w - 1, h - 1, 0);
		} else{ //x-
			////printf("else %d\n", points[r1_index].x - points[r2_index].x);
			createRoom(offsetPoint(points[r2_index], 0, 0, -mid), w, points[r1_index].x - points[r2_index].x + w, h, colour);
			fillVolume(offsetPoint(points[r2_index], 1, 1, -mid + 1), w - 1, points[r1_index].x - points[r2_index].x + w - 1, h - 1, 0);
		}
		//clear the vertical tunnels
		fillVolume(offsetPoint(points[r1_index], 1, 1, dimensions[r1_index].z), mid + w - 1, w - 1, h - 1, 0);
		fillVolume(offsetPoint(points[r2_index], 1, 1, -mid + 1), mid + w - 1, w - 1, h - 1, 0);
		////printf("%d/%d's corridor pair generated.\n", r1_index, r2_index);
	}
	else { //mode 1 - z*
		mid = ((points[r2_index].x - (points[r1_index].x + dimensions[r1_index].x)) / 2) - 1;
		bool odd = !(mid % 2);
		//draw the corridors from and to two rooms
		createRoom(offsetPoint(points[r1_index], dimensions[r1_index].x, 0, 0), w, mid, h, colour); //offset wrong?
		createRoom(offsetPoint(points[r2_index], -mid, 0,  0), w, mid, h, colour);
		//draw the horizontal (connecting) corridor
		if (points[r1_index].z < points[r2_index].z) { //z+?
			////printf("todoroki %d\n", points[r2_index].z - points[r1_index].z); //w is passed as the l because its horizontal
			createRoom(offsetPoint(points[r1_index], mid + dimensions[r1_index].x, 0, 0), points[r2_index].z - points[r1_index].z + w, w, h, colour); //TODO: + w in w dubious?
			fillVolume(offsetPoint(points[r1_index], mid + dimensions[r1_index].x + 1, 1, 1), points[r2_index].z - points[r1_index].z + w - 1, w - 1, h - 1, 0);
		} 
		else{ //z-
			////printf("else %d\n", points[r1_index].z - points[r2_index].z);
			createRoom(offsetPoint(points[r2_index], -(mid + w), 0, 0), points[r1_index].z - points[r2_index].z + w, w, h, colour);
			fillVolume(offsetPoint(points[r2_index], -(mid + w) + 1, 1, 1), points[r1_index].z - points[r2_index].z + w - 1, w - 1, h - 1, 0);
		}
		//clear the "horizontal tunnels"
		fillVolume(offsetPoint(points[r1_index], dimensions[r1_index].x, 1, 1), w - 1, mid + 1, h - 1, 0); //aioubdiuq3TODO: THIS FIXME: THIS
		fillVolume(offsetPoint(points[r2_index], (-mid), 1, 1),  w - 1, mid + 1, h - 1, 0);
		////printf("%d/%d's corridor pair generated.\n", r1_index, r2_index);
	} //FIXME: for some reason, this generates holes in walls and doesnt make doorways. maybe the connection isnt center? maybe fix later
}

/* CODE FROM WIKIPEDIA
 * Function to linearly interpolate between a0 and a1
 * Weight w should be in the range [0.0, 1.0]
 */
float interpolate(float a0, float a1, float w) {
    /* // You may want clamping by inserting:
     * if (0.0 > w) return a0;
     * if (1.0 < w) return a1;
     */
    return (a1 - a0) * w + a0;
    /* // Use this cubic interpolation [[Smoothstep]] instead, for a smooth appearance:
     * return (a1 - a0) * (3.0 - w * 2.0) * w * w + a0;
     *
     * // Use [[Smootherstep]] for an even smoother result with a second derivative equal to zero on boundaries:
     * return (a1 - a0) * ((w * (w * 6.0 - 15.0) + 10.0) * w * w * w) + a0;
     */
}

typedef struct {
    float x, y;
} vector2;

/* Create random direction vector
 */
vector2 randomGradient(int ix, int iy) {
    // Random float. No precomputed gradients mean this works for any number of grid coordinates
    float random = 2920.f * sin(ix * 21942.f + iy * 171324.f + 8912.f) * cos(ix * 23157.f * iy * 217832.f + 9758.f);
    return (vector2) { .x = cos(random), .y = sin(random) };
}

// Computes the dot product of the distance and gradient vectors.
float dotGridGradient(int ix, int iy, float x, float y) {
    // Get gradient from integer coordinates
    vector2 gradient = randomGradient(ix, iy);

    // Compute the distance vector
    float dx = x - (float)ix;
    float dy = y - (float)iy;

    // Compute the dot-product
    return (dx*gradient.x + dy*gradient.y);
}

// Compute Perlin noise at coordinates x, y
float perlin(float x, float y) {
    // Determine grid cell coordinates
    int x0 = (int)x;
    int x1 = x0 + 1;
    int y0 = (int)y;
    int y1 = y0 + 1;

    // Determine interpolation weights
    // Could also use higher order polynomial/s-curve here
    float sx = x - (float)x0;
    float sy = y - (float)y0;

    // Interpolate between grid point gradients
    float n0, n1, ix0, ix1, value;

    n0 = dotGridGradient(x0, y0, x, y);
    n1 = dotGridGradient(x1, y0, x, y);
    ix0 = interpolate(n0, n1, sx);

    n0 = dotGridGradient(x0, y1, x, y);
    n1 = dotGridGradient(x1, y1, x, y);
    ix1 = interpolate(n0, n1, sx);

    value = interpolate(ix0, ix1, sy);
    return value;
}

int loadMaze(int which) { //which is experimental
	FILE* prev;
	
}

void genMaze() {
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
	//printf("beginnign maze generation!\n");
	const int min_d = 8;
	const int max_d = 15;
	const int padding = min_d + (min_d/2) + 2;
	const int roomsepMargin = 0; //play w this like u play with my heart
	int borderz1 = WORLDZ/3; //alice in borderland(s 2)
	int borderz2 = 2*(WORLDZ/3);
	int borderz3 = WORLDZ - max_d;
	int borderx1 = WORLDX/3;
	int borderx2 = 2*(WORLDX/3);
	int borderx3 = WORLDX - max_d;
			
	bool valid = false; //✨✨✨ FIXME: deprecated, remnant of quadtree implementation pre-three rows

	struct coord points[NUMROOMS];
	struct coord dimensions[NUMROOMS];
	struct coord stairs[2]; //0 is u; 1 is down
	struct coord spawn;

	const int w = 5;
	const int h = 5; //could make this random, min = 2, max = dimensions[i].h; but i dont really care
	GLbyte colour = 1;
	int mid;
	
	bool generationSuccess = true;
	do {
		generationSuccess = true;
		for(int i=0; i<WORLDX; i++) {
			for(int j=0; j<WORLDY; j++) {
				for(int k=0; k<WORLDZ; k++) {
					world[i][j][k] = 0;
				}
			}
		}
		////printf("generation attempt\n");
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
		for (int i = 0; i < NUMROOMS; i++) {
			createRoom(points[i], dimensions[i].z, dimensions[i].x, dimensions[i].y, ((i+1)%7) + 1); //lwh args are out of order oops
			//we can also create the one random block in the rooms now:
			struct coord randomBlockPos;
			randomBlockPos = generatePoint(points[i].x+1, points[i].x + dimensions[i].x - 1, points[i].z + 1, points[i].z + dimensions[i].z - 1);
			world[randomBlockPos.x][randomBlockPos.y + 1][randomBlockPos.z] = 7;
		}
		for(int c = 0; c < NUMROOMS; c++) {
			if((points[c].x + dimensions[c].x) >= WORLDX || (points[c].z + dimensions[c].z) >= WORLDZ || !proximityCheck(dimensions, points, c, 1)) {
				generationSuccess = false;
				break;
			}
		} //the check loop. could ideally be integrated elsewhere but i don't care i don't ahve time to think too much about these things
	} while(!generationSuccess);
	//also, if everything is good to go, generate 2 stairs in random rooms, but not the same room:
	int ds = getRandomNumber(0, NUMROOMS-1); //short for "downstairs", it's the index of the room that this staircase will be in
	int us = getRandomNumber(0, NUMROOMS-1); //    "      "upstairs",                           "
	if(ds == us) {us = (ds + us) % NUMROOMS;}
	struct coord dspoint = generatePoint(points[ds].x+1, points[ds].x + dimensions[ds].x - 1, points[ds].z + 1, points[ds].z + dimensions[ds].z - 1);
	struct coord uspoint = generatePoint(points[us].x+1, points[us].x + dimensions[us].x - 1, points[us].z + 1, points[us].z + dimensions[us].z - 1);
	stairs[0] = uspoint;
	stairs[1] = dspoint;
	world[stairs[STAIRS_UP].x][stairs[STAIRS_UP].y][stairs[STAIRS_UP].z] = 10; //creating up
	world[stairs[STAIRS_DOWN].x][stairs[STAIRS_DOWN].y][stairs[STAIRS_DOWN].z] = 10; //creating down
	/*imagine doing things programmatically... OH WAIT, i cant, because you took away my quadtree @dcalvert
	anyways, now, because @dcalvert is a literal hater and hates people using fitting and appropriate algorithms
	to procedurally generate mazes as he describes in HIS OWN LECTURES, i will continue this trend of simply basically
	hard-coding everything. thanks @dcalvert. let me remind you, TA reading this, i really did not want to do this. i 
	really really wanted to use a quadtree. but @dcalvert forced my hand.*/
	/*okay, i've taken a week off after i submitted that terrible first landmark
	i've fully accepted that i wasted a whole lot of time trying to make a different program
	it's best i move on from that. also deleted a bunch of really angry comments. keeping the above one though*/
		
	//the corridors are just rooms LLOL iof u really THINK about it
	for(int i = 0; i < (NUMROOMS/3) * 2; i++) { //i think my math on numrooms/3 * 2 is wrong but tbh in this case we want 6 so it works (9/3 * 2 = 6)
		joinRooms(points, dimensions, i, i+(NUMROOMS/3), w, h, colour); //3 would be NUMROWS if calvert's generation was well-advised but i fear he may change it any time
	}
	/*now let's connect the rows to make an H joining shape:
	this uses a "jump table" of sorts because i am quite lazy right now and couldn't think of a mathematically
	better way to do "choose two".*/
	int corridors[2][3][2] = {{{0, 1}, {3, 4}, {6, 7}}, {{1, 2}, {4, 5}, {7, 8}}};
	//those are the valid pairs, we pick one pair from each set:
	int chosenindex = getRandomNumber(0, 2);
	joinRooms(points, dimensions, corridors[0][chosenindex][0], corridors[0][chosenindex][1], w, h, colour);
	chosenindex = getRandomNumber(0, 2);
	joinRooms(points, dimensions, corridors[1][chosenindex][0], corridors[1][chosenindex][1], w, h, colour);
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
	int offset = getRandomNumber(5, min_d);
	//TODO: change back after debug to "us"
	spawn = offsetPoint(points[0], offset, 3, offset); //spawn beside the up stairs
	//printf("attemptint to spawn er in %d %d %d\n", -spawn.x, -spawn.y, -spawn.z);
	//and now, we write the maze's info to the lowestmost un-written index
	int seek = 0;
	//printf("saving.\n");
	while (floors[seek]) {seek++;} //? i think?
	floors[seek] = calloc(1, sizeof(struct record));
	for(int sx=0; sx<WORLDX; sx++) {
		for(int sy=0; sy<WORLDY; sy++) {
			for(int sz=0; sz<WORLDZ; sz++) {
				////printf("saving %d %d %d\n", sx, sy, sz);
				floors[seek]->world[sx][sy][sz] = world[sx][sy][sz];
			}
		}
	}
	floors[seek]->spawn = spawn;
	floors[seek]->stairs[STAIRS_UP] = stairs[STAIRS_UP];
	floors[seek]->stairs[STAIRS_DOWN] = stairs[STAIRS_DOWN];
	//printf("done!\n");
	printf("points[0] = %d %d %d\n", points[0].x, points[0].y, points[0].z);
	setViewPosition(-(float)spawn.x, -(float)spawn.y, -(float)spawn.z);
	//remember not to try to load after the maze is generated, the generation IS the loading
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
	getViewPosition(&x, &y, &z);

	if (testWorld) {

		/* update old position so it contains the correct value */
		/* -otherwise view position is only correct after a key is */
		/*  pressed and keyboard() executes. */
		#if 0
		// Fire a ray in the direction of forward motion
		float xx, yy, zz;
		getViewPosition(&x, &y, &z);
		getOldViewPosition(&xx, &yy, &zz);
		////printf("%f %f %f %f %f %f\n", xx, yy, zz, x, y, z);
		////printf("%f %f %f\n",  -xx+((x-xx)*25.0), -yy+((y-yy)*25.0), -zz+((z-zz)*25.0));
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

	/* offset counter for animated texture */
      static float textureOffset = 0.0;

	/* scaling values for fish mesh */
      static float fishScale = 1.0;
      static int scaleCount = 0;
      static GLfloat scaleOffset = 0.0;

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

	/* move texture for lava effect */
      textureOffset -= 0.01;
      setTextureOffset(18, 0.0, textureOffset);

	/* make fish grow and shrink (scaling) */
      if (scaleCount == 1) scaleOffset += 0.01;
      else scaleOffset -= 0.01;
      if (scaleOffset >= 0.5) scaleCount = 0;
      if (scaleOffset <= 0.0) scaleCount = 1;
      setScaleMesh(1, 0.5 + scaleOffset);

	/* make cow with id == 2 appear and disappear */
	/* use scaleCount as switch to flip draw/hide */
	/* rotate cow while it is visible */
      if (scaleCount == 0) {
         drawMesh(2);
         setRotateMesh(2, 0.0, 180.0 + scaleOffset * 100.0, 0.0);
      } else {
         hideMesh(2);
      }

	 	/* end testworld animation */


	} else {
		/* your code goes here */
		//why would i put it here i want gravity to exist on the test world too?
		setOldViewPosition(x,y,z); //i'm like 90% sure this should be called before gvp(i i i)
		getViewPosition(&x, &y, &z);
		getViewOrientation(&rx, &ry, &rz);
		//printf("viewpos %f %f %f\n", x, y, z);
		int noncalvertx = -(int)x;
		int noncalverty = -(int)y;
		int noncalvertz = -(int)z;
		////printf("non-calvertpos %d %d %d\n", noncalvertx, noncalverty, noncalvertz);
		bool hmm = false;
		if (floors[currfloor]->stairs[STAIRS_UP].x == noncalvertx) {
			//printf("x match.u\n");
			if (floors[currfloor]->stairs[STAIRS_UP].z == noncalvertz) {
				//printf("z matchedu\n");
				if (floors[currfloor]->stairs[STAIRS_UP].y == noncalverty - 1) {
					//printf("y-1 (maybe + 1?) matchu\n");
					//printf("loading triggered.\n");
					if (currfloor == 0) {
						//printf("Trying to ascend to heaven: not allowed, due to furry.\nAlso up staircase on overworld, fix.\n");
					} else {
						currfloor -= 1;
						hmm = true;
					}
				}
			}
		}
		else if (floors[currfloor]->stairs[STAIRS_DOWN].x == noncalvertx) {
			//printf("x match.d\n");
			if (floors[currfloor]->stairs[STAIRS_DOWN].z == noncalvertz) {
				//printf("z matchedd\n");
				if (floors[currfloor]->stairs[STAIRS_DOWN].y == noncalverty - 1) {
					//printf("y-1 (maybe + 1?) matchd\n");
					//printf("ok, loading downstairs\n");
					currfloor += 1;
					hmm = true;
				}
			}
		} //i've been wrong all alonG! could refactor! hope it works anyways though! ahha
		if (hmm) {
			if (!(floors[currfloor])) {
				//printf("No deeper maze exists. creating one! Or, there has been some catastrophe\n");
				genMaze();
			}
			else {
				for(int lx = 0; lx < WORLDX; lx++) {
					for (int ly = 0; ly < WORLDY; ly++) {
						for (int lz = 0; lz < WORLDZ; lz++) {
							world[lx][ly][lz] = floors[currfloor]->world[lx][ly][lz];
						}
					}
				} //it just occured to me that i can't set dimensions or points from here sooooo maybe they dont matter at all
			}
			//remember to calvertize (make negative) the positions first
			////printf("okay hopefully? %f %f %f\n", -(float)floors[currfloor]->spawn.x,-(float)floors[currfloor]->spawn.y,-(float)floors[currfloor]->spawn.z);
			setViewPosition(-(float)floors[currfloor]->spawn.x,-(float)floors[currfloor]->spawn.y,-(float)floors[currfloor]->spawn.z);
		}
		else {
			if(plsnocrash && usegravity){
				if(world[-(int)x][-(int)(y+1)][-(int)z] == 0) {
					//////printf("grabity\n");
					setViewPosition(x,y+0.1,z);
					//collisionResponse();
				}
				setPlayerPosition(0, -x-1, -y, -z-1, -ry);
			}
		} //FIXME: jank
		//i know the following is bad, but i still want a quadtree and its february
		const int updatefreq = 50; 
		if (((++framecount) % INT32_MAX) % updatefreq == 0) {
			float divisor = 10;
			if (currfloor == 0) {
				cloudoffset = cloudoffset + 0.1;
				//uh isn't this going to cause some fucking EPIC lag
				for (int x = 0; x < WORLDX; x++) {
					//world[x][WORLDY-7][z] = 0;
					for (int z = 0; z < WORLDZ; z++) {
						float x1 = ((float)x/divisor) + cloudoffset;
						float z1 = ((float)z/divisor);
						float y1 = (WORLDY - 7) + perlin(x1, z1) * 5;
						////printf("x: %f, y: %f, z: %f\n", (float)x1, y1, (float)z1);
						if (y1 > WORLDY-5) {
							world[x][(int)y1][z] = 5;
						}
						else {
							for (int y = WORLDY - 5; y < WORLDY; y++) {
								world[x][y][z] = 0;
							}
						}
					}
				}
			} //HOLY SHIT they're going to TANK it
		}
	}
}

/*end of code from Wikipedia https://en.wikipedia.org/wiki/Perlin_noise*/
/*"Note that the code below is very basic, for illustration only, will be slow, and not usable in applications." 
  :trollface: you think any code we're writing for this is usable./..*/
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
		
	/* create sample player */
      createPlayer(0, 52.0, 27.0, 52.0, 0.0);

	/* texture examples */

	/* create textured cube */
	/* create user defined colour with an id number of 11 */
      setUserColour(11, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0);
	/* attach texture 22 to colour id 11 */
      setAssignedTexture(11, 22);
	/* place a cube in the world using colour id 11 which is texture 22 */
      world[59][25][50] = 11;

	/* create textured cube */
      setUserColour(12, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0);
      setAssignedTexture(12, 27);
      world[61][25][50] = 12;

	/* create textured cube */
      setUserColour(10, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0);
      setAssignedTexture(10, 26);
      world[63][25][50] = 10;

	/* create textured floor */
      setUserColour(13, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0);
      setAssignedTexture(13, 8);
      for (i=57; i<67; i++)
         for (j=45; j<55; j++)
            world[i][24][j] = 13;

	/* create textured wall */
      setUserColour(14, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0);
      setAssignedTexture(14, 18);
      for (i=57; i<67; i++)
         for (j=0; j<4; j++)
            world[i][24+j][45] = 14;

	/* create textured wall */
      setUserColour(15, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0);
      setAssignedTexture(15, 42);
      for (i=45; i<55; i++)
         for (j=0; j<4; j++)
            world[57][24+j][i] = 15;

		// two cubes using the same texture but one is offset
		// cube with offset texture 33
      setUserColour(16, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0);
      setAssignedTexture(16, 33);
      world[65][25][50] = 16;
      setTextureOffset(16, 0.5, 0.5);
		// cube with non-offset texture 33
      setUserColour(17, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0);
      setAssignedTexture(17, 33);
      world[66][25][50] = 17;

		// create some lava textures that will be animated
      setUserColour(18, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0);
      setAssignedTexture(18, 24);
      world[62][24][55] = 18;
      world[63][24][55] = 18;
      world[64][24][55] = 18;
      world[62][24][56] = 18;
      world[63][24][56] = 18;
      world[64][24][56] = 18;

		// draw cow mesh and rotate 45 degrees around the y axis
		// game id = 0, cow mesh id == 0
      setMeshID(0, 0, 48.0, 26.0, 50.0);
      setRotateMesh(0, 0.0, 45.0, 0.0);

		// draw fish mesh and scale to half size (0.5)
		// game id = 1, fish mesh id == 1
      setMeshID(1, 1, 51.0, 28.0, 50.0);
      setScaleMesh(1, 0.5);

		// draw cow mesh and rotate 45 degrees around the y axis
		// game id = 2, cow mesh id == 0
      setMeshID(2, 0, 59.0, 26.0, 47.0);

		// draw bat
		// game id = 3, bat mesh id == 2
      setMeshID(3, 2, 61.0, 26.0, 47.0);
      setScaleMesh(3, 0.5);
		// draw cactus
		// game id = 4, cactus mesh id == 3
      setMeshID(4, 3, 63.0, 26.0, 47.0);
      setScaleMesh(4, 0.5);


   } else {
	   
		/* your code to build the world goes here */
		
		plsnocrash = true;
		struct qtNode* head = calloc(1, sizeof(struct qtNode));
		struct coord origin; origin.x = 0; origin.y = QTHEIGHT; origin.z = 0;
		struct coord limit; limit.x = WORLDX; limit.y = QTHEIGHT; limit.z = WORLDZ;
		initQuadtree(head, origin, limit);
		struct qtNode* target = head;
		for(i=0; i<WORLDX; i++) {
			for(j=0; j<WORLDY; j++) {
				for(k=0; k<WORLDZ; k++) {
					world[i][j][k] = 0;
				}
			}
		} //init to empty TODO: removedebug and compartmentalize

		//outside world generation
		float divisor1 = 20.0;
		float divisor2 = 20.0;
		float offset = (float)getRandomNumber(0,GL_MAX); //idc
		struct coord stairslocation;
		for (int x = 0; x < WORLDX; x++) {
			for (int z = 0; z < WORLDZ; z++) {
				//the ground:
				float x0 = (float)x/divisor1 + offset;
				float z0 = (float)z/divisor2 + offset;
				float y = perlin(x0, z0) * 17;
				GLbyte elevation;
				
				if(y > 4.0) {elevation = 5;}
				else if (y < -4.0) {elevation = 9;}
				else {elevation = 1;}
				for(int deep = 0; deep < 3; deep++) {
					world[x][(int)y+QTHEIGHT - deep][z] = elevation;
				}
				if(x == 50 && z == 50) { 
					//someone told me you could do the following to assign structs:
					stairslocation = (struct coord){x, (int)y+QTHEIGHT, z};
					//totally blew my mind. didnt know it was possible.
					world[x][(int)y+QTHEIGHT][z] = 10;
				}
			}
		} //uh huh
		
		//printf("done! saving now.\n");
		floors[0] = calloc(1, sizeof(struct record));
		//inefficient? lawl.
		for(int sx = 0; sx < WORLDX; sx++) {
			for (int sy = 0; sy < WORLDY; sy++){
				for (int sz = 0; sz < WORLDZ; sz++){
					////printf("saving %d %d %d...\n", sx, sy, sz);
					floors[0]->world[sx][sy][sz] = world[sx][sy][sz];
				}
			}
		}
		struct coord dummy = {.x = 0, .y = 0, .z = 0}; //c99 sexy
		struct coord spawn = {.x = 53, .y = QTHEIGHT + 7, .z = 53}; //hopefully never gens in ground. just for testing
		//printf("stairs: %d %d %d %d %d %d\n", dummy.x, dummy.y, dummy.z, spawn.x, spawn.y, spawn.z);
		floors[0]->stairs[STAIRS_UP] = dummy;
		floors[0]->stairs[STAIRS_DOWN] = stairslocation;
		floors[0]->spawn = spawn;
		//setViewPosition((float)spawn.x, (float)spawn.y, (float)spawn.z);
		//printf("saving done I think!\n");
		setViewPosition(-(float)floors[currfloor]->spawn.x,-(float)floors[currfloor]->spawn.y,-(float)floors[currfloor]->spawn.z);
	}

	/* starts the graphics processing loop */
	/* code after this will not run until the program exits */
	glutMainLoop();
	return 0; 
}

