/*
 * macros.h
 *
 *  Created on: May 27, 2013
 *      Author: fengzh
 */

// aug20 bin:20->10, i_noiseRatio:0.3->0.5
#ifndef MACROS_H_
#define MACROS_H_

//#include "incheader.h"
#include <string>

using namespace std;
using namespace cv;

#define DEBUG 0

#define PRELOAD false

/***********************************************************
 * macro definition.
 */
//#define ROOTPATH "/scratch/projects/KOD/dataset/Game11/"
//#define DPATH "/scratch/projects/KOD/dataset/"
//#define FOLDER "Game02/"
//
//// calculate slope range
//#define MIN_SLOPE_VAL 10000000
//
//#define SILENCE !DEBUG
//
//#define NUM_BIN_ENTROPY 10

// 4-d array
struct int4
{
	int a[4];
	int4(){	a[0] = a[1] = a[2] = a[3] = 0;}

	int4( int a0, int a1, int a2, int a3){
		a[0] = a0;
		a[1] = a1;
		a[2] = a2;
		a[3] = a3;
	}
};

/***********************************************************
 * func.
 */

//void kltHMat( const Mat &m1, const Mat &m2, Mat &H);

//void playVideos();

//string num2str( int, int num);

//void Vect2Mat(const vector<vector<float> > &vect, Mat &mtx);

//void loadVideo( const string &capName, vector<Mat> &frame,int &vlen, int &width, int &height, bool);

//void getPtCoord( int width, int height, vector<Point2f> &pts);

//void ldVP( const string &name, vector<Point2f> &vp);

//string ldFolder( int n , int,bool addPrefix, const string &rtPath, const string &r, const string &pofix );

void descriptorSIFT(const Mat &src,
					vector<KeyPoint> &keypoint,
					Mat &descriptor,
					bool sc,
					double scalar);

void getSIFTHomoMat(const Mat &fir, const Mat &sec, Mat &H, bool sc , double scalar);

#endif /* MACROS_H_ */
