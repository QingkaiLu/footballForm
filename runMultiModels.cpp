#include <fstream>
#include <iostream>
#include <vector>
#include <string>

#include "play.h"
#include "formation.h"
#include "runMultiModels.h"

using namespace std;

//#if wrLabel == 1
//		string annotPath = "multiModelsTrainData/Game" + gameId + "/twoWrs/twoWrs.annotation";
//		avgWgtFilePath = "Results/Game" + gameId + "/twoWrs/twoWrs.model";
//		playLabelPath = "Results/Game" + gameId + "/twoWrs/twoWrs.labels";
//#elif wrLabel == 2
//		string annotPath = "multiModelsTrainData/Game" + gameId + "/lowWr/lowWr.annotation";
//		avgWgtFilePath = "Results/Game" + gameId + "/lowWr/lowWr.model";
//		playLabelPath = "Results/Game" + gameId + "/lowWr/lowWr.labels";
//#elif wrLabel == 3
//		string annotPath = "multiModelsTrainData/Game" + gameId + "/upWr/upWr.annotation";
//		avgWgtFilePath = "Results/Game" + gameId + "/upWr/upWr.model";
//		playLabelPath = "Results/Game" + gameId + "/upWr/upWr.labels";
//#endif


void setUpMultiModes(vector<string> &modelWtFilePaths, vector<string> &playLabelFilePaths, vector< vector<playerType> > &pTypesOfAllClasses)
{
	// it would be good to read these set up info below from a file. Do this later.
	vector<playerType> pTypes;
	pTypes.push_back(lowWR);
	pTypes.push_back(lowCB);
	pTypes.push_back(upWR);
	pTypes.push_back(upCB);
	pTypesOfAllClasses.push_back(pTypes);

	string WgtFilePath = "Results/Game08/twoWrs/twoWrs.model";
	modelWtFilePaths.push_back(WgtFilePath);

	string playLabelPath = "Results/Game08/twoWrs/twoWrs.labels";
	playLabelFilePaths.push_back(playLabelPath);

	vector<playerType> pTypes2;
	pTypes2.push_back(lowWR);
	pTypes2.push_back(lowCB);;
	pTypesOfAllClasses.push_back(pTypes2);

	string WgtFilePath2 = "Results/Game08/lowWr/lowWr.model";
	modelWtFilePaths.push_back(WgtFilePath2);

	string playLabelPath2 = "Results/Game08/lowWr/lowWr.labels";
	playLabelFilePaths.push_back(playLabelPath2);

	vector<playerType> pTypes3;
	pTypes3.push_back(upWR);
	pTypes3.push_back(upCB);;
	pTypesOfAllClasses.push_back(pTypes3);

	string WgtFilePath3 = "Results/Game08/upWr/upWr.model";
	modelWtFilePaths.push_back(WgtFilePath3);

	string playLabelPath3 = "Results/Game08/upWr/upWr.labels";
	playLabelFilePaths.push_back(playLabelPath3);

}

void readModelWeight(const vector<string> &modelWtFilePaths, vector< vector<double> > &modelWt)
{
	for(unsigned int i = 0; i < modelWtFilePaths.size(); ++i)
	{
		cout << "reading " << modelWtFilePaths[i] << endl;
		ifstream fin(modelWtFilePaths[i].c_str());
		if(!fin.is_open())
		{
			cout << "Can't open file " << modelWtFilePaths[i] << endl;
			return;
		}

		fin.seekg(0, ios::end);
		if (fin.tellg() == 0) {
			cout<<"Empty file " << modelWtFilePaths[i] << endl;
			return;
		}
		fin.seekg(0, ios::beg);

		vector<double> wtOnePlay;
		double w = NEGINF;
		for(int j = 0; j < featureNumEachPlay; ++j)
		{
			fin >> w;
			cout << w << " ";
			wtOnePlay.push_back(w);
		}
		cout << endl;

		modelWt.push_back(wtOnePlay);

		fin.close();
	}
	cout << "modelWt.size() " << modelWt.size() << endl;
}

void readPlayLabel(const vector<string> &playLabelFilePaths, vector<playId> &pIds, vector<int> &pLabels)
{
	for(unsigned int i = 0; i < playLabelFilePaths.size(); ++i)
	{
		cout << "reading " << playLabelFilePaths[i] << endl;
		ifstream fin(playLabelFilePaths[i].c_str());
		if(!fin.is_open())
		{
			cout << "Can't open file " << playLabelFilePaths[i] << endl;
			return;
		}

		fin.seekg(0, ios::end);
		if (fin.tellg() == 0) {
			cout<<"Empty file " << playLabelFilePaths[i] << endl;
			return;
		}
		fin.seekg(0, ios::beg);

		while(!fin.eof())
		{
			playId pId;
			int pLabel;
			pId.gameId = -1;
			fin >> pId.gameId >> pId.vidId >> pLabel;
			if(pId.gameId == -1)//file ends
				break;
			cout << pId.gameId << " " << pId.vidId  << " " << pLabel;
			cout << endl;
			pIds.push_back(pId);
			pLabels.push_back(pLabel);
		}
		cout << endl;


		fin.close();
	}
	cout << "pIds.size() " << pIds.size() << endl;
	cout << "pLabels.size() " << pLabels.size() << endl;

}

void runMultiModels()
{
	vector<string> modelWtFilePaths, playLabelFilePaths;
	vector< vector<playerType> > pTypesOfAllClasses;

	setUpMultiModes(modelWtFilePaths, playLabelFilePaths, pTypesOfAllClasses);

	vector< vector<double> > modelWt;
	readModelWeight(modelWtFilePaths, modelWt);

	vector<playId> pIds;
	vector<int> pLabels;
	readPlayLabel(playLabelFilePaths, pIds, pLabels);

	cout << "Running multiModels... " << endl;

	vector< vector<double> > fVecsAllPlay;
	//compute feature vector for each play with all models
	for(unsigned int i = 0; i < pIds.size(); ++i)
	{
		vector<double> fVecOnePlay;
		play *p = NULL;
//		play p(pIds[i]);
		cout << "gameId: " << pIds[i].gameId << " vidId: " << pIds[i].vidId << endl;
//		p.setUp();
		for(unsigned int j = 0; j < pTypesOfAllClasses.size(); ++j)
		{
			if (p != NULL)
				delete p;
			p = new play(pIds[i]);
			p->setUp();
			p->computeFormDir(pTypesOfAllClasses[j], modelWt[j], featureNumEachPlay, (j + 1));
			fVecOnePlay.push_back(p->form->totalScore);//model score
			fVecOnePlay.push_back(1.0);//bias
		}

		fVecsAllPlay.push_back(fVecOnePlay);

	}

	string fVecsFilePath = "Results/Game08/featureVecs";

	ofstream fout(fVecsFilePath.c_str());
	for(unsigned int i = 0; i < fVecsAllPlay.size(); ++i)
	{
		fout << pLabels[i] << " ";
		for(unsigned int j = 0; j < fVecsAllPlay[i].size(); ++j)
			fout << (j + 1) << ":" << fVecsAllPlay[i][j] << " ";
		fout << "\n";
	}
	fout.close();

}

//int main()
//{
//	runMultiModels();
//	return 1;
//}
