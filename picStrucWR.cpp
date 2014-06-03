
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <math.h>
#include <fstream>

#include "play.h"
//#include "wrPicStrModel.h"
#include "perceptron.h"

using namespace std;

//int main()
int picWr()
{
//	string leftAnnot = "wrAnnotation/ResultsTrueMos/Game02/leftNew/left.annotation";
//	string rightAnnot = "wrAnnotation/ResultsTrueMos/Game02/rightNew/right.annotation";
//	wrPicStrModel leftModel(leftDir, leftAnnot);
//	leftModel.constructModel();
//
//	wrPicStrModel rightModel(rightDir, rightAnnot);
//	rightModel.constructModel();

	//cout << "NEGINF: " << -1.0 * INF << endl;

//	vector<playerType> pTypes;
////	pTypes.push_back(lowWR);
////	pTypes.push_back(lowCB);
//	pTypes.push_back(upWR);
//	pTypes.push_back(upCB);
//
//	vector<int> annotGames;
//	annotGames.push_back(8);
//	//annotGames.push_back(3);
//	perceptron percep(annotGames, pTypes, featureNumEachPlayer);
//	percep.computeWeight();


	unsigned int gamesVidsNum[10] = {132, 141, 173, 149, 135, 156, 152, 159, 128, 146};
	ofstream fAllGamesAcc("overallAcc");
	int totalErrNum = 0, totalCrtNum = 0;

//	double weight[12] = {85.8059, 0.108555, -1.79343, 78.2491, 9.93621, 8.85802, 94.3081, 18.3604, 4.45693, 98.8027, -0.504632, -1.68638};
//	vector<double> w(12, 0.0);
//	for(int i = 0; i < 12; ++i)
//		//w[i] = 1.0;
//		w[i] = weight[i];

	for(unsigned int i = 2; i < 11; ++i)
	{
		ostringstream convertGameId;
		convertGameId << i;
		string gameId = convertGameId.str();

		if(i< 10)
			gameId = "0" + gameId;

//		vector<string> dirGt, preDir;
//		vector<double> scores;

//		string dirGtFilePath = "ODKGt/Game" + gameId + "/game" + gameId + "_wr_new";
//
//		ifstream finDirGt(dirGtFilePath.c_str());
//		if(!finDirGt.is_open())
//		{
//			cout<<"Can't open direction GT file!"<<endl;
//			return -1;
//		}
//
//		while(!finDirGt.eof())
//		{
//			string d = "NULL";
//			finDirGt >> d;
//			//cout << "d: " << d << endl;
//			if(d.compare("NULL") != 0)
//			{
//				dirGt.push_back(d);
//				//preDir.push_back("NULL");
//			}
//		}
//
//		finDirGt.close();
		//cout << dirGt.size() << " ";

		string preDirPath = "predDir/Game" + gameId + "/preDir";
		ofstream fPreDirOut(preDirPath.c_str());

		int errNum = 0, crtNum = 0;
		for(unsigned int j = 1; j <= gamesVidsNum[i - 2]; ++j)
		//for(unsigned int j = 0; j < percep.pIds.size(); ++j)
		{
			struct playId pId;
			pId.gameId = i;
			pId.vidId = j;
//			pId.gameId = percep.pIds[j].gameId;
//			pId.vidId = percep.pIds[j].vidId;
			play p(pId);
//			p.computeDir(leftModel, rightModel);
			//p.computeTopBoxesDir(leftModel, rightModel);
			//p.computeLosSideDir();
			//p.computeAllTracksDir();
			//vector<double> f(12, 0.0);
			p.setUp();
			p.computeDirLosArea();
//			p.ellipseWrClassifier();
//			p.computeFormDir(pTypes, percep.wAvg, percep.featureNum);
			//p.saveMosFrm();
//			p.preDir = rightDir;
//			p.totalScore = 1.0;
//			p.getTrueDir();
			cout << "gameId: " << pId.gameId << " vidId: " << pId.vidId << endl;
			if(p.preDir == leftDir)
			{
				cout << "l" << endl;
				//preDir.push_back("l");
				//fPreDirOut << pId.vidId << " l " << p.finalScore << endl;
			}
			else if(p.preDir == rightDir)
			{
				cout << "r" << endl;
				//preDir.push_back("r");
				//fPreDirOut << pId.vidId << " r " << p.finalScore << endl;
			}
			else
			{
				cout << "n" << endl;
				//preDir.push_back("r");
				//fPreDirOut << pId.vidId << " n " << p.finalScore << endl;
			}
			//cout << "score: " << p.finalScore << endl;

			if((p.trueDir == p.preDir) && (p.trueDir != nonDir))
			{
				++crtNum;
				++totalCrtNum;
			}
			else if(p.trueDir != nonDir)
			{
				++errNum;
				++totalErrNum;
			}
			//scores.push_back(p.totalScore);
//			preDir.push_back("r");
//			scores.push_back(1.0);
		}

		fPreDirOut.close();

//		string preDirPath = "predDir/Game" + gameId + "/preDir";
//		ofstream fPreDirOut(preDirPath.c_str());
//		//fPreDirOut << "Hell0" << endl;
//		int errNum = 0, crtNum = 0;
//		if(preDir.size() != dirGt.size())
//		{
//			cout << "preDir size not same with dirGt!" << endl;
//			return -1;
//		}
//		for(unsigned int k = 0; k < preDir.size(); ++k)
//		{
//			fPreDirOut << (k + 1) << " " << preDir[k] << " " << scores[k] << endl;
//			if( (dirGt[k].compare("l") == 0) || (dirGt[k].compare("r") == 0))
//			{
//				if(preDir[k].compare(dirGt[k]) == 0)
//				{
//					++crtNum;
//					++totalCrtNum;
//				}
//				else
//				{
//					++errNum;
//					++totalErrNum;
//				}
//			}
//		}
//		fPreDirOut.close();

		double acc = (crtNum + 0.0) / (errNum + crtNum + 0.0);
		fAllGamesAcc << "game " << i << " Accuracy: " << acc << " (" << crtNum << "/" << (errNum + crtNum) << ")";
		fAllGamesAcc << endl;

	}

	fAllGamesAcc << "Accuracy of all games: " << ( (totalCrtNum + 0.0) / (totalErrNum + totalCrtNum + 0.0) );
	fAllGamesAcc << " (" << totalCrtNum << "/" << (totalErrNum + totalCrtNum) << ")" << endl;

	fAllGamesAcc.close();


	return 1;
}

//int main()
//{
//	unsigned int gamesVidsNum[10] = {132, 141, 173, 149, 135, 156, 152, 159, 128, 146};
//	for(unsigned int i = 2; i < 3; ++i)
//	{
//		ostringstream convertGameId;
//		convertGameId << i ;
//		string gameIdStr = convertGameId.str();
//
//		if(i < 10)
//			gameIdStr = "0" + gameIdStr;
//		string odPIdsPath = "odGame" + gameIdStr;
//		ofstream foutOdPIds(odPIdsPath.c_str());
//		for(unsigned int j = 1; j <= gamesVidsNum[i - 2]; ++j)
//		{
//			struct playId pId;
//			pId.gameId = i;
//			pId.vidId = j;
//
//			play p(pId);
//			p.setUp();
//			if(p.trueDir != nonDir)
//			{
//				string leftTeam;
//				if(p.trueDir == leftDir)
//					leftTeam = "d";
//				else if(p.trueDir == rightDir)
//					leftTeam = "o";
//				foutOdPIds << p.pId.gameId << " " << p.pId.vidId << " l " << leftTeam << endl;
//			}
//		}
//		foutOdPIds.close();
//	}
//
//
//	return 0;
//}

//int main()
//{
//	struct playId pId;
//	pId.gameId = 9;
//	pId.vidId = 7;
//
//	play p(pId);
//	p.setUp();
//	p.rectification();
////	Mat homoMat;
////	p.getOverheadFieldHomo(homoMat);
//}

//int main()
//{
//	unsigned int gamesVidsNum[10] = {132, 141, 173, 149, 135, 156, 152, 159, 128, 146};
//	for(unsigned int i = 8; i < 9; ++i)
//	{
//		ostringstream convertGameId;
//		convertGameId << i ;
//		string gameIdStr = convertGameId.str();
//
//		if(i < 10)
//			gameIdStr = "0" + gameIdStr;
//		string odPIdsPath = "odGame" + gameIdStr;
////		ofstream foutOdPIds(odPIdsPath.c_str());
////		for(unsigned int j = 1; j <= gamesVidsNum[i - 2]; ++j)
//		for(unsigned int j = 83; j <= 83; ++j)
//		{
//			struct playId pId;
//			pId.gameId = i;
//			pId.vidId = j;
//			cout << i << " " << j << endl;
//
//			play p(pId);
//			p.setUp();
////			p.mos = p.mos - 3;
////			p.getMos();
////			if(i == 28)
////				p.mos = 1;
//			p.saveMosFrm();
//
////			if(p.trueDir != nonDir)
////			{
////				string leftTeam;
////				if(p.trueDir == leftDir)
////					leftTeam = "d";
////				else if(p.trueDir == rightDir)
////					leftTeam = "o";
////				foutOdPIds << p.pId.gameId << " " << p.pId.vidId << " l " << leftTeam << endl;
////			}
//
//		}
////		foutOdPIds.close();
//	}
//
//
//	return 0;
//}

