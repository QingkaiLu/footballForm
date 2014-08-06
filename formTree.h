#ifndef _FORM_TREE_H_
#define _FORM_TREE_H_
#include <cv.h>

struct part
{
	std::string partName;
//	std::vector<struct part* > nbParts;
	int depth;
	struct part* parent;
	std::vector<struct part* > children;
	cv::Point2d location;
	cv::Point2d relLocToPar;
	std::vector<cv::Point2d> locSet;
	std::vector<double> locSetAppScore;
	double score, appScore;
	//only for star model
	double spaScore;
};

class formTree{
public:
	formTree(std::string formFile);
	void setupFormTree(const std::string &formFile);
//	void setOLAsRoot();
//	void sortParts();
//	void setupAllPartsLocSet(std::vector<std::vector<cv::Point2d> >);
//	void setupPartLocSet(struct part &p, std::vector<cv::Point2d>);
	void setupPartsLocSetStarModel(const cv::Point2d &rectLosCnt,
			const std::vector<cv::Point2d> &pLocSetFld, const std::vector<double> &score);
	void findBestFormStarModel();
	void plotFormOrigImg(cv::Mat &img, const cv::Mat &fldToOrgHMat);
public:
	std::vector<part> parts;
	std::string formName;
	double formBestScore;
};

#endif
