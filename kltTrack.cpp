//#include <iostream>
//#include <fstream>
//#include <cv.h>
//#include <highgui.h>
//#include <vector>
//
//#include "klt.h"
//#include "pnmio.h"
//#include "kltTrack.h"
//
//
//using namespace std;
//using namespace cv;
//
//namespace kltTrack{
//
//bool is2PtsStillToLines(Mat& frame, CvPoint pt1, vector<struct yardLine> lines1, CvPoint pt2, vector<struct yardLine> lines2)
//{
//	if((lines1.size() <= 3) || (lines2.size() <= 3))
//		return false;
////	double absMinDist = 1000000;
//	int minDistLineIdxPt1 = -1;
//	//double minDist;
//	int minDistLineIdxPt2 = -1;
//	vector<struct yardLine> lines1Cpy = lines1;
//	while( (minDistLineIdxPt2 == -1) && (!lines1.empty()) )
//	{
//		double absMinDist = 1000000;
//
//		for(int i = 0; i < lines1.size(); ++i)
//		{
//			double dist = abs(distFromPntToLine(pt1, lines1[i]));
////			Point2d intersect;
////			double dist = distFromPnt0ToLineIntersectPnt(pt1, pt2, lines1[i], intersect);
////			circle(frame, intersect, 5,CV_RGB(0, 255, 0), -1);
//			if( (dist < absMinDist))// && (dist > 5))
//			{
//				//circle(frame, intersect, 5,CV_RGB(0, 255, 0), -1);
//				absMinDist = dist;
//				minDistLineIdxPt1 = i;
//				//cout<<"absMinDist: "<<absMinDist<<endl;
//			}
//			//cout<<lines1[i].index<<endl;
//		}
//		//return false;
//		for(int i = 0; i < lines2.size(); ++i)
//		{
//			if(lines2[i].index == lines1[minDistLineIdxPt1].index)
//			{
//				minDistLineIdxPt2 = i;
//				//cout<<"line2 index: "<<lines2[i].index<<"line1 index: "<<lines1[minDistLineIdxPt1].index<<endl;
//				break;
//			}
//		}
//
//		lines1.erase(lines1.begin() + minDistLineIdxPt1);
//	}
//
//	if((minDistLineIdxPt2 != -1) && (minDistLineIdxPt1 != -1))
//	{
//		double dist1 = distFromPntToLine(pt1, lines1Cpy[minDistLineIdxPt1]);
//		double dist2 = distFromPntToLine(pt2, lines2[minDistLineIdxPt2]);
////		Point2d intersect1, intersect2;
////		double dist1 = distFromPnt0ToLineIntersectPnt(pt1, pt2, lines1Cpy[minDistLineIdxPt1], intersect1);
////		double dist2 = distFromPnt0ToLineIntersectPnt(pt2, pt1, lines2[minDistLineIdxPt2], intersect2);
////		cout<<"dist1: "<<dist1<<"dist2: "<<dist2<<endl;
////		line(frame, pt1, intersect1, CV_RGB(0, 0, 255), 2, 8, 0);
////		circle(frame, intersect1, 5,CV_RGB(0, 0, 255), -1);
////		line(frame, pt2, intersect2, CV_RGB(255, 0, 0), 2, 8, 0);
////		circle(frame, intersect2, 5,CV_RGB(255, 0, 0), -1);
////		Point2d intersect1, intersect2;
////		Point2d pt1PerpToLnPnt, pt2PerpToLnPnt;
////		perpPntFromPnt0ToLine(pt1, lines1Cpy[minDistLineIdxPt1], pt1PerpToLnPnt);
////		perpPntFromPnt0ToLine(pt2, lines2[minDistLineIdxPt2], pt2PerpToLnPnt);
////		double dist1 = distFromPnt0ToLineIntersectPnt(pt1, pt1PerpToLnPnt, lines1Cpy[minDistLineIdxPt1], intersect1);
////		double dist2 = distFromPnt0ToLineIntersectPnt(pt2, pt2PerpToLnPnt, lines2[minDistLineIdxPt2], intersect2);
//		//cout<<"dist1: "<<dist1<<"dist2: "<<dist2<<endl;
////		Point2d pt1Prime = Point2d(dist1*(pt1PerpToLnPnt.x - pt1.x), dist1*(pt1PerpToLnPnt.y - pt1.y)) ;
////		pt1Prime.x = pt1.x - pt1Prime.x;
////		pt1Prime.y = pt1.y - pt1Prime.y;
////		line(frame, pt1, pt1Prime, CV_RGB(0, 0, 255), 2, 8, 0);
////		circle(frame, pt1Prime, 3, CV_RGB(0, 0, 255), -1);
////		Point2d pt2Prime = Point2d(dist2*(pt2PerpToLnPnt.x - pt2.x), dist2*(pt2PerpToLnPnt.y - pt2.y)) ;
////		pt2Prime.x = pt2.x + pt2Prime.x;
////		pt2Prime.y = pt2.y + pt2Prime.y;
////		line(frame, pt2, pt2Prime, CV_RGB(255, 0, 0), 2, 8, 0);
////		circle(frame, pt2Prime, 5,CV_RGB(255, 0, 0), -1);
//
////		if(intersect1.x != 0)
////		{
////			line(frame, pt1, intersect1, CV_RGB(0, 0, 255), 2, 8, 0);
////			circle(frame, intersect1, 5,CV_RGB(0, 0, 255), -1);
////		}
//
////		line(frame, pt1, intersect1, CV_RGB(0, 0, 255), 2, 8, 0);
////		circle(frame, intersect1, 5,CV_RGB(0, 0, 255), -1);
////		line(frame, pt2, intersect2, CV_RGB(255, 0, 0), 2, 8, 0);
////		circle(frame, intersect2, 5,CV_RGB(255, 0, 0), -1);
//		//return false;
//		if((abs(dist1 - dist2) / abs(closestLnDist(lines1Cpy))) < 0.05)
//		{
//			//cout<<"true"<<endl;
//			return true;
//		}
//		return false;
//	}
//
//	return false;
//}
//
//
//double distFromPntToLine(Point2d pt0, struct yardLine wLine)
//{
//	float rho = wLine.rho;
//	float theta = wLine.theta;
////	rho =  2;
////	theta = 0.7854;
////	pt0.x = 2.0;
////	pt0.y = 2.0;
//	Point2d pt1, pt2;
//	double cosTheta = cos(theta), sinTheta = sin(theta);
//	double a = -cosTheta / sinTheta, b = rho / sinTheta;
//	double x0 = cosTheta*rho, y0 = sinTheta*rho;
//	pt1.x = x0 + 1000*(-sinTheta);
//	pt1.y = y0 + 1000*(cosTheta);
//	pt2.x = x0 - 1000*(-sinTheta);
//	pt2.y = y0 - 1000*(cosTheta);
////	double dist = abs((pt2.x - pt1.x) * (pt1.y - pt0.y) * 1.0 - (pt1.x - pt0.x) * (pt2.y - pt1.y) * 1.0)
////			/ sqrt((pt2.x - pt1.x) * (pt2.x - pt1.x) * 1.0 + (pt2.y - pt1.y) * (pt2.y - pt1.y) * 1.0);
//	double dist = ((pt2.x - pt1.x) * (pt1.y - pt0.y) * 1.0 - (pt1.x - pt0.x) * (pt2.y - pt1.y) * 1.0)
//			/ sqrt((pt2.x - pt1.x) * (pt2.x - pt1.x) * 1.0 + (pt2.y - pt1.y) * (pt2.y - pt1.y) * 1.0);
//	//cout<<"dist: "<<dist<<endl;
//	return dist;
//}
//
//
//
//int kltTrack(string vidPath, int startFrm, int numFrmsToTrk, string trksPath)
//{
//    string postfix = ".avi";
//    int postfixPos = vidPath.find(postfix);
//    char vidNumChar[10];
//    vidPath.copy(vidNumChar, 3, (postfixPos - 3));
//    vidNumChar[3]='\0';
//    string vidNumStr(vidNumChar);
//	stringstream convert(vidNumStr);
//
//	int vidNum = -1;
//	if ( !(convert >> vidNum) )
//		vidNum = -1;
//	//cout<<"vidNum: "<<vidNum<<endl;
//
//	trksPath += vidNumStr;
//
//	VideoCapture capture(vidPath);
//	if (!capture.isOpened()) {
//		cout << "Error in opening the video";
//		return -1;
//	}
//
//	int endFrm = startFrm + numFrmsToTrk - 1;
//	cout << endFrm << endl;
//	int frmsNum = capture.get(CV_CAP_PROP_FRAME_COUNT);
//	cout << "frames number:" << frmsNum << endl;
//	if(endFrm > frmsNum)
//	{
//		endFrm = frmsNum;
//		numFrmsToTrk = endFrm - startFrm + 1;
//	}
//
//	unsigned char *img1, *img2;
//	KLT_TrackingContext tc;
//	KLT_FeatureList fl;
//	KLT_FeatureTable ft;
//	int numFeat = 300;
//
//    tc = KLTCreateTrackingContext();
//	fl = KLTCreateFeatureList(numFeat);
//	ft = KLTCreateFeatureTable(numFrmsToTrk, numFeat);
//	tc->sequentialMode = TRUE;
//	tc->writeInternalImages = FALSE;
//	tc->affineConsistencyCheck = 2;
//
////	CvCapture* capture2 = 0;
////	capture2 = cvCreateFileCapture(vidPath.c_str());
////	double fps = cvGetCaptureProperty (capture2, CV_CAP_PROP_FPS);
////
////	CvSize size = cvSize((int)cvGetCaptureProperty(capture2, CV_CAP_PROP_FRAME_WIDTH),
////			(int)cvGetCaptureProperty( capture2, CV_CAP_PROP_FRAME_HEIGHT));
////
////	CvVideoWriter *writer = cvCreateVideoWriter("Output.avi",CV_FOURCC('M','J','P','G'), fps, size);
//
//	vector<track> tracks;
//	Mat currentFrm, grayCurFrm;
//	for(int i = 0; i < startFrm; ++i)
//		capture >> currentFrm;
//	cvtColor(currentFrm, grayCurFrm, CV_BGR2GRAY);
//	int ncols = currentFrm.cols, nrows = currentFrm.rows;
//
//	img1 = (unsigned char *) malloc(ncols * nrows * sizeof(unsigned char));
//	img2 = (unsigned char *) malloc(ncols * nrows * sizeof(unsigned char));
//	int idx = 0;
//	for (int i = 0; i < grayCurFrm.rows; ++i)
//	{
//		for (int j = 0; j < grayCurFrm.cols; ++j)
//		{
//			img1[idx] = grayCurFrm.at<uchar>(i, j);
//			idx++;
//		}
//	}
//
//	KLTSelectGoodFeatures(tc, img1, ncols, nrows, fl);
//	KLTStoreFeatureList(fl, ft, 0);
//
//	for (int frmIdx = 1; frmIdx < numFrmsToTrk; ++frmIdx)
//	{
//		cout << frmIdx << endl;
//		capture >> currentFrm;
//		cvtColor(currentFrm, grayCurFrm, CV_BGR2GRAY);
//
//		int idx = 0;
//		for (int i = 0; i < grayCurFrm.rows; ++i)
//		{
//			for (int j = 0; j < grayCurFrm.cols; ++j)
//			{
//				img2[idx] = grayCurFrm.at<uchar>(i, j);
//				idx++;
//			}
//		}
//
//		KLTTrackFeatures(tc, img1, img2, ncols, nrows, fl);
//		KLTReplaceLostFeatures(tc, img2, ncols, nrows, fl);
//		KLTStoreFeatureList(fl, ft, frmIdx);
//
//		/**
//		 * Creating the Video
//		 */
////		for (int i = 0; i < ft->nFeatures; i++) {
////			for (int j = frmIdx;
////					j >= 0 && ft->feature[i][j]->val >= 0; j--) {
////				if (ft->feature[i][j]->val > 0) {
////					circle(currentFrm,
////							Point(ft->feature[i][j]->x,
////									ft->feature[i][j]->y), 4,
////							CV_RGB(0, 255, 0), -1);
////
////					break;
////				} else if (ft->feature[i][j]->val == 0) {
////					circle(currentFrm,
////							Point(ft->feature[i][j]->x,
////									ft->feature[i][j]->y), 2,
////							CV_RGB(255, 0, 0), -1);
////				}
////
////				if (ft->feature[i][j]->val < 0)
////					cout << "#########Negative##########" << endl;
////			}
////		}
//
//
//		for (int n = 0; n < ft->nFeatures; ++n)
//		{
////			cout << ft->feature[n][frmIdx - 1]->val << endl;
////			cout << " " << (ft->feature[n][frmIdx - 1]->val == 0 )<< endl;
//			struct track trk;
//			//trk.trackId = n;
//			if (ft->feature[n][frmIdx]->val == 0)
//			{
////				struct track trk;
//				trk.endFrm = frmIdx + startFrm + 1;
//				trk.endPos.x = ft->feature[n][frmIdx]->x;
//				trk.endPos.y = ft->feature[n][frmIdx]->y;
//				int startIdx = frmIdx - 1;
//				for (; startIdx >= 0 && ft->feature[n][startIdx]->val == 0; --startIdx);
//				if(startIdx >= 0 && ft->feature[n][startIdx]->val > 0)
//				{
//					trk.startPos.x = ft->feature[n][startIdx]->x;
//					trk.startPos.y = ft->feature[n][startIdx]->y;
//					trk.startFrm = startIdx + startFrm + 1;
//
//					line(currentFrm, trk.startPos, trk.endPos, CV_RGB(255, 0, 0), 1, 8, 0);
//					circle(currentFrm, trk.endPos, 2, CV_RGB(255, 0, 0), -1);
//				}
//
//				if(frmIdx == (numFrmsToTrk - 1) )
//				{
//					Point2d trkVec;
//					trkVec.x = trk.endPos.x - trk.startPos.x;
//					trkVec.y = trk.endPos.y - trk.startPos.y;
//					if(norm(trkVec) > 2.0)
//						tracks.push_back(trk);
//				}
//
////				if(ft->feature[n][frmIdx]->val == 0)// if the track ends, push it into tracks
////					tracks.push_back(trk);
//			}
//			else // ft->feature[n][frmIdx]->val > 0: old track ends and new track starts
//			{
//				//trk.trackId = n * 1000;
//				if(ft->feature[n][frmIdx - 1]->val ==0)
//				{
//					trk.endFrm = frmIdx + startFrm;
//					trk.endPos.x = ft->feature[n][frmIdx - 1]->x;
//					trk.endPos.y = ft->feature[n][frmIdx - 1]->y;
//
//					int startIdx = frmIdx - 2;
//					for (; startIdx >= 0 && ft->feature[n][startIdx]->val == 0; --startIdx);
//					if(startIdx >= 0 && ft->feature[n][startIdx]->val > 0)
//					{
//						trk.startPos.x = ft->feature[n][startIdx]->x;
//						trk.startPos.y = ft->feature[n][startIdx]->y;
//						trk.startFrm = startIdx + startFrm + 1;
//
//						Point2d trkVec;
//						trkVec.x = trk.endPos.x - trk.startPos.x;
//						trkVec.y = trk.endPos.y - trk.startPos.y;
//						if(norm(trkVec) > 2.0)
//							tracks.push_back(trk);
//
//					}
//
//				}
//
//			}
//		}
//
//		imshow(vidPath, currentFrm);
//		cvWaitKey(2);
//
//	}
//
//
//	ofstream fout(trksPath.c_str());
//	cout << tracks.size() << endl;
//	for(unsigned int i = 0; i < tracks.size(); ++i)
//	{
//		//fout << tracks[i].trackId << " ";
//		fout << tracks[i].startFrm << " " << tracks[i].endFrm << " " << tracks[i].startPos.x << " ";
//		fout << tracks[i].startPos.y << " " << tracks[i].endPos.x << " " << tracks[i].endPos.y << endl;
//	}
//
//	fout.close();
//
//	//KLTWriteFeatureTable(ft, "features.txt", "%5.1f");
////	cvReleaseCapture(&capture2);
////	cvReleaseVideoWriter(&Writer);
//	free(img1);
//	free(img2);
//	KLTFreeFeatureTable(ft);
//	KLTFreeFeatureList(fl);
//	KLTFreeTrackingContext(tc);
//	return 1;
//}
//
//int main(int argc, char* argv[]) {
//	if(argc != 2)
//	{
//		//cout<<argc<<endl;
//		cout<<"Wrong arguments!"<<endl;
//		return 0;
//	}
//
//	string vidPath(argv[1]);
//
//	cout<<vidPath<<endl;
//	clock_t t;
//	t = clock();
//	int startFrm = 100;
//	int numFrmsToTrk = 45;
//	string trksPath = "Tracks/video";
//	kltTrack(vidPath, startFrm, numFrmsToTrk, trksPath);
//	//int kltRet = kltTrack(vidPath);//, lineDir, mosPathStr);
//	t = clock() - t;
//	printf ("KLT took (%f seconds).\n",((float)t)/CLOCKS_PER_SEC);
//}
//
//} // namespace kltTrack
