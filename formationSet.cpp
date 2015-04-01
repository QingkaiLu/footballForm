#include <vector>
#include <string>
#include <utility>
#include <stdlib.h>
#include <time.h>
#include <unordered_map>
#include <list>
#include "formationSet.h"
#include "playAuxFunc.h"
#include "extractOdVidFeats.h"
#include "play.h"
#include "detectForm.h"
#include "commonStructs.h"
using namespace std;
using namespace cv;

formationSet::formationSet(string formFilePath)
{
	vector<string> dirs;
	readOdLabels(formFilePath, pIds, dirs, odLabels);
}

void formationSet::getPlayersPosType()
{
	for(unsigned int i = 0; i < pIds.size(); ++i)
	{
		ostringstream convertGameId;
		convertGameId << pIds[i].gameId ;
		string gameIdStr = convertGameId.str();

		if(pIds[i].gameId < 10)
			gameIdStr = "0" + gameIdStr;

		ostringstream convertVidId;
		convertVidId << pIds[i].vidId;
		string vidIdxStr = convertVidId.str();
		if(pIds[i].vidId < 10)
			vidIdxStr = "00" + vidIdxStr;
		else if(pIds[i].vidId < 100 )
			vidIdxStr = "0" + vidIdxStr;

		vector<Point2d> playersPos;
		string playersPosFile = "formsLearningSet/Game" + gameIdStr + "/vid" + vidIdxStr + ".pos";
		readPlayersPos(playersPosFile, playersPos);
		//erase LOS center position
		playersPos.erase(playersPos.begin());
		rectPlayersFormSet.push_back(playersPos);
		vector<string> playersType;
		string playerTypesFile = "formsLearningSet/Game" + gameIdStr + "/vid" + vidIdxStr + ".type";
		readPlayersType(playerTypesFile, playersType);
		playersType.erase(playersType.begin());
		playersTypeFormSet.push_back(playersType);
	}
}

void formationSet::computeFeatureSet(int fld)
{
	getPlayersPosType();
	for(unsigned int i = 0; i < pIds.size(); ++i)
//	for(unsigned int i = 0; i < 2; ++i)
	{
		play* p = new play(pIds[i], fld);
		vector<double> maxFeatureVec;
		vector<vector<double> > featureVecs;
		int appFeatureNum = 3;
		vector<Point2d> allPos;
		cout << "Computing " << i << "th formation feature in formation set..." << endl;
		p->getAllPosFeatureVecs(maxFeatureVec, allPos, odLabels[i]);
		for(unsigned int j = 0; j < rectPlayersFormSet[i].size(); ++j)
		{
			vector<double> fVec = p->getOnePosFeatureVec(rectPlayersFormSet[i][j]);
//			for(int m = 0; m < fVec.size(); ++m)
//				cout << fVec[m] << " ";
//			cout << endl;
			for(int k = 0; k < appFeatureNum; ++k)
				if(maxFeatureVec[k] != 0)
					fVec[k] /= maxFeatureVec[k];
			featureVecs.push_back(fVec);
		}
		featureFormSet.push_back(featureVecs);
		maxFeatFormSet.push_back(maxFeatureVec);
		delete p;
	}
}

vector<vector<vector<double> > > formationSet::getFeatureFormSet()
{
	return featureFormSet;
}

vector<vector<Point2d> > formationSet::getRectPlayersFormSet()
{
	return rectPlayersFormSet;
}

vector<vector<string> > formationSet::getPlayersTypeFormSet()
{
	return playersTypeFormSet;
}
vector<playId> formationSet::getPIds()
{
	return pIds;
}

void formationSet::getTrainingFeats(int fld, string playerType,
		vector<vector<double> > &features, vector<int> &labels)
{
	getPlayersPosType();
	srand (time(NULL));
	for(unsigned int i = 0; i < pIds.size(); ++i)
//	for(unsigned int i = 0; i < 1; ++i)
	{
		play* p = new play(pIds[i], fld);
		vector<double> maxFeatureVec;
		vector<vector<double> > featureVecs;
		int appFeatureNum = 3;
		vector<Point2d> allPos;
		cout << "Computing " << i << "th formation feature in formation set..." << endl;
		vector<vector<double> > allPosFeat = p->getAllPosFeatureVecs(maxFeatureVec, allPos, odLabels[i]);
		int playerIdx = -1;
		for(unsigned int j = 0; j < rectPlayersFormSet[i].size(); ++j)
		{
			vector<double> fVec = p->getOnePosFeatureVec(rectPlayersFormSet[i][j]);
			for(int k = 0; k < appFeatureNum; ++k)
				if(maxFeatureVec[k] != 0)
					fVec[k] /= maxFeatureVec[k];
			featureVecs.push_back(fVec);
			if(playersTypeFormSet[i][j] == playerType)
			{
				features.push_back(fVec);
				labels.push_back(1);
				playerIdx = j;
			}
		}
		int rnd = rand() % (rectPlayersFormSet[i].size() - 1);
		if(rnd >= playerIdx)
			++rnd;
		features.push_back(featureVecs[rnd]);
		labels.push_back(-1);
		int winX = 2 * 15, winY = 5 * 15;
//		int rndFld = rand() % (allPos.size());
//		while(abs(allPos[rndFld].x -  rectPlayersFormSet[i][playerIdx].x) < winX / 2 &&
//				abs(allPos[rndFld].y -  rectPlayersFormSet[i][playerIdx].y) < winY / 2)
//			rndFld = rand() % (allPos.size());
//		features.push_back(allPosFeat[rndFld]);
//		labels.push_back(-1);
		for(int j = 0; j < 10; ++j)
		{
			int rndFld = rand() % (allPos.size());
			while(abs(allPos[rndFld].x -  rectPlayersFormSet[i][playerIdx].x) < winX / 2 &&
					abs(allPos[rndFld].y -  rectPlayersFormSet[i][playerIdx].y) < winY / 2)
				rndFld = rand() % (allPos.size());
			features.push_back(allPosFeat[rndFld]);
			labels.push_back(-1);
		}
		delete p;
	}
	Point2d gridSz = Point2d(75 * 0.5, 75);
	for(unsigned int i = 0; i < features.size(); ++i)
	{
//		features[i][0] = 0;
//		features[i][1] = 0;
//		features[i][2] = 0;

		//discretization
//		features[i][3] /= gridSz.x;
//		features[i][4] /= gridSz.y;
//		features[i][3] *= gridSz.x;
//		features[i][4] *= gridSz.y;

		features[i][3] /= 100;
		features[i][4] /= 100;

//		features[i][3] = 0;
//		features[i][4] = 0;

		//bias
		features[i].push_back(1);
	}
}


void formationSet::getTrainingFeats(int fld, vector<string> playerTypes,
		vector< vector<vector<double> > > &features, vector<vector<int> > &labels)
{
	features.assign(playerTypes.size(), vector<vector<double> >());
	labels.assign(playerTypes.size(), vector<int>());
	getPlayersPosType();
	srand (time(NULL));
	for(unsigned int i = 0; i < pIds.size(); ++i)
//	for(unsigned int i = 0; i < 1; ++i)
	{
		play* p = new play(pIds[i], fld);
		vector<double> maxFeatureVec;
		vector<vector<double> > featureVecs;
		int appFeatureNum = 3;
		vector<Point2d> allPos;
		cout << "Computing " << i << "th formation feature in formation set..." << endl;
		vector<vector<double> > allPosFeat = p->getAllPosFeatureVecs(maxFeatureVec, allPos, odLabels[i]);
		for(unsigned int j = 0; j < rectPlayersFormSet[i].size(); ++j)
		{
			vector<double> fVec = p->getOnePosFeatureVec(rectPlayersFormSet[i][j]);
			for(int k = 0; k < appFeatureNum; ++k)
				if(maxFeatureVec[k] != 0)
					fVec[k] /= maxFeatureVec[k];
			featureVecs.push_back(fVec);
		}
		for(unsigned int j = 0; j < rectPlayersFormSet[i].size(); ++j)
		{
			for(unsigned int k = 0; k < playerTypes.size(); ++k)
			{
				if(playersTypeFormSet[i][j] == playerTypes[k])
				{
					features[k].push_back(featureVecs[j]);
					labels[k].push_back(1);
					int rnd = rand() % (rectPlayersFormSet[i].size());
					while(rnd == (int)j)
						rnd = rand() % (rectPlayersFormSet[i].size());
					features[k].push_back(featureVecs[rnd]);
					labels[k].push_back(-1);
					int winX = 2 * 15, winY = 5 * 15;
					for(int j = 0; j < 10; ++j)
					{
						int rndFld = rand() % (allPos.size());
						while(abs(allPos[rndFld].x -  rectPlayersFormSet[i][j].x) < winX / 2 &&
								abs(allPos[rndFld].y -  rectPlayersFormSet[i][j].y) < winY / 2)
							rndFld = rand() % (allPos.size());
						features[k].push_back(allPosFeat[rndFld]);
						labels[k].push_back(-1);
					}
				}
			}
		}

		delete p;
	}
	for(unsigned int k = 0; k < features.size(); ++k)
		for(unsigned int i = 0; i < features[k].size(); ++i)
		{

			features[k][i][3] /= 100;
			features[k][i][4] /= 100;

			//bias
			features[k][i].push_back(1);
		}
}

double norm(const vector<double> &u)
{
    double accum = 0.;
    for(unsigned int i = 0; i < u.size(); ++i) {
        accum += u[i] * u[i];
    }
    double norm = sqrt(accum);
    return norm;
}
double dotPro(const vector<double> &u, const vector<double> &v)
{
    double accum = 0.;
    for(unsigned int i = 0; i < u.size(); ++i) {
        accum += u[i] * v[i];
    }
    return accum;
}
vector<double> add(const vector<double> &u, const vector<double> &v)
{
	vector<double> res(u.size(), 0);
    for(unsigned int i = 0; i < u.size(); ++i) {
        res[i] = u[i] + v[i];
    }
    return res;
}

vector<double> minusVec(const vector<double> &u, const vector<double> &v)
{
	vector<double> res(u.size(), 0);
    for(unsigned int i = 0; i < u.size(); ++i) {
        res[i] = u[i] - v[i];
    }
    return res;
}
vector<double> multiply(const vector<double> &u, double v)
{
	vector<double> res(u.size(), 0);
    for (unsigned int i = 0; i < u.size(); ++i) {
        res[i] = u[i] * v;
    }
    return res;
}

vector<double> formationSet::perceptronTrain(pair<int, int> leaveOutRng, const vector<vector<double> > &features,
		const vector<int> &labels)
{
	int featureNum = features[0].size();
//	vector<double> w(featureNum, 1), delta(featureNum, 0);
	vector<double> w(featureNum, 0), delta(featureNum, 0);
//	vector<double> wAvg(featureNum, 0);
	double threshold = EPS;
	int times = 0;
	int iterNum = 500;
	while(times < iterNum)
	{
		++times;
		delta.assign(featureNum, 0);
		for(unsigned int i = 0; i < features.size(); ++i)
		{
			if((int)i >= 3 * leaveOutRng.first && (int)i <= 3 * (leaveOutRng.second + 1) - 1)
				continue;
//			cout << "features: " << endl;
//			for(unsigned int j = 0; j < features[i].size(); ++j)
//				cout << features[i][j] << " ";
//			cout << endl;
//			cout << "label: " << labels[i] << endl;
			int pred = dotPro(features[i], w);
			if(labels[i] * pred <= 0)
			{
				if(labels[i] < 0)
					delta = add(delta, features[i]);
				else
					delta = minusVec(delta, features[i]);
			}
		}
		delta = multiply(delta, 1.0 /
				(features.size() - (leaveOutRng.second - leaveOutRng.first + 1)));
		w = minusVec(w, delta);
//		wAvg = add(wAvg, w);
//		cout << norm(delta) << endl;
		if(norm(delta) < threshold)
			return w;
	}
//	wAvg = multiply(wAvg, 1.0 / (iterNum *
//			(features.size() - (leaveOutRng.second - leaveOutRng.first + 1))) );
//	return wAvg;
	return w;
}

vector<vector<double> > formationSet::votedPerceptronTrain(pair<int, int> leaveOutRng, const vector<vector<double> > &features,
		const vector<int> &labels, vector<int> &votes)
{
	int featureNum = features[0].size();
//	vector<double> w(featureNum, 0), delta(featureNum, 0);
	vector<vector<double> > weights(1, vector<double>(featureNum, 0));
	votes.push_back(0);
	//double threshold = EPS;
	int times = 0;
	while(times < 500)
	{
		++times;
		for(unsigned int i = 0; i < features.size(); ++i)
		{
			if((int)i >= 3 * leaveOutRng.first && (int)i <= 3 * (leaveOutRng.second + 1) - 1)
				continue;
			cout << "features: " << endl;
			for(unsigned int j = 0; j < features[i].size(); ++j)
				cout << features[i][j] << " ";
			cout << endl;
			cout << "label: " << labels[i] << endl;
			vector<double> w = weights.back();
			int pred = dotPro(features[i], w);
			if(labels[i] * pred <= 0)
			{
				vector<double> wNew;
				if(labels[i] > 0)
					wNew = add(w, features[i]);
				else
					wNew = minusVec(w, features[i]);
				weights.push_back(wNew);
				votes.push_back(1);
			}
			else
			{
				++votes.back();
			}
		}
//		delta = multiply(delta, 1.0 /
//				(features.size() - (leaveOutRng.second - leaveOutRng.first + 1)));
//		w = minusVec(w, delta);
//		cout << norm(delta) << endl;
//		if(norm(delta) < threshold)
//			break;
	}
	return weights;
}

struct predScore
{
	double predScore;
	int index;
};

struct byScore {
    bool operator()(predScore const &left, predScore const &right) {
        return left.predScore > right.predScore;
    }
};

void testOnePlay(play *p, const vector<double> &w, string odLabel, string playerType)
{
	vector<double> maxFeatureVec;
	vector<Point2d> allPos;
	vector<vector<double> > allPosFeatureVecs = p->getAllPosFeatureVecs(maxFeatureVec, allPos, odLabel);
	vector<Point2d> predPos;
	vector<string> playerTypes;
	vector<predScore> scores;
	//int maxPredScore = -1, maxPredIdx = -1;
	Point2d gridSz = Point2d(75 * 0.5, 75);
	for(unsigned int i = 0; i < allPosFeatureVecs.size(); ++i)
	{
//		allPosFeatureVecs[i][0] = 0;
//		allPosFeatureVecs[i][1] = 0;
//		allPosFeatureVecs[i][2] = 0;

		//discretization
//		allPosFeatureVecs[i][3] /= gridSz.x;
//		allPosFeatureVecs[i][4] /= gridSz.y;
//		allPosFeatureVecs[i][3] *= gridSz.x;
//		allPosFeatureVecs[i][4] *= gridSz.y;

		allPosFeatureVecs[i][3] /= 100;
		allPosFeatureVecs[i][4] /= 100;
//		allPosFeatureVecs[i][3] = 0;
//		allPosFeatureVecs[i][4] = 0;
		//bias
		allPosFeatureVecs[i].push_back(1);
		double pred = dotPro(allPosFeatureVecs[i], w);
		//if(pred > 0)
		{
			struct predScore ps;
			ps.index = i;
			ps.predScore = pred;
			scores.push_back(ps);
//			predPos.push_back(allPos[i]);
//			playerTypes.push_back(playerType);
		}
//		if(pred > maxPredScore)
//		{
//			maxPredScore = pred;
//			maxPredIdx = i;
//		}
	}
//	predPos.push_back(allPos[maxPredIdx]);
//	playerTypes.push_back(playerType);
	sort(scores.begin(), scores.end(), byScore());
	for(int i = 0; i < min(2, (int)scores.size()); ++i)
	{
		predPos.push_back(allPos[scores[i].index]);
		playerTypes.push_back(playerType);
	}
	p->showForm(predPos, playerTypes);
}

void testOnePlay(play *p, const vector<vector<double> > &weights, const vector<int> &votes, string odLabel, string playerType)
{
	vector<double> maxFeatureVec;
	vector<Point2d> allPos;
	vector<vector<double> > allPosFeatureVecs = p->getAllPosFeatureVecs(maxFeatureVec, allPos, odLabel);
	vector<Point2d> predPos;
	vector<string> playerTypes;
	vector<predScore> scores;
	//int maxPredScore = -1, maxPredIdx = -1;
	Point2d gridSz = Point2d(75 * 0.5, 75);
	for(unsigned int i = 0; i < allPosFeatureVecs.size(); ++i)
	{
//		allPosFeatureVecs[i][0] = 0;
//		allPosFeatureVecs[i][1] = 0;
//		allPosFeatureVecs[i][2] = 0;

		//discretization
//		allPosFeatureVecs[i][3] /= gridSz.x;
//		allPosFeatureVecs[i][4] /= gridSz.y;
//		allPosFeatureVecs[i][3] *= gridSz.x;
//		allPosFeatureVecs[i][4] *= gridSz.y;

		allPosFeatureVecs[i][3] /= 100;
		allPosFeatureVecs[i][4] /= 100;
//		allPosFeatureVecs[i][3] = 0;
//		allPosFeatureVecs[i][4] = 0;
		//bias
		allPosFeatureVecs[i].push_back(1);
		int pred = 0;
		for(unsigned int j = 0; j < weights.size(); ++j)
		{
			int votePred = dotPro(allPosFeatureVecs[i], weights[j]);
			if(votePred > 0)
				pred += votes[j];
			else if(votePred < 0)
				pred -= votes[j];

		}
		//if(pred > 0)
		{
			struct predScore ps;
			ps.index = i;
			ps.predScore = pred;
			scores.push_back(ps);
//			predPos.push_back(allPos[i]);
//			playerTypes.push_back(playerType);
		}
//		if(pred > maxPredScore)
//		{
//			maxPredScore = pred;
//			maxPredIdx = i;
//		}
	}
//	predPos.push_back(allPos[maxPredIdx]);
//	playerTypes.push_back(playerType);
	sort(scores.begin(), scores.end(), byScore());
	for(int i = 0; i < min(2, (int)scores.size()); ++i)
	{
		predPos.push_back(allPos[scores[i].index]);
		playerTypes.push_back(playerType);
	}
	p->showForm(predPos, playerTypes);
}

double formationSet::crossValid()
{
	string playerType = "WRTop";
	vector<vector<double> > features;
	vector<int> labels;
	getTrainingFeats(1, playerType, features, labels);
	double acc = 0;
	int k = features.size() / 11 / 10;
	for(int i = 0; i < k; ++i)
	{
		pair<int, int> leaveOutRng = make_pair(i * 10, i * 10 + 9);
		vector<double> w = perceptronTrain(leaveOutRng, features, labels);
		cout << "w: ";
		for(unsigned int j = 0; j < w.size(); ++j)
			cout << w[j] << " ";
		cout << endl;
		for(int j = leaveOutRng.first; j <= leaveOutRng.second; ++j)
		{
			play* p = new play(pIds[j], 1);
//			testOnePlay(p, w, odLabels[i], playerType);
			testOnePlay(p, w, odLabels[j], playerType);
		}
	}
//	pair<int, int> leaveOutRng = make_pair(1, 39);
//	vector<double> w = perceptronTrain(leaveOutRng, features, labels);
//	cout << "w: ";
//	for(unsigned int j = 0; j < w.size(); ++j)
//		cout << w[j] << " ";
//	cout << endl;
//	for(int j = 0; j <= 0; ++j)
//	{
//		play* p = new play(pIds[j], 1);
////			testOnePlay(p, w, odLabels[i], playerType);
//		testOnePlay(p, w, odLabels[j], playerType);
//	}
	return acc;
}

double formationSet::crossValidVoted()
{
	string playerType = "WRMid";
	vector<vector<double> > features;
	vector<int> labels;
	getTrainingFeats(1, playerType, features, labels);
	double acc = 0;
//	int k = features.size() / 11 / 10;
//	for(int i = 0; i < k; ++i)
//	{
//		pair<int, int> leaveOutRng = make_pair(i * 10, i * 10 + 9);
//		vector<int> votes;
//		vector<vector<double> > weights = votedPerceptronTrain(leaveOutRng, features, labels, votes);
////		cout << "w: ";
////		for(unsigned int j = 0; j < w.size(); ++j)
////			cout << w[j] << " ";
//		cout << endl;
//		for(int j = leaveOutRng.first; j <= leaveOutRng.second; ++j)
//		{
//			play* p = new play(pIds[j], 1);
////			testOnePlay(p, w, odLabels[i], playerType);
//			testOnePlay(p, weights, votes, odLabels[j], playerType);
//		}
//	}
	pair<int, int> leaveOutRng = make_pair(10, 39);
	vector<double> w = perceptronTrain(leaveOutRng, features, labels);
	cout << "w: ";
	for(unsigned int j = 0; j < w.size(); ++j)
		cout << w[j] << " ";
	cout << endl;
	for(int j = 0; j <= 9; ++j)
	{
		play* p = new play(pIds[j], 1);
//			testOnePlay(p, w, odLabels[i], playerType);
		testOnePlay(p, w, odLabels[j], playerType);
	}
	return acc;
}

vector<vector<double> > formationSet::getPlayerWts(vector<string> playerTypes, pair<int, int> leaveOutRng)
{
	vector<vector<double> > weights;
//	string pTypes[] = {"WRTop", "WRBot", "WRMid", "H-b", "T", "G", "C", "QB", "RB"};
	vector<vector<vector<double> > > features;
	vector<vector<int> > labels;;
	getTrainingFeats(1, playerTypes, features, labels);
	for(unsigned int k = 0; k < playerTypes.size(); ++k)
	{
		//cout << playerTypes[k] << endl;
		vector<double> w;
		if(!features[k].empty())
			w = perceptronTrain(leaveOutRng, features[k], labels[k]);
		weights.push_back(w);
	}
	return weights;
}

vector<Point2d> testOnePlay(vector<vector<double> > allPosFeatureVecs, vector<Point2d> allPos,
		unordered_map<string, vector<double> > playerWts, const vector<string> &playerTypes,
		vector<vector<double> > &playersFeature, double &totalScore)
{
	totalScore = 0;
	vector<vector<double> > allPosFVecCopy = allPosFeatureVecs;
	vector<Point2d> playersPos;
	unordered_map<string, int> playersMap;
	for(unsigned int i = 0; i < playerTypes.size(); ++i)
	{
		if(playersMap.find(playerTypes[i]) == playersMap.end())
			playersMap[playerTypes[i]] = 1;
		else
			++playersMap[playerTypes[i]];
	}
	for(unsigned int i = 0; i < allPosFeatureVecs.size(); ++i)
	{
		allPosFeatureVecs[i][3] /= 100;
		allPosFeatureVecs[i][4] /= 100;
		//bias
		allPosFeatureVecs[i].push_back(1);
	}
	unordered_map<string, list<vector<double> > > playersFeatMap;
	unordered_map<string, list<Point2d > > playersPosMap;
	for(auto it = playersMap.begin(); it != playersMap.end(); ++it)
	{
		vector<double> w = playerWts[it->first];
		vector<predScore> scores;
		for(unsigned int i = 0; i < allPosFeatureVecs.size(); ++i)
		{
			double pred = dotPro(allPosFeatureVecs[i], w);
			struct predScore ps;
			ps.index = i;
			ps.predScore = pred;
			scores.push_back(ps);
		}
		sort(scores.begin(), scores.end(), byScore());
		list<vector<double> > featList(1, allPosFeatureVecs[scores[0].index]);
		playersFeatMap[it->first] = featList;
		list<Point2d> posList(1, allPos[scores[0].index]);
		playersPosMap[it->first] = posList;
		totalScore += scores[0].predScore;
		for(int i = 1; i < min(it->second, (int)scores.size()); ++i)
		{
			playersFeatMap[it->first].push_back(allPosFVecCopy[scores[i].index]);
			playersPosMap[it->first].push_back(allPos[scores[i].index]);
			totalScore += scores[i].predScore;
		}
	}
	for(unsigned int i = 0; i < playerTypes.size(); ++i)
	{
		playersPos.push_back(playersPosMap[playerTypes[i]].front());
		playersPosMap[playerTypes[i]].pop_front();
		playersFeature.push_back(playersFeatMap[playerTypes[i]].front());
		playersFeatMap[playerTypes[i]].pop_front();
	}
	return playersPos;
}

double matchTwoPlaysForm(const vector<vector<double> > &playersFeat1, const vector<vector<double> > &playersFeat2)
{
	double cost = 0;
	if(playersFeat1.size() != playersFeat2.size())
	{
		cout << "playersFeat1 and playersFeat2 have different size!" << endl;
		return cost;
	}
	vector<CvSize> gridSizes;
	setUpGrids(gridSizes, 1);
	for(unsigned int i = 0; i < playersFeat1.size(); ++i)
	{
		Point3d appPlayVec1(playersFeat1[i][0], playersFeat1[i][1], playersFeat1[i][2]);
		Point2d spPlayVec1(abs(playersFeat1[i][3]), playersFeat1[i][4]);
		Point3d appPlayVec2(playersFeat2[i][0], playersFeat2[i][1], playersFeat2[i][2]);
		Point2d spPlayVec2(abs(playersFeat2[i][3]), playersFeat2[i][4]);
		double tmpAppCost = norm(appPlayVec1 - appPlayVec2);
		double tmpSpCost = getGridSpCost(gridSizes, spPlayVec1, spPlayVec2);
		double tmpCost = 100 * tmpAppCost + tmpSpCost;
		cost += tmpCost;
	}

	return cost;
}

void formationSet::findSimForm()
{
	vector<string> playerTypes;
	playerTypes.push_back("WRTop");
	playerTypes.push_back("WRBot");
	playerTypes.push_back("WRMid");
	playerTypes.push_back("H-b");
	playerTypes.push_back("T");
	playerTypes.push_back("G");
	playerTypes.push_back("C");
	playerTypes.push_back("TE");
	playerTypes.push_back("QB");
	playerTypes.push_back("RB");
	computeFeatureSet(1);
	unordered_map<string, vector<double> > playerWts;
	for(unsigned int i = 0; i < pIds.size(); ++i)
	{
		cout << "leave out " << i << "th play..." << endl;
		pair<int, int> leaveOutRng = make_pair(i, i);
		vector<vector<double> > weights = getPlayerWts(playerTypes, leaveOutRng);
		for(unsigned int j = 0; j < playerTypes.size(); ++j)
			playerWts[playerTypes[j]] = weights[j];
		play* p = new play(pIds[i], 1);
		vector<double> maxFeatureVec;
		vector<Point2d> allPos;
		vector<vector<double> > allPosFeatureVecs = p->getAllPosFeatureVecs(maxFeatureVec, allPos, odLabels[i]);
//		double minCost = INF;
		double maxScore = NEGINF;
		playId bestPId;
		vector<Point2d> bestPlayersPos;
		vector<string> bestPlayersTypes;
		for(unsigned int j = 0; j < pIds.size(); ++j)
		{
			if(j == i)
				continue;
			vector<vector<double> > playersFeature;
			double score;
			vector<Point2d> playersPos = testOnePlay(allPosFeatureVecs, allPos, playerWts, playersTypeFormSet[j], playersFeature, score);
//			double cost = matchTwoPlaysForm(playersFeature, featureFormSet[j]);
//			if(cost < minCost)
//			{
//				minCost = cost;
//				bestPId = pIds[j];
//				bestPlayersPos = playersPos;
//				bestPlayersTypes = playersTypeFormSet[j];
//			}
			if(score > maxScore)
			{
				maxScore = score;
				bestPId = pIds[j];
				bestPlayersPos = playersPos;
				bestPlayersTypes = playersTypeFormSet[j];
			}
		}
		p->showForm(bestPlayersPos, bestPlayersTypes, bestPId);
	}
}

int computeStarModelLabel(vector<string> playerTypes1, vector<string> playerTypes2)
{
	if(playerTypes1.size() != playerTypes2.size())
		return -1;
	unordered_map<string, int> playersCntMap;
	for(unsigned int i = 0; i < playerTypes1.size(); ++i)
	{
		if(playersCntMap.find(playerTypes1[i]) == playersCntMap.end())
			playersCntMap[playerTypes1[i]] = 1;
		else
			++playersCntMap[playerTypes1[i]];
	}
	for(unsigned int i = 0; i < playerTypes2.size(); ++i)
	{
		if(playersCntMap.find(playerTypes2[i]) == playersCntMap.end())
			return -1;
		else
		{
			if(--playersCntMap[playerTypes2[i]] < 0)
				return -1;
		}
	}
	return 1;
}

//apply the formation of pIdsIdx1 to play pIdsIdx2 to extract star model feature
vector<double> formationSet::computeStarModelFVec(int pIdsIdx1, int pIdsIdx2, vector<Point2d> &transPlayPos)
{
	vector<double> fVec;
	int appFeatureNum = 3;
	if(pIdsIdx1 == pIdsIdx2)
	{
		for(unsigned int i = 0; i < featureFormSet[pIdsIdx1].size(); ++i)
			for(unsigned int j = 0; j < featureFormSet[pIdsIdx1][i].size(); ++j)
			{
				if((int)j >= appFeatureNum)
					fVec.push_back(featureFormSet[pIdsIdx1][i][j] / 100);
				else
					fVec.push_back(featureFormSet[pIdsIdx1][i][j]);
			}
		transPlayPos = rectPlayersFormSet[pIdsIdx1];
	}
	else
	{
		play* p = new play(pIds[pIdsIdx2], 1);
		play* pForm = new play(pIds[pIdsIdx1], 1);
		for(unsigned int i = 0; i < rectPlayersFormSet[pIdsIdx1].size(); ++i)
		{
			Point2d playerPos = rectPlayersFormSet[pIdsIdx1][i] - pForm->getRectLosCntGt();
			if(odLabels[pIdsIdx2] == "o")
				playerPos.x = -abs(playerPos.x );
			else if(odLabels[pIdsIdx2] == "d")
				playerPos.x = abs(playerPos.x );
			else
			{
				cout << "Wrong od labels!" << endl;
				return fVec;
			}
			playerPos += p->getRectLosCntGt();
			transPlayPos.push_back(playerPos);
			vector<double> fVecOnePlayer = p->getOnePosFeatureVec(playerPos);
			for(int k = 0; k < appFeatureNum; ++k)
				if(maxFeatFormSet[pIdsIdx2][k] != 0)
					fVecOnePlayer[k] /= maxFeatFormSet[pIdsIdx2][k];
			for(unsigned int j = 0; j < fVecOnePlayer.size(); ++j)
			{
				if((int)j >= appFeatureNum)
					fVec.push_back(fVecOnePlayer[j] / 100);
				else
					fVec.push_back(fVecOnePlayer[j]);
			}

		}
		delete p;
		delete pForm;
	}
	//bias
	fVec.push_back(1);
	return fVec;
}

vector<vector<double> > formationSet::trainStarModelCostWt(pair<int, int> leaveOutRng)
{
	vector<vector<double> > weights;
	for(unsigned int i = 0; i < pIds.size(); ++i)
	{
		if((int)i >= leaveOutRng.first && (int)i <= leaveOutRng.second)
			continue;
		vector<vector<double> > fVecs;
		vector<int> labels;
		for(unsigned int j = 0; j < pIds.size(); ++j)
		{
			if((int)j >= leaveOutRng.first && (int)j <= leaveOutRng.second)
				continue;
			int lab = computeStarModelLabel(playersTypeFormSet[i], playersTypeFormSet[j]);
			labels.push_back(lab);
			vector<Point2d> transPlayPos;
			vector<double> fVec = computeStarModelFVec(i, j, transPlayPos);
			fVecs.push_back(fVec);
		}
		int featureNum = fVecs[0].size();
		vector<double> w(featureNum, 0), delta(featureNum, 0);
		double threshold = EPS;
		int times = 0;
		int iterNum = 500;
		while(times < iterNum)
		{
			++times;
			delta.assign(featureNum, 0);
			for(unsigned int i = 0; i < fVecs.size(); ++i)
			{
				int pred = dotPro(fVecs[i], w);
				if(labels[i] * pred <= 0)
				{
					if(labels[i] < 0)
						delta = add(delta, fVecs[i]);
					else
						delta = minusVec(delta, fVecs[i]);
				}
			}
			delta = multiply(delta, 1.0 / (double)fVecs.size());
			w = minusVec(w, delta);
			if(norm(delta) < threshold)
				break;
		}
		weights.push_back(w);
	}
	return weights;
}

int formationSet::testToPickBestForm(pair<int, int> leaveOutRng, const vector<vector<double> > &weights)
{
	if(leaveOutRng.first != leaveOutRng.second)
	{
		cout << "Not leaving one out!" << endl;
		return -1;
	}
	int pTestIdx = leaveOutRng.first;
	play* pTest = new play(pIds[pTestIdx], 1);
	int wIdx = 0;
	int appFeatureNum = 3;
	double maxScore = NEGINF;
	int bestMatchIdx = -1;
	vector<Point2d> bestPlayersPos;
	for(unsigned int j = 0; j < pIds.size(); ++j)
	{
		if((int)j >= leaveOutRng.first && (int)j <= leaveOutRng.second)
			continue;
		vector<Point2d> transPlayPos;
		vector<double> fVec = computeStarModelFVec(j, pTestIdx, transPlayPos);
		double predScore = dotPro(fVec, weights[wIdx]);
		if(predScore >= maxScore)
		{
			maxScore = predScore;
			bestMatchIdx = j;
			bestPlayersPos = transPlayPos;
		}
		++wIdx;
	}
	pTest->showForm(bestPlayersPos, playersTypeFormSet[bestMatchIdx], pIds[bestMatchIdx]);
	delete pTest;
	return computeStarModelLabel(playersTypeFormSet[pTestIdx], playersTypeFormSet[bestMatchIdx]);
}

int formationSet::testToPickBestForm(int pTestIdx, const std::vector<std::vector<double> > &weights)
{
	play* pTest = new play(pIds[pTestIdx], 1);
	int wIdx = 0;
	int appFeatureNum = 3;
	double maxScore = NEGINF;
	int bestMatchIdx = -1;
	vector<Point2d> bestPlayersPos;
	for(unsigned int j = 0; j < pIds.size(); ++j)
	{
		vector<Point2d> transPlayPos;
		vector<double> fVec = computeStarModelFVec(j, pTestIdx, transPlayPos);
		double predScore = dotPro(fVec, weights[wIdx]);
		if(predScore >= maxScore)
		{
			maxScore = predScore;
			bestMatchIdx = j;
			bestPlayersPos = transPlayPos;
		}
		++wIdx;
	}
	pTest->showForm(bestPlayersPos, playersTypeFormSet[bestMatchIdx], pIds[bestMatchIdx]);
	delete pTest;
	cout << "bestMatchIdx: " << bestMatchIdx << endl;
	return computeStarModelLabel(playersTypeFormSet[pTestIdx], playersTypeFormSet[bestMatchIdx]);
}

void formationSet::leaveOneOutBestFormTest()
{
	computeFeatureSet(1);
	double acc = .0;
//	for(unsigned int i = 0; i < pIds.size(); ++i)
	for(unsigned int i = 0; i < 1; ++i)
	{
		cout << "Leave out " << i << "th play out..." << endl;
		pair<int, int> leaveOutRng = make_pair(i, i);
		vector<vector<double> > weights = trainStarModelCostWt(leaveOutRng);
		for(unsigned int k = 0; k < weights.size(); ++k)
		{
			cout << weights[k].size() << endl;
			for(unsigned int j = 0; j < weights[k].size(); ++j)
			{
				cout << weights[k][j] << " ";
				if((j + 1) % 5 == 0)
					cout << playersTypeFormSet[k + 1][j/5] << endl;
			}
			cout << endl;
		}
		int label = testToPickBestForm(leaveOutRng, weights);
		acc += (label > 0 ? 1 : 0);
	}
	acc /= double(pIds.size());
	cout << "Leave one out accuracy:" << acc << endl;
}

void formationSet::testBestFormTrainingSet()
{
	computeFeatureSet(1);
	double acc = .0;
	pair<int, int> leaveOutRng = make_pair(-1, -1);
	vector<vector<double> > weights = trainStarModelCostWt(leaveOutRng);
	for(unsigned int i = 0; i < weights.size(); ++i)
	{
		cout << weights[i].size() << endl;
		for(unsigned int j = 0; j < weights[i].size(); ++j)
		{
			cout << weights[i][j] << " ";
		}
		cout << endl;
	}
	for(unsigned int i = 0; i < pIds.size(); ++i)
	{
		cout << "Testing " << i << "th play out..." << endl;
		int label = testToPickBestForm(i, weights);
		acc += (label > 0 ? 1 : 0);
	}
	acc /= double(pIds.size());
	cout << "Testing accuracy:" << acc << endl;

//	getPlayersPosType();
//	for(unsigned int i = 0; i < pIds.size(); ++i)
//		for(unsigned int j = 0; j < pIds.size(); ++j)
//		{
//			cout << i << " " << j << endl;
//			if(j == i)
//				continue;
////			int lab = computeStarModelLabel(playersTypeFormSet[i], playersTypeFormSet[j]);
////			if(lab == -1)
////				cout << lab << endl;
//			if(computeStarModelLabel(playersTypeFormSet[i], playersTypeFormSet[j]) == 1)
//			{
//				acc += 1.0;
//				break;
//			}
//		}
//	acc /= double(pIds.size());
//	cout << "Testing accuracy:" << acc << endl;
}

