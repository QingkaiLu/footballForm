#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "blob_HUDL.h"

using namespace cv;
using namespace std;

//Mat readMask(char *fileName) {
//	ifstream fin(fileName);
//	int imgWidth = 720, imgHeight = 480;
//	Mat mask;
//	mask.create(imgHeight, imgWidth, CV_8U);
//	for (int i = 0; i < imgHeight; ++i) {
//		for (int j = 0; j < imgWidth; ++j) {
//			int val;
//			fin >> val;
//			if (val == 0) mask.at<uchar>(i, j) = 0;
//			else mask.at<uchar>(i, j) = 255;
//		}
//	}
//	return mask;
//	fin.close();
//}

void FindBlobs(const Mat &binary, vector < vector<Point2i> > &blobs)
{
	blobs.clear();

	// Fill the label_image with the blobs
	// 0  - background
	// 1  - unlabelled foreground
	// 2+ - labelled foreground

	Mat label_image;
	binary.convertTo(label_image, CV_32FC1); // weird it doesn't support CV_32S!

	int label_count = 2; // starts at 2 because 0,1 are used already

	for(int y=0; y < binary.rows; y++) {
		for(int x=0; x < binary.cols; x++) {
			if((int)label_image.at<float>(y,x) != 1) {
				continue;
			}

			Rect rect;
			floodFill(label_image, cv::Point(x,y), cv::Scalar(label_count), &rect, cv::Scalar(0), cv::Scalar(0), 4);

			vector <cv::Point2i> blob;

			for(int i=rect.y; i < (rect.y+rect.height); i++) {
				for(int j=rect.x; j < (rect.x+rect.width); j++) {
					if((int)label_image.at<float>(i,j) != label_count) {
						continue;
					}

					blob.push_back(cv::Point2i(j,i));
				}
			}

			blobs.push_back(blob);

			label_count++;
		}
	}
}

vector<Rect> filterRect(vector<Rect> rectList) {
	vector<Rect> filtered;
	int *flag = new int[rectList.size()]();

	for (unsigned int i = 0; i < rectList.size(); ++i) {
		if (flag[i] != 0) continue;
		Rect rect1 = rectList[i];
		unsigned int j;
		for (j = i+1; j < rectList.size(); ++j) {
			if (flag[j] != 0) continue;
			Rect rect2 = rectList[j];
			if ((abs(rect1.x-rect2.x) < 8) && (abs(rect1.x+rect1.width-rect2.x-rect2.width) < 8) && (MIN(abs(rect1.y-rect2.y-rect2.height), abs(rect2.y-rect1.y-rect1.height)) < 20) ) {
				Rect newRect(MIN(rect1.x, rect2.x), MIN(rect1.y, rect2.y), (MAX(rect1.x+rect1.width, rect2.x+rect2.width)-MIN(rect1.x, rect2.x)), (MAX(rect1.y+rect1.height, rect2.y+rect2.height)-MIN(rect1.y, rect2.y)));
				filtered.push_back(newRect);
				flag[j] = 1;
				break;
			}
		}
		if (j == rectList.size()) {
			filtered.push_back(rect1);
		}
		flag[i] = 1;
	}

	delete flag;

	return filtered;
}


void blob(string inputFile, string outputFile, string imageFile)
{
	//namedWindow("labelled");
	
//	for (int frameNum = 0; frameNum <= 449; frameNum++) {
//		cout << frameNum << endl;

//		char inputFile[80];
//		sprintf(inputFile, "/scratch/HUDL/pixelLabeling/game9/video0003/%05d.bmp", frameNum);
		Mat img = imread(inputFile, 0);

		Mat binary;
		vector < vector<Point2i > > blobs;
//		imshow("bb", img);
//		cvWaitKey(5);
//		for(int i = 0; i < img.rows; ++i)
//			for(int j = 0; j < img.cols; ++j)
//				cout << (int)img.at<uchar>(i, j) << " ";
//		cout << endl;

		threshold(img, binary, 150, 1.0, THRESH_BINARY);

		FindBlobs(binary, blobs);

//		char outputFile[80];
//		sprintf(outputFile, "/scratch/HUDL/pixelLabeling/game9/video0003/%05d.ccDet", frameNum);
		ofstream fout(outputFile.c_str());

		Mat outputImage = Mat::zeros(img.size(), CV_8UC1);

		vector<Rect> rectList;
		cout << "blobs.size(): " << blobs.size() << endl;
		for(size_t i=0; i < blobs.size(); i++) {
			/*unsigned char r = 255 * (rand()/(1.0 + RAND_MAX));
			unsigned char g = 255 * (rand()/(1.0 + RAND_MAX));
			unsigned char b = 255 * (rand()/(1.0 + RAND_MAX));*/

			int tlx = blobs[i][0].x, tly = blobs[i][0].y;
			int brx = blobs[i][0].x, bry = blobs[i][0].y;

			for(size_t j=0; j < blobs[i].size(); j++) {
				int x = blobs[i][j].x;
				int y = blobs[i][j].y;

				if (x < tlx) tlx = x;
				if (y < tly) tly = y;
				if (x > brx) brx = x;
				if (y > bry) bry = y;

				/*output.at<Vec3b>(y,x)[0] = b;
				output.at<Vec3b>(y,x)[1] = g;
				output.at<Vec3b>(y,x)[2] = r;*/
			}
			rectList.push_back(Rect(tlx, tly, brx-tlx+1, bry-tly+1));
		}

		vector<Rect> filteredRectList = rectList;
		/*while (1) {
			filteredRectList = filterRect(rectList);
			if (filteredRectList.size() == rectList.size()) break;
			rectList = filteredRectList;
		}*/

		int currId = 0;

		for (unsigned int i = 0; i < filteredRectList.size(); i++) {
			int tlx = filteredRectList[i].x, tly = filteredRectList[i].y;
			int brx = filteredRectList[i].x+filteredRectList[i].width, bry = filteredRectList[i].y+filteredRectList[i].height;

			if ((tly + bry) / 2 < 100) continue;

			Point pt1(tlx, tly), pt2(brx, bry);
			//rectangle(output, pt1, pt2, CV_RGB(255, 0, 0));
			if (brx - tlx + 1 < 15) continue;
			if (bry - tly + 1 < 30) continue;
			fout << tlx << " " << tly << " " << brx-tlx << " " << bry-tly << endl;

			currId += 1;
			for (size_t j=0; j < blobs[i].size(); j++) {
				int x = blobs[i][j].x;
				int y = blobs[i][j].y;
				outputImage.at<uchar>(y,x) = currId;
			}

			rectangle(img, pt1, pt2, CV_RGB(255, 255, 255));
		}
		cout << currId << endl;
//		char imageFile[80];
//		imshow("aa", img);
//		waitKey();
//		sprintf(imageFile, "/scratch/HUDL/pixelLabeling/game9/video0003/%05d_id.bmp", frameNum);
		imwrite(imageFile, outputImage);
		fout.close();

//	}

	/*imshow("labelled", output);
	  waitKey(0);*/

	return;
}
