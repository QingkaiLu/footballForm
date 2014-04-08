#ifndef _FORMATION_H_
#define _FORMATION_H_

#include "commonStructs.h"
#include "player.h"

#define lnModel 1

class play;
//class player;

class formation{
public:
	formation(const play *pl, const std::vector<playerType> &pTypes);
	formation(const play *pl);
//	formation(play *pl, const std::vector<playerType> &pTypes);
//	formation(play *pl);
	//formation(play *pl, const std::vector<playerType> &pTypes);
	~formation();
	void setPlayerTypes(const std::vector<playerType> &pTypes);
	void setPlayers(const std::vector<player> &ps);
	//compute formation
	void compForm();
	void compForm(std::vector<double> &weight, std::vector<double> &feature);
	void compForm(const std::vector<playerBndBox> &topBoxes);
	void compPlayersRange(struct rect &lowRng, struct rect &upRng);
	void compPlayersRange(struct rect &lowLeftRng, struct rect &lowRightRng, struct rect &upLeftRng, struct rect &upRightRng);
public:
	std::vector<player> players;
	const play *p;
	direction dir;
	struct rect lowRange, upRange;
	//play *p;
	double totalScore;
};

//void compBestPlayerPos(const play *p, int startY, int endY, int startX, int endX, double boxXLen, double boxYLen, std::vector<player> &players);
void compBestPlayerPos(const play *p, int bnd[4], double boxXLen, double boxYLen, std::vector<player> &players);

//compute player positions for left/right direction model
double compOneDirBestPlayerPos(const play *p, int bnd[4], double boxXLen, double boxYLen, std::vector<player> &players, direction dir);

double compOneDirBestPlayerPos(const play *p, int bnd[4], double boxXLen, double boxYLen, std::vector<double> &weight, std::vector<player> &players, direction dir, std::vector<double> &feature);

double compOneDirBestPlayerPos(const play *p, const std::vector<playerBndBox> &topBoxes, std::vector<player> &players, direction dir);

//compute player positions for left/right direction model with a range for receivers and cornerbacks
double compOneDirBestPlayerPos(const play *p, int bnd[4], const struct rect &range, double boxXLen, double boxYLen, std::vector<player> &players, direction dir);

double compOneDirBestPlayerPos(const play *p, int bnd[4], const struct rect &range, double boxXLen, double boxYLen, player &pler, direction dir);

//compute the score of one position for one player
double compOnePosPlayerScr(const play *p, const struct rect &scanRect, player &pler, direction dir, double dotPro, double dotProPerp, playerBndBox &box);

double compOnePosPlayerScr(const play *p, const struct rect &scanRect, player &pler, direction dir, double dotPro, double dotProPerp, playerBndBox &box, double featureScores[3]);

double compOnePosPlayerScr(const play *p, const struct rect &scanRect, player &pler, direction dir, double dotPro, double dotProPerp, double weight[3], playerBndBox &box, double featureScores[3]);

double compOnePosPlayerScr(const play *p, const struct rect &scanRect, player &pler, direction dir, cv::Point2d vecScrimCntToBoxCnt, double angScrimCntToBoxCnt, double weight[4], playerBndBox &box, double featureScores[4]);

//for top boxes method
double compOnePosPlayerScr(const play *p, player &pler, direction dir, double dotPro, double dotProPerp, const playerBndBox &box);

//compute the formation of left/right model with a tree model which has an edge between low/up receiver and low/up cornerback
double compOneDirBestPlayerPos2(const play *p, int bnd[4], const struct rect &range, double boxXLen, double boxYLen, std::vector<player> &players, direction dir);

double compOneDirBestPlayerPos2(const play *p, const std::vector<playerBndBox> &topBoxes, std::vector<player> &players, direction dir);

//return the starting postion of the player's feature elements in the total feature vector.
int playerStartPosInFVec(const playerType &pType, int fNumEachPlayer);


#endif
