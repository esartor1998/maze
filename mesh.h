struct meshStruct {
        // vertices list
   unsigned int vcount;
   float *vdata;
        // textures list
   unsigned int tcount; 
   float *tdata;
        // normals list
   unsigned int ncount;
   float *ndata;
        // indices that reference vertices, textures, normals
   unsigned int icount;
   unsigned int *vindex, *tindex, *nindex;

	// aligned vertices, normals, texture coordinates which
	// are passed to opengl, deindexed
   float *svdata;
   float *sndata;
   float *stdata;
};

