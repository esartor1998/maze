
/* calls fastObj and reads data from .obj file */
/* shows interface to fastObj */

#include <stdio.h>
#include <stdlib.h>

#include "mesh.h"

/* NOTE: The data for the vertices, normals, and texture data arrays start at
   index 1 instead of 0 (e.g. a[1] not a[1]). This is because the index
   values from the file start counting from 1. A vertex with an index of 1
   in the list of indices in the file will be located at vdata[3], vdata[4],
   vdata[5]. The locations vdata[0], vdata[1], and vdata[2] are empty. 
   Corrspondingly, the counts vcount, ncount, tcount start at 1 (which would
   mean they are empty. */

#define FAST_OBJ_IMPLEMENTATION
#include "fast_obj.h"


int readObjFile(char *str, struct meshStruct *m) {
int i;


   fastObjMesh* mesh = fast_obj_read(str);

	// determine the number of indices (number of faces/triangles)
   m->icount = mesh->face_count;

	// check if mesh is entirely made of triangles
   for(i=0; i < m->icount; i++) {
	// if not 3 then not a triangle (3 sides) return failure
      if (mesh->face_vertices[i] != 3) {
         printf("ERROR, only obj files containing triangle meshes will load.\n");
         return(1);
      }
   }


	// NOTE: vertex, norma, texture arrays, the data starts at index 1
	// so the indice values will match data locations in the arrays
	// -array[0-2] elements are empty, indices start at 1 not 0

	// vertices
	// vertex count
	// multiply by 3 because each coordinate is x,y,z
   m->vcount = mesh->position_count;
	// allocate space for vertex data 
   m->vdata = (float *) malloc(sizeof(float) * m->vcount * 3);
	// copy vertices into vertex  data array
   memcpy(m->vdata, &(mesh->positions[0]), m->vcount * 3 * sizeof(float)); 


	// texture coordinates
	// multiply by 2 because each coordinate is u,v
   m->tcount = mesh->texcoord_count;
	// if texture coordinates are in file then load
   if (m->tcount != 0) {
      m->tdata = (float *) malloc(sizeof(float) * m->tcount * 2);
	// copy texture coordinate into texture data array
      memcpy(m->tdata, &(mesh->texcoords[0]), m->tcount * 2 * sizeof(float)); 
   } else {
      m->tdata = NULL;
   }



	// normal coordinates
	// multiply by 3 because each coordinate is x,y,z
   m->ncount = mesh->normal_count;
	// if texture coordinates are in file then load
   if (m->ncount != 0) {
      m->ndata = (float *) malloc(sizeof(float) * m->ncount * 3);
	// copy texture coordinate into texture data array
      memcpy(m->ndata, &(mesh->normals[0]), m->ncount * 3 * sizeof(float)); 
   } else {
      m->ndata = NULL;
   }



	// vertex faces, three indices which indicate the three points
	// which make a face/polygon
	// store vertex face list in array
   m->vindex = (unsigned int *) malloc(sizeof(unsigned int) * m->icount * 3);
   for(i=0; i < m->icount * 3; i++) {
      m->vindex[i] = mesh->indices[i].p;
   }


	// texture coordinates, three indices which indicate the three points
	// to which the texture coordinates are attached
	// store texture coordinate face list in array
   if (m->tcount != 0) {
      m->tindex = (unsigned int *) malloc(sizeof(unsigned int) * m->icount * 3);
      for(i=0; i < m->icount * 3; i++) {
         m->tindex[i] = mesh->indices[i].t;
      }
   }

	// normal coordinates, three indices which indicate the three points
	// to which the normal coordinates are attached
	// store normal coordinate face list in array
   if (m->ncount != 0) {
      m->nindex = (unsigned int *) malloc(sizeof(unsigned int) * m->icount * 3);
      for(i=0; i < m->icount * 3; i++) {
         m->nindex[i] = mesh->indices[i].n;
      }
   }

//ZZZ
	// sorted vertices, normals, texture coordinates
	// use indices to order arrays so vertex, normal, uv coords are aligned
	// needed to get around inability to use different indices for
	//    each of vertices, normals, uv coords
   m->svdata = (float *) malloc(sizeof(float) * m->icount * 3 * 3);

   for(i=0; i< m->icount*3; i++) {
		// vertices
      m->svdata[i * 3] = m->vdata[ m->vindex[i] * 3 ];
      m->svdata[i * 3 + 1] = m->vdata[ m->vindex[i] * 3 + 1 ];
      m->svdata[i * 3 + 2] = m->vdata[ m->vindex[i] * 3 + 2 ];
   }

   if (m->ncount > 1) {
      m->sndata = (float *) malloc(sizeof(float) * m->icount * 3 * 3);
      for(i=0; i< m->icount*3; i++) {
		// normals
         m->sndata[i * 3] = m->ndata[ m->nindex[i] * 3 ];
         m->sndata[i * 3 + 1] = m->ndata[ m->nindex[i] * 3 + 1 ];
         m->sndata[i * 3 + 2] = m->ndata[ m->nindex[i] * 3 + 2 ];
      }
   }


   if (m->tcount > 1) {
      m->stdata = (float *) malloc(sizeof(float) * m->icount * 2 * 3);
      for(i=0; i< m->icount*3; i++) {
		// texture coordinates
         m->stdata[i * 2] =  m->tdata[ m->tindex[i] * 2 ];
         m->stdata[i * 2 + 1] =  1.0 - m->tdata[ m->tindex[i] * 2 + 1 ];
      }
   }

   fast_obj_destroy(mesh);

	// free data read from file
   free(m->vdata);
   free(m->tdata);
   free(m->ndata);
   free(m->vindex);
   free(m->tindex);
   free(m->nindex);

   return(0);
}

