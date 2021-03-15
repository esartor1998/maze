

Building and Running the Graphics System
----------------------------------------
The program consists of three .c files.  The a1.c file contains the main()
routine and the update() function. All of the changes necessary for the
assignments can be made to this file.  The graphics.c file contains all
of the code to create the 3D graphics for the assignment. The visible.c
file contains the visible surface determination code. You should not
need to change graphics.c or visible.c.

There is a makefile which will compile the code on OSX or Linux.
The code should build by typing make.

The executable is named a1. The command line arguments for the program
are:
	-full         run in fullscreen.
	-testworld    start with a simple world to demonstrate the system
	-fps          print the frames per second being displayed
	-help         print a list of command line arguments
	-drawall      draw all cubes in the world without using visible surface
                  detection to remove none visible cubes (very slow).
				  Don't use this normally. 
To quickly see the engine running you can type ./a1 -testworld.
You can run it in fullscreen using ./a1 -testworld -full.
Exit the program by typing q.


When the program runs the view is controlled through the mouse and
keyboard. The mouse moves the viewpoint left-right and up-down.
The keyboard controls do the following:
	w  move forward
	a  strafe left
	s  move backward
	d  strafe right
	q  quit

The 1,2,3 buttons also change the rendering options.
	1 = wire frame mode
	2 = flat shading
	3 = smooth shading
	
Note: If the controls appear to be reversed then the viewpoint is upside down.
Move the mouse until you turn over.

In the sample world created using the -testworld flag there are a few
sample boxes and animations drawn in the middle of the world. There is also
a set of blue boxes which show the outer width and depth of the world.

The f key toggles fly mode but only when gravity has been implemented.


Programming Interface to the Graphics System
--------------------------------------------

1. Drawing the world
--------------------

The game world is made of cubes. The data structure which holds all of
the objects is the three dimensional array:

	GLubyte world[100][50][100]

The GLubyte is an unsigned byte defined by OpenGL.

The indices of the array correspond to the dimensions of the world.
In order from left to right they are x,y,z.  This means the world is 100 units
in the x dimension (left to right), 50 units in the y dimension (up and down),
and 100 units in z (back to front).

The cube at location world[0][0][0] is in the lower corner of the 3D world.
The cube at location world[99][49][99] is diagonally across from
world[0][0][0] in the upper corner of the world.

Each cube drawn in the world is one unit length in each dimension.

Values are stored in the array to indicate if that position in the
world is occupied or are empty. The following would mean that
position 25,25,25 is empty:
	world[25][25][25] = 0

If the following were used:
	world[25][25][25] = 1
then position 25,25,25 would contain a green cube. 

Cubes can be drawn in different colours depending on that value stored
in the world array. The current colours which can be drawn are:
	0 empty
	1 green
	2 blue
	3 red
	4 black
	5 white
	6 purple
	7 orange
	8 yellow

User defined colours can be set using the setUserColour() function which
is defined as:

int setUserColour(int id, GLfloat ambRed, GLfloat ambGreen, GLfloat ambBlue,
  GLfloat ambAlpha, GLfloat difRed, GLfloat difGreen, GLfloat difBlue,
  GLfloat difAlpha);
where:
   id is the number to which the colour is allocated. It must be
      9 and 99. This is also the value that must be stored in the 
      world array.
   ambRed, ambGreen, ambBlue, ambAlpha are the RGBA values for the ambient
      colour component
   difRed, difGreen, difBlue, difAlpha are the RGBA values for the diffuse
      colour component

You must use setUserColour() before the colour can be stored in the world
array.  For example, the following will create a dark grey colour and store
as colour id == 9. A cube in the world array is then set to the newly defined
colour with the id == 9.
         setUserColour(9, 0.5, 0.5, 0.5, 1.0, 0.2, 0.2, 0.2, 1.0)
         world[25][25][25] = 9

Changing the colour values by calling setUserColour() with an existing id
number will cause objects with that colour to change while the game is running.
For example, in the above example if the values for id == 9 are changed
then all of the cubes in the world with that colour will to the new colour.
The following line would change all of the grey cubes to red.
         setUserColour(9, 0.1, 0.0, 0.0, 1.0, 0.5, 0.0, 0.0, 1.0)

To retrieve the RGBA values for a user defined colour you can use
the function:

void getUserColour(int id, GLfloat *ambRed, GLfloat *ambGreen, GLfloat *ambBlue,
  GLfloat *ambAlpha, GLfloat *difRed, GLfloat *difGreen, GLfloat *difBlue,
  GLfloat *difAlpha);

where the RGBA values for the ambient and diffuse colours are returned
for the colour with id number that is passed to the function. Note that this
function only works for user defined colours and the RGBA values for colours
0 to 9 will not be returned.


2. Viewpoint Manipulation Functions
-----------------------------------
These can be used to find and set the location of the viewpoint.
They are used for implementing operations such as collision detection
and gravity.  

void getViewPosition(float *x, float *y, float *z);
-Returns the position where the viewpoint will move to on the next step.
-Returns negative numbers which you may need to make positive for some
 calculations such as using them as an index into the world array.
 You will also need to make them ints if you wish to use them as array
 indices.

void setViewPosition(float x, float y, float z);
-Sets the position where the viewpoint will move to on the next step.
-Numbers taken from the world array need to be made negative before they
 are used with setViewPosition.

void setOldViewPosition(float x, float y, float z);
-Sets the position where the viewpoint is currently.
-Numbers taken from the world array need to be made negative before they
 are used with setOldViewPosition.

void getOldViewPosition(float *x, float *y, float *z);
-Returns the position where the viewpoint is currently.
-Returns negative numbers which you may need to make positive for some
 calculations such as using them as an index into the world array.

void getViewOrientation(float *xaxis, float *yaxis, float *zaxis); 
-Returns the direction the mouse is pointing. 
-The xaxis, yaxis, and zaxis values are the amount of rotation around the
 x, y, and z  axis respectively.
-The values can be larger then 360 degrees which indicates more than
 one rotation.

3. Collision Response Function
------------------------------
void collisionResponse()
-The collision detection and response code is written in this function. 
-It is located in the a1.c file.
-Note that the f key can turn off the effect of gravity. It will
 only work once you have gravity implemented. If you press f it will allow
 you to fly around the world and look at it from above. Pressing f again
 toggles gravity back on.


4. Timing Events
----------------
OpenGL is event driven. The events which this program will respond to 
include keyboard and mouse input. The glutMainLoop() function receives
these inputs and processes them. 

The glutMainLoop() function will loop until the program ends. This means
that all of your code to initialize the world must be run before this
function is called. It also means that changes to the world must occur
inside this function which is called by OpenGL. The only functions which you
have access to to make these updates are update() and collisionResponse()
in a1.c.

When it is not otherwise drawing the scene the system will call the
update() function. This is where you can make changes to the world
array and lighting while program is running.

If you make changes to the world or the light in the update()
function then you may need to call glutPostRedisplay() to refresh the screen.

The update() function is not called on a predictable schedule. You will
need to check the time during updates to control the rate at which
the world changes. 


5. World Notes
--------------
-The cubes measure one unit along each axis.
-Cubes are positioned so their centre is at 0.5 greater than their
 x,y,z coordinates. So the cube at 0,0,0 is centred at 0.5, 0.5, 0.5. 
-You may see the edges of the screen don't update quickly when the viewpoint
 moves quickly. It looks like the edge of the world stops and there is a
 blocky edge visible. This isn't something you need to fix. 


6. Important Notes
-------------------
Do not remove or modify the code which builds the sample world in a1.c
in the main() when testworld == 1. 

There are three places in a1.c where it indicates that you should
add your own code.

You can make changes to graphics.c if you wish but you are responsible
for making them work. If you break the graphics system then you have
to fix it yourself. The graphics system may change in later assignments
so you will be need to merge your changes into the new code.



7. Mob Controls
---------------
The Mobs are entities controlled by the program. They appear as spheres.

The following functions have been added to control the creation and
movement of the mobs:

   void createMob(int number, float x, float y, float z, float roty);
        -creates mob number at position x,y,z with rotation y
   void setMobPosition(int number, float x, float y, float z, float roty);
        -move a created mob to a new position and rotation
   void hideMob(int number);
        -stops drawing mob number, it become invisible
	-making mobs invisible is equivalent to removing them from the world
   void showmob(int number);
        -start drawing mob number, make it visible again

In all of the above functions:
number  -is the identifier for each mob. There can be a maximum of 10
         mobs in the game. They are numbered from 0 to 9 and this number
         is passed to all functions to indicate which mob you are updating.
x,y,z   -are the x,y,z coordinates in the world space. They are floats.
         These are world coordinates.
roty    -is the rotation of the mob around the y axis. This allows you
         to position the mob so it is facing in the direction it is
         moving or looking. It is a float.

A small sample of the mob control is included in a1.c. 
To see this demo you need to run the sample world using:
	./a1 -testworld


8. Access to the Mouse Operations
----------------------------------
The mouse() function allows you to respond to mouse button events.



9, Client-Server Flags
----------------------
Flags were added so the user can identify if the program is running as
a client or a server. The -client flag sets the variable netClient equal
to 1. The -server flag sets the variable netServer equal to 1. They are
initially set to 0.


10. Setting the  View Orientation
---------------------------------
The counterpart to the getViewOrientation() function allows you to set
the viewpoint rotation. When combined with the getViewPosition() and
setViewPosition() you can position the viewpoint
in the world and rotate it to face in the desired direction.

void setViewOrientation(float xaxis, float yaxis, float zaxis); 
-Sets the orientation of the viewpoint.
-The xaxis, yaxis, and zaxis values are the amount of rotation around the
 x, y, and z axis respectively.
-Note that the rotations are around the world axis and not around the current
 viewpoint. This means you will need to perform some calculations to
 convert from the world axis to the local viewpoint axis if you wish to
 rotate relative to the current viewpoint.
-The values can be larger then 360 degrees which indicates more than
 one rotation.

11. 2D Drawing Functions
------------------------
Several function can be used to draw two dimensional shapes on the screen.
These are useful for displaying information such as maps, health bars,
inventory.

All two dimensional drawing functions must be placed in the:
	void draw2D()
function in a1.c. This is the only place where they will execute correctly.
There is a comment which indicates where your code can be added.

There is a sample of the 2D drawing functions which is run when
the -testworld command line argument is used.

The screen is two dimensional with the (0,0) coordinate in the lower
left corner and the maximum default screen coordinates are (1023, 767)
in the upper right corner.

Inside the draw2D() function you can call the following functions:

void  set2Dcolour(float colour[]);
-Sets the RGBA colour for the 2D images which are drawn after it.
-The colour array contains four floats which contain the red, green, blue
 and alpha values. 
-The colour stays the same until it is changed by a later call to
 set2Dcolour() with different parameters.

void  draw2Dline(int x1, int y1, int x2, int y2, int lineWidth);
-Draw a line from (x1, y1) to (x2, y2).
-The lineWidth parameter indicates the width in pixels of the line.

void  draw2Dbox(int x1, int y1, int x2, int y2);
-Draw a box with the lower left corner at (x1, y1) and the upper right
 corner at (x2, y2).

void  draw2Dtriangle(int x1, int y1, int x2, int y2, int x3, int y3);
-Draw a triangle with the coordinates of it's three vertices at (x1,y1),
 (x2,y2), and (x3,y3).


12. Display Map Flag
--------------------
A flag has been added which can be used to toggle a map on and off and
to change the size of the map.  The flag is:
	int displayMap;

It is toggled using the m key. The flag can have three values, 0, 1, and 2.
When the m key is pressed the value of displayMap is increased by 1.
When the value is greater than 2 it is reset to be equal to 0.
The flag is set to 1 on startup.

The meaning of the values stored in displayMap are:
	0   no map displayed
	1   a small map in the corner is displayed
	2   a larger map is displayed in the centre of the screen

You can use this with the draw2D() function to display a map on the
screen.

The testworld has some 2D shapes drawn on the screen. These can be toggled
on and off using the m key.


13. Screen Size Variables
-------------------------
The variables:
	int screenWidth, screenHeight;
indicate the width and height of the current display window in pixels.
They will reflect the correct values when the display window is resized.



14. Player Controls
-------------------
Players can now be drawn. This is necessary to see other players in
a networked game.  Players are gray with red eyes.

The functions to control the players are identical to the mob control
functions with the names changed to indicate they control players. 
   void createPlayer(int number, float x, float y, float z, float roty);
   void setPlayerPosition(int number, float x, float y, float z, float roty);
   void hidePlayer(int number);
   void showPlayer(int number);

There is an example of a player drawn in the sample world.


15. Space Flag
--------------
A variable named space has been added to the program. It is set equal
to 1 when the space bar is pressed. You can use this to respond to
the player pushing the space bar.

Reset the variable to be equal to 0 once you have responded to the key press.




16 Setting the Light Position
-----------------------------
There is a single light in the world.  The position of the light
is controlled through two functions:

	void setLightPosition(GLfloat x, GLfloat y, GLfloat z);
	GLfloat* getLightPosition();

The setLightPosition() function moves the light source to location x,y,z in the
world. The getLightPosition() function returns a pointer to an array
of floats containing current the x,y,z location of the light source.

To see the effect of a change through setLightPosition() you will
need to call glutPostRedisplay() to update the screen. 


17 Drawing Tubes (actually crossed quadrilaterals)
--------------------------------------------------
These functions let you draw tube shapes.  

void createTube(int number, float sx, float sy, float sz,
        float ex, float ey, float ez, int colour);
-Creates a tube with the starting position at sx,sy,sz and and ending
 position at ex, ey, ez.
-Colour is set using the int colour.  These objects can use predefined
 colours (1-8) or used defined colours (9+).
-number is the identifier for this tube. Used to either change the parameters
 of the object or with the show and hide functions.
-Call the function again with different parameters to animate the tube.
-The tube will initially be visible.

void hideTube(int number);
-Stops drawing the tube. It remains in memory and can be drawn again if
 using showTube().
-number is the identifier of the tube.

void showTube(int number);
-Restart drawing the tube. Used only after hideTube() stops drawing. 
-number is the identifier of the tube.

18. Applying Textures to Cubes
------------------------------
Textures are associated with the id numbers that represent user defined
colours. You can assign a colour, a texture, or both to a colour id.
The texture currently replaces all colour information.

To use a texture, create a used defined colour and then assign a texture
id number to it using setAssignedTexture(). The texture id number is
the numeric part of the file name in the /textures directory.
The sequence of operations to make a texture appear is:
      setUserColour(10, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0);
      setAssignedTexture(10, 22);
      world[59][25][50] = 10;
This will create user defined colour number 10. 
Then assign a texture 22 to that colour number 10.
Then place a block in the world array with that colour/texture 10.


int setAssignedTexture(int colourid, int textureid);
where:
   colourid is the user defined colour number that will have a texture
      assigned to it
   textureid is the texture number to assign to that colour id
-returns 0 on success and 1 on failure

int getAssignedTexture(int id);
where:
   the return value is the texture number associated with id

void setTextureOffset(int id, float uoffset, float voffset);
where
   id is the colour id to which the texture offset will be applied
   uoffset is the amount of offset is the texture u coordinate (horizontal)
   voffset is the amount of offset is the texture v coordinate (vertical)
Note that the texture offsets are stored with the colourId and not the
textures. This means you can use the same texture with different colours
and use different texture offset. For example, some textures can be offset
while others are not (or they are offset to different values).

The colour of the texture will be a blend of the object colour which
is set using setUserColour() and the colours in the texture image.
If you set the colour of the object to be all white using:
	setUserColour(12, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0);
then only the colour in the texture image will be displayed. This is a
reasonable default method for using textures. If you make the base colour
something other than white then the result will be a tinted version of the
original texture. This is a way to add variety to textures by using the
same texture but tinting it with different colours.

Texture files are stored in a subdirectory named textures. Each texture
file is named with a number followed by the .ppm suffix. The numbers from
0 to 50 are used for textures that are supplied with the game engine.
These are textures 0.ppm to 50.ppm. 

The texture files must be ppm files of type 3 (ASCII) or type 6 ppm (binary)
files No other format of texture files will load correctly.
Textures must be 256x256 pixels.

More textures can be added by converting other file formats to .ppm. To do
this, load the texture images into Gimp (GNU Image Manipulation Program),
resize them to 256 pixels square, and export them as .ppm files.

If you add more textures to the system then place them in the /textures
directory. They must be named with a number followed by .ppm. The number
will be the textureid that is used with the function:
     int setAssignedTexture(int colourid, int textureid);
to identify the texture in the game. Do not use any other format for the
names of that texture files.

The current maximum number of textures the system can support is 100. If you
want to increase this then change the NUMBERTEXTURES macro and recompile the
code.


19. Using 3D Models from obj files
----------------------------------
3D models are also associated with an id number. These id numbers are different
to those used for textures that are applied to cubes.  Each 3D model that
you display will have its own id number. It will also have a mesh number which
determines which model will be drawn. For instance, the mesh of the cow 
has a mesh number of 0. The mesh numbers for the currently available models
that you can use in the game are:

	Mesh Number	Model
	0		cow
	1		fish
	2		bat
	3		cactus

If you wanted to create three fish, they would all have the mesh number of 1.
Each of the fish would have it's own unique id which is assigned by the
user.

The functions used to add and control 3D meshes are:

void setMeshID(int id, int meshNumber, float xpos, float ypos, float zpos)
	-create a 3D mesh mesh and place it in the game world
	-the id is the unique identifier for that mesh, it is used to
	 modify the size, location, rotation, visibility of the mesh.
	-the meshNumber identifies which mesh to draw
	-xpos, ypos, zpos are the location in the world where the mesh will
	 be drawn. These correspond to the location in the world array but are
	 floating point numbers. This allows the mesh objects to be positioned
	 anywhere in the world array and not only at the integer positions
	 of the cubes.

void unsetMeshID(int id)
	-frees the mesh id number and removes the mesh from the screen
	-this deallocates the mesh id so it can be reused again
	-use only if you want to reuse the id number, if you want to stop
	 drawing a mesh then you can use hideMesh() instead.

void setTranslateMesh(int id, float xpos, float ypos, float zpos)
	-move a mesh to a new (x,y,z) location
	-gradually changing these values can be used to animate motion

void setRotateMesh(int id, float xrot, float yrot, float zrot)
	-rotate the mesh around the x, y, and z axis
	-rotation values are measured in degrees
	-rotations are done in the order of x rotation first, y rotation second,
	 z rotation third
	-mesh objects are not oriented the same way, you will need to rotate
	 them (most likely around the y axis) to make them face in the
	 direction you wish 

void setScaleMesh(int id, float scale)
	-resizes the mesh, a scale of 1.0 is default the size of the mesh,
	 a value of 0.5 would reduce the size by half in three dimensions,
	 a value of 2.0 would double the size of the mesh in three dimensions. 
	-the default mesh sizes are not related to the size of the game
	 world cubes. You may need to scale the meshes to make them fit within
	 the world

void drawMesh(int id);
void hideMesh(int id);
	-these two functions turn the drawing on and off for an individual mesh
	-the mesh id is still associated with the object but it is not
	 drawn when hideMesh() is used


An example of using these functions.
If you do this:
   setMeshID(10, 2, 10, 10, 10);
it will create an id number if 10, and associate model 2 (bat) with
that number. It will also place a copy of the bat at location (10,10,10)
in the world.  If you follow that function with:
   setRotateMesh(10, 0.0, 9.0, 0.0);
it will rotate the fish associated with id 10 by 90 degrees around the
y axis. This fish will continue to be drawn in this positions until another
function is called with moves or hides the fish. 

The models and textures for the models are stored in numbered file names.
For example, the first file is named 0.obj and the associated texture is 0.ppm.
The files must be stored in the /models directory. The numbers of the files
in the /models directory is the same as the mesh numbers listed above
(e.g. 0==cow, 1==fish, 2==bat, 3==cactus).

The .obj files must contain only triangles.
The textures must be .ppm files. They can be either binary or ASCII.
Textures are limited to 256x256 pixels.
Only texture colouring is used with the mesh. Vertex colours are not used
by the game engine. 

To convert a mesh for use with the game:
1. Make sure the model is made entirely of triangles. You can load the mesh
   into a 3D modelling program such as Blender and convert all of polygons
   to triangles. Then save the model as a .obj file (Wavefront .obj format).
2. Make the texture file is 256x256 pixels. This can be done using an image
   editing program such as Gimp. Resize the image to be 256x256 pixels.
3. The image must be in the .ppm format. Export the file to .ppm format.
   It can be either binary or ASCII format.
4. Name both of the files with a number. The system comes with models 0 to 4
   so add numbers after 4. Both the texture and the model file must be
   named with the same number. For example, 5.obj and 5.ppm would be the
   model and the texture files for object 5.

If the mesh is not made entirely of triangles or the texture is not in the
correct format then the game engine will exit and print an error message
which indicates the problem it found while reading the files.


====================================================================

Appendix A - Culling Objects that Cannot Be Seen
------------------------------------------------
You probably don't need this unless you are modifying the frustum
culling code. 

Display Lists
--------------
This is only used for the visible surface determination part of the system.
Unless you are changing visible.c then you do not need to use this.
You should not create objects in the world using addDisplayList().

An array named displayList has been created which you put the cube indices
that you want to be drawn. The function addDisplayList() is used to
add cubes to the list.
        e.g. The following would set the cube at world[1][3][5] to be drawn.
            addDisplayList(1,3,5);
This is used so the entire world is not drawn with each frame.
Only the cubes which you determine are visible should be added
to the display list.

Add the cubes you derive from visibility testing to the list.
There is also a counter named displayCount which contains the
number of elements in displayList[][].  You do not need to increment
displayCount but you need to set it equal to zero when you build a new
display list.  You need to build a new displayList each time you
perform culling (each time buildDisplayList() is called).


Empty Functions
---------------
void buildDisplayList()
-This is where you perform culling and add visible cubes to the display
 list.  There is some sample code here which moves all of the cubes in
 the world to the display list. This duplicates the original behaviour of
 assignment 1.  This should be replaced with your visibility/culling code.


Culling Information
-------------------
-The web page at:
	http://www.crownandcutlass.com/features/technicaldetails/frustum.html
contains a good explanation of how to determine the viewing frustum for
a viewpoint in OpenGL. There is also some useful code there. 



Frames Per Second (FPS) Printing
--------------------------------
The FPS are no longer printed automatically. There is a -fps command
line flag which turns this functionality one.

====================================================================

Appendix B - The PPM File Format
----------------------------
The ppm files are assumed to have the one of the following formats.

ASCII Files
-----------
P3
#comment line
width height
pixel-depth
--- rgb data in ASCII format---

Binary Files
------------
P6
#comment line
width height
pixel-depth
--- rgb data in ASCII format---

This is the format created by exporting to ppm using GIMP.
The file specification for ppm files allows comments to end of line
anywhere after the # symbol. If the files do not have the exact format
listed above then they will not work correctly. Additional comments or
comments after data values will not parse correctly. There must be one
comment line after either the P3 or P6 file type indicator.

The file width and height is currently limited to 256x256 pixel.


References
----------
These people and web sites were all helpful in building this code.

Fast C object parser fast_obj from:
   https://github.com/thisistherk/fast_obj
Author: Richard Knight, thisistherk

OpenGL ideas and code from:
-Starting code derived from scene.c in the The OpenGL Programming Guide
-Keyboard and mouse rotation taken from Swiftless Tutorials #23 Part 2 
 http://www.swiftless.com/tutorials/opengl/camera2.html

Frustum culling source code taken from:
        http://www.crownandcutlass.com/features/technicaldetails/frustum.html


