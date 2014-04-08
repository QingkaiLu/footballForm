
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <math.h>

#include "play.h"
#include "wrPicStrModel.h"
#include "perceptron.h"

using namespace std;

int cvtmain()
//int picWr()
{
//	string leftAnnot = "wrAnnotation/ResultsTrueMos/Game02/leftNew/left.annotation";
//	string rightAnnot = "wrAnnotation/ResultsTrueMos/Game02/rightNew/right.annotation";
//	wrPicStrModel leftModel(leftDir, leftAnnot);
//	leftModel.constructModel();
//
//	wrPicStrModel rightModel(rightDir, rightAnnot);
//	rightModel.constructModel();

	//cout << "NEGINF: " << -1.0 * INF << endl;
	vector<int> annotGames;
	annotGames.push_back(2);
	annotGames.push_back(3);
//	perceptron percep(annotGames);
//	percep.computeWeight();


	unsigned int gamesVidsNum[10] = {132, 141, 173, 149, 135, 156, 152, 159, 128, 146};
	ofstream fAllGamesAcc("overallAcc");

	vector<playerType> pTypes;
	pTypes.push_back(lowWR);
	pTypes.push_back(lowCB);
	pTypes.push_back(upWR);
	pTypes.push_back(upCB);

//	double weight[12] = {85.8059, 0.108555, -1.79343, 78.2491, 9.93621, 8.85802, 94.3081, 18.3604, 4.45693, 98.8027, -0.504632, -1.68638};
//	vector<double> w(12, 0.0);
//	for(int i = 0; i < 12; ++i)
//		//w[i] = 1.0;
//		w[i] = weight[i];

	for(unsigned int i = 2; i < 3; ++i)
	{
		ostringstream convertGameId;
		convertGameId << i;
		string gameId = convertGameId.str();

		if(i< 10)
			gameId = "0" + gameId;

		string preDirPath = "predDir/Game" + gameId + "/preDir";
		ofstream fPreDirOut(preDirPath.c_str());

		for(unsigned int j = 1; j <= gamesVidsNum[i - 2]; ++j)
		//for(unsigned int j = 3; j <= 3; ++j)
		{
			struct playId pId;
			pId.gameId = i;
			pId.vidId = j;
			play p(pId);
			p.setUp();
			p.cvtFgImgFrmBmpToJpg();

			cout << "gameId: " << pId.gameId << " vidId: " << pId.vidId << endl;
		}
	}


	return 1;
}

