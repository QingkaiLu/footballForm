#ifndef _IMG_RECTIFICATION_H_
#define _IMG_RECTIFICATION_H_

//FieldModel == 1: => high school field model;
//FieldModel == 2: => college field model;
//#define FieldModel 1
//
//
//#if FieldModel == 1
//	//high school football field model with size: 300 * 159(approximation of 160) feet
//	//http://www.sportsknowhow.com/football/field-dimensions/high-school-football-field-dimensions.html
//	#define FieldLength 795
//	#define FieldWidth 1800
//	#define EndZoneWidth 150
//	#define YardLinesDist 75
//	#define HashLinesDist 15
//	#define HashToSideLineDist 265
//	#define HashLinesLen 10
//#elif FieldModel == 2
//	//college football field model with size: 300 * 160 feet
//	//http://www.sportsknowhow.com/football/field-dimensions/ncaa-football-field-dimensions.html
//	#define FieldLength 800
//	#define FieldWidth 1800
//	#define EndZoneWidth 150
//	#define YardLinesDist 75
//	#define HashLinesDist 15
//	#define HashToSideLineDist 300
//	#define HashLinesLen 10
//#endif

//int FieldLength;
//int FieldWidth;
//int EndZoneWidth;
//int YardLinesDist;
//int HashLinesDist;
//int HashToSideLineDist;
//int HashLinesLen;

class fieldModel;

class imgRectfication
{
public:

	//if fldModel is 1, high school model
	//if fldModel is 2, college model
	//void setFldModel(int fldModel);
	imgRectfication(int fldModel);
	~imgRectfication();

	void drawFieldModel(cv::Mat &fieldModel);

	void getFieldYardLines(std::vector<std::vector<cv::Point2d> > &yardLines);

	bool readMatches(std::string matchesFile, std::vector<cv::Point2f> &srcPoints, std::vector<cv::Point2f> &dstPoints);

	bool readMatchesNew(std::string matchesFile, std::vector<cv::Point2f> &srcPoints, std::vector<cv::Point2f> &dstPoints);

	void rectifyImageToField(std::string matchesFile, const cv::Mat &srcImg, cv::Mat &dstImg, cv::Mat &homoMat);

	void rectifyImageToField(std::string matchesFile, const cv::Mat &srcImg, cv::Mat &dstImg, cv::Mat &homoMat, const cv::Mat &tMosFrmToPMosFrmHomo);

	//void homoTransPoint(const cv::Point2d &srcPnt, const cv::Mat &homoMat, cv::Point2d &dstPnt);

	void transFieldToImage(std::string matchesFile, cv::Mat &dstImg, cv::Mat &homoMat);

	void transFieldToImage(std::string matchesFile, const cv::Mat &srcImg, cv::Mat &dstImg, cv::Mat &homoMat);

	void saveMat(std::string fileName, cv::Mat& matData);

	void readMat(std::string fileName, cv::Mat& matData, int rows, int cols);

	fieldModel* fld;

};

#endif
