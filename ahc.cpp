#include "stdafx.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <vector>
#include <fstream>
#include "dataanalysis.h"
#include "ahc.h"
#include "extractOdVidFeats.h"
#include "kernelSvm.h"
#include "play.h"

using namespace std;
using namespace alglib;

void computeOffFeatsNoExp(const vector<int> &games, vector<vector<double> > &offFeatsVec)
{

	for(unsigned int g = 0; g < games.size(); ++g)
	{
		vector<string> leftFeatFiles, rightFeatFiles, losCntFileNames;
		int gameId = games[g];
		ostringstream convertGameId;
		convertGameId << gameId ;
		string gameIdStr = convertGameId.str();

		if(gameId < 10)
			gameIdStr = "0" + gameIdStr;

		cout << "Computing offense feature vectors of Game" << gameIdStr
				<< " without expectation compensation..."<< endl;

		//string featuresFile = "randTreesTrainData/featureVecsGame" + gameIdStr;
		string leftFeatFile = "randTreesTrainData/features/featsLeftRectGame" + gameIdStr;
		leftFeatFiles.push_back(leftFeatFile);
		string rightFeatFile = "randTreesTrainData/features/featsRightRectGame" + gameIdStr;
		rightFeatFiles.push_back(rightFeatFile);

		vector<vector<double> > fVecPlaysLeft, fVecPlaysRight;
		vector<double> leftLabels, rightLabels;

		readOdFeatData(leftFeatFiles, fVecPlaysLeft, leftLabels, fNumPerPlayOneSide);
		readOdFeatData(rightFeatFiles, fVecPlaysRight, rightLabels, fNumPerPlayOneSide);

		string featuresFile = "randTreesTrainData/features/offFeatsGame" + gameIdStr + "Rect";
		ofstream fout(featuresFile.c_str());

		for(unsigned int i = 0; i < fVecPlaysLeft.size(); ++i)
		{
			vector<double> fVecOnePlayLeft = fVecPlaysLeft[i];
			vector<double> fVecOnePlayRight = fVecPlaysRight[i];
			vector<double> fVecOnePlay;
			if((char)leftLabels[i] == 'o')
			{
				for(unsigned int j = 0; j < fVecOnePlayLeft.size(); ++j)
						fVecOnePlay.push_back(fVecOnePlayLeft[j]);
			}
			else if((char)leftLabels[i] == 'd')
			{
				for(unsigned int j = 0; j < fVecOnePlayRight.size(); ++j)
						fVecOnePlay.push_back(fVecOnePlayRight[j]);
			}
			else
			{
				cout << "Wrong od label." << endl;
				return;
			}
			offFeatsVec.push_back(fVecOnePlay);

			fout << (char)leftLabels[i] << ",";

			double maxFVal = .0;
			for(unsigned int j = 0; j < fVecOnePlay.size(); ++j)
					maxFVal += fVecOnePlay[j];

			if(maxFVal != 0)
			{
				for(unsigned int j = 0; j < fVecOnePlay.size(); ++j)
					fout << fVecOnePlay[j] / maxFVal << ",";
			}
			else
			{
				for(unsigned int j = 0; j < fVecOnePlay.size(); ++j)
					fout << fVecOnePlay[j] << ",";
			}
			fout << endl;
		}
		fout.close();
	}

}


void computeOffSpPmdPKernel(const vector<int> &games,int& playsNum)
{
	cout << "Computing offense spatial pyramid feature kernel..."<< endl;

//	vector<playId> pIds;
//	getPlayIds(games, pIds);

	vector<vector<double> > fVecPlaysOffAllGames;
	vector<double> offLabelsAllGames;//, rightLabelsAllGames;
	for(unsigned int g = 0; g < games.size(); ++g)
	{
		vector<string> offFeatFiles;
		int gameId = games[g];
		ostringstream convertGameId;
		convertGameId << gameId ;
		string gameIdStr = convertGameId.str();

		if(gameId < 10)
			gameIdStr = "0" + gameIdStr;

		string offFeatFile = "randTreesTrainData/features/offFeatsGame" + gameIdStr + "Rect";
		offFeatFiles.push_back(offFeatFile);

		vector<vector<double> > fVecPlayOff;
		vector<double> offLabels;

		readOdFeatData(offFeatFiles, fVecPlayOff, offLabels, fNumPerPlayOneSide);

		for(unsigned int i = 0; i < fVecPlayOff.size(); ++i)
			fVecPlaysOffAllGames.push_back(fVecPlayOff[i]);
		for(unsigned int i = 0; i < offLabels.size(); ++i)
			offLabelsAllGames.push_back(offLabels[i]);
	}

	vector<vector<double> > kernels;
	for(unsigned int i = 0; i < fVecPlaysOffAllGames.size(); ++i)
	{
		vector<double> kerVec;
		for(unsigned int j = 0; j < fVecPlaysOffAllGames.size(); ++j)
		{
			if(j >= i)
			{
				double k = computeTwoFVecKernel(fVecPlaysOffAllGames[i], fVecPlaysOffAllGames[j]);
				kerVec.push_back(k);
			}
			else
				kerVec.push_back(kernels[j][i]);
		}
		kernels.push_back(kerVec);
	}

	string kernelPath = "randTreesTrainData/features/ahcSPDistMat";
	ofstream fout(kernelPath.c_str());
	for(unsigned int i = 0; i < fVecPlaysOffAllGames.size(); ++i)
	{
		//fout << (char)offLabelsAllGames[i] << ",";
		for(unsigned int j = 0; j < fVecPlaysOffAllGames.size(); ++j)
			fout << kernels[i][j] << ",";
		fout << endl;
	}

	fout.close();
//	cout << fVecPlaysOffAllGames.size() << endl;
	playsNum = fVecPlaysOffAllGames.size();
}

bool readSpKernelAhc(string fileName,
		vector<vector<double> > &kernels, int playsNum)
{
	cout << "reading " << fileName << endl;
	ifstream fin(fileName.c_str());
	if(!fin.is_open())
	{
		cout << "Can't open file " << fileName << endl;
		return false;
	}

	fin.seekg(0, ios::end);
	if (fin.tellg() == 0) {
		cout << "Empty file " << fileName << endl;
		return false;
	}
	fin.seekg(0, ios::beg);
	while(!fin.eof())
	{
		char comma;

		vector<double> kernelsOneSample;
		bool fileEnd = false;
		for(int j = 0; j < playsNum; ++j)
		{
			double ker = NEGINF;
			fin >> ker >> comma;
			kernelsOneSample.push_back(ker);
			if(ker == NEGINF)
			{
				fileEnd = true;
				break;
			}
		}

		if(fileEnd)
			break;
		kernels.push_back(kernelsOneSample);

	}
	fin.close();

	return true;
}

//int main()
int ahc()
{
	vector<int> games;
	games.push_back(2);
//	games.push_back(8);
//	games.push_back(9);
//	games.push_back(10);
	vector<vector<double> > offFeatsVec;
	computeOffFeatsNoExp(games, offFeatsVec);
	int playsNum = 0;
	computeOffSpPmdPKernel(games, playsNum);
	vector<vector<double> > kernels;
	string kernelPath = "randTreesTrainData/features/ahcSPDistMat";
	readSpKernelAhc(kernelPath, kernels, playsNum);

    real_2d_array featuresMat;
    featuresMat.setlength(offFeatsVec.size(), offFeatsVec[0].size());
    for(unsigned int i = 0; i < offFeatsVec.size(); ++i)
    	for(unsigned int j = 0; j < offFeatsVec[i].size(); ++j)
    		featuresMat[i][j] = offFeatsVec[i][j];
    int numClsts = 10;

    clusterizerstate s;
    ahcreport rep;
    clusterizercreate(s);
    clusterizersetahcalgo(s, 1);

//    //Euclidean distance
//    ae_int_t disttype = 2;
//    clusterizersetpoints(s, featuresMat, disttype);

    real_2d_array d;
    d.setlength(playsNum, playsNum);
    double ratio = 1.0; // playsNum;
    for(unsigned int i = 0; i < kernels.size(); ++i)
    	for(unsigned int j = 0; j < kernels.size(); ++j)
    		d[i][j] = ratio - kernels[i][j];
    clusterizersetdistances(s, d, true);


    clusterizerrunahc(s, rep);
    printf("%s\n", rep.z.tostring().c_str());
    integer_1d_array cidx;
    integer_1d_array cz;
    clusterizergetkclusters(rep, numClsts, cidx, cz);
//    printf("%s\n", cidx.tostring().c_str());
    cout << cidx.tostring() << endl;

//    clusterizerstate s;
//    kmeansreport kMeansRep;
//
//    clusterizercreate(s);
//    clusterizersetpoints(s, featuresMat, 2);
//    clusterizersetkmeanslimits(s, 5, 0);
//    clusterizerrunkmeans(s, numClsts, kMeansRep);
//    integer_1d_array cidx = kMeansRep.cidx;

	vector<playId> pIdsGames;

	for(unsigned int i = 0; i < games.size(); ++i)
	{
		vector<playId> pIds;
		getPlayIds(games[i], pIds);
		for(unsigned int j = 0; j < pIds.size(); ++j)
		pIdsGames.push_back(pIds[j]);
	}

	string clstFile = "playsClusters/clusters";
	ofstream fout(clstFile.c_str());
	for(unsigned int i = 0; i < pIdsGames.size(); ++i)
	{
		play p(pIdsGames[i]);
		fout << pIdsGames[i].gameId << " " << pIdsGames[i].vidId << " ";
		fout << cidx[i] << endl;
		p.saveMosFrmToClst(cidx[i]);
	}
	fout.close();
	return 0;
}

int testMain(int argc, char **argv)
{
    //
    // We have three points in 4D space:
    //     (P0,P1,P2) = ((1, 2, 1, 2), (6, 7, 6, 7), (7, 6, 7, 6))
    //
    // We want to try clustering them with different distance functions.
    // Distance function is chosen when we add dataset to the clusterizer.
    // We can choose several distance types - Euclidean, city block, Chebyshev,
    // several correlation measures or user-supplied distance matrix.
    //
    // Here we'll try three distances: Euclidean, Pearson correlation,
    // user-supplied distance matrix. Different distance functions lead
    // to different choices being made by algorithm during clustering.
    //
    clusterizerstate s;
    ahcreport rep;
    ae_int_t disttype;
    real_2d_array xy = "[[1, 2, 1, 2], [6, 7, 6, 7], [7, 6, 7, 6]]";
    clusterizercreate(s);

    // With Euclidean distance function (disttype=2) two closest points
    // are P1 and P2, thus:
    // * first, we merge P1 and P2 to form C3=[P1,P2]
    // * second, we merge P0 and C3 to form C4=[P0,P1,P2]
    disttype = 2;
    clusterizersetpoints(s, xy, disttype);
    clusterizerrunahc(s, rep);
    printf("%s\n", rep.z.tostring().c_str()); // EXPECTED: [[1,2],[0,3]]

    // With Pearson correlation distance function (disttype=10) situation
    // is different - distance between P0 and P1 is zero, thus:
    // * first, we merge P0 and P1 to form C3=[P0,P1]
    // * second, we merge P2 and C3 to form C4=[P0,P1,P2]
    disttype = 10;
    clusterizersetpoints(s, xy, disttype);
    clusterizerrunahc(s, rep);
    printf("%s\n", rep.z.tostring().c_str()); // EXPECTED: [[0,1],[2,3]]

    // Finally, we try clustering with user-supplied distance matrix:
    //     [ 0 3 1 ]
    // P = [ 3 0 3 ], where P[i,j] = dist(Pi,Pj)
    //     [ 1 3 0 ]
    //
    // * first, we merge P0 and P2 to form C3=[P0,P2]
    // * second, we merge P1 and C3 to form C4=[P0,P1,P2]
    real_2d_array d = "[[0,3,1],[3,0,3],[1,3,0]]";
    clusterizersetdistances(s, d, true);
    clusterizerrunahc(s, rep);
    printf("%s\n", rep.z.tostring().c_str()); // EXPECTED: [[0,2],[1,3]]

    integer_1d_array cidx;
    integer_1d_array cz;

//    clusterizercreate(s);
//    clusterizersetpoints(s, xy, 2);
//    clusterizerrunahc(s, rep);

    clusterizergetkclusters(rep, 2, cidx, cz);
    printf("%s\n", cidx.tostring().c_str());
    cout << cidx[1] << endl;


//    alglib::real_1d_array r1;
//    r1.setlength(2);
//    r1[1] = 100;
//    cout << r1[1] << endl;
//
////    string tmp = "[[2,3],[3,4]]";
//    integer_2d_array r2("[[2,3],[3,4]]");
//    //cout << r2.tostring() << endl;
//    printf("%s\n", r2.tostring().c_str());

    return 0;
}
