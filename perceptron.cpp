/*
 * perceptron.cpp
 *
 *  Created on: 2013-11-11
 *      Author: qingkai
 */
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cv.h>
#include <highgui.h>
#include <math.h>

#include "commonStructs.h"
#include "perceptron.h"
#include "play.h"
#include "player.h"
#include "playerBndBox.h"
#include "formation.h"

using namespace std;
using namespace cv;

//#define featureNum 12;


perceptron::perceptron() {
	// TODO Auto-generated constructor stub
	playersNumFVec = 4;
	playerNum = 4;
	featureNum = 16;
	vector<double> initVec(featureNum, 0.0);
	//fGt = initVec;
	w = initVec;
	wAvg = initVec;
	//p = NULL;
}

perceptron::~perceptron() {
	// TODO Auto-generated destructor stub
//	if(p != NULL)
//		delete p;
}

//perceptron::perceptron(string annotPath)
//{
//	annotFilePath = annotPath;
//	playerNum = 4;
//	featureNum = 12;
//	vector<double> initVec(featureNum, 0.0);
//	fGt = initVec;
//	w = initVec;
//	wAvg = initVec;
//	//p = NULL;
//}

perceptron::perceptron(vector<string> annotPaths)
{
	playersNumFVec = 4;
	annotFilePaths = annotPaths;
	playerNum = 4;
	featureNum = 16;
	vector<double> initVec(featureNum, 0.0);
	//fGt = initVec;
	w = initVec;
	wAvg = initVec;
	//p = NULL;
}

perceptron::perceptron(vector<int> annotGames, const vector<playerType> &pTps, int fNumEachPlayer)
{
	pTypes = pTps;
	playerNum = pTypes.size();
	playersNumFVec = 4;
	//playerNum = 4;
	featureNum = fNumEachPlayer * playersNumFVec; // 16
	vector<double> initVec(featureNum, 0.0);
	//fGt = initVec;
	w = initVec;
	wAvg = initVec;
	//p = NULL;
	for(unsigned int i = 0; i < annotGames.size(); ++i)
	{
		ostringstream convertGameId;
		convertGameId << annotGames[i];
		string gameId = convertGameId.str();

		if(i< 10)
			gameId = "0" + gameId;
		//string annotPath = "percerpTrainData/Game" + gameId + "/annotation.annotation";
		///home/qingkai/workspace/picStrucWR/Results/Game08/upWr/000032.jpg
#if wrLabel == 1
		string annotPath = "multiModelsTrainData/Game" + gameId + "/twoWrs/twoWrs.annotation";
		avgWgtFilePath = "Results/Game" + gameId + "/twoWrs/twoWrs.model";
		playLabelPath = "Results/Game" + gameId + "/twoWrs/twoWrs.labels";
#elif wrLabel == 2
		string annotPath = "multiModelsTrainData/Game" + gameId + "/lowWr/lowWr.annotation";
		avgWgtFilePath = "Results/Game" + gameId + "/lowWr/lowWr.model";
		playLabelPath = "Results/Game" + gameId + "/lowWr/lowWr.labels";
#elif wrLabel == 3
		string annotPath = "multiModelsTrainData/Game" + gameId + "/upWr/upWr.annotation";
		avgWgtFilePath = "Results/Game" + gameId + "/upWr/upWr.model";
		playLabelPath = "Results/Game" + gameId + "/upWr/upWr.labels";
#endif
		annotFilePaths.push_back(annotPath);
	}
}
bool perceptron::readAnnotFile()
{
	for(unsigned int i = 0; i < annotFilePaths.size(); ++i)
	{
		cout << "reading " << annotFilePaths[i] << endl;
		ifstream fin(annotFilePaths[i].c_str());
		if(!fin.is_open())
		{
			cout << "Can't open file " << annotFilePaths[i] << endl;
			return false;
		}

		fin.seekg(0, ios::end);
		if (fin.tellg() == 0) {
			cout<<"Empty file"<<annotFilePaths[i]<<endl;
			return false;
		}
		fin.seekg(0, ios::beg);

	//	Mat image;
	//	image.create(imgYLen, imgXLen, CV_32FC3 );
	////	image = cvCreateImage(cvSize(852,480),IPL_DEPTH_32F,3);
	//	image = cv::Scalar(0,0,0);
	//	string plotPath = "Results/vid66.jpg";
		while(!fin.eof())
		{
			string tmp;
			//for(int i = 0; i < 5; ++i)
			fin >> tmp;
			//cout <<tmp<<endl;
			if(tmp.empty())
			{
				break;
			}
			fin >> tmp >> tmp >> tmp >> tmp;
			//cout <<tmp<<endl;
			string imgPath;
			fin >> imgPath;
			//cout << imgPath << endl;
			playId pId = parseImgPath(imgPath);
			pIds.push_back(pId);
			cout << pId.gameId << " " << pId.vidId << endl;
			play p(pId);
			p.setUp();
			//pIds.push_back(pId);
			double boxXLen = boxXLenToYardLns;
			boxXLen *= p.yardLnsDist;
			double boxYLen = boxYLenToYardLns;
			boxYLen *= p.yardLnsDist;
//			vector<playerType> pTypes;
//			pTypes.push_back(lowWR);
//			pTypes.push_back(lowCB);
//			pTypes.push_back(upWR);
//			pTypes.push_back(upCB);
			vector<player> players;
			for(int i = 0; i < playerNum; ++i)
			{
				fin >> tmp >> tmp >> tmp;
				struct bndBox tmpRecBox;
				char comma;
				fin >> tmpRecBox.leftUpVert.x >> comma >> tmpRecBox.leftUpVert.y >> comma;
				fin >> tmpRecBox.xLength >> comma >> tmpRecBox.yLength;
//				if(players[1].pBox->bndRect.a.x > players[0].pBox->bndRect.a.x)
//					dir = rightDir;
//				else
//					dir = leftDir;
	//			cout << tmpRecBox.leftUpVert.x << comma << tmpRecBox.leftUpVert.y << comma;
	//			cout << tmpRecBox.xLength << comma << tmpRecBox.yLength << endl;
				Point2d boxCnt = Point2d(tmpRecBox.leftUpVert.x + tmpRecBox.xLength / 2.0, tmpRecBox.leftUpVert.y + tmpRecBox.yLength / 2.0);
				player pler(pTypes[i]);
				struct rect tmpRect;
				tmpRect.a = boxCnt + Point2d(-1.0 * boxXLen / 2.0, -1.0 * boxYLen / 2.0);
				tmpRect.b = boxCnt + Point2d(-1.0 * boxXLen / 2.0, 1.0 * boxYLen / 2.0);
				tmpRect.c = boxCnt + Point2d(1.0 * boxXLen / 2.0, 1.0 * boxYLen / 2.0);
				tmpRect.d = boxCnt + Point2d(1.0 * boxXLen / 2.0, -1.0 * boxYLen / 2.0);
				pler.pBox->bndRect = tmpRect;
				players.push_back(pler);
				//plotRect(image, tmpRect);

			}
			formsGt.push_back(players);
		}

	//imwrite(plotPath, image);

	fin.close();

	}

	return true;
}

void perceptron::computeFGt()
{
	cout << "Computing ground truth feature vectors..." << endl;
//	ofstream fout("percerpTrainData/fGts");
	ofstream fout("multiModelsTrainProcess/fGts");
	for(unsigned int j = 0; j < formsGt.size(); ++j)
	{
		//p->setUp();
		play p(pIds[j]);
		p.setUp();
		Point2d vecVpToScrimCnt;
		if(p.vpExist)
		{
			vecVpToScrimCnt = p.scrimCnt - p.vp;
			vecVpToScrimCnt *= 1.0 / norm(vecVpToScrimCnt);
		}
		else
		{
			vecVpToScrimCnt = Point2d(.0, 1.0);
		}

		Point2d perpVecVpToScrimCnt = Point2d(-1.0 * vecVpToScrimCnt.y, vecVpToScrimCnt.x);
		if(perpVecVpToScrimCnt.x < 0.0)
			perpVecVpToScrimCnt = -1.0 * perpVecVpToScrimCnt;

		vector<player> players = formsGt[j];
		direction dir = nonDir;
		if(players[1].pBox->bndRect.a.x > players[0].pBox->bndRect.a.x)
			dir = rightDir;
		else
			dir = leftDir;

	//	if(dir == rightDir)
	//		cout << "right" << endl;
		vector<double> fGt(featureNum, 0.0);
		for(unsigned int i = 0; i < players.size(); ++i)
		{
			//cout << "i " << i << endl;
			struct rect scanRect = players[i].pBox->bndRect;
			scanRect.trksNum = 0;

			Point2d boxCnt = (scanRect.a + scanRect.b + scanRect.c + scanRect.d) * 0.25;
			Point2d vecScrimCntToBoxCnt = boxCnt - p.scrimCnt;
			double dotPro = vecScrimCntToBoxCnt.dot(vecVpToScrimCnt);
			double dotProPerp = vecScrimCntToBoxCnt.dot(perpVecVpToScrimCnt);
			double angScrimCntToBoxCnt = atan(dotProPerp / dotPro);
			double feature[4] = {0.0, 0.0, 0.0, 0.0};
			double weight[4] = {1.0, 1.0, 1.0, 1.0};

			compOnePosPlayerScr(&p, scanRect, players[i], dir, vecScrimCntToBoxCnt, angScrimCntToBoxCnt, weight, *players[i].pBox, feature);

			//double tmpFeature[4], tmpWeight[4];
//			int t = -1;
//			switch (players[i].pType) {
//			case lowWR:
//				t = 0;
//				break;
//			case lowCB:
//				t = 1;
//				break;
//			case upWR:
//				t = 2;
//				break;
//			case upCB:
//				t = 3;
//				break;
//			default:
//				t = -1;
//				break;
//
//			}

			int t = playerStartPosInFVec(players[i].pType, featureNumEachPlayer);

			fGt[t] = feature[0];
			fGt[t + 1] = feature[1];
			fGt[t + 2] = feature[2];
			fGt[t + 3] = feature[3];
		}

		fGts.push_back(fGt);

		fout << "Game: " << pIds[j].gameId << " Video: " << pIds[j].vidId << endl;
		for(unsigned int i = 0; i < fGt.size(); ++i)
			fout << fGt[i] << " ";
		fout << endl;
		fout << "########################################" << endl;
	}
	fout.close();

}

void perceptron::computeWeight()
{
	readAnnotFile();

	computeFGt();

	cout << "Learning weight..." << endl;
	double wUpdate = 1.0;

//	ofstream fout("percerpTrainData/iteration");
	ofstream fout("multiModelsTrainProcess/iteration");

	int iterateIdx = 0, itTimes = 20;
	double overlapThresh = 0.5;

	//for(int x = 0; x < itTimes; ++x)
	while(iterateIdx < itTimes)
	{
		++iterateIdx;
		fout << "iterateIdx: " << iterateIdx << endl;
		cout << "iterateIdx: " << iterateIdx << endl;
		vector<double> wU(featureNum, 0.0);
		wUpdate = 0.0;
		clock_t t;
		t = clock();
		for(unsigned int j = 0; j < pIds.size(); ++j)
		//for(unsigned int j = 0; j < 4; ++j)
		{
			play p(pIds[j]);
			p.setUp();

			vector<double> f(featureNum, 0.0);
			p.computeFormDir(pTypes, iterateIdx, w, f, formsGt[j]);

			fout << "Game: " << pIds[j].gameId << " Video: " << pIds[j].vidId << "   features with current w " << endl;
			for(int i = 0; i < playerNum; ++i)
				if(!formsGt[j][i].pBox->intersectAnotherBndBox(*p.form->players[i].pBox, overlapThresh))
				{
					int t = playerStartPosInFVec(formsGt[j][i].pType, featureNumEachPlayer);

					for(int k = 0; k < featureNumEachPlayer; ++k)
						wU[t + k] += fGts[j][t + k] - f[t + k];
				}
				else
				{
					cout << "Game: " << pIds[j].gameId << " Video: " << pIds[j].vidId << " player " << i << endl;
					cout << "Not update" << endl;
				}

			for(int i = 0; i < featureNum; ++i)
			{
//				wU[i] += fGts[j][i] - f[i];
				fout << f[i] << " ";
			}
			fout << endl;


		}

		fout << "current weight: " << endl;
		double alpha = pIds.size() + 0.0;
		for(int i = 0; i < featureNum; ++i)
		{
			//wU[i] /= (featureNum + 0.0);
			wU[i] /= alpha;
			w[i] += wU[i];
			wUpdate += wU[i] * wU[i];
			wAvg[i] += w[i];
			fout << w[i] << " ";
		}
		fout << endl;
		wUpdate = sqrt(wUpdate);
		fout << "wUpdate: " << wUpdate << endl;
		t = clock() - t;
		fout << "time: " << ((float)t)/CLOCKS_PER_SEC << endl;

		fout << "****************************************" << endl;

	}

	fout << "************************************************" << endl;
	fout << "average weight: " << endl;
	for(int i = 0; i < featureNum; ++i)
	{
		wAvg[i] /= (itTimes + 0.0);
		fout << wAvg[i] << " ";
	}
	fout << endl;
	fout.close();

	saveAvgWeight();
	genPlaysLabels();

}

//void perceptron::test()
//{
//	vector<playerType> pTypes;
//	pTypes.push_back(lowWR);
//	pTypes.push_back(lowCB);
//	pTypes.push_back(upWR);
//	pTypes.push_back(upCB);
//
//	if(p != NULL)
//		delete p;
//	p = new play(pId);
//	p->setUp();
//
//	vector<double> f(featureNum, 0.0);
//
//	p->computeFormDir(pTypes, -1, wAvg, f);
//}

void perceptron::saveAvgWeight()
{
	ofstream fout(avgWgtFilePath.c_str());

	for(int i = 0; i < featureNum; ++i)
		fout << wAvg[i] << " ";

	fout.close();

}

void perceptron::genPlaysLabels()
{
	ofstream fout(playLabelPath.c_str());
	for(unsigned int i = 0; i < pIds.size(); ++i)
		fout << pIds[i].gameId << " " << pIds[i].vidId << " " << wrLabel << endl;

	fout.close();
}
