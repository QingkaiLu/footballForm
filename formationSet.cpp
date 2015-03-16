#include <vector>
#include <string>
#include <utility>
#include <stdlib.h>
#include <time.h>
#include "formationSet.h"
#include "playAuxFunc.h"
#include "extractOdVidFeats.h"
#include "play.h"
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
	double threshold = EPS;
	int times = 0;
	while(times < 500)
	{
		++times;
		delta.assign(featureNum, 0);
		for(unsigned int i = 0; i < features.size(); ++i)
		{
			if((int)i >= 3 * leaveOutRng.first && (int)i <= 3 * (leaveOutRng.second + 1) - 1)
				continue;
			cout << "features: " << endl;
			for(unsigned int j = 0; j < features[i].size(); ++j)
				cout << features[i][j] << " ";
			cout << endl;
			cout << "label: " << labels[i] << endl;
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
		cout << norm(delta) << endl;
		if(norm(delta) < threshold)
			break;
	}
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
	int predScore, index;
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
	string playerType = "QB";
	vector<vector<double> > features;
	vector<int> labels;
	getTrainingFeats(1, playerType, features, labels);
	double acc = 0;
	int k = features.size() / 11 / 10;
	for(int i = 0; i < k; ++i)
	{
		pair<int, int> leaveOutRng = make_pair(i * 10, i * 10 + 9);
		vector<int> votes;
		vector<vector<double> > weights = votedPerceptronTrain(leaveOutRng, features, labels, votes);
//		cout << "w: ";
//		for(unsigned int j = 0; j < w.size(); ++j)
//			cout << w[j] << " ";
		cout << endl;
		for(int j = leaveOutRng.first; j <= leaveOutRng.second; ++j)
		{
			play* p = new play(pIds[j], 1);
//			testOnePlay(p, w, odLabels[i], playerType);
			testOnePlay(p, weights, votes, odLabels[j], playerType);
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
