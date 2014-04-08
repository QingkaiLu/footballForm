#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "ellipse.h"
#include "commonStructs.h"

using namespace cv;
using namespace std;

//Mat src; Mat src_gray; Mat img;
//int thresh = 160;
//int max_thresh = 255;
//RNG rng(12345);

///// Function header
//void thresh_callback(string fileName, const Mat &src_gray, Mat &img);

/** @function main */
void ellipse(string inputFileName, string outputFileName, vector<RotatedRect> &ellipses)
{
	Mat src;
	Mat src_gray;
	Mat img;

  /// Load source image and convert it to gray
  //char fileName[80];
//  for (int i = 0; i < 449; i++) {
//	  cout << i << endl;
	  //sprintf(fileName, "/scratch/HUDL/pixelLabeling/game9/video0003/%05d_id.bmp", i);
	  src = imread( inputFileName, 1 );

//	  sprintf(fileName, "/scratch/HUDL/imgs/%05d.jpg", i+1);
	  img = imread( inputFileName, 1 );

	  /// Convert image to gray and blur it
	  cvtColor( src, src_gray, CV_BGR2GRAY );
	  threshold(src_gray, src_gray, 0, 255, THRESH_BINARY);
	  blur( src_gray, src_gray, Size(3,3) );

	  //sprintf(fileName, "/scratch/HUDL/pixelLabeling/game9/video0003/%05d.ellipse", i);

	  thresh_callback(outputFileName, src_gray, img, ellipses);

	  //waitKey();

//  }
  return;
}

/** @function thresh_callback */
//void thresh_callback(char *fileName)
void thresh_callback(string fileName, const Mat &src_gray, Mat &img, vector<RotatedRect> &ellipses)
{
  int thresh = 160;
  //int max_thresh = 255;
  RNG rng(12345);
  Mat threshold_output;
  vector<vector<Point> > contours;
  vector<Vec4i> hierarchy;
  unsigned int contourSize = 20;
  double maxWidthThresh = imgXLen / 2;
  double maxHeightThresh = imgYLen / 2;
  /// Detect edges using Threshold
  threshold( src_gray, threshold_output, thresh, 255, THRESH_BINARY );
  /// Find contours
  findContours( threshold_output, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );

  /// Find the rotated rectangles and ellipses for each contour
  vector<RotatedRect> minRect( contours.size() );
  vector<RotatedRect> minEllipse( contours.size() );

  vector<int> flag;
  for(unsigned int i = 0; i < contours.size(); i++ )
     { minRect[i] = minAreaRect( Mat(contours[i]) );
		 flag.push_back(1);
       if( contours[i].size() >  contourSize)
         { minEllipse[i] = fitEllipse( Mat(contours[i]) ); }
     }

  for (unsigned int i = 0; i < contours.size(); i++) {
	  if (minEllipse[i].size.height > 115 && minEllipse[i].size.height / minEllipse[i].size.width > 1.5 && minEllipse[i].angle < 200) {
		  //cout << minEllipse[i].size.height << " " << minEllipse[i].size.width << " " << minEllipse[i].angle << endl;
		  vector<Point> tmpContour1;
		  vector<Point> tmpContour2;
		  for (unsigned int j = 0; j < contours[i].size(); j++) {
			  if (contours[i][j].y < minEllipse[i].center.y) {
				  tmpContour1.push_back(contours[i][j]);
			  }
			  else {
				  tmpContour2.push_back(contours[i][j]);
			  }
		  }
		  minEllipse.push_back(fitEllipse(tmpContour1));
		  minEllipse.push_back(fitEllipse(tmpContour2));
		  minRect.push_back(minAreaRect(Mat(tmpContour1)));
		  minRect.push_back(minAreaRect(Mat(tmpContour2)));
		  flag[i] = 0;
		  continue;
	  }
	  /*if (minEllipse[i].size.height * minEllipse[i].size.width > 3000 && minEllipse[i].size.height / minEllipse[i].size.width < 1.5) {
		  //cout << "aa " << minEllipse[i].size.height << " " << minEllipse[i].size.width << " " << minEllipse[i].angle << endl;
		  vector<Point> tmpContour1;
		  vector<Point> tmpContour2;
		  for (int j = 0; j < contours[i].size(); j++) {
			  if (contours[i][j].x < minEllipse[i].center.x) {
				  tmpContour1.push_back(contours[i][j]);
			  }
			  else {
				  tmpContour2.push_back(contours[i][j]);
			  }
		  }
		  minEllipse.push_back(fitEllipse(tmpContour1));
		  minEllipse.push_back(fitEllipse(tmpContour2));
		  minRect.push_back(minAreaRect(Mat(tmpContour1)));
		  minRect.push_back(minAreaRect(Mat(tmpContour2)));
		  flag[i] = 0;
	  }*/
  }

  ofstream fout(fileName.c_str());
  /// Draw contours + rotated rects + ellipses
  Mat drawing = Mat::zeros( threshold_output.size(), CV_8UC3 );
  for(unsigned int i = 0; i< contours.size(); i++ )
     {
		 if (minEllipse[i].center.x > 800) continue;
		 if (minEllipse[i].center.y > 400) continue;
       Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
       // contour
       drawContours( drawing, contours, i, color, 1, 8, vector<Vec4i>(), 0, Point() );
       // ellipse
       ellipse( drawing, minEllipse[i], color, 2, 8 );
       ellipse( img, minEllipse[i], color, 2, 8 );
       // rotated rectangle
       Point2f rect_points[4]; minRect[i].points( rect_points );
	   if (minEllipse[i].size.width != 0 && minEllipse[i].center.x < 840
			   && minEllipse[i].size.width < maxWidthThresh && minEllipse[i].size.height < maxHeightThresh) {
		   fout << minEllipse[i].center.x << " " << minEllipse[i].center.y << " " << minEllipse[i].size.width << " " << minEllipse[i].size.height << " " << minEllipse[i].angle << " " << flag[i] << " ";
	   
		   if(flag[i])
			   ellipses.push_back(minEllipse[i]);
	       for( int j = 0; j < 4; j++ ) {
    	      line( drawing, rect_points[j], rect_points[(j+1)%4], color, 1, 8 );
	   		  fout << rect_points[j].x << " " << rect_points[j].y << " ";
		   }
		   fout << endl;
	   }
     }

	for (unsigned int i = contours.size(); i < minEllipse.size(); i++) {
		 if (minEllipse[i].center.x > 800) continue;
		 if (minEllipse[i].center.y > 400) continue;
		 if(minEllipse[i].size.width > maxWidthThresh || minEllipse[i].size.height > maxHeightThresh)
			 continue;

       Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
       Point2f rect_points[4]; minRect[i].points( rect_points );
       ellipse( img, minEllipse[i], color, 2, 8 );
	   fout << minEllipse[i].center.x << " " << minEllipse[i].center.y << " " << minEllipse[i].size.width << " " << minEllipse[i].size.height << " " << minEllipse[i].angle << " 1" << " ";
	   ellipses.push_back(minEllipse[i]);
	   for( int j = 0; j < 4; j++ ) {
          line( drawing, rect_points[j], rect_points[(j+1)%4], color, 1, 8 );
		  fout << rect_points[j].x << " " << rect_points[j].y << " ";
	   }
	   fout << endl;

	}
	fout.close();

	//ellipses = minEllipse;
  /// Show in a window
  namedWindow( "Contours", CV_WINDOW_AUTOSIZE );
  imshow( "Contours", drawing );
  imshow( "Src", img );
  //cvWaitKey(10);
}
