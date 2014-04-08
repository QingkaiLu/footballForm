/*
 * perceptron.h
 *
 *  Created on: 2013-11-11
 *      Author: qingkai
 */

#ifndef PERCEPTRON_H_
#define PERCEPTRON_H_

#include "commonStructs.h"
#include "player.h"
#include "wrPicStrModel.h"

class play;

//#define featureNum 12;
//#define playerNum 4;
#define wrLabel 3 //1 => both low and up wrs; 2 => low; 3 => up

class perceptron {
public:
	perceptron();
	perceptron(std::vector<std::string> annotPaths);
	perceptron(std::vector<int> annotGames, const vector<playerType> &pTps, int fNumEachPlayer);
	~perceptron();
	bool readAnnotFile();
	void computeFGt();
	void computeWeight();
	//void test();
	void saveAvgWeight();
	//generate a file containing the training plays and their corresponding wr labels
	void genPlaysLabels();
public:
	//play *p;
	std::vector<double> w;//weight vector
	std::vector<double> wAvg;//average weight vector
	//std::vector<double> fGt;//feature vector of ground truth
//	std::vector<player> players;
//	std::string annotFilePath;
	int featureNum, playerNum;
	//struct playId pId;
	std::vector<struct playId> pIds;
	std::vector<std::vector<double> > fGts;
	//order for annotation files: lowWR, lowCB, upWR, upCB.
	std::vector<std::string> annotFilePaths;
	std::vector<std::vector<player> > formsGt;
	vector<playerType> pTypes;
	// number of players in whole feature vector. dimension of feature vector = playersNumFVec * fNumEachPlayer
	// the feature of players not needed will all be set to zero
	int playersNumFVec;
	std::string avgWgtFilePath;
	std::string playLabelPath;
};

#endif /* PERCEPTRON_H_ */
