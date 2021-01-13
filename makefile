
# INSTRUCTIONS: You will need to uncomment the LIBS line for your operating
# system. Only one of the LIBS lines should be uncommented.

# OSX 
# If you are running a Mac with OSX then try the first LIBS line under
# Recent MACOS. If that doesn't work and you are running an older version of
# OSX then you can try uncommenting the LIBS line under Old MACOS.

# Linux
# If you are running Linux then uncomments the LIBS line under LINUX.
# and try to compile the code.
# If the libraries or header files can't be found then you may need to add
# the -L compiler argument to indicate the path to where the libraries are
# located or the -I compiler argument to indicate where the headers are
# located.  

# Windows
# There are two options to compile under Windows.
# You can install the Linux kernel on Windows 10. Instructions for this are
# in another file included with this software. In this case the makefile LIBS
# line for LINUX should work if you uncomment it.
#
# You can use the native Windows compiler. In this case you need to be sure
# that OpenGL and glut are installed on your computer.  The development
# environment will need to know where to find the header files and the libraries
# for OpenGL and glut. This may be done automatically or you may need to enter
# the paths for these in your development environment.
# In this case you can probably use the the header files for __LINUX__
# in the C code.


# IMPORTANT NOTE: In all cases, leave the existing ifdefs for __LINUX__ and
# __MACOS__ in the C code. The TA's will need them to mark the assignments.


#########################################

# Recent MACOS
# frameworks for newer MACOS, where include files are moved 
#LIBS = -F/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/System/Library/Frameworks/ -framework OpenGL -framework GLUT -lm -Wno-deprecated-declarations

# Old MACOS
# framework information for older version of MACOS
#LIBS = -F/System/Library/Frameworks -framework OpenGL -framework GLUT -lm

# LINUX - Note that these will probably work but they can differ depending
# on your distribution.
LIBS = -lGL -lGLU -lglut -lm -D__LINUX__

a1: a1.c graphics.c visible.c graphics.h
	gcc a1.c graphics.c visible.c -o a1 $(LIBS)

