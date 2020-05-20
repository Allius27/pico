/*
 *  This code is released under the MIT License.
 *  Copyright (c) 2013 Nenad Markus
 */

#ifndef PICORNT_H
#define PICORNT_H

#include <stdint.h>
#include <stdio.h>

#include <string>


class picornt
{
	void* cascade = 0;

	int update_memory
	(
		int* slot,
		float memory[], int counts[], int nmemslots, int maxslotsize,
		float rcsq[], int ndets, int maxndets
	);
	
	int run_cascade( void* cascade, 
		float* o, int r, int c, int s, 
		void* vppixels, int nrows, int ncols, int ldim );

	int run_rotated_cascade( void* cascade, 
		float* o, int r, int c, int s, float a, 
		void* vppixels, int nrows, int ncols, int ldim);

	float get_overlap(float r1, float c1, float s1, float r2, float c2, float s2);

	void ccdfs(int a[], int i, float rcsq[], int n);

	int find_connected_components(int a[], float rcsq[], int n);

public:
	picornt() {}

	struct modelInfo{
		int version;
		int tdepth;
		int ntrees;
	};

	bool loadModel(const std::string & model);
	bool getModelInfo(modelInfo & info);

	// * `angle` is a number between 0 and 1 that determines the counterclockwise in-plane rotation of the cascade:
	//		0.0f corresponds to 0 radians and 1.0f corresponds to 2*pi radians
	int find_objects
	(
		float rcsq[], int maxndetections,
		float angle,
		void* pixels, int nrows, int ncols, int ldim,
		float scalefactor, float stridefactor, float minsize, float maxsize
	);

	int cluster_detections(float rcsq[], int n);
};


#endif // PICORNT_H