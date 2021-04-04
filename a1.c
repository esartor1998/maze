
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

//so many externs...
extern GLubyte  world[WORLDX][WORLDY][WORLDZ];
extern float frustum[6][4];
extern void ExtractFrustum();
extern int PointInFrustum(float x, float y, float z);
extern int SphereInFrustum(float x, float y, float z, float radius);
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

/* x1 y1 x2 y2 */
extern void  draw2Dline(int, int, int, int, int);
/* x1 y1 x2 y2 */
extern void  draw2Dbox(int, int, int, int);
/* x1 y1 x2 y2 */
extern void  draw2Dtriangle(int, int, int, int, int, int);
extern void  set2Dcolour(float []);
	/* texture functions */
extern int setAssignedTexture(int, int);
extern void unsetAssignedTexture(int);
extern int getAssignedTexture(int);
extern void setTextureOffset(int, float, float);

extern float getx(int);
extern float gety(int);
extern float getz(int);
extern float getoldx(int);
extern float getoldy(int);
extern float getoldz(int);
extern int getVisible(int);
extern bool getChase(int);
extern void setChase(int, bool);
extern int getDestination(int);
extern void setDestination(int, int);
extern bool getActive(int);
extern void setActive(int, bool);
extern bool getPresent(int);
extern void setPresent(int, bool);

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
/* flag toggles gravity */
extern bool usegravity;
/* flag toggles collision */
extern bool collisions;
/* frustum corner coordinates, used for visibility determination  */
extern float corners[4][3];

/* determine which cubes are visible e.g. in view frustum */
extern void ExtractFrustum();
extern void tree(float, float, float, float, float, float, int); //so nostalgic

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
extern void resetTranslateMesh(int);
extern void setRotateMesh(int, float, float, float);
extern void setScaleMesh(int, float);
extern void drawMesh(int);
extern void hideMesh(int);
extern int getMeshNumber(int); //"mesh number" is a bit misleading, it actually refers to the type
extern int getMeshUsed(int);

/********* end of extern variable declarations **************/

/********* utility functions *********/

//and temp globals
#define FLOORHEIGHT 24
#define NUMROOMS 9
#define MAX_FLOORS 100

//i dont care. this makes it more readable and i want to do it
enum direction {n, ne, e, se, s, sw, w, nw, none}; //directions from north clockwise, with a special fail case

enum direction combineDirections(enum direction v, enum direction h) { //i could do this instead by arranging the enum in a clever way but i dont have time
	if (v == n) {
		if (h == e) {
			return ne;
		}
		else if (h == w) {
			return nw;
		}
	}
	else if (v == s) {
		if (h == e) {
			return se;
		}
		else if (h == w) {
			return sw;
		}
	}
	return n; //if everything else fails just return north. this will happen if two composite vectors are added.
}

struct coord {
	int x;
	int y;
	int z;
}; //just to make working with this shit easier although i might not end up using it

struct record { //this might need to store mobs too... and other things...
	GLbyte world[WORLDX][WORLDY][WORLDZ];
	struct coord spawn;
	struct coord stairs[2]; //this is stored because the stairs hold special data + also need to be cross referenced from several scopes
};

const int map_offset = 5; //ATTN: 5 pixels?
bool world_inited = false;
time_t timings[100] = {0L}; //timings[0] will hold the clouds, [1] the mobs, then whatever else.
struct record* floors[MAX_FLOORS] = {NULL}; //if only things were different... @dcalvert...
struct coord points[MAX_FLOORS * NUMROOMS];
struct coord dimensions[MAX_FLOORS * NUMROOMS];
struct coord random_things[MAX_FLOORS * NUMROOMS]; //TODO: INFROOMS scale up for infinite room gen
struct coord curr_viewpos = {0, 0, 0};
int currfloor = 0; //global memory management for records.the first one will be the outside level,
int prevfloor = 0;
int numMesh = 0;   //dcalvert, your mob system hurts my soul
int meshIndex = 0; //this is actually the global offset for indices.
float scaling = 5.0;
int hjoined[2 * MAX_FLOORS][2 * MAX_FLOORS];

//#define STAIRS_UP (currfloor * 2) + 0
//#define STAIRS_DOWN (currfloor * 2) + 1
#define STAIRS_UP 0
#define STAIRS_DOWN 1
#define SIDE_A currfloor + 0
#define SIDE_B currfloor + 2 //NOTE THIS LEAVES SOME SLOTS OPEN!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! FIXME:FIXME:
/*actually, i won't be fixing this. i'm so fucking overworked i havent had time to shower in 4.5 days
  and this prof has the fucking gall to give the students who went to class a salty little deal n boost their grades
  jesus fuck my guy some people have other classes with massive workloads and not to mention two jobs
  and then he has the GUMPTION not to do course evals (even though i know they dont matter) to avoid having to read
  people complaining that him giving 20% grade boosters to students that show up to class might be unfair to people
  who need to pay for their own schooling or who have poor mental health. and you know what i'm fine with getting a 
  60 in this course, i just feel frustrated at the lack of compassion and of consideration this professor has. the
  only reason he granted me extensions for these previous assignments is because all the marking and stuff is done
  by his TAs so he couldnt give two shits when things are marked, not because he genuinely had any interest in the
  well-being of his students, or that he wished for them to succeed at all. i've wasted like 5 minutes writing this
  rant and that's far more than i can afford, but at least it felt good despite that no-one will read it. if you do
  read this rekkab thank you so much for correcting my a2 grade, and for the nice e-mail. you're a legend and
  would probably make a better prof than this guy*/

float squared(float n) {return n*n;}

//utility function. rand should be seeded before this is called, please! if you do this then you're the nicest. also min and max are inclusive!
int getRandomNumber(int min, int max) {
	return min + (rand() % (max - min + 1));
}

double getDistance2d(struct coord a, struct coord b) {
	double n_squared = (double)((b.x - a.x) * (b.x - a.x)); //not my usual style but i don't want to mess anything up!!
	double m_squared = (double)((b.z - a.z) * (b.z - a.z));
	return sqrt(n_squared + m_squared);
}

struct coord getMobCoord(int id) {
	return (struct coord){(int)getx(id), (int)gety(id), (int)getz(id)};
}

int determineRoom(struct coord pos) { //-1 means to find the player's room
	float x, y, z;
	int roomindex = -1;
	for (int i = meshIndex; i < meshIndex + NUMROOMS; i++) {
		if (pos.x > points[i].x && pos.x < points[i].x + dimensions[i].x) {
			if (pos.z > points[i].z && pos.z < points[i].z + dimensions[i].z) {
				//if (noncalverty > points[i].y && noncalverty < points[i].y + dimensions[i].y) {
				roomindex = i;
				break;
				//}
			}
		}
	}
	return roomindex;
}

long getTime() { //in ms
	struct timeval time;
	gettimeofday(&time, NULL);
	long s1 = (long)(time.tv_sec) * 1000;
	long s2 = (time.tv_usec / 1000);
	return s1 + s2;
}

struct coord generatePoint(int minx, int maxx, int minz, int maxz) { //i dont care that its shit you made me do this calvert. three rows
	struct coord point = {.x = getRandomNumber(minx, maxx), .y = FLOORHEIGHT, .z = getRandomNumber(minz, maxz)};
	//////printf("Generated point: %d %d %d\n", point.x, point.y, point.z);
	return point;
}

//offsets a point by some value. negative numbers work, obv
struct coord offsetPoint(struct coord a, int offsetx, int offsety, int offsetz) {
	struct coord b = {.x = a.x + offsetx, .y = a.y + offsety, .z = a.z + offsetz};
	return b;
}

void fillVolume(struct coord origin, int l, int w, int h, GLbyte colour) {
	//////printf("origin %d %d %d; l=%d, w=%d, h=%d\n", origin.x, origin.y, origin.z, l, w, h);
	for(int x = origin.x; x < origin.x+w; x++) {
		//////printf("x: %d\t", x);
		for(int y = origin.y; y < origin.y+h; y++) {
			//////printf("y: %d\t", y);
			for(int z = origin.z; z < origin.z+l; z++) {
				//////printf("z: %d\n", z);
				world[x][y][z] = colour;
			}
		}
	}
}

bool attack_check() {
	return getRandomNumber(0, 100) > 50 ? true : false;
}

bool adjacencyCheck(struct coord a, struct coord b) { //NOTE: doesn't check y!
	return abs(a.x - b.x) <= 1 && abs(a.z - b.z) <= 1;
}

/*** collisionResponse() ***/
/* -performs collision detection and response */
/*  sets new xyz  to position of the viewpoint after collision */
/* -can also be used to implement gravity by updating y position of vp*/
/* note that the world coordinates returned from getViewPosition()
	will be the negative value of the array indices */
void collisionResponse() {
	if (collisions) {
		float x, y, z, ox, oy, oz = 0.0;
		getViewPosition(&x, &y, &z);
		const float margin = 0.3; //calvert's favourite random number
		bool collisionX = false;
		bool ascendY = false;
		bool collisionZ = false;

		//getOldViewPosition(&ox, &oy, &oz);
		//setViewPosition(ox, oy, oz);
		if(world[-(int)(x-margin)][-(int)y][-(int)(z)] || world[-(int)(x+margin)][-(int)y][-(int)(z)] || -(int)x > WORLDX-1 || -(int)x <= 0) {
			//printf("x axis collision?\n");
			collisionX = true;
		}
		if (world[-(int)(x)][-(int)y][-(int)(z-margin)] || world[-(int)(x)][-(int)y][-(int)(z+margin)] || -(int)z > WORLDZ-1 || -(int)z <= 0) {
			//printf("z axis collision?\n");
			collisionZ = true;
		}

		if (collisionX || collisionZ) { //cis1500 tier code structure dont @ me fuckers because i _know_
			if(!(-(int)x > WORLDX-1 || -(int)z > WORLDZ-1 || -(int)x <= 0 || -(int)z <= 0) && world[-(int)(x-margin)][-(int)y + 1][-(int)(z-margin)] == 0 && world[-(int)(x+margin)][-(int)y + 1][-(int)(z+margin)] == 0) {
				ascendY = true;
			}
			getOldViewPosition(&ox, &oy, &oz);
			setViewPosition((collisionX ? ox : x), (ascendY ? y-1.0 : y), (collisionZ ? oz : z));
		}

		if (currfloor > 0) { //this loop would crash on the outside level, also it's not useful there anyways
			//this function checks if the player is colliding with any mobs on the floor
			for (int id = meshIndex; id < (currfloor * NUMROOMS); id++) {
				if (getActive(id) && getVisible(id)) {
					float mx = getx(id); // these arent calvertized for some reason. very good thank you mr calvreyt
					float mz = getz(id); // no need to check for y overlap because fuck you
					//printf("mx = %d mz = %d x = %d z = %d\n", -(int)mx, -(int)mz, -(int)x, -(int)z);
					if (-(int)x == (int)mx && -(int)z == (int)mz) {
						getOldViewPosition(&x, &y, &z);
						setViewPosition(x, y, z);
					}
				}
			}
		}
	}
}

enum direction determineVerticalDirection(struct coord mob, struct coord dest) {
	if (dest.z > mob.z) { //player is north somewhere -- also i say player but i mean destination
		return n;
	}
	else if (dest.z < mob.z) { //player is south somewhere
		return s;
	}
	return none;
}

enum direction determineHorizontalDirection(struct coord mob, struct coord dest) {
	if (dest.x > mob.x) { //player is east somewhere
		return e;
	}
	else if (dest.x < mob.x) { //player is west somewhere
		return w;
	}
	return none;
}

enum direction determineDirectionToDestination(int id, struct coord dest) {
	struct coord mob = getMobCoord(id); //in the wise words of dr. kremer: i could do this in a really clever way. OR, i could write 17 if statements
	enum direction horizontal = determineHorizontalDirection(mob, dest);
	enum direction vertical = determineVerticalDirection(mob, dest);
	if (horizontal != none && vertical != none) { //then we have a composite direction!
		return combineDirections(vertical, horizontal);
	}
	else if (horizontal != none) return horizontal;
	return vertical;
} //reworked to be a lil messier than the classy thing i had before... but it had to be done.

void moveInDirection(int id, enum direction dir) { //could have done this all so much more cleverly i know but i havent time to think
	const float speed = 0.75;
	float x = getx(id);
	float y = gety(id);
	float z = getz(id);
	switch(dir) {
		case n:
			setTranslateMesh(id, x, y, z + speed);
			break;
		case ne:
			setTranslateMesh(id, x + speed, y, z + speed);
			break;
		case e:
			setTranslateMesh(id, x + speed, y, z);
			break;
		case se:
			setTranslateMesh(id, x + speed, y, z - speed);
			break;
		case s:
			setTranslateMesh(id, x, y, z - speed);
			break;
		case sw:
			setTranslateMesh(id, x - speed, y, z - speed);
			break;
		case w:
			setTranslateMesh(id, x - speed, y, z);
			break;
		case nw:
			setTranslateMesh(id, x - speed, y, z + speed);
			break;
		default:
			;
			//printf("Cannot move in %d (probably none) direction\n", dir);
	}
}

/*int chooseBestDoor(struct coord pos, int dest_index) {
	//if the room you're in is an h-joiner and you need it, take it, otherwise path down or up as needed
	int cur_index = determineRoom(pos);
	if (points[dest_index].x < points[cur_index].x) {
		for (int i = 0; i < 2; i++) {
			for (int j = 0; )
		}
	}
	return lowest_index;
}*/

bool mobCollisionResponse(int id) {
	bool collisionX, collisionZ = false;
	bool collisionMob = false;
	if (getActive(id) && getPresent(id)) {
		//int dest = dest = getDestination(id);
		float x = getx(id);
		float y = gety(id);
		float z = getz(id);
		float ox = getoldx(id);
		float oy = getoldy(id);
		float oz = getoldz(id);

		int mob_hit = -1;

		const float margin = 0.3;
		//the following could definitely be improved:
		if(world[(int)(x-margin)][(int)y][(int)(z)] || world[(int)(x+margin)][(int)y][(int)(z)] || (int)x > WORLDX-1 || (int)x < 0) {
			//printf("x axis collision?\n");
			collisionX = true;
		}
		if (world[(int)(x)][(int)y][(int)(z-margin)] || world[(int)(x)][(int)y][(int)(z+margin)] || (int)z > WORLDZ-1 || (int)z < 0) {
			//printf("z axis collision?\n");
			collisionZ = true;
		}

		if (currfloor > 0) { //this shouldnt trigger on floor0 but we'll check just in case, since it WILL crash if it somehow doees
			struct coord mob = getMobCoord(id);
			for (int i = meshIndex; i < (currfloor * NUMROOMS); i++) { //FIXME: INFROOMS doesn't scale right!
				if(id != i && getActive(id)) {
					struct coord other_mob = getMobCoord(i);
					if (mob.x == other_mob.x && mob.y == other_mob.y && mob.z == other_mob.z) {
						resetTranslateMesh(id); //turns are kinda wasted when they run into walls tho
						collisionMob = true; //just collision SOMETHing, for the return value
						mob_hit = meshIndex;
					}
				}
			}
		}

		//if we're not an arrow, we'll only move in the direction that doesn't put us inside a wall. otherwise, we die.
		if (collisionX || collisionZ || collisionMob) { //cis1500 tier code structure dont @ me fuckers because i _know_
			if (getMeshNumber(id) != 18) {
				setTranslateMesh(id, (collisionX ? ox : x), y, (collisionZ ? oz : z));
			}
			else {
				if (collisionMob) {
					//we have struck the mob with the id "mob_hit"! (actually, this is the LAST mob we hit but since they can't overlap...)
				}
				setActive(id, false); //note that setActive has the "side effect" of also setting the mob to invisible.
				setPresent(id, false);
				unsetMeshID(id);
			}
		}
		/*if (world[mob.x-margin][mob.y][mob.z] || world[mob.x+margin][mob.y][mob.z]) { //collision on the x axis
			resetTranslateMesh(id);
			correction = determineVerticalDirection(mob, (dest == -1 ? curr_viewpos : offsetPoint(points[dest], 2, 0, 2)));
			//printf("correctionA = %d\n", correction);
			moveInDirection(id, correction);
		}
		if (world[mob.x][mob.y][mob.z-margin] || world[mob.x][mob.y][mob.z+margin]) { //collision on the z axis
			resetTranslateMesh(id);
			correction = determineHorizontalDirection(mob, (dest == -1 ? curr_viewpos : offsetPoint(points[dest], 2, 0, 2)));
			//printf("correctionB = %d\n", correction);
			moveInDirection(id, correction);
		}
		if (mob.x > WORLDX-1 || mob.z > WORLDZ-1 || mob.x < 0 || mob.z < 0) { //out of bounds, just reset!
			resetTranslateMesh(id);
		}*/
	}
	return (collisionX || collisionZ || collisionMob);
}



void drawPlayerMap() {
	float px, py, pz;
	getViewPosition(&px, &py, &pz);
	int ix, iy, iz;
	ix = -(int)px;
	iy = -(int)py;
	iz = -(int)pz;
	GLfloat red[] = {1.0, 0.0, 0.0, 0.5};
	set2Dcolour(red);
	draw2Dtriangle((ix + 3) * scaling, iz * scaling, ix * scaling, iz * scaling, ix * scaling, (iz + 3) * scaling);
}

void drawRoomMap(int id) {
	GLfloat black[] = {0.0, 0.0, 0.0, 0.5};
	set2Dcolour(black);
	//printf("drawing box at %d %d %d %d\n", points[id].x, points[id].z, points[id].x + dimensions[id].x, points[id].z + dimensions[id].z);
	draw2Dbox(points[id].x * scaling, points[id].z * scaling, (points[id].x + dimensions[id].x) * scaling, (points[id].z + dimensions[id].z) * scaling);
}

void drawStairsMap() {
	GLfloat green[] = {0.0, 1.0, 0.0, 0.5}; //for down
	GLfloat blue[] = {0.0, 0.2, 1.0, 0.5}; //for up
	set2Dcolour(green);
	draw2Dbox(floors[currfloor]->stairs[STAIRS_DOWN].x * scaling, floors[currfloor]->stairs[STAIRS_DOWN].z * scaling, 
			  (floors[currfloor]->stairs[STAIRS_DOWN].x + 1) * scaling, (floors[currfloor]->stairs[STAIRS_DOWN].z + 1) * scaling);
	if (currfloor > 0) {
		set2Dcolour(blue);
		draw2Dbox(floors[currfloor]->stairs[STAIRS_UP].x * scaling, floors[currfloor]->stairs[STAIRS_UP].z * scaling, 
			  	 (floors[currfloor]->stairs[STAIRS_UP].x + 1) * scaling, (floors[currfloor]->stairs[STAIRS_UP].z + 1) * scaling);
	}
}

//draw2Dline: int x1, int y1, int x2, int y2, int lineWidth

void drawJoiningRoomMap(int r1_index, int r2_index) {
	//printf("currfloor is %d\n", currfloor);
	const int w = 3;
	GLfloat black[] = {0.0, 0.0, 0.0, 0.5};
	set2Dcolour(black);
	struct coord corner_for_z_join = offsetPoint(points[r1_index], 0, 0, dimensions[r1_index].z);
	draw2Dline(points[r1_index].x * scaling, corner_for_z_join.z * scaling, points[r2_index].x * scaling, points[r2_index].z * scaling, w);
	for (int i = SIDE_A; i <= SIDE_B; i++) {
		//printf("i=%d\n", i);
		struct coord corner_for_x_join = offsetPoint(points[hjoined[i][SIDE_A]], dimensions[hjoined[i][SIDE_A]].x, 0, 0);
		draw2Dline(corner_for_x_join.x * scaling, points[hjoined[i][SIDE_A]].z * scaling, points[hjoined[i][SIDE_B]].x * scaling, points[hjoined[i][SIDE_B]].z * scaling, w);
	} //TODO: revamp this to scale, to fix the (secret little bug) that nobody will notice.                        1mo later: fuck i dont't even notice it
}

void drawMobs() {
	GLfloat red[] = {1.0, 0.0, 0.0, 0.5};
	GLfloat black[] = {0.0, 0.0, 0.0, 0.87};
	GLfloat cactusgreen[] = {0.2, 1.0, 0.2, 0.5};
	for(int i = meshIndex; i <= meshIndex + NUMROOMS; i++) {
		int visible = getVisible(i);
		int mob_type = getMeshNumber(i);
		switch(mob_type) {
			case 1:
				set2Dcolour(red);
				break;
			case 2:
				set2Dcolour(black);
				break;
			case 3:
				set2Dcolour(cactusgreen);
				break;
			default:
				set2Dcolour(red);
		}
		//printf("mesh %d's visibility is %d\n", i, visible);
		//don't draw arrows on the map
		if (getActive(i) && getMeshNumber(i) != 18) {
			int x = (int)getx(i);
			int z = (int)getz(i);
			draw2Dbox(x * scaling, z * scaling, (x + 1) * scaling, (z + 1) * scaling);
		}
	}
}

void drawRandomBlocks(int i) {
	GLfloat yellow[] = {1.0, 1.0, 0.0, 0.5};
	set2Dcolour(yellow);
	if (currfloor > 0) {
		draw2Dbox(random_things[i].x * scaling, random_things[i].z * scaling, 
				 (random_things[i].x + 1) * scaling, (random_things[i].z + 1) * scaling);
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
		//also we should draw the map (jank but i have 8 hours left):
		GLfloat black[] = {0.0, 0.0, 0.0, 0.5};
		if (displayMap == 1) {
			if (currfloor > 0) {
				drawMobs();
				for (int i = meshIndex; i < meshIndex + NUMROOMS; i++) {
					drawRandomBlocks(i);
					drawStairsMap();
					drawPlayerMap();
					drawRoomMap(i);
				}
				//printf("meshindex is %d and currfloor is %d\n", meshIndex, currfloor);
				for(int i = meshIndex; i < meshIndex + ((NUMROOMS/3) * 2); i++) { //i think my math on numrooms/3 * 2 is wrong but tbh in this case we want 6 so it works (9/3 * 2 = 6)
					drawJoiningRoomMap(i, i+(NUMROOMS/3)); //3 would be NUMROWS if calvert's generation was well-advised but i fear he may change it any time
				}
			}
			else {
				drawPlayerMap();
				drawStairsMap();
				set2Dcolour(black);
				draw2Dbox(0, 0, WORLDX * scaling, WORLDZ * scaling);
			}
		}
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
		float xp, yp, zp;
		getViewPosition(&xp, &yp, &zp);
		printf("viewpos %f %f %f\n", xp, yp, zp);
	}
}

/*(all ints pls) origin, l w h, colour.
 the origin coords will point to the bottom left of the room*/
void createRoom(struct coord origin, int l, int w, int h, GLbyte colour) { //FIXME: could be done faster.
	//dx, dy, dz = drwaing x,y,z. the n stands for nega
	GLbyte floorcol = colour - 1;
	for (int dy = origin.y; dy <= origin.y + h; dy++) {
		for (int dx = origin.x; dx <= origin.x + w; dx++) {
			world[dx][dy][origin.z] = colour;
			world[dx][dy][origin.z + l] = colour;
			for (int floorz = origin.z; floorz <= origin.z + l; floorz++) {
				world[dx][origin.y][floorz] = floorcol;
				world[dx][origin.y + h][floorz] = colour; //roof should match the walls
			}
		}
		for (int dz = origin.z; dz <= origin.z + l; dz++) {
			world[origin.x][dy][dz] = colour;
			world[origin.x + w][dy][dz] = colour;
		}
	}
	fillVolume(offsetPoint(origin, 1, 1, 1), l-1, w-1, h-1, 0); //why didnt i do this before.
} //i like pot_hole but i guess i'll SnakeStance this shit

struct coord getMidpoint(struct coord a, struct coord b) {
	struct coord midpoint;
	midpoint.x = (a.x + b.x) / 2; //int math
	midpoint.y = FLOORHEIGHT;
	midpoint.z = (a.z + b.z) / 2;
	return midpoint;
} //FIXME: not deprecated! :)

struct coord getRandomDimensions(int min, int max) {
	//this is the brain damaged version of the above function. rooms cant exceed max size in any dimension.
	struct coord dimension;
	dimension.x = getRandomNumber(min, max);
	dimension.y = getRandomNumber(min, max);
	dimension.z = getRandomNumber(min, max);
	//////printf("generated dimensions: %d %d %d\n", dimensions.x, dimensions.y, dimensions.z);
	return dimension; //why has god forsaken me i just wanted to use a quadtree.
} //x = w; y = h; z = l

int meshVisibilityCheck(int id){
	//printf("mesh #%d's active status is (%d) and presence is (%d)\n", getActive(id), getPresent(id));
	if (getActive(id) && getPresent(id)) {
		struct coord mob = getMobCoord(id);
		const double viewDistance = 40.0;
		int visible = SphereInFrustum((float)mob.x, (float)(mob.y + 1), (float)mob.z, 1.5); //we'll just pretend y doesnt exist
		if (getDistance2d(curr_viewpos, mob) > viewDistance) {
			visible = 0;
		} //if the object is too far we'll say its not visible
		if (visible) {
			//printf("Cow mesh #%d is visible\n",id);
			drawMesh(id);
		} else if (!visible){ //TODO: performance will tank if tehres tons of mobs even if they're invisible won't it?
			//printf("Cow mesh #%d is not visible\n",id);
			hideMesh(id);
		}
	}
}

bool proximityCheck(int pindex, int margin) { //idc idgaf that its bad this is not my fault i just wanted qtree
	bool result = true; //yes i know this doesnt account for the dimensions of point 2 but dont fuCking worry ab it
	//int NUMROOMS = sizeof(points)/sizeof(*points); //ONLY PASS AN ARRAY HERE!!!
	//////printf("NUMROOMS resolves to %d\n", NUMROOMS);
	struct coord point1_max;
	int longer_side = 0;
	point1_max.x = points[pindex].x + dimensions[pindex].x;
	point1_max.y = FLOORHEIGHT;
	point1_max.z = points[pindex].z + dimensions[pindex].z;
	struct coord center = getMidpoint(points[pindex], point1_max); //this is the center of our circle;
	if (dimensions[pindex].x > dimensions[pindex].z) {
		longer_side = dimensions[pindex].x;
	} else {longer_side = dimensions[pindex].z;} //the longer side len / 2 will be used as the radius
	int r1 = (longer_side / 2) + margin;
	//TODO: optimally, this would only check adjacents, but i dont have time to do that right now
	for(int i = meshIndex; i < meshIndex + NUMROOMS; i++) {
		if (!(i == pindex)) {
			struct coord point2_max;
			int longer_side_r2 = 0;
			point2_max.x = points[i].x + dimensions[i].x;
			point2_max.y = FLOORHEIGHT;
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

void createDoor(struct coord a, int w, int h) {
	for (int dw = a.x; dw < a.x + w; dw++) {
		for (int dh = a.y + 1; dh < a.y + h; dh++) {
			world[dw][dh][a.z] = 0;
		}
	}
} //FIXME: deprecated: bad

void joinRooms(int r1_index, int r2_index, const int w, const int h, GLbyte colour) {
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
			//////printf("todoroki %d\n", points[r2_index].x - points[r1_index].x); //w is passed as the l because its horizontal
			createRoom(offsetPoint(points[r1_index], 0, 0, mid + dimensions[r1_index].z), w, points[r2_index].x - points[r1_index].x + w, h, colour); //TODO: + w in w dubious?
			fillVolume(offsetPoint(points[r1_index], 1, 1, mid + dimensions[r1_index].z + 1), w - 1, points[r2_index].x - points[r1_index].x + w - 1, h - 1, 0);
		} else{ //x-
			//////printf("else %d\n", points[r1_index].x - points[r2_index].x);
			createRoom(offsetPoint(points[r2_index], 0, 0, -mid), w, points[r1_index].x - points[r2_index].x + w, h, colour);
			fillVolume(offsetPoint(points[r2_index], 1, 1, -mid + 1), w - 1, points[r1_index].x - points[r2_index].x + w - 1, h - 1, 0);
		}
		//clear the vertical tunnels
		fillVolume(offsetPoint(points[r1_index], 1, 1, dimensions[r1_index].z), mid + w - 1, w - 1, h - 1, 0);
		fillVolume(offsetPoint(points[r2_index], 1, 1, -mid + 1), mid + w - 1, w - 1, h - 1, 0);
		//////printf("%d/%d's corridor pair generated.\n", r1_index, r2_index);
	}
	else { //mode 1 - z*
		mid = ((points[r2_index].x - (points[r1_index].x + dimensions[r1_index].x)) / 2) - 1;
		bool odd = !(mid % 2);
		//draw the corridors from and to two rooms
		createRoom(offsetPoint(points[r1_index], dimensions[r1_index].x, 0, 0), w, mid, h, colour); //offset wrong?
		createRoom(offsetPoint(points[r2_index], -mid, 0,  0), w, mid, h, colour);
		//draw the horizontal (connecting) corridor
		if (points[r1_index].z < points[r2_index].z) { //z+?
			//////printf("todoroki %d\n", points[r2_index].z - points[r1_index].z); //w is passed as the l because its horizontal
			createRoom(offsetPoint(points[r1_index], mid + dimensions[r1_index].x, 0, 0), points[r2_index].z - points[r1_index].z + w, w, h, colour); //TODO: + w in w dubious?
			fillVolume(offsetPoint(points[r1_index], mid + dimensions[r1_index].x + 1, 1, 1), points[r2_index].z - points[r1_index].z + w - 1, w - 1, h - 1, 0);
		} 
		else{ //z-
			//////printf("else %d\n", points[r1_index].z - points[r2_index].z);
			createRoom(offsetPoint(points[r2_index], -(mid + w), 0, 0), points[r1_index].z - points[r2_index].z + w, w, h, colour);
			fillVolume(offsetPoint(points[r2_index], -(mid + w) + 1, 1, 1), points[r1_index].z - points[r2_index].z + w - 1, w - 1, h - 1, 0);
		}
		//clear the "horizontal tunnels"
		fillVolume(offsetPoint(points[r1_index], dimensions[r1_index].x, 1, 1), w - 1, mid + 1, h - 1, 0); //aioubdiuq3TODO: THIS FIXME: THIS
		fillVolume(offsetPoint(points[r2_index], (-mid), 1, 1),  w - 1, mid + 1, h - 1, 0);
		//////printf("%d/%d's corridor pair generated.\n", r1_index, r2_index);
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

void generateCave() {
	//zeroth, let's clear everything:
	for (int x = 0; x < WORLDX; x++) {
		for (int y = 0; y < WORLDY; y++) {
			for (int z = 0; z < WORLDZ; z++) {
				world[x][y][z] = 0;
			}
		}
	}
	//first, let's generate the flat floor: (this could be combined with the last loop but idc)
	for (int x = 0; x < WORLDX; x++) {
		for (int z = 0; z < WORLDZ; z++) {
			world[x][FLOORHEIGHT][z] = 26;
		}
	}

	const float max_height = 25.0;
	const float scalar = 15.0;
	const float offset = (float)getRandomNumber(0,GL_MAX);
	float y_offset = 0.0;
	float x, y, z = 0.0;
	//now, let's make the cave:
	for (int x_world = 0; x_world < WORLDX; x_world++) { //note that x_world & z_world are integers!~
		x = ((float)(x_world - WORLDX/2))/(float)(WORLDX/2);
		for (int z_world = 0; z_world < WORLDZ; z_world++) {
			z = ((float)(z_world - WORLDZ/2))/(float)(WORLDZ/2);
			y_offset = perlin((float)x_world / scalar + offset, (float)z_world / scalar + offset);
			y = (1 - (squared(x) + squared(z)) / 2.0) + y_offset;
			int y_world = (int)(y * max_height) + FLOORHEIGHT;
			for (int thickness = 0; thickness < 5; thickness++) {
				world[x_world][y_world-thickness][z_world] = 26;
			}
		}
	}

	//now, let's generate the mobs:
	for (int cow_room = meshIndex; cow_room < (currfloor * NUMROOMS); cow_room++) {
		int mesh_identity = 1; //they're all fish
		
		//let's just spawn em at random spots
		//we need to move the fish up more cuz if we dont it spawns in the ground lol
		struct coord cow_pos = {getRandomNumber(3,WORLDX-3),
								FLOORHEIGHT + 2,
								getRandomNumber(3,WORLDZ-3)}; 

		//cow_room actually also represents the index of the mesh, conveniently. the math just checks out, maybe i should rename it ?
		setMeshID(cow_room, mesh_identity, (float)cow_pos.x,
							   			   (float)cow_pos.y, //he flies
							   			   (float)cow_pos.z); //don't forget to de-calvertize
	}

	floors[currfloor] = calloc(1, sizeof(struct record));

	//the stairs will just be placed randomly somewhere
	floors[currfloor]->stairs[STAIRS_UP]   = (struct coord){getRandomNumber(10, WORLDX-10), FLOORHEIGHT, getRandomNumber(10, WORLDZ-10)};
	floors[currfloor]->stairs[STAIRS_DOWN] = (struct coord){getRandomNumber(10, WORLDX-10), FLOORHEIGHT, getRandomNumber(10, WORLDZ-10)};
	printf("stairs1 %d %d %d stairs2 %d %d %d\n", floors[currfloor]->stairs[STAIRS_UP].x, floors[currfloor]->stairs[STAIRS_UP].y, floors[currfloor]->stairs[STAIRS_UP].z, floors[currfloor]->stairs[STAIRS_DOWN].x, floors[currfloor]->stairs[STAIRS_DOWN].y, floors[currfloor]->stairs[STAIRS_DOWN].z);
	//HERE'S HOPING THEY DON'T OVERLAP LOL
	world[floors[currfloor]->stairs[STAIRS_UP].x][floors[currfloor]->stairs[STAIRS_UP].y][floors[currfloor]->stairs[STAIRS_UP].z] = 24;       //creates up
	world[floors[currfloor]->stairs[STAIRS_DOWN].x][floors[currfloor]->stairs[STAIRS_DOWN].y][floors[currfloor]->stairs[STAIRS_DOWN].z] = 25; //creates down

	//spawn should be near the stairs
	struct coord spawn = offsetPoint(floors[currfloor]->stairs[STAIRS_UP], getRandomNumber(4,7), 2, getRandomNumber(4,7));

	//now, we'll handle saving.
	for(int sx=0; sx<WORLDX; sx++) {
		for(int sy=0; sy<WORLDY; sy++) {
			for(int sz=0; sz<WORLDZ; sz++) {
				floors[currfloor]->world[sx][sy][sz] = world[sx][sy][sz];
			}
		}
	}
	floors[currfloor]->spawn = spawn;
	curr_viewpos.x = spawn.x; curr_viewpos.y = spawn.y; curr_viewpos.z = spawn.z;
	setViewPosition((float)spawn.x, (float)spawn.y, (float)spawn.z);
}

void generateMaze() {
	//just read i need to generate the maze in three rows
	//three rows >:(
	//imagine making a perfectly fine quadtree then being forced to mutilate it into "three rows" waht a stupid clause
	setUserColour(24, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0); //for up stairs
	setAssignedTexture(24, 31);
	setUserColour(25, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0); //for down stairs
	setAssignedTexture(25, 32); //please
	/*in order to generate "three rows", i will do the following instead of just normally using the quadtree as described in lecture
	  (i really do not understand why you'd detail a really nice method like quadtree and then make everyone generate a completely
	  different structure. seems pretty mean-spirited to me and i really wish i could just generate 9 rooms in any way i want): i will
	  generate 9 points in predefined zones, then then generate 9 rooms of random sizes in three three "rows" that you have described.
	  these rooms will also be guaranteed to be at least 1 block away from each other and not intersect. why cant i just quadtree*/
	////printf("beginnign maze generation!\n");
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

	struct coord stairs[2]; //shouldn't this actually always be 2 since its stored in the floors[] array...
	struct coord spawn;

	const int w = 5;
	const int h = 5; //could make this random, min = 2, max = dimensions[i].h; but i dont really care
	GLbyte colour = 18;
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
		printf("generation attempt\n");
		//the following code generates 9 rooms in 9 zones with padding so they don't intersect.
		points[meshIndex + 0] = generatePoint(0, borderx1 - padding, 0, borderz1 - padding);
		dimensions[meshIndex + 0] = getRandomDimensions(min_d, max_d);
		points[meshIndex + 1] = generatePoint(borderx1 - padding, borderx2 - padding, 0, borderz1 - padding);
		dimensions[meshIndex + 1] = getRandomDimensions(min_d, max_d);
		points[meshIndex + 2] = generatePoint(borderx2 + padding, WORLDX - max_d, 0, borderz1 - padding);
		dimensions[meshIndex + 2] = getRandomDimensions(min_d, max_d);
		points[meshIndex + 3] = generatePoint(0, borderx1 - padding, borderz1 + padding, borderz2 - padding);
		dimensions[meshIndex + 3] = getRandomDimensions(min_d, max_d);
		points[meshIndex + 4] = generatePoint(borderx1, borderx2, borderz1, borderz2);
		dimensions[meshIndex + 4] = getRandomDimensions(min_d, max_d);
		points[meshIndex + 5] = generatePoint(borderx2 + padding, WORLDX - max_d, borderz1 + padding, borderz2 - padding);
		dimensions[meshIndex + 5] = getRandomDimensions(min_d, max_d);
		points[meshIndex + 6] = generatePoint(0, borderx1 - padding, borderz2 + padding, WORLDZ - max_d);
		dimensions[meshIndex + 6] = getRandomDimensions(min_d, max_d);
		points[meshIndex + 7] = generatePoint(borderx1 + padding, borderx2 - padding, borderz2 + padding, WORLDZ - max_d);
		dimensions[meshIndex + 7] = getRandomDimensions(min_d, max_d);
		points[meshIndex + 8] = generatePoint(borderx2 + padding, WORLDX - max_d, borderz2 + padding, WORLDZ - max_d);
		dimensions[meshIndex + 8] = getRandomDimensions(min_d, max_d); //"14 if statements" type beat because you hate quadtrees @dcalvert
		for (int i = meshIndex; i < meshIndex + NUMROOMS; i++) {
			createRoom(points[i], dimensions[i].z, dimensions[i].x, dimensions[i].y, 18); //lwh args are out of order oops
			//oh, drawing the map like this is gonna be tricky to do with fog of war isnt it...
			if (displayMap == 1) draw2Dbox(points[i].x, points[i].y, points[i].x + dimensions[i].x, points[i].y + dimensions[i].y); 
			//TODO: maybe needs to be calvertized? also, this has some pretty bad coupling with other functions
			//we can also create the one random block in the rooms now:
			struct coord randomBlockPos;
			randomBlockPos = generatePoint(points[i].x+1, points[i].x + dimensions[i].x - 1, points[i].z + 1, points[i].z + dimensions[i].z - 1);
			random_things[i] = randomBlockPos;
			world[randomBlockPos.x][randomBlockPos.y + 1][randomBlockPos.z] = 23;
		}
		for(int c = meshIndex; c < meshIndex + NUMROOMS; c++) {
			if((points[c].x + dimensions[c].x) >= WORLDX || (points[c].z + dimensions[c].z) >= WORLDZ || !proximityCheck(c, 1)) {
				generationSuccess = false;
				break;
			}
		} //the check loop. could ideally be integrated elsewhere but i don't care i don't ahve time to think too much about these things
	} while(!generationSuccess);
	//also, if everything is good to go, generate 2 stairs in random rooms, but not the same room:
	//printf("meshindex is %d\n", meshIndex);
	int ds = meshIndex + getRandomNumber(0, NUMROOMS-1); //short for "downstairs", it's the index of the room that this staircase will be in
	int us = meshIndex + getRandomNumber(0, NUMROOMS-1); //    "      "upstairs",                           "
	if(ds == us) {us = meshIndex + ((ds + us) % NUMROOMS);}
	struct coord dspoint = generatePoint(points[ds].x+1, points[ds].x + dimensions[ds].x - 1, points[ds].z + 1, points[ds].z + dimensions[ds].z - 1);
	struct coord uspoint = generatePoint(points[us].x+1, points[us].x + dimensions[us].x - 1, points[us].z + 1, points[us].z + dimensions[us].z - 1);
	stairs[STAIRS_UP] = uspoint;
	stairs[STAIRS_DOWN] = dspoint;
	world[stairs[STAIRS_UP].x][stairs[STAIRS_UP].y][stairs[STAIRS_UP].z] = 24; //creating up
	world[stairs[STAIRS_DOWN].x][stairs[STAIRS_DOWN].y][stairs[STAIRS_DOWN].z] = 25; //creating down
	
	/*imagine doing things programmatically... OH WAIT, i cant, because you took away my quadtree @dcalvert
	anyways, now, because @dcalvert is a literal hater and hates people using fitting and appropriate algorithms
	to procedurally generate mazes as he describes in HIS OWN LECTURES, i will continue this trend of simply basically
	hard-coding everything. thanks @dcalvert. let me remind you, TA reading this, i really did not want to do this. i 
	really really wanted to use a quadtree. but @dcalvert forced my hand.*/
	/*okay, i've taken a week off after i submitted that terrible first landmark
	i've fully accepted that i wasted a whole lot of time trying to make a different program
	it's best i move on from that. also deleted a bunch of really angry comments. keeping the above one though*/
		
	//the corridors are just rooms LLOL iof u really THINK about it
	for(int i = meshIndex; i < meshIndex + ((NUMROOMS/3) * 2); i++) { //i think my math on numrooms/3 * 2 is wrong but tbh in this case we want 6 so it works (9/3 * 2 = 6)
		joinRooms(i, i+(NUMROOMS/3), w, h, colour); //3 would be NUMROWS if calvert's generation was well-advised but i fear he may change it any time
	}
	/*now let's connect the rows to make an H joining shape:
	this uses a "jump table" of sorts because i am quite lazy right now and couldn't think of a mathematically
	better way to do "choose two" in order to pick the 2 sets of rooms to join (by their index).*/
	int corridors[2][3][2] = {{{0 + meshIndex, 1 + meshIndex},
							{3 + meshIndex, 4 + meshIndex},{6 + meshIndex, 7 + meshIndex}},
							{{1 + meshIndex, 2 + meshIndex},
							{4 + meshIndex, 5 + meshIndex}, {7 + meshIndex, 8 + meshIndex}}};
	//those are the valid pairs, we pick one pair from each set:
	int chosenindex = getRandomNumber(0, 2);
	
	hjoined[SIDE_A][SIDE_A] = corridors[0][chosenindex][0]; //hjoined [0][x] contains the first pair of rooms, [1][x] the second
	hjoined[SIDE_A][SIDE_B] = corridors[0][chosenindex][1];
	joinRooms(corridors[0][chosenindex][0], corridors[0][chosenindex][1], w, h, colour);
	chosenindex = getRandomNumber(0, 2);
	hjoined[SIDE_B][SIDE_A] = corridors[1][chosenindex][0];
	hjoined[SIDE_B][SIDE_B] = corridors[1][chosenindex][1];
	joinRooms(corridors[1][chosenindex][0], corridors[1][chosenindex][1], w, h, colour);
	//instead of using functions, i dont want to. i just dont care. this is so borked idc idgaf why three rows dude just why
	//three rows
	//the next two create the player but he is kinda scary so let's just not for now
	//getViewPosition(&x, &y, &z);
	//createPlayer(0, x-5, -y, z-5, 0.0);
	//TODO: change back after debug to "us" - what did i mean by this. i forget which var us is. STAIRS_UP?
	
	//printf("us = %d, ds = %d\n", us, ds);
	spawn = offsetPoint(points[us], (dimensions[us].x / 2) + getRandomNumber(1,3), 3, (dimensions[us].z / 2) + getRandomNumber(1,3)); //spawn beside the up stairs
	////printf("attemptint to spawn er in %d %d %d\n", -spawn.x, -spawn.y, -spawn.z);
	//and now, we write the maze's info to the lowestmost un-written index
	int seek = 0;
	////printf("saving.\n");
	while (floors[seek]) {seek++;} //? i think?
	floors[seek] = calloc(1, sizeof(struct record));

	//STOP. cow generation time.
	//it pains me to make another NUMROOMS length for loop, but I can't generate the cows until I know all the rooms are good...
	for (int cow_room = meshIndex; cow_room < (currfloor * NUMROOMS); cow_room++) {
		//they're all cows, you can't stop me. the cow model is just too cute
		//edit: no longer are they all cows. in fact, none of them are cows. in memoria nominis vaccarum ego servabo

		int mesh_identity = getRandomNumber(1,3);

		//we need to move the fish up more cuz if we dont it spawns in the ground lol
		struct coord cow_pos = {.x = points[cow_room].x + (dimensions[cow_room].x/2),
								.y = FLOORHEIGHT + (mesh_identity == 1 ? 2 : 1),
								.z = points[cow_room].z + (dimensions[cow_room].z/2)}; 

		printf("Generating 'cow' #%d at %d, %d, %d.\n", cow_room, cow_pos.x, cow_pos.y, cow_pos.z);

		//cow_room actually also represents the index of the mesh, conveniently. the math just checks out, maybe i should rename it ?
		setMeshID(cow_room, mesh_identity, (float)cow_pos.x,
							   				(float)cow_pos.y, //he flies
							   				(float)cow_pos.z); //don't forget to de-calvertize
	}

	for(int sx=0; sx<WORLDX; sx++) {
		for(int sy=0; sy<WORLDY; sy++) {
			for(int sz=0; sz<WORLDZ; sz++) {
				//////printf("saving %d %d %d\n", sx, sy, sz);
				floors[seek]->world[sx][sy][sz] = world[sx][sy][sz];
			}
		}
	}
	floors[seek]->spawn = spawn;
	floors[seek]->stairs[STAIRS_UP] = stairs[STAIRS_UP];
	floors[seek]->stairs[STAIRS_DOWN] = stairs[STAIRS_DOWN];
	////printf("done!\n");
	//printf("points[0] = %d %d %d\n", points[0].x, points[0].y, points[0].z);
	setViewPosition(-(float)spawn.x, -(float)spawn.y, -(float)spawn.z);
	//remember not to try to load after the maze is generated, the generation IS the loading
}

bool check_timing(time_t target_time, time_t current_time) {
	return (current_time >= target_time) ? true : false;
}

void plant(int id) {
	if (getVisible(id) && getPresent(id) && getActive(id)) {
		bool adj = adjacencyCheck(curr_viewpos, getMobCoord(id));
		//printf("%d adjacent: %d\n", id, adj);
		if (adj) {
			if(attack_check()) {
				printf("The plant has struck its target.\n");
			}
			else {
				printf("The plant whiffed. Hard.\n");
			}
		}
	}
}

void responsive(int id) {
	if (getVisible(id) && getPresent(id) && getActive(id)) {
		struct coord mob = getMobCoord(id);
		int mob_room = determineRoom(getMobCoord(id));
		if (currfloor == 2 || currfloor == 4) {
			if (getDistance2d(mob, curr_viewpos) < 10.0) {
				setChase(id, true);
			}
		} else {
			if (determineRoom(curr_viewpos) == mob_room/* && mob_room != -1*/) {
				//printf("playerroom = %d, mobroom = %d\n", determineRoom(-1), determineRoom(id));
				setChase(id, true); //doesnt really matter if this gets set true when they're both in a hallway
			}
		}
		if (adjacencyCheck(mob, curr_viewpos)) {
			//printf("adj? %d %d, %d %d\n", mob.x, mob.z, curr_viewpos.x, curr_viewpos.z);
			if(attack_check()) {
				printf("The shark has successfully chomped. %d\n", id);
			}
			else {
				printf("Epic fail: the shark missed the chomp! %d\n", id);
			}
		} else if (getChase(id)) {
			enum direction dir = determineDirectionToDestination(id, curr_viewpos);
			moveInDirection(id, dir);
		}
	}
}

void search(int id) {
	//this mob actually moves around even while invisible, while it's not in chase mode:
	if (getPresent(id) && getActive(id)) {
		setChase(id, (bool)getVisible(id));
		bool chasing = getChase(id);
		struct coord mob = getMobCoord(id);
		if (chasing) { //if the player is in our sights we chase like normal
			if (adjacencyCheck(mob, curr_viewpos)) {
				//printf("adj? %d %d, %d %d\n", mob.x, mob.z, curr_viewpos.x, curr_viewpos.z);
				if(attack_check()) {
					printf("The bat. %d\n", id);
				}
				else {
					printf("bat sux! %d\n", id);
				}
			} else {
				enum direction dir = determineDirectionToDestination(id, curr_viewpos);
				moveInDirection(id, dir);
			}
		} else { //otherwise, we meander around
			int dest = getDestination(id);
			if(dest == -1 || (points[dest].x + 2 == mob.x && points[dest].z + 2 == mob.z)) { //NOTE: +2 so it doenst just ram into walls!!
				//if there's no destination or we're at the destination, pick a new one:
				int choice = getRandomNumber(0, NUMROOMS-1);
				dest = choice;
				setDestination(id, choice); //picks the index of the room it's going to randomly path to the door of
			}
			enum direction dir = determineDirectionToDestination(id, offsetPoint(points[dest], 2, 0, 2)); //to accomodate the +2
			moveInDirection(id, dir);
		}
	}
}

void arrow(int id) { //arrow ai

}

void schuut() {
	const register int arrow = MAXMESH-1; //it's register just for fun.
	const int arrow_id = 18;
	printf("oh it's hittin baby\n");
	if(!getMeshUsed(arrow) && getMeshNumber(arrow) != arrow_id) { 
		//then, since i'm putting the arrow at index MAXMESH-1 every time, we know it's not in flight, and so we can shoot another!
		float x, y, z = 0.0;
		float rx, ry, rz = 0.0;
		getViewPosition(&x, &y, &z);
		getViewOrientation(&rx, &ry, &rz); //rx is pitch, ry is yaw? i believe they're inverse though...
		x = -x; y = -y; z = -z;
		printf("Here's the deets: x=%f y=%f, z=%f, rot: %f %f %f. vector: %f %f %f?\n", x, y, z, rx, ry, rz, cos(ry) * cos(rx), sin(rx), sin(ry) * cos(rx));
		//setMeshID(arrow, arrow_id, x);
	}
}

void activate_ai(int id) {
	if (getActive(id)) {
		switch(getMeshNumber(id)) { //1 is fish. 2 is bat. 3 is cactus
			case 1:
				responsive(id);
				break;
			case 2:
				search(id);
				break;
			case 3:
				plant(id);
				break;
			default:
				printf("Possible error in AI movement");
		}
	}
}

/*** update() ***/
/* background process, it is called when there are no other events */
/* -used to control animations and perform calculations while the  */
/*  system is running */
/* -gravity must also implemented here, duplicate collisionResponse */
void update() {
	/*for (int fuck = 0; fuck < NUMROOMS; fuck++) {
		printf("Cow #%d's visibility is %d\n", fuck, getVisible(fuck));
	}*/
	float x, y, z = 0.0;
	float rx, ry, rz = 0.0;
	/* sample animation for the testworld, don't remove this code */
	/* demo of animating mobs */
	getViewPosition(&x, &y, &z);
	ExtractFrustum();
	////printf("meshindex: %d\n", meshIndex);
	if (testWorld) {

		/* update old position so it contains the correct value */
		/* -otherwise view position is only correct after a key is */
		/*  pressed and keyboard() executes. */
		#if 0
		// Fire a ray in the direction of forward motion
		float xx, yy, zz;
		getViewPosition(&x, &y, &z);
		getOldViewPosition(&xx, &yy, &zz);
		//////printf("%f %f %f %f %f %f\n", xx, yy, zz, x, y, z);
		//////printf("%f %f %f\n",  -xx+((x-xx)*25.0), -yy+((y-yy)*25.0), -zz+((z-zz)*25.0));
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
		//update timings if necesasry:
		time_t currtime = getTime();
		////printf("timings[0] = %ld\ncurrtime = %ld\n", timings[0], currtime);
		if(timings[0] == 0L) {timings[0] = currtime + (time_t)100L;} //100ms is the timing for the clouds
		if(currfloor != 0 && timings[1] == 0L) {timings[1] = currtime + (time_t)100L;} //30ms is the timing for the mob animation, hopefully it works.
		if(currfloor != 0 && timings[2] == 0L) {timings[2] = currtime + (time_t)2000L;}
		/* your code goes here */
		//why would i put it here i want gravity to exist on the test world too?
		getViewPosition(&x, &y, &z);
		if (currfloor != 0 &&
			(curr_viewpos.x != -(int)x || curr_viewpos.z != -(int)z)) { //turn is also forfeit if the player attacked but we'll handle that later
			//AI CAN ACT
			//printf("ai should be triggered now! viewpos moved from %d to %d or %d to %d\n", curr_viewpos.x, -(int)x, curr_viewpos.z, -(int)z);
			curr_viewpos.x = -(int)x;
			curr_viewpos.y = -(int)y;
			curr_viewpos.z = -(int)z;
			//TODO: AI TURN ACTIONS!
			for (int meshid = meshIndex; meshid < (currfloor * NUMROOMS); meshid++) {
				if (getActive(meshid)) {
					activate_ai(meshid);
					mobCollisionResponse(meshid);
				}
			}
			for (int id = meshIndex; id < meshIndex + NUMROOMS; id++) { //getpresent should never be false here
				if (adjacencyCheck(curr_viewpos, getMobCoord(id)) && getActive(id) && getPresent(id)) {
					if(attack_check()) {
						printf("The player has mercilessly killed another being.\n");
						setActive(id, false);
						setChase(id, false);
					}
					else {
						printf("The creature that the player was attempting to slay lives on another day.\n");
					}
					getOldViewPosition(&x, &y, &z);
					setViewPosition(x, y, z);
					curr_viewpos.x = -(int)x;
					curr_viewpos.y = -(int)y;
					curr_viewpos.z = -(int)z;
				}
			}
		}
		getViewOrientation(&rx, &ry, &rz);
		int noncalvertx = -(int)x;
		int noncalverty = -(int)y;
		int noncalvertz = -(int)z;
		bool on_stairs = false;
		if (floors[currfloor]->stairs[STAIRS_UP].x == noncalvertx) {
			////printf("x match.u\n");
			if (floors[currfloor]->stairs[STAIRS_UP].z == noncalvertz) {
				////printf("z matchedu\n");
				if (floors[currfloor]->stairs[STAIRS_UP].y == noncalverty - 1) {
					////printf("y-1 (maybe + 1?) matchu\n");
					////printf("loading triggered.\n");
					if (currfloor == 0) {
						////printf("Trying to ascend to heaven: not allowed, due to furry.\nAlso up staircase on overworld, fix.\n");
					} else {
						prevfloor = currfloor;
						currfloor -= 1;
						meshIndex = (currfloor - 1) * NUMROOMS;
						on_stairs = true;
						if (meshIndex >= 0) {   //prevents negative indices being accessed
							for (int i = meshIndex + NUMROOMS; i < meshIndex + (2 * NUMROOMS); i++) {
								//printf("hiding mob #%d\n", i);
								hideMesh(i); //calvert's mob management system makes less sense than string theory
								setPresent(i, false);
							}
						} else if (meshIndex == 0) {   //prevents negative indices being accessed
							for (int i = 0; i < NUMROOMS; i++) { //theres a way to get the math to work right thats classy but its 5am
								//printf("hiding mob #%d\n", i);
								hideMesh(i);
								setPresent(i, false);
							}
						}
					}
				}
			}
		}
		else if (floors[currfloor]->stairs[STAIRS_DOWN].x == noncalvertx) {
			////printf("x match.d\n");
			if (floors[currfloor]->stairs[STAIRS_DOWN].z == noncalvertz) {
				////printf("z matchedd\n");
				if (floors[currfloor]->stairs[STAIRS_DOWN].y == noncalverty - 1) {
					////printf("y-1 (maybe + 1?) matchd\n");
					////printf("ok, loading downstairs\n");
					prevfloor = currfloor;
					currfloor += 1;
					meshIndex = (currfloor - 1) * NUMROOMS; //floor1 is 0, floor2 is 9
					//printf("MI %d\n", meshIndex);
					on_stairs = true;
					if (meshIndex > 0) {   //prevents negative indices being accessed
						for (int i = meshIndex - NUMROOMS; i < meshIndex; i++) {
							//printf("hiding mob #%d\n", i);
							hideMesh(i); //calvert's mob management system makes less sense than string theory
							setPresent(i, false);
						}
					}
				}
			}
		} //i've been wrong all alonG! could refactor! hope it works anyways though! ahha
		if (on_stairs) { //"currfloor" here actually is the index of the floor we're loading currently
			if (!(floors[currfloor])) { //if no maze exists, generate one!
				if (currfloor == 2 || currfloor == 4) {
					generateCave();
				}
				else {
					generateMaze();
				}
			}
			else {
				//since there's a saved floor at this level, we'll load it. we also need to hide the cows from the last level
				for(int lx = 0; lx < WORLDX; lx++) {
					for (int ly = 0; ly < WORLDY; ly++) {
						for (int lz = 0; lz < WORLDZ; lz++) {
							world[lx][ly][lz] = floors[currfloor]->world[lx][ly][lz];
						}
					}
				}
				if (prevfloor != 1) { //this is in a weird spot because we hide the current mobs every level change, but only load mobs when loading
					for (int i = meshIndex; i < meshIndex + NUMROOMS; i++) {
						//printf("showing mob #%d if it's alive\n", i);
						if (getActive(i)) {
							drawMesh(i); //calvert's mob management system makes less sense than string theory
							setPresent(i, true);
						}
					}
				}
			}
			//remember to calvertize (make negative) the positions first
			////printf("okay hopefully? %f %f %f\n", -(float)floors[currfloor]->spawn.x,-(float)floors[currfloor]->spawn.y,-(float)floors[currfloor]->spawn.z);
			setViewPosition(-(float)floors[currfloor]->spawn.x,-(float)floors[currfloor]->spawn.y,-(float)floors[currfloor]->spawn.z);
			curr_viewpos.x = floors[currfloor]->spawn.x;
			curr_viewpos.y = floors[currfloor]->spawn.y;
			curr_viewpos.z = floors[currfloor]->spawn.z;
		}
		else {
			if(world_inited && usegravity){
				if(world[-(int)x][-(int)(y+1)][-(int)z] == 0) {
					////////printf("grabity\n");
					setViewPosition(x,y+0.1,z);
					//collisionResponse();
				}
				setPlayerPosition(0, -x-1, -y, -z-1, -ry);
			}
		} //jank
		//i know the following is bad, but i still want a quadtree and its february
		static float cloud_offset = 0.0;
		float divisor = 10;
		static int movement_dir = 0;
		//these are incredibly laggy and could be done better LAWL
		if (currfloor == 0 && check_timing(timings[0], currtime)) {
			cloud_offset = cloud_offset + 0.1;
			//uh isn't this going to cause some fucking EPIC lag
			for (int x = 0; x < WORLDX; x++) {
				//world[x][WORLDY-7][z] = 0;
				for (int z = 0; z < WORLDZ; z++) {
					float x1 = ((float)x/divisor) + cloud_offset;
					float z1 = ((float)z/divisor);
					float y1 = (WORLDY - 7) + perlin(x1, z1) * 5;
					//////printf("x: %f, y: %f, z: %f\n", (float)x1, y1, (float)z1);
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
			//HOLY SHIT they're going to TANK it
			timings[0] = (time_t)0;
		}
		if(check_timing(timings[2], currtime)) { //yeah i think currtime can just be obtained in the function... why didnt i do that...
			movement_dir = !movement_dir;
			timings[2] = (time_t)0;
		} //rudimentary mob movement
		if (currfloor != 0 && check_timing(timings[1], currtime)) {
			for (int i = meshIndex - 1; i >= meshIndex - NUMROOMS; i--) {
				meshVisibilityCheck(i);
				if (getMeshNumber(i) == 1 && !getChase(i)) { //translate the meshes if they're un-activated fish
					if (getVisible(i)) {
						float xpos = getx(i);
						float ypos = gety(i);
						float zpos = getz(i);
						if (movement_dir == 0) {
							setTranslateMesh(i, xpos + 0.01, ypos, zpos);
						}
						else {
							setTranslateMesh(i, xpos - 0.01, ypos, zpos);
						}
					}
				}
			}
			timings[1] = 0;
		}
	}
}

/*end of code from Wikipedia https://en.wikipedia.org/wiki/Perlin_noise*/
/*"Note that the code below is very basic, for illustration only, will be slow, and not usable in applications."*/
/*
░░░░░▄▄▄▄▀▀▀▀▀▀▀▀▄▄▄▄▄▄░░░░░░░
░░░░░█░░░░▒▒▒▒▒▒▒▒▒▒▒▒░░▀▀▄░░░░
░░░░█░░░▒▒▒▒▒▒░░░░░░░░▒▒▒░░█░░░
░░░█░░░░░░▄██▀▄▄░░░░░▄▄▄░░░░█░░
░▄▀▒▄▄▄▒░█▀▀▀▀▄▄█░░░██▄▄█░░░░█░
█░▒█▒▄░▀▄▄▄▀░░░░░░░░█░░░▒▒▒▒▒░█
█░▒█░█▀▄▄░░░░░█▀░░░░▀▄░░▄▀▀▀▄▒█
░█░▀▄░█▄░█▀▄▄░▀░▀▀░▄▄▀░░░░█░░█░
░░█░░░▀▄▀█▄▄░█▀▀▀▄▄▄▄▀▀█▀██░█░░
░░░█░░░░██░░▀█▄▄▄█▄▄█▄████░█░░░
░░░░█░░░░▀▀▄░█░░░█░█▀██████░█░░
░░░░░▀▄░░░░░▀▀▄▄▄█▄█▄█▄█▄▀░░█░░
░░░░░░░▀▄▄░▒▒▒▒░░░░░░░░░░▒░░░█░
░░░░░░░░░░▀▀▄▄░▒▒▒▒▒▒▒▒▒▒░░░░█░
░░░░░░░░░░░░░░▀▄▄▄▄▄░░░░░░░░█░░
*/
int main(int argc, char** argv) {
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

		world_inited = true;
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
		
		world_inited = true;

		for(i=0; i<WORLDX; i++) {
			for(j=0; j<WORLDY; j++) {
				for(k=0; k<WORLDZ; k++) {
					world[i][j][k] = 0;
				}
			}
		}

		//outside world generation
		setUserColour(17, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0);
		setAssignedTexture(17, 9);
		setUserColour(18, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0);
		setAssignedTexture(18, 18); //18 18 would be ok fdor a normal ass wall
		setUserColour(19, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0);
		setAssignedTexture(19, 37);

		setUserColour(20, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0); //for grass
		setAssignedTexture(20, 41);
		setUserColour(21, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0); //for mud
		setAssignedTexture(21, 12);
		setUserColour(22, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0); //for mountaintops
		setAssignedTexture(22, 5); //looks a little dingy, but..

		setUserColour(23, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0); //for random dungeon blocks
		setAssignedTexture(23, 22);
		
		setUserColour(24, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0); //for up stairs
		setAssignedTexture(24, 31);
		setUserColour(25, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0); //for down stairs
		setAssignedTexture(25, 32);

		setUserColour(26, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0);
		setAssignedTexture(26, 30); //it's definitely rust, but... the cave

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
				
				if(y > 4.0) {elevation = 22;}
				else if (y < -4.0) {elevation = 21;}
				else {elevation = 20;}
				for(int deep = 0; deep < 3; deep++) {
					world[x][(int)y+FLOORHEIGHT - deep][z] = elevation;
				}
				if(x == 50 && z == 50) { 
					//someone told me you could do the following to assign structs:
					stairslocation = (struct coord){x, (int)y+FLOORHEIGHT, z};
					//totally blew my mind. didnt know it was possible.
					world[x][(int)y+FLOORHEIGHT][z] = 25;
				}
			}
		} //uh huh
		
		////printf("done! saving now.\n");
		floors[0] = calloc(1, sizeof(struct record));
		//inefficient? lawl.
		for(int sx = 0; sx < WORLDX; sx++) {
			for (int sy = 0; sy < WORLDY; sy++){
				for (int sz = 0; sz < WORLDZ; sz++){
					//////printf("saving %d %d %d...\n", sx, sy, sz);
					floors[0]->world[sx][sy][sz] = world[sx][sy][sz];
				}
			}
		}
		struct coord dummy = {.x = 0, .y = 0, .z = 0}; //c99 sexy
		struct coord spawn = {.x = 53, .y = FLOORHEIGHT + 8, .z = 53}; //hopefully never gens in ground. just for testing
		////printf("stairs: %d %d %d %d %d %d\n", dummy.x, dummy.y, dummy.z, spawn.x, spawn.y, spawn.z);
		floors[0]->stairs[STAIRS_UP] = dummy;
		floors[0]->stairs[STAIRS_DOWN] = stairslocation;
		floors[0]->spawn = spawn;
		GLfloat black[] = {0.0, 0.0, 0.0, 0.5};
		set2Dcolour(black);
		draw2Dbox(0, 0, WORLDX, WORLDZ);
		//floors[0]->mobs[0] = NULL;
		//setViewPosition((float)spawn.x, (float)spawn.y, (float)spawn.z);
		////printf("saving done I think!\n");
		setViewPosition(-(float)floors[currfloor]->spawn.x,-(float)floors[currfloor]->spawn.y,-(float)floors[currfloor]->spawn.z);
		curr_viewpos.x = floors[currfloor]->spawn.x;
		curr_viewpos.y = floors[currfloor]->spawn.y;
		curr_viewpos.z = floors[currfloor]->spawn.z;
	}

	/* starts the graphics processing loop */
	/* code after this will not run until the program exits */
	glutMainLoop();
	return 0; 
}
