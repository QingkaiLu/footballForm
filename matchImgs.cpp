/*
 * macros.cpp
 *
 *  Created on: May 27, 2013
 *      Author: fengzh
 */

#include <vector>
//#include <string>
#include <cv.h>
#include <highgui.h>
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/nonfree/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/nonfree/nonfree.hpp"
#include <opencv2/legacy/legacy.hpp>

#include "commonStructs.h"
#include "matchImgs.h"

using namespace std;
using namespace cv;


//string num2str( int base, int num)
//{
//	vector<string> strnum(base);
//	for ( int i = 0; i < base; ++i)
//		strnum[i] = "0";
//	if ( num < 10)
//	{
//		ostringstream out;
//		out << num;
//		strnum[0] = out.str();
//	}
//	else if ( num >= 10 && num < 100)
//	{
//		ostringstream ba, ten;
//		int t = floor(double(num) / 10), b = num % 10;
//		ba << b;
//		ten << t;
//		strnum[0] = ba.str();
//		strnum[1] = ten.str();
//	}
//	else
//	{
//		ostringstream ba, ten, hun;
//		int b = num % 10, t = int(floor( double(num / 10))) % 10, h = num / 100;
//		ba << b, ten << t, hun << h;
//		strnum[0] = ba.str();
//		strnum[1] = ten.str();
//		strnum[2] = hun.str();
//	}
//	string name;
//	for ( int i = base - 1; i >= 0; --i)
//	{
//		name += strnum[i];
//	}
//
//	return name;
//}

//string ldFolder( int n , int base, bool addPrefix, const string &rtPath, const string &prefix, const string &pofix )
//{
////	string rtPath = "./video/videos/";
//
////	string prefix = "video";//, pofix = ".avi";
//	string num = num2str( base, n);
//	string name = "";
//	if ( addPrefix)
//		name = rtPath + prefix + num + pofix;
//	else name = prefix + num ;
//	return name;
//}

//void ldVP ( const string &fname,
//			vector<Point2f> &vp)
//{
//	bool isEOF = false;
//
//	FILE *fp = fopen(fname.c_str(), "r");
//
//	if (!fp) {
//		printf("open vanishing point file failed.\n"); return;
//	}
//
//	while( !isEOF)
//	{
//		int fid, idx,idy;
//
//		if(fscanf(fp, "%d",&fid)!=EOF)
//		{
//			if ( fscanf(fp,"%d %d",&idx,&idy)!=EOF)
//				vp.push_back(Point2f(idx, idy));
//			else
//				isEOF = true;
//		}
//		else isEOF = true;
//	}
//}

//void loadVideo( const string &capName, vector<Mat> &frame,
//				int &vlen, int &width, int &height, bool ldframe = false)
//{
//	VideoCapture cap ( capName);
//	vlen = cap.get ( CV_CAP_PROP_FRAME_COUNT);
//	width= cap.get( CV_CAP_PROP_FRAME_WIDTH);
//	height = cap.get( CV_CAP_PROP_FRAME_HEIGHT);
//	frame.clear();frame.reserve( vlen);
//	if (ldframe)
//		for(int n = 0; n < vlen;++n) {Mat f; cap>>f; frame.push_back(f.clone());}
////	exit(0);
//}

//void getPtCoord( int width, int height, vector<Point2f> &pts)
//{
//	pts.reserve( width * height);
//	for ( int w = 0; w < width; ++w)
//		for( int h = 0; h < height; ++h)
//		{
//			pts.push_back(Point2f(w,h));
//		}
//}

//void playVideos()
//{
//	string rtpath = DPATH, prefix = "video",pofix = ".mp4";
//	 rtpath += FOLDER, rtpath += "video/";
//	int vbase = 3;
//	for ( int n = 1; n <= 50; ++n)
//	{
//		namedWindow("check MoS", CV_WINDOW_NORMAL);
//
//		//string videoName = ldFolder(n, vbase, true,rtpath, prefix,pofix);
//		string vpath = "/scratch/projects/qt/qt_rectify/rectified/v_homog/game02/video/";
//		string videoName = vpath ;
//		ostringstream id; id<<n;
//		videoName += id.str() + ".avi";
//		VideoCapture cap( videoName);
//		if(!cap.isOpened()){
//			printf("video[%d] not valid\n",n);continue;
//		}
//
//		int nLen = cap.get ( CV_CAP_PROP_FRAME_COUNT);
//
//		vector<Mat> frames;//(nLen);
//
//		int beginFrame = 0;// gtMOS[n];
//		int curframe = beginFrame;
//
//		for ( int m = 0; m < beginFrame; ++m)
//		{
//			Mat frame;
//			bool status = cap.read(frame);
//			if ( !status )
//				break;
//
//			frames.push_back( frame.clone());
//		}
//
//		for ( ; ; )
//		{
//			Mat frame;
//			bool status = cap.read(frame);
//			if ( !status && curframe == nLen - 1 )
//				break;
//
//			frames.push_back( frame.clone());
//
//			int seq = min(max(0, curframe), nLen - 1 );
//
//			Mat tframe = frames[seq].clone();
//			ostringstream curf, vseq, vlen, mos;
//			curf << curframe; vseq << n;vlen << nLen;
//			string strDisp = "video:" + vseq.str()+"len:"+vlen.str();
//			putText( tframe, strDisp, Point(10,30), FONT_HERSHEY_SIMPLEX,
//					 1.0, Scalar(255,0,0),2);
//			strDisp = "gtMOS:" + mos.str();
//			putText( tframe, strDisp, Point(10,70), FONT_HERSHEY_SIMPLEX,
//					 1.0, Scalar(255,0,0),2);
//			strDisp = "frame:" + curf.str();
//			putText( tframe, strDisp, Point(10,110), FONT_HERSHEY_SIMPLEX,
//					 1.0, Scalar(255,0,0),2);
//			imshow("check MoS",tframe);
//
//			int c = waitKey(2);
////			if ( c != -1)
////				int db = 1;
//			switch ( c)
//			{
//				case 1113939: // right arrow
//					curframe = curframe + 1 > nLen - 1 ? nLen - 1: curframe + 1; break;
//
//				case 1113937: // left arrow
//					curframe = curframe - 1 < 0 ? 0 : curframe - 1; break;
//
//				case 1113940: // down
//					goto nextVideo; break;
//
//				case 1114083: // quicker
//					curframe = curframe + 30 > nLen - 1 ? nLen - 1:curframe + 30; break;
//
//				case 1113938: //up
//					n = max(0,n-2); goto nextVideo;
//			}
//		}
//		nextVideo: ;
//	}
//
//	exit(0);
//}

//void Vect2Mat(const vector<vector<float> > &vect, Mat &mtx)
//{
//	if (mtx.empty())
//		mtx = Mat::zeros( vect.size(), vect[0].size(), CV_32FC1);
//
//    for (uint i=0; i<vect.size(); i++)
//        for (uint j=0; j<vect[i].size(); j++)
//        {
//            mtx.at<float>(i,j) = vect[i][j];
//        }
//}

//float vecdist( const vector<float> &v1, const vector<float> &v2){
//	float sum = 0;
//	for ( uint n = 0; n < v1.size(); ++n)
//		sum += (v1[n] - v2[n]) * (v1[n] - v2[n]);
//	return sqrt(sum);
//}

void getSIFTHomoMat(const Mat &fir, const Mat &sec, Mat &H, bool sc = false , double scalar = 1.0)
{
	bool useFAST = false;
	if ( useFAST){
		Ptr<DescriptorExtractor> FAST_descriptor;
		Ptr<DescriptorMatcher> FAST_matcher;
		vector<cv::DMatch> FAST_matches;
		vector<KeyPoint> FAST_train_kpts, FAST_query_kpts;
		vector<Point2f> FAST_train_pts, FAST_query_pts;
		Mat FAST_train_desc, FAST_query_desc;
		FAST_descriptor = new BriefDescriptorExtractor(32);
		FAST_matcher = DescriptorMatcher::create("BruteForce-Hamming");
		Mat firG, secG;
		cvtColor( fir, firG, CV_RGB2GRAY);
		cvtColor( sec, secG, CV_RGB2GRAY);

		FAST(firG, FAST_train_kpts,30);
		FAST_descriptor->compute(firG, FAST_train_kpts, FAST_train_desc);
		FAST(secG, FAST_query_kpts,30);
		FAST_descriptor->compute(secG, FAST_query_kpts, FAST_query_desc);

		Mat FAST_mask = windowedMatchingMask(FAST_query_kpts, FAST_train_kpts, 25, 25);

		FAST_matcher->match(FAST_query_desc, FAST_train_desc, FAST_matches, FAST_mask);

		FAST_train_pts.reserve(FAST_matches.size());
		FAST_query_pts.reserve(FAST_matches.size());

		size_t i = 0;

		for (; i < FAST_matches.size(); i++)
		{
			const DMatch & dmatch = FAST_matches[i];
			FAST_train_pts.push_back(FAST_train_kpts[dmatch.queryIdx].pt);
			FAST_query_pts.push_back(FAST_query_kpts[dmatch.trainIdx].pt);
		}
		H = findHomography(FAST_train_pts, FAST_query_pts, RANSAC);
		if ( !H.empty())
			H = Mat::eye(3,3,CV_32FC1);

//		FAST_descriptor.delete_obj();
		return;
	}
	Mat in = sec.clone();

	vector<vector<float> > dsp1, dsp2;
	vector<KeyPoint> kp1, kp2;
	Mat dpmat1, dpmat2;

	vector<int4> bound1, bound2;

	IplImage nfir = (IplImage)fir, nsec = (IplImage)sec;

	Mat contour;
	descriptorSIFT(fir,kp1,dpmat1, sc, scalar);
	descriptorSIFT(sec,kp2,dpmat2, sc, scalar);

	if ( kp1.size() == 0 || kp2.size() == 0)
	{
		printf(" # key points 0. exit\n");
		H = Mat::eye(3,3,CV_32FC1);
		return;
	}
	/******************************************************
	 * matching
	 */
	BruteForceMatcher<L2<float> > matcher;
	vector<DMatch> matches;
	matcher.match(dpmat1, dpmat2, matches);

	vector<int> pairOfsrcKP(matches.size()), pairOfdstKP(matches.size());
	for (size_t i = 0; i < matches.size(); i++)
	{
		pairOfsrcKP[i] = matches[i].queryIdx;
		pairOfdstKP[i] = matches[i].trainIdx;
	}

	vector<Point2f> sPoints;
	KeyPoint::convert(kp1, sPoints, pairOfsrcKP);
	vector<Point2f> dPoints;
	KeyPoint::convert(kp2, dPoints, pairOfdstKP);

	/******************************************************
	 * homography matrix
	 */
	vector<uchar> inlier;
	H.release();
	// if less than 4 points
	if ( dPoints.size() < 4 || dPoints.size() < 4)
	{
//		printf(" points not enough. exit\n");
		H = Mat::eye(Size(3,3), CV_32FC1);
		return ;
	}
	H = findHomography(sPoints, dPoints, inlier, CV_RANSAC);

}

void descriptorSIFT(const Mat &src,
					vector<KeyPoint> &keypoint,
					Mat &descriptor,
					bool sc,
					double scalar)
{

	Mat in = src.clone();
	if ( sc)
		resize(in, in, Size(int(in.rows * scalar), int(in.cols * scalar)), 0, 0);

	SiftFeatureDetector detector;

	SiftDescriptorExtractor extractor;

	detector.detect( in, keypoint );

	int imgHeight = in.rows;

	// rm edge area points
	int bdValue = 50;
	for ( vector<KeyPoint>::iterator itr = keypoint.begin();
			itr != keypoint.end();)
	{
		Point2f curPt = (*itr).pt;
		if ( curPt.y < bdValue || curPt.y > imgHeight - bdValue){
			itr = keypoint.erase(itr);
		}
		else{
			++itr;
		}
	}

	// compute sift descriptor
	extractor.compute( in, keypoint, descriptor );

}

