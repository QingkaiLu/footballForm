#include <string>
#include <sstream>
#include <cv.h>
#include <highgui.h>
#include "imgRectification.h"

using namespace std;
using namespace cv;

void rectifyPano(int gameId)
{
	ostringstream convertGameId;
	convertGameId << gameId ;
	string gameIdStr = convertGameId.str();

	if(gameId < 10)
		gameIdStr = "0" + gameIdStr;

	string panoPath = "panorama/panoGame" + gameIdStr + ".jpg";
	string matchesFilePath = "panorama/panoGame" + gameIdStr + "RectMatches";

	Mat pano = imread(panoPath.c_str(), CV_LOAD_IMAGE_COLOR);

	int fldModType = 1;
	imgRectfication imgRect(fldModType);

	Mat dstImg, homoMat;
	imgRect.rectifyImageToField(matchesFilePath, pano, dstImg, homoMat);

	string hMatPath = "panorama/panoGame" + gameIdStr + "HomoMat";
	imgRect.saveMat(hMatPath, homoMat);
//	imgRect.readMat(hMatPath, homoMat, 3, 3);
//	imgRect.saveMat(hMatPath, homoMat);

	string rectPanoPath = "panorama/rectPanoGame" + gameIdStr + ".jpg";
	imwrite(rectPanoPath, dstImg);
//	imshow("rectPano", dstImg);
//	waitKey();
}

//int main()
//{
//	rectifyPano(2);
//	return 1;
//}
