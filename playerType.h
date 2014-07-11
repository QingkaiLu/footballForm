#ifndef _PLAYER_TYPE_H_
#define _PLAYER_TYPE_H_

//enum playerTypeId{
//	lowWr,
//	upWr,
//	runBack
//};

//rectangle range in horizontal and vertical direction.
struct rectHorzVertRng{
	//std::vector<double> xRng, yRng;
	double xMin, xMax, yMin, yMax;
};

class playerType{
public:
	playerType(playerTypeId pId);
	void setUpPlayerTypeStr();
	void getSearchRng(const struct rect &imgBndRect, const cv::Point2d &rectLosCnt,
			direction offDir, struct rectHorzVertRng &rng);
	void getSearchRng(int fldModel, const cv::Point2d &rectLosCnt,
			direction offDir, struct rectHorzVertRng &rng);
	//get the model vector from the ideal position of the player type to the los center
	void getModelVec(const cv::Point2d &rectLosCnt, int fldModel, direction offDir,
			cv::Point2d &modelVec);

	playerTypeId pTypeId;
	std::string pTypeStr;
};

#endif
