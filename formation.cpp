#include <vector>
#include <math.h>

#include "formation.h"
#include "playerBndBox.h"
#include "play.h"
//#include "player.h"

using namespace std;
using namespace cv;

formation::formation(const play *pl, const vector<playerType> &pTypes)
//formation::formation(play *pl, const vector<playerType> &pTypes)
{
	p = pl;
	dir = nonDir;
	totalScore = NEGINF;
	for(unsigned int i = 0; i < pTypes.size(); ++i)
	{
		player pler(pTypes[i]);
//		pler.pType = pTypes[i];
//		pler.pBox = new playerBndBox;
//		pler.score = -1.0;
		players.push_back(pler);
	}
}

formation::formation(const play *pl)
{
	totalScore = NEGINF;
	p = pl;
	dir = nonDir;
}


formation::~formation()
{
//	for(unsigned int i = 0; i < players.size(); ++i)
//		delete players[i].pBox;
}

void formation::setPlayerTypes(const std::vector<playerType> &pTypes)
{
	for(unsigned int i = 0; i < pTypes.size(); ++i)
	{
//		player pler;
//		pler.pType = pTypes[i];
//		pler.pBox = new playerBndBox;
//		pler.score = -1.0;
		player pler(pTypes[i]);
		players.push_back(pler);
	}
}


void formation::setPlayers(const vector<player> &ps)
{
	for(unsigned int i = 0; i < ps.size(); ++i)
	{
//		player pler;
//		pler.pType = pTypes[i];
//		pler.pBox = new playerBndBox;
//		pler.score = -1.0;
		player pler(ps[i]);
		players.push_back(pler);
	}
}


void compBestPlayerPos(const play *p, int bnd[4], double boxXLen, double boxYLen, vector<player> &players)
{
	int startY = bnd[0], endY = bnd[1], startX = bnd[2], endX = bnd[3];

	//scores of low players, including spatial and appearance
		//vector<double> playerScores(players.size(), -1.0);
		for(int y = startY; y < endY; y += 2)
			for(int x = startX; x < endX; x += 2)
			{
				struct rect scanRect = {};
				scanRect.trksNum = 0;

				scanRect.a = Point2d(x, y);
				scanRect.b = Point2d(x, y + boxYLen * p->yardLnsDist);
				scanRect.c = Point2d(x + boxXLen * p->yardLnsDist, y + boxYLen * p->yardLnsDist);
				scanRect.d = Point2d(x + boxXLen * p->yardLnsDist, y);

				Point2d boxCnt = (scanRect.a + scanRect.b + scanRect.c + scanRect.d) * 0.25;
				//double distBoxToLos = norm(boxCnt - scrimCnt) / yardLnsDist;

	//			if(!isScanRectInsdRngRect(scanRect, recSearchRng))
	//				continue;
				playerBndBox leftBox(scanRect);
				leftBox.setDir(leftDir);
				vector<struct track> leftTrksInside;
				for(unsigned int i = 0; i < p->tracks.size(); ++i)
				{
	//				if(isTrkInsideRect(tracks[i], scrimLn))
	//					continue;

					if(isTrkInsideRect(p->tracks[i], scanRect) && (trackDir(p->tracks[i]) == leftDir))
					{
						leftTrksInside.push_back(p->tracks[i]);
						leftBox.addTrk(i);
					}
				}

				double leftLongestPath = computeTrksConsistScr(leftTrksInside) / p->yardLnsDist;
				leftBox.setLongestPath(leftLongestPath);

				playerBndBox rightBox(scanRect);
				rightBox.setDir(rightDir);
				vector<struct track> rightTrksInside;
				for(unsigned int i = 0; i < p->tracks.size(); ++i)
				{
					if(isTrkInsideRect(p->tracks[i], p->scrimLn))
						continue;

					if(isTrkInsideRect(p->tracks[i], scanRect) && (trackDir(p->tracks[i]) == rightDir))
					{
						rightTrksInside.push_back(p->tracks[i]);
						rightBox.addTrk(i);
					}
				}

				double rightLongestPath = computeTrksConsistScr(rightTrksInside) / p->yardLnsDist;
				rightBox.setLongestPath(rightLongestPath);

				playerBndBox box;
				if(leftLongestPath > rightLongestPath)
					box = leftBox;
				else
					box = rightBox;

				Point2d vecVpToScrimCnt;
				if(p->vpExist)
				{
					vecVpToScrimCnt = p->scrimCnt - p->vp;
					vecVpToScrimCnt *= 1.0 / norm(vecVpToScrimCnt);
				}
				else
				{
					vecVpToScrimCnt = Point2d(.0, 1.0);
				}

				Point2d perpVecVpToScrimCnt = Point2d(-1.0 * vecVpToScrimCnt.y, vecVpToScrimCnt.x);
				if(perpVecVpToScrimCnt.x < 0.0)
					perpVecVpToScrimCnt = -1.0 * perpVecVpToScrimCnt;

				Point2d vecScrimCntToBoxCnt = boxCnt - p->scrimCnt;
				double distYardLn = abs(vecScrimCntToBoxCnt.dot(vecVpToScrimCnt));
				double distYardLnPerp = abs(vecScrimCntToBoxCnt.dot(perpVecVpToScrimCnt));
				for(unsigned int i = 0; i < players.size(); ++i)
				{
					double score = 0.0;
					double normDist = p->yardLnsDist / 10.0;
					//normDist = 1.0;
					if( (players[i].pType == lowWR) || (players[i].pType == upWR) )
						score = (distYardLn / normDist) * (box.longestTrksPath / normDist)
						/ (1.0 + distYardLnPerp / normDist);
						//score = box.longestTrksPath;
					else if((players[i].pType == lowCB) || (players[i].pType == upCB) )
						score = (distYardLn / normDist) * (box.longestTrksPath / normDist)
						* (distYardLnPerp / normDist);
					//score /= p->yardLnsDist;
					if(score > players[i].score)
					{
						*players[i].pBox = box;
						players[i].score = score;
					}
				}
			}

}

double compOneDirBestPlayerPos(const play *p, int bnd[4], double boxXLen, double boxYLen, vector<player> &players, direction dir)
{
	int startY = bnd[0], endY = bnd[1], startX = bnd[2], endX = bnd[3];

//	double wRYardLn = 0, wRPerpYardLn = 0, wRTrkLen = 0;
//	double qBYardLn = 0, qBPerpYardLn = 0, qBTrkLen = 0;

	//scores of low players, including spatial and appearance
		//vector<double> playerScores(players.size(), -1.0);
		for(int y = startY; y < endY; y += 2)
			for(int x = startX; x < endX; x += 2)
			{
				struct rect scanRect = {};
				scanRect.trksNum = 0;

				scanRect.a = Point2d(x, y);
				scanRect.b = Point2d(x, y + boxYLen * p->yardLnsDist);
				scanRect.c = Point2d(x + boxXLen * p->yardLnsDist, y + boxYLen * p->yardLnsDist);
				scanRect.d = Point2d(x + boxXLen * p->yardLnsDist, y);

				Point2d boxCnt = (scanRect.a + scanRect.b + scanRect.c + scanRect.d) * 0.25;
				//double distBoxToLos = norm(boxCnt - scrimCnt) / yardLnsDist;

	//			if(!isScanRectInsdRngRect(scanRect, recSearchRng))
	//				continue;
				Point2d vecVpToScrimCnt;
				if(p->vpExist)
				{
					vecVpToScrimCnt = p->scrimCnt - p->vp;
					vecVpToScrimCnt *= 1.0 / norm(vecVpToScrimCnt);
				}
				else
				{
					vecVpToScrimCnt = Point2d(.0, 1.0);
				}

				Point2d perpVecVpToScrimCnt = Point2d(-1.0 * vecVpToScrimCnt.y, vecVpToScrimCnt.x);
				if(perpVecVpToScrimCnt.x < 0.0)
					perpVecVpToScrimCnt = -1.0 * perpVecVpToScrimCnt;

				Point2d vecScrimCntToBoxCnt = boxCnt - p->scrimCnt;
				double dotPro = vecScrimCntToBoxCnt.dot(vecVpToScrimCnt);
				double dotProPerp = vecScrimCntToBoxCnt.dot(perpVecVpToScrimCnt);

				//cout << "dotProPerp: " << dotProPerp << endl;
				//cout << "p->scrimCnt.x: " << p->scrimCnt.x << endl;
//				if(dir == rightDir)
//				{
//					if(dotProPerp <= 0.0)
//						continue;
//				}
//				else if(dir == leftDir)
//				{
//					if(dotProPerp >= 0.0)
//						continue;
//				}
				playerBndBox leftBox(scanRect);
				leftBox.setDir(leftDir);
				vector<struct track> leftTrksInside;
				for(unsigned int i = 0; i < p->tracks.size(); ++i)
				{
	//				if(isTrkInsideRect(tracks[i], scrimLn))
	//					continue;

					if(isTrkInsideRect(p->tracks[i], scanRect) && (trackDir(p->tracks[i]) == leftDir))
					{
						leftTrksInside.push_back(p->tracks[i]);
						leftBox.addTrk(i);
					}
				}

				double leftLongestPath = computeTrksConsistScr(leftTrksInside) / p->yardLnsDist;
				leftBox.setLongestPath(leftLongestPath);

				playerBndBox rightBox(scanRect);
				rightBox.setDir(rightDir);
				vector<struct track> rightTrksInside;
				for(unsigned int i = 0; i < p->tracks.size(); ++i)
				{
					if(isTrkInsideRect(p->tracks[i], p->scrimLn))
						continue;

					if(isTrkInsideRect(p->tracks[i], scanRect) && (trackDir(p->tracks[i]) == rightDir))
					{
						rightTrksInside.push_back(p->tracks[i]);
						rightBox.addTrk(i);
					}
				}

				double rightLongestPath = computeTrksConsistScr(rightTrksInside) / p->yardLnsDist;
				rightBox.setLongestPath(rightLongestPath);

				playerBndBox box;
//				if(leftLongestPath > rightLongestPath)
//					box = leftBox;
//				else
//					box = rightBox;



//				Point2d vecScrimCntToBoxCnt = boxCnt - p->scrimCnt;
				double distYardLn = abs(dotPro);
				double distYardLnPerp = abs(dotProPerp);
				for(unsigned int i = 0; i < players.size(); ++i)
				{
					double score = NEGINF;
					double normDist = p->yardLnsDist / 10.0;
					//normDist = 1.0;
					if( (players[i].pType == lowWR) || (players[i].pType == upWR) )
					{
						if(dir == leftDir)
							box = leftBox;
						else if(dir == rightDir)
							box = rightBox;

						#if lnModel == 1
//						score = log(distYardLn / normDist) + log(box.longestTrksPath / normDist)
//						- log(distYardLnPerp / normDist + 1.0);
						score = log(distYardLn) + log(box.longestTrksPath)
						- log(distYardLnPerp / normDist + 1.0);
//						score = (distYardLn / normDist) + (box.longestTrksPath / normDist)
//						- (distYardLnPerp / normDist);
						#elif lnModel == 0
						score = (distYardLn / normDist) * (box.longestTrksPath / normDist)
						/ (1.0 + distYardLnPerp / normDist);
						#endif

//						if(score > players[i].score)
//						{
//							wRYardLn = distYardLn / normDist;
////							wRPerpYardLn = distYardLnPerp / normDist;
//							wRPerpYardLn = distYardLnPerp;
//							wRTrkLen = box.longestTrksPath / normDist;
//							*players[i].pBox = box;
//							players[i].score = score;
//						}
					}
						//score = box.longestTrksPath;
					else if((players[i].pType == lowCB) || (players[i].pType == upCB) )
					{
						if(leftLongestPath > rightLongestPath)
							box = leftBox;
						else
							box = rightBox;

						#if lnModel == 1
						score = log(distYardLn / normDist) + log(box.longestTrksPath / normDist)
						+ log(distYardLnPerp / normDist);
//						score = (distYardLn / normDist) + (box.longestTrksPath / normDist)
//						+ (distYardLnPerp / normDist);
						#elif lnModel == 0
						score = (distYardLn / normDist) * (box.longestTrksPath / normDist)
						* (distYardLnPerp / normDist);
						#endif

//						if(score > players[i].score)
//						{
//							qBYardLn = distYardLn / normDist;
////							qBPerpYardLn = distYardLnPerp / normDist;
//							qBPerpYardLn = distYardLnPerp;
//							qBTrkLen = box.longestTrksPath / normDist;
//							*players[i].pBox = box;
//							players[i].score = score;
//						}
					}
					//score /= p->yardLnsDist;
					if(score > players[i].score)
					{
						*players[i].pBox = box;
						players[i].score = score;
					}
				}
			}

		double totalScore = 0.0;
		for(unsigned int i = 0; i < players.size(); ++i)
			totalScore += players[i].score;

//		cout << "wRYardLn: " << wRYardLn << "wRPerpYardLn: " << wRPerpYardLn << "wRTrkLen: " << wRTrkLen << endl;
//		cout << "qBYardLn: " << qBYardLn << "qBPerpYardLn: " << qBPerpYardLn << "qBTrkLen: " << qBTrkLen << endl;

		return totalScore;
}

double compOneDirBestPlayerPos(const play *p, int bnd[4], double boxXLen, double boxYLen, vector<double> &weight, vector<player> &players, direction dir, vector<double> &feature)
{
	if(players.size() == 0)
		return 0.0;
	double totalScore = 0.0;

	int startY = bnd[0], endY = bnd[1], startX = bnd[2], endX = bnd[3];

	Point2d vecVpToScrimCnt;
	if(p->vpExist)
	{
		vecVpToScrimCnt = p->scrimCnt - p->vp;
		vecVpToScrimCnt *= 1.0 / norm(vecVpToScrimCnt);
	}
	else
	{
		vecVpToScrimCnt = Point2d(.0, 1.0);
	}

	Point2d perpVecVpToScrimCnt = Point2d(-1.0 * vecVpToScrimCnt.y, vecVpToScrimCnt.x);
	if(perpVecVpToScrimCnt.x < 0.0)
		perpVecVpToScrimCnt = -1.0 * perpVecVpToScrimCnt;

	for(int y = startY; y < endY; y += scanYStep)
		for(int x = startX; x < endX; x += scanXStep)
		{
			struct rect scanRect = {};
			scanRect.trksNum = 0;

			scanRect.a = Point2d(x, y);
			scanRect.b = Point2d(x, y + boxYLen * p->yardLnsDist);
			scanRect.c = Point2d(x + boxXLen * p->yardLnsDist, y + boxYLen * p->yardLnsDist);
			scanRect.d = Point2d(x + boxXLen * p->yardLnsDist, y);

			Point2d boxCnt = (scanRect.a + scanRect.b + scanRect.c + scanRect.d) * 0.25;
			Point2d vecScrimCntToBoxCnt = boxCnt - p->scrimCnt;
			double dotPro = vecScrimCntToBoxCnt.dot(vecVpToScrimCnt);
			double dotProPerp = vecScrimCntToBoxCnt.dot(perpVecVpToScrimCnt);
			double angScrimCntToBoxCnt = atan(dotProPerp / dotPro);

//			if(dir == rightDir)
//			{
//				if(dotProPerp <= 0.0)
//					continue;
//			}
//			else if(dir == leftDir)
//			{
//				if(dotProPerp >= 0.0)
//					continue;
//			}

			for(unsigned int i = 0; i < players.size(); ++i)
			{
				if((players[i].pType == lowCB) || (players[i].pType == upCB))
				{
						if(dir == rightDir)
						{
							if(dotProPerp <= 0.0)
								continue;
						}
						else if(dir == leftDir)
						{
							if(dotProPerp >= 0.0)
								continue;
						}
				}

				playerBndBox box;
				int t = playerStartPosInFVec(players[i].pType, featureNumEachPlayer);
				double tmpFeature[4], tmpWeight[4];
//				int t = -1;
//				switch (players[i].pType) {
//				case lowWR:
//					t = 0;
//					break;
//				case lowCB:
//					t = 4;
//					break;
//				case upWR:
//					t = 8;
//					break;
//				case upCB:
//					t = 12;
//					break;
//				default:
//					t = -1;
//					break;
//
//				}
				for(int j = 0; j < featureNumEachPlayer; ++j)
					tmpWeight[j]= weight[t + j];
				double score = compOnePosPlayerScr(p, scanRect, players[i], dir, vecScrimCntToBoxCnt, angScrimCntToBoxCnt, tmpWeight, box, tmpFeature);
//				cout << "tmpFeature[0]: " << tmpFeature[0] << endl;
				if(score > players[i].score)
				{
					*players[i].pBox = box;
					players[i].score = score;
					for(int j = 0; j < featureNumEachPlayer; ++j)
						feature[t + j] = tmpFeature[j];
				}

				//cout << "feature[0]: " << feature[0] << endl;
			}
		}

		//double totalScore = 0.0;
		for(unsigned int i = 0; i < players.size(); ++i)
			totalScore += players[i].score;

		return totalScore;

}


double compOneDirBestPlayerPos(const play *p, int bnd[4], const struct rect &range, double boxXLen, double boxYLen, vector<player> &players, direction dir)
{
	int startY = bnd[0], endY = bnd[1], startX = bnd[2], endX = bnd[3];

//	double wRYardLn = 0, wRPerpYardLn = 0, wRTrkLen = 0;
//	double qBYardLn = 0, qBPerpYardLn = 0, qBTrkLen = 0;

	//scores of low players, including spatial and appearance
		//vector<double> playerScores(players.size(), -1.0);
		for(int y = startY; y < endY; y += 2)
			for(int x = startX; x < endX; x += 2)
			{
				struct rect scanRect = {};
				scanRect.trksNum = 0;

				scanRect.a = Point2d(x, y);
				scanRect.b = Point2d(x, y + boxYLen * p->yardLnsDist);
				scanRect.c = Point2d(x + boxXLen * p->yardLnsDist, y + boxYLen * p->yardLnsDist);
				scanRect.d = Point2d(x + boxXLen * p->yardLnsDist, y);

				if(!isScanRectInsdRngRect(scanRect, range))
				{
					//cout << " *** "<< endl;
					continue;
				}

				Point2d boxCnt = (scanRect.a + scanRect.b + scanRect.c + scanRect.d) * 0.25;

				//double distBoxToLos = norm(boxCnt - scrimCnt) / yardLnsDist;

	//			if(!isScanRectInsdRngRect(scanRect, recSearchRng))
	//				continue;
				Point2d vecVpToScrimCnt;
				if(p->vpExist)
				{
					vecVpToScrimCnt = p->scrimCnt - p->vp;
					vecVpToScrimCnt *= 1.0 / norm(vecVpToScrimCnt);
				}
				else
				{
					vecVpToScrimCnt = Point2d(.0, 1.0);
				}

				Point2d perpVecVpToScrimCnt = Point2d(-1.0 * vecVpToScrimCnt.y, vecVpToScrimCnt.x);
				if(perpVecVpToScrimCnt.x < 0.0)
					perpVecVpToScrimCnt = -1.0 * perpVecVpToScrimCnt;

				Point2d vecScrimCntToBoxCnt = boxCnt - p->scrimCnt;
				double dotPro = vecScrimCntToBoxCnt.dot(vecVpToScrimCnt);
				double dotProPerp = vecScrimCntToBoxCnt.dot(perpVecVpToScrimCnt);

				//cout << "dotProPerp: " << dotProPerp << endl;
				//cout << "p->scrimCnt.x: " << p->scrimCnt.x << endl;
//				if(dir == rightDir)
//				{
//					if(dotProPerp <= 0.0)
//						continue;
//				}
//				else if(dir == leftDir)
//				{
//					if(dotProPerp >= 0.0)
//						continue;
//				}
				playerBndBox leftBox(scanRect);
				leftBox.setDir(leftDir);
				vector<struct track> leftTrksInside;
				for(unsigned int i = 0; i < p->tracks.size(); ++i)
				{
	//				if(isTrkInsideRect(tracks[i], scrimLn))
	//					continue;

					if(isTrkInsideRect(p->tracks[i], scanRect) && (trackDir(p->tracks[i]) == leftDir))
					{
						leftTrksInside.push_back(p->tracks[i]);
						leftBox.addTrk(i);
					}
				}

				double leftLongestPath = computeTrksConsistScr(leftTrksInside) / p->yardLnsDist;
				leftBox.setLongestPath(leftLongestPath);

				playerBndBox rightBox(scanRect);
				rightBox.setDir(rightDir);
				vector<struct track> rightTrksInside;
				for(unsigned int i = 0; i < p->tracks.size(); ++i)
				{
					if(isTrkInsideRect(p->tracks[i], p->scrimLn))
						continue;

					if(isTrkInsideRect(p->tracks[i], scanRect) && (trackDir(p->tracks[i]) == rightDir))
					{
						rightTrksInside.push_back(p->tracks[i]);
						rightBox.addTrk(i);
					}
				}

				double rightLongestPath = computeTrksConsistScr(rightTrksInside) / p->yardLnsDist;
				rightBox.setLongestPath(rightLongestPath);

				playerBndBox box;
//				if(leftLongestPath > rightLongestPath)
//					box = leftBox;
//				else
//					box = rightBox;



//				Point2d vecScrimCntToBoxCnt = boxCnt - p->scrimCnt;
				double distYardLn = abs(dotPro);
				double distYardLnPerp = abs(dotProPerp);
				for(unsigned int i = 0; i < players.size(); ++i)
				{
					double score = NEGINF;
					double normDist = p->yardLnsDist / 10.0;
					//normDist = 1.0;
					if( (players[i].pType == lowWR) || (players[i].pType == upWR) )
					{
						if(dir == leftDir)
							box = leftBox;
						else if(dir == rightDir)
							box = rightBox;

						#if lnModel == 1
//						score = log(distYardLn / normDist) + log(box.longestTrksPath / normDist)
//						- log(distYardLnPerp / normDist + 1.0);
						score = log(distYardLn) + log(box.longestTrksPath)
						- log(distYardLnPerp / normDist + 1.0);
//						score = (distYardLn / normDist) + (box.longestTrksPath / normDist)
//						- (distYardLnPerp / normDist);
						#elif lnModel == 0
						score = (distYardLn / normDist) * (box.longestTrksPath / normDist)
						/ (1.0 + distYardLnPerp / normDist);
						#endif

//						if(score > players[i].score)
//						{
//							wRYardLn = distYardLn / normDist;
////							wRPerpYardLn = distYardLnPerp / normDist;
//							wRPerpYardLn = distYardLnPerp;
//							wRTrkLen = box.longestTrksPath / normDist;
//							*players[i].pBox = box;
//							players[i].score = score;
//						}
					}
						//score = box.longestTrksPath;
					else if((players[i].pType == lowCB) || (players[i].pType == upCB) )
					{
						if(leftLongestPath > rightLongestPath)
							box = leftBox;
						else
							box = rightBox;

						#if lnModel == 1
						score = log(distYardLn / normDist) + log(box.longestTrksPath / normDist)
						+ log(distYardLnPerp / normDist);
//						score = (distYardLn / normDist) + (box.longestTrksPath / normDist)
//						+ (distYardLnPerp / normDist);
						#elif lnModel == 0
						score = (distYardLn / normDist) * (box.longestTrksPath / normDist)
						* (distYardLnPerp / normDist);
						#endif

//						if(score > players[i].score)
//						{
//							qBYardLn = distYardLn / normDist;
////							qBPerpYardLn = distYardLnPerp / normDist;
//							qBPerpYardLn = distYardLnPerp;
//							qBTrkLen = box.longestTrksPath / normDist;
//							*players[i].pBox = box;
//							players[i].score = score;
//						}
					}
					//score /= p->yardLnsDist;
					if(score > players[i].score)
					{
						*players[i].pBox = box;
						players[i].score = score;
					}
				}
			}

		double totalScore = 0.0;
		for(unsigned int i = 0; i < players.size(); ++i)
			totalScore += players[i].score;

//		cout << "wRYardLn: " << wRYardLn << "wRPerpYardLn: " << wRPerpYardLn << "wRTrkLen: " << wRTrkLen << endl;
//		cout << "qBYardLn: " << qBYardLn << "qBPerpYardLn: " << qBPerpYardLn << "qBTrkLen: " << qBTrkLen << endl;

		return totalScore;
}


double compOneDirBestPlayerPos(const play *p, int bnd[4], const struct rect &range, double boxXLen, double boxYLen, player &pler, direction dir)
{
	int startY = bnd[0], endY = bnd[1], startX = bnd[2], endX = bnd[3];

	//scores of low players, including spatial and appearance
		//vector<double> playerScores(players.size(), -1.0);
		for(int y = startY; y < endY; y += 2)
			for(int x = startX; x < endX; x += 2)
			{
				struct rect scanRect = {};
				scanRect.trksNum = 0;

				scanRect.a = Point2d(x, y);
				scanRect.b = Point2d(x, y + boxYLen * p->yardLnsDist);
				scanRect.c = Point2d(x + boxXLen * p->yardLnsDist, y + boxYLen * p->yardLnsDist);
				scanRect.d = Point2d(x + boxXLen * p->yardLnsDist, y);

				if(!isScanRectInsdRngRect(scanRect, range))
				{
					//cout << " *** "<< endl;
					continue;
				}

				Point2d boxCnt = (scanRect.a + scanRect.b + scanRect.c + scanRect.d) * 0.25;

				//double distBoxToLos = norm(boxCnt - scrimCnt) / yardLnsDist;

	//			if(!isScanRectInsdRngRect(scanRect, recSearchRng))
	//				continue;
				Point2d vecVpToScrimCnt;
				if(p->vpExist)
				{
					vecVpToScrimCnt = p->scrimCnt - p->vp;
					vecVpToScrimCnt *= 1.0 / norm(vecVpToScrimCnt);
				}
				else
				{
					vecVpToScrimCnt = Point2d(.0, 1.0);
				}

				Point2d perpVecVpToScrimCnt = Point2d(-1.0 * vecVpToScrimCnt.y, vecVpToScrimCnt.x);
				if(perpVecVpToScrimCnt.x < 0.0)
					perpVecVpToScrimCnt = -1.0 * perpVecVpToScrimCnt;

				Point2d vecScrimCntToBoxCnt = boxCnt - p->scrimCnt;
				double dotPro = vecScrimCntToBoxCnt.dot(vecVpToScrimCnt);
				double dotProPerp = vecScrimCntToBoxCnt.dot(perpVecVpToScrimCnt);

				//cout << "dotProPerp: " << dotProPerp << endl;
				//cout << "p->scrimCnt.x: " << p->scrimCnt.x << endl;
//				if(dir == rightDir)
//				{
//					if(dotProPerp <= 0.0)
//						continue;
//				}
//				else if(dir == leftDir)
//				{
//					if(dotProPerp >= 0.0)
//						continue;
//				}
				playerBndBox leftBox(scanRect);
				leftBox.setDir(leftDir);
				vector<struct track> leftTrksInside;
				for(unsigned int i = 0; i < p->tracks.size(); ++i)
				{
	//				if(isTrkInsideRect(tracks[i], scrimLn))
	//					continue;

					if(isTrkInsideRect(p->tracks[i], scanRect) && (trackDir(p->tracks[i]) == leftDir))
					{
						leftTrksInside.push_back(p->tracks[i]);
						leftBox.addTrk(i);
					}
				}

				double leftLongestPath = computeTrksConsistScr(leftTrksInside) / p->yardLnsDist;
				leftBox.setLongestPath(leftLongestPath);

				playerBndBox rightBox(scanRect);
				rightBox.setDir(rightDir);
				vector<struct track> rightTrksInside;
				for(unsigned int i = 0; i < p->tracks.size(); ++i)
				{
					if(isTrkInsideRect(p->tracks[i], p->scrimLn))
						continue;

					if(isTrkInsideRect(p->tracks[i], scanRect) && (trackDir(p->tracks[i]) == rightDir))
					{
						rightTrksInside.push_back(p->tracks[i]);
						rightBox.addTrk(i);
					}
				}

				double rightLongestPath = computeTrksConsistScr(rightTrksInside) / p->yardLnsDist;
				rightBox.setLongestPath(rightLongestPath);

				playerBndBox box;

				double distYardLn = abs(dotPro);
				double distYardLnPerp = abs(dotProPerp);
				double score = NEGINF;
				double normDist = p->yardLnsDist / 10.0;
				//normDist = 1.0;
				if( (pler.pType == lowWR) || (pler.pType == upWR) )
				{
					if(dir == leftDir)
						box = leftBox;
					else if(dir == rightDir)
						box = rightBox;

					#if lnModel == 1
//						score = log(distYardLn / normDist) + log(box.longestTrksPath / normDist)
//						- log(distYardLnPerp / normDist + 1.0);
					score = log(distYardLn) + log(box.longestTrksPath)
					- log(distYardLnPerp / normDist + 1.0);
//						score = (distYardLn / normDist) + (box.longestTrksPath / normDist)
//						- (distYardLnPerp / normDist);
					#elif lnModel == 0
					score = (distYardLn / normDist) * (box.longestTrksPath / normDist)
					/ (1.0 + distYardLnPerp / normDist);
					#endif

				}
					//score = box.longestTrksPath;
				else if((pler.pType == lowCB) || (pler.pType == upCB) )
				{
					if(leftLongestPath > rightLongestPath)
						box = leftBox;
					else
						box = rightBox;

					#if lnModel == 1
					score = log(distYardLn / normDist) + log(box.longestTrksPath / normDist)
					+ log(distYardLnPerp / normDist);
//						score = (distYardLn / normDist) + (box.longestTrksPath / normDist)
//						+ (distYardLnPerp / normDist);
					#elif lnModel == 0
					score = (distYardLn / normDist) * (box.longestTrksPath / normDist)
					* (distYardLnPerp / normDist);
					#endif

				}
				//score /= p->yardLnsDist;
				if(score > pler.score)
				{
					*pler.pBox = box;
					pler.score = score;
				}
			}

		double totalScore = pler.score;

		return totalScore;
}


double compOnePosPlayerScr(const play *p, const struct rect &scanRect, player &pler, direction dir, double dotPro, double dotProPerp, playerBndBox &box)
{
	double weight[3] = {1.0, 1.0, 1.0};
	double featureScores[3];
	double score = compOnePosPlayerScr(p, scanRect, pler, dir, dotPro, dotProPerp, weight, box, featureScores);
	return score;
}

double compOnePosPlayerScr(const play *p, const struct rect &scanRect, player &pler, direction dir, double dotPro, double dotProPerp, playerBndBox &box, double featureScores[3])
{
	double weight[3] = {1.0, 1.0, 1.0};
	double score = compOnePosPlayerScr(p, scanRect, pler, dir, dotPro, dotProPerp, weight, box, featureScores);
	return score;
}


double compOnePosPlayerScr(const play *p, const struct rect &scanRect, player &pler, direction dir, double dotPro, double dotProPerp, double weight[3], playerBndBox &box, double featureScores[3])
{
			playerBndBox leftBox(scanRect);
			leftBox.setDir(leftDir);
			vector<struct track> leftTrksInside;
			for(unsigned int i = 0; i < p->tracks.size(); ++i)
			{
//				if(isTrkInsideRect(tracks[i], scrimLn))
//					continue;

				if(isTrkInsideRect(p->tracks[i], scanRect) && (trackDir(p->tracks[i]) == leftDir))
				{
					leftTrksInside.push_back(p->tracks[i]);
					leftBox.addTrk(i);
				}
			}

			double leftLongestPath = computeTrksConsistScr(leftTrksInside) / p->yardLnsDist;
			leftBox.setLongestPath(leftLongestPath);

			playerBndBox rightBox(scanRect);
			rightBox.setDir(rightDir);
			vector<struct track> rightTrksInside;
			for(unsigned int i = 0; i < p->tracks.size(); ++i)
			{
				if(isTrkInsideRect(p->tracks[i], p->scrimLn))
					continue;

				if(isTrkInsideRect(p->tracks[i], scanRect) && (trackDir(p->tracks[i]) == rightDir))
				{
					rightTrksInside.push_back(p->tracks[i]);
					rightBox.addTrk(i);
				}
			}

			double rightLongestPath = computeTrksConsistScr(rightTrksInside) / p->yardLnsDist;
			rightBox.setLongestPath(rightLongestPath);

			//playerBndBox box;
			double distYardLn = abs(dotPro);
			double distYardLnPerp = abs(dotProPerp);
			double score = NEGINF;
			double normDist = p->yardLnsDist / 10.0;
			//normDist = 1.0;
			if( (pler.pType == lowWR) || (pler.pType == upWR) )
			{
				if(dir == leftDir)
					box = leftBox;
				else if(dir == rightDir)
					box = rightBox;

				#if lnModel == 1
				featureScores[0] = log(box.longestTrksPath + 1.0);
				featureScores[1] = log(distYardLn + 1.0);
				featureScores[2] = - log(distYardLnPerp / normDist + 1.0);
				//featureScores[3] =

				score = weight[0] * featureScores[0] + weight[1] * featureScores[1] + weight[2] * featureScores[2];

//						score = (distYardLn / normDist) + (box.longestTrksPath / normDist)
//						- (distYardLnPerp / normDist);
				#elif lnModel == 0
				score = (distYardLn / normDist) * (box.longestTrksPath / normDist)
				/ (1.0 + distYardLnPerp / normDist);
				#endif

			}
			else if((pler.pType == lowCB) || (pler.pType == upCB) )
			{
				if(leftLongestPath > rightLongestPath)
					box = leftBox;
				else
					box = rightBox;

				#if lnModel == 1
//				score = log(distYardLn / normDist) + log(box.longestTrksPath/ normDist)
//				+ log(distYardLnPerp / normDist);

//				featureScores[0] = log(box.longestTrksPath/ normDist + 1.0);
//				//featureScores[0] = 1.0;
//				featureScores[1] = log(distYardLn / normDist + 1.0);
//				featureScores[2] = log(distYardLnPerp / normDist + 1.0);
//				featureScores[0] = log(box.longestTrksPath / normDist + EPS * EPS * EPS);
//				featureScores[1] = log(distYardLn / normDist + EPS * EPS * EPS);
//				featureScores[2] = log(distYardLnPerp / normDist + EPS * EPS * EPS);
				featureScores[0] = log(box.longestTrksPath / normDist + 1.0);
				featureScores[1] = log(distYardLn / normDist + 1.0);
				featureScores[2] = log(distYardLnPerp / normDist + 1.0);
				score = weight[0] * featureScores[0] + weight[1] * featureScores[1] + weight[2] * featureScores[2];
				//score = featureScores[0] + featureScores[1] + featureScores[2];


				#elif lnModel == 0
				score = (distYardLn / normDist) * (box.longestTrksPath / normDist)
				* (distYardLnPerp / normDist);
				#endif

			}

			//cout << "featureScores[0]: " << featureScores[0] << endl;
			//cout << "box.longestTrksPath: " << box.longestTrksPath << endl;
				return score;
}

double compOnePosPlayerScr(const play *p, const struct rect &scanRect, player &pler, direction dir, cv::Point2d vecScrimCntToBoxCnt, double angScrimCntToBoxCnt, double weight[4], playerBndBox &box, double featureScores[4])
{
	playerBndBox leftBox(scanRect);
	leftBox.setDir(leftDir);
	vector<struct track> leftTrksInside;
	for(unsigned int i = 0; i < p->tracks.size(); ++i)
	{
//				if(isTrkInsideRect(tracks[i], scrimLn))
//					continue;

		if(isTrkInsideRect(p->tracks[i], scanRect) && (trackDir(p->tracks[i]) == leftDir))
		{
			leftTrksInside.push_back(p->tracks[i]);
			leftBox.addTrk(i);
		}
	}

	double leftLongestPath = computeTrksConsistScr(leftTrksInside) / p->yardLnsDist;
	leftBox.setLongestPath(leftLongestPath);

	playerBndBox rightBox(scanRect);
	rightBox.setDir(rightDir);
	vector<struct track> rightTrksInside;
	for(unsigned int i = 0; i < p->tracks.size(); ++i)
	{
		if(isTrkInsideRect(p->tracks[i], p->scrimLn))
			continue;

		if(isTrkInsideRect(p->tracks[i], scanRect) && (trackDir(p->tracks[i]) == rightDir))
		{
			rightTrksInside.push_back(p->tracks[i]);
			rightBox.addTrk(i);
		}
	}

	double rightLongestPath = computeTrksConsistScr(rightTrksInside) / p->yardLnsDist;
	rightBox.setLongestPath(rightLongestPath);


	double score = NEGINF;
	//double normDist = p->yardLnsDist / 10.0;
	//normDist = 1.0;
	if( (pler.pType == lowWR) || (pler.pType == upWR) )
	{
		if(dir == leftDir)
			box = leftBox;
		else if(dir == rightDir)
			box = rightBox;

		featureScores[0] = box.longestTrksPath;
		featureScores[1] = norm(vecScrimCntToBoxCnt) / p->yardLnsDist;
		featureScores[2] = abs(angScrimCntToBoxCnt);
		double boxSize = (box.bndRect.c.y - box.bndRect.a.y) * (box.bndRect.c.x - box.bndRect.a.x);
		featureScores[3] = fgPixelsInsideBoxXY(p, box.bndRect) / (boxSize / 5);
		//cout << "fgPixelNum: " << fgPixelsInsideBox(p, box.bndRect) << endl;

		score = weight[0] * featureScores[0] + weight[1] * featureScores[1] + weight[2] * featureScores[2] + weight[3] * featureScores[3];


	}
	else if((pler.pType == lowCB) || (pler.pType == upCB) )
	{
		if(leftLongestPath > rightLongestPath)
			box = leftBox;
		else
			box = rightBox;

		featureScores[0] = box.longestTrksPath;
		featureScores[1] = norm(vecScrimCntToBoxCnt) / p->yardLnsDist;
		featureScores[2] = abs(angScrimCntToBoxCnt);
		double boxSize = (box.bndRect.c.y - box.bndRect.a.y) * (box.bndRect.c.x - box.bndRect.a.x);
		featureScores[3] = fgPixelsInsideBoxXY(p, box.bndRect) / (boxSize / 5);

//		featureScores[0] = .0;
//		featureScores[1] = .0;
//		featureScores[2] = .0;
//		featureScores[3] = .0;
		score = weight[0] * featureScores[0] + weight[1] * featureScores[1] + weight[2] * featureScores[2] + weight[3] * featureScores[3];
		//score = featureScores[0] + featureScores[1] + featureScores[2];
	}

		return score;
}
double compOnePosPlayerScr(const play *p, player &pler, direction dir, double dotPro, double dotProPerp, const playerBndBox &box)
{
//				playerBndBox leftBox(scanRect);
//				leftBox.setDir(leftDir);
//				vector<struct track> leftTrksInside;
//				for(unsigned int i = 0; i < p->tracks.size(); ++i)
//				{
//	//				if(isTrkInsideRect(tracks[i], scrimLn))
//	//					continue;
//
//					if(isTrkInsideRect(p->tracks[i], scanRect) && (trackDir(p->tracks[i]) == leftDir))
//					{
//						leftTrksInside.push_back(p->tracks[i]);
//						leftBox.addTrk(i);
//					}
//				}
//
//				double leftLongestPath = computeTrksConsistScr(leftTrksInside) / p->yardLnsDist;
//				leftBox.setLongestPath(leftLongestPath);
//
//				playerBndBox rightBox(scanRect);
//				rightBox.setDir(rightDir);
//				vector<struct track> rightTrksInside;
//				for(unsigned int i = 0; i < p->tracks.size(); ++i)
//				{
//					if(isTrkInsideRect(p->tracks[i], p->scrimLn))
//						continue;
//
//					if(isTrkInsideRect(p->tracks[i], scanRect) && (trackDir(p->tracks[i]) == rightDir))
//					{
//						rightTrksInside.push_back(p->tracks[i]);
//						rightBox.addTrk(i);
//					}
//				}
//
//				double rightLongestPath = computeTrksConsistScr(rightTrksInside) / p->yardLnsDist;
//				rightBox.setLongestPath(rightLongestPath);

				//playerBndBox box;
				double distYardLn = abs(dotPro);
				double distYardLnPerp = abs(dotProPerp);
				double score = NEGINF;
				double normDist = p->yardLnsDist / 10.0;
				//normDist = 1.0;
				if( (pler.pType == lowWR) || (pler.pType == upWR) )
				{
//					if(dir == leftDir)
//						box = leftBox;
//					else if(dir == rightDir)
//						box = rightBox;
					if(dir != box.dir)
					{
						score = NEGINF;
					}
					else
					{
						#if lnModel == 1
		//						score = log(distYardLn / normDist) + log(box.longestTrksPath / normDist)
		//						- log(distYardLnPerp / normDist + 1.0);
						score = log(distYardLn) + log(box.longestTrksPath)
						- log(distYardLnPerp / normDist + 1.0);
		//						score = (distYardLn / normDist) + (box.longestTrksPath / normDist)
		//						- (distYardLnPerp / normDist);
						#elif lnModel == 0
						score = (distYardLn / normDist) * (box.longestTrksPath / normDist)
						/ (1.0 + distYardLnPerp / normDist);
						#endif
					}

				}
				else if((pler.pType == lowCB) || (pler.pType == upCB) )
				{
//					if(leftLongestPath > rightLongestPath)
//						box = leftBox;
//					else
//						box = rightBox;

					#if lnModel == 1
					score = log(distYardLn / normDist) + log(box.longestTrksPath / normDist)
					+ log(distYardLnPerp / normDist);
					#elif lnModel == 0
					score = (distYardLn / normDist) * (box.longestTrksPath / normDist)
					* (distYardLnPerp / normDist);
					#endif

				}

					return score;
}


double compOneDirBestPlayerPos2(const play *p, int bnd[4], const struct rect &range, double boxXLen, double boxYLen, vector<player> &players, direction dir)
{
	double totalScore = NEGINF;

	int startY = bnd[0], endY = bnd[1], startX = bnd[2], endX = bnd[3];

	Point2d vecVpToScrimCnt;
	if(p->vpExist)
	{
		vecVpToScrimCnt = p->scrimCnt - p->vp;
		vecVpToScrimCnt *= 1.0 / norm(vecVpToScrimCnt);
	}
	else
	{
		vecVpToScrimCnt = Point2d(.0, 1.0);
	}

	Point2d perpVecVpToScrimCnt = Point2d(-1.0 * vecVpToScrimCnt.y, vecVpToScrimCnt.x);
	if(perpVecVpToScrimCnt.x < 0.0)
		perpVecVpToScrimCnt = -1.0 * perpVecVpToScrimCnt;

	for(int y = startY; y < endY; y += 5)
		for(int x = startX; x < endX; x += 5)
		{
			struct rect scanRect = {};
			scanRect.trksNum = 0;

			scanRect.a = Point2d(x, y);
			scanRect.b = Point2d(x, y + boxYLen * p->yardLnsDist);
			scanRect.c = Point2d(x + boxXLen * p->yardLnsDist, y + boxYLen * p->yardLnsDist);
			scanRect.d = Point2d(x + boxXLen * p->yardLnsDist, y);

			Point2d boxCnt = (scanRect.a + scanRect.b + scanRect.c + scanRect.d) * 0.25;
			Point2d vecScrimCntToBoxCnt = boxCnt - p->scrimCnt;
			double dotPro = vecScrimCntToBoxCnt.dot(vecVpToScrimCnt);
			double dotProPerp = vecScrimCntToBoxCnt.dot(perpVecVpToScrimCnt);

			if(!isScanRectInsdRngRect(scanRect, range))
				continue;

//			if(dir == rightDir)
//			{
//				if(dotProPerp <= 0.0)
//					continue;
//			}
//			else if(dir == leftDir)
//			{
//				if(dotProPerp >= 0.0)
//					continue;
//			}

			playerBndBox box;
			double score = compOnePosPlayerScr(p, scanRect, players[0], dir, dotPro, dotProPerp, box);
			for(int y2 = startY; y2 < endY; y2 += 5)
				for(int x2 = startX; x2 < endX; x2 += 5)
				{
					struct rect scanRect2 = {};
					scanRect2.trksNum = 0;

					scanRect2.a = Point2d(x2, y2);
					scanRect2.b = Point2d(x2, y2 + boxYLen * p->yardLnsDist);
					scanRect2.c = Point2d(x2 + boxXLen * p->yardLnsDist, y2 + boxYLen * p->yardLnsDist);
					scanRect2.d = Point2d(x2 + boxXLen * p->yardLnsDist, y2);

					if(!isScanRectInsdRngRect(scanRect2, range))
						continue;

					Point2d boxCnt2 = (scanRect2.a + scanRect2.b + scanRect2.c + scanRect2.d) * 0.25;
					Point2d vecScrimCntToBoxCnt2 = boxCnt2 - p->scrimCnt;
					double dotPro2 = vecScrimCntToBoxCnt2.dot(vecVpToScrimCnt);
					double dotProPerp2 = vecScrimCntToBoxCnt2.dot(perpVecVpToScrimCnt);

					//cout << "dotProPerp: " << dotProPerp << endl;
					//cout << "p->scrimCnt.x: " << p->scrimCnt.x << endl;
//					if(dir == rightDir)
//					{
//						if(dotProPerp2 <= 0.0)
//							continue;
//					}
//					else if(dir == leftDir)
//					{
//						if(dotProPerp2 >= 0.0)
//							continue;
//					}

					playerBndBox box2;
					double score2 = compOnePosPlayerScr(p, scanRect2, players[1], dir, dotPro2, dotProPerp2, box2);
					double wRcBScore = 0.0;
					Point2d vecFromBox1ToBox2 = boxCnt2 - boxCnt;
					if(norm(vecFromBox1ToBox2) < 2 * boxXLen * p->yardLnsDist)
						wRcBScore = 0.0;
					else
						wRcBScore = log((abs(vecFromBox1ToBox2.x) - abs(vecFromBox1ToBox2.y)) / p->yardLnsDist);

					if( (score + score2 + wRcBScore) > totalScore)
					{
						totalScore = score + score2 + wRcBScore;
						*players[0].pBox = box;
						players[0].score = score;
						*players[1].pBox = box2;
						players[1].score = score2;
					}
				}

		}

//		double totalScore = 0.0;
//		for(unsigned int i = 0; i < players.size(); ++i)
//			totalScore += players[i].score;

//		cout << "wRYardLn: " << wRYardLn << "wRPerpYardLn: " << wRPerpYardLn << "wRTrkLen: " << wRTrkLen << endl;
//		cout << "qBYardLn: " << qBYardLn << "qBPerpYardLn: " << qBPerpYardLn << "qBTrkLen: " << qBTrkLen << endl;

		return totalScore;
}


double compOneDirBestPlayerPos2(const play *p, const std::vector<playerBndBox> &topBoxes, vector<player> &players, direction dir)
{
	double totalScore = NEGINF;

	double boxXLen = 1.95752 * 2.0 / 3.0;
	//double boxYLen = 0.603402 / 2.0;

	//int startY = bnd[0], endY = bnd[1], startX = bnd[2], endX = bnd[3];

	Point2d vecVpToScrimCnt;
	if(p->vpExist)
	{
		vecVpToScrimCnt = p->scrimCnt - p->vp;
		vecVpToScrimCnt *= 1.0 / norm(vecVpToScrimCnt);
	}
	else
	{
		vecVpToScrimCnt = Point2d(.0, 1.0);
	}

	Point2d perpVecVpToScrimCnt = Point2d(-1.0 * vecVpToScrimCnt.y, vecVpToScrimCnt.x);
	if(perpVecVpToScrimCnt.x < 0.0)
		perpVecVpToScrimCnt = -1.0 * perpVecVpToScrimCnt;

//	for(int y = startY; y < endY; y += 2)
//		for(int x = startX; x < endX; x += 2)
//		{
	for(unsigned int i = 0; i < topBoxes.size(); ++i)
	{
		struct rect scanRect = topBoxes[i].bndRect;
		scanRect.trksNum = 0;
		Point2d boxCnt = (scanRect.a + scanRect.b + scanRect.c + scanRect.d) * 0.25;
		//struct rect scanRect = {};

//		scanRect.a = Point2d(x, y);
//		scanRect.b = Point2d(x, y + boxYLen * p->yardLnsDist);
//		scanRect.c = Point2d(x + boxXLen * p->yardLnsDist, y + boxYLen * p->yardLnsDist);
//		scanRect.d = Point2d(x + boxXLen * p->yardLnsDist, y);

		//Point2d boxCnt = (scanRect.a + scanRect.b + scanRect.c + scanRect.d) * 0.25;
		Point2d vecScrimCntToBoxCnt = boxCnt - p->scrimCnt;
		double dotPro = vecScrimCntToBoxCnt.dot(vecVpToScrimCnt);
		double dotProPerp = vecScrimCntToBoxCnt.dot(perpVecVpToScrimCnt);

//		if(!isScanRectInsdRngRect(scanRect, range))
//			continue;

//		if(dir == rightDir)
//		{
//			if(dotProPerp <= 0.0)
//				continue;
//		}
//		else if(dir == leftDir)
//		{
//			if(dotProPerp >= 0.0)
//				continue;
//		}

		if(dotPro >= 0.0)
		{
			if((players[0].pType == upWR) || (players[0].pType == upCB))
			{
				//cout << "true" << endl;
				continue;
			}
		}
		else if(dotPro < 0.0)
		{
			//cout << "true" << endl;
			if((players[0].pType == lowWR) || (players[0].pType == lowCB))
				continue;
		}

		playerBndBox box = topBoxes[i];
		double score = compOnePosPlayerScr(p, players[0], dir, dotPro, dotProPerp, box);
//		for(int y2 = startY; y2 < endY; y2 += 2)
//			for(int x2 = startX; x2 < endX; x2 += 2)
//			{
		for(unsigned int j = 0; j < topBoxes.size(); ++j)
		{
			struct rect scanRect2 = topBoxes[j].bndRect;
			scanRect2.trksNum = 0;
			Point2d boxCnt2 = (scanRect2.a + scanRect2.b + scanRect2.c + scanRect2.d) * 0.25;
			//struct rect scanRect2 = {};


//			scanRect2.a = Point2d(x2, y2);
//			scanRect2.b = Point2d(x2, y2 + boxYLen * p->yardLnsDist);
//			scanRect2.c = Point2d(x2 + boxXLen * p->yardLnsDist, y2 + boxYLen * p->yardLnsDist);
//			scanRect2.d = Point2d(x2 + boxXLen * p->yardLnsDist, y2);

//			if(!isScanRectInsdRngRect(scanRect2, range))
//				continue;

			//Point2d boxCnt2 = (scanRect2.a + scanRect2.b + scanRect2.c + scanRect2.d) * 0.25;
			Point2d vecScrimCntToBoxCnt2 = boxCnt2 - p->scrimCnt;
			double dotPro2 = vecScrimCntToBoxCnt2.dot(vecVpToScrimCnt);
			double dotProPerp2 = vecScrimCntToBoxCnt2.dot(perpVecVpToScrimCnt);

			//cout << "dotProPerp: " << dotProPerp << endl;
			//cout << "p->scrimCnt.x: " << p->scrimCnt.x << endl;
//			if(dir == rightDir)
//			{
//				if(dotProPerp2 <= 0.0)
//					continue;
//			}
//			else if(dir == leftDir)
//			{
//				if(dotProPerp2 >= 0.0)
//					continue;
//			}

			if(dotPro2 >= 0.0)
			{
				if((players[1].pType == upWR) || (players[1].pType == upCB))
				{
					//cout << "true" << endl;
					continue;
				}
			}
			else if(dotPro2 < 0.0)
			{
				//cout << "true" << endl;
				if((players[1].pType == lowWR) || (players[1].pType == lowCB))
					continue;
			}

			playerBndBox box2 = topBoxes[j];
			double score2 = compOnePosPlayerScr(p, players[1], dir, dotPro2, dotProPerp2, box2);
			double wRcBScore = NEGINF;
			double normDist = p->yardLnsDist / 10.0;
			Point2d vecFromBox1ToBox2 = boxCnt2 - boxCnt;
			if(norm(vecFromBox1ToBox2) < boxXLen * p->yardLnsDist)
				wRcBScore = -1000.0;
			else
				wRcBScore = log(normDist / (abs(vecFromBox1ToBox2.x) / normDist  + 1)) + 2 * log(normDist / (abs(vecFromBox1ToBox2.y) / normDist + 1));
				//wRcBScore = log(p->yardLnsDist / (abs(vecFromBox1ToBox2.x) / normDist  + 1)) + 2 * log(p->yardLnsDist / (abs(vecFromBox1ToBox2.y) / normDist + 1));
				//wRcBScore = log(abs(p->yardLnsDist / vecFromBox1ToBox2.x)) + 2 * log(abs(p->yardLnsDist / vecFromBox1ToBox2.y));
//				wRcBScore = log((abs(vecFromBox1ToBox2.x) - abs(vecFromBox1ToBox2.y)) / p->yardLnsDist);

			if( (score + score2 + wRcBScore) > totalScore)
			{
				totalScore = score + score2 + wRcBScore;
				*players[0].pBox = box;
				players[0].score = score;
				players[0].wrCbScore = wRcBScore;
				*players[1].pBox = box2;
				players[1].score = score2;
				players[1].wrCbScore = wRcBScore;
				//cout << "*********" << endl;
			}
		}

		}

		return totalScore;
}



double compOneDirBestPlayerPos(const play *p, const vector<playerBndBox> &topBoxes, vector<player> &players, direction dir)
{
	for(unsigned int j = 0; j < topBoxes.size(); ++j)
	{
		struct rect scanRect = topBoxes[j].bndRect;
		Point2d boxCnt = (scanRect.a + scanRect.b + scanRect.c + scanRect.d) * 0.25;
		Point2d vecVpToScrimCnt;
		if(p->vpExist)
		{
			vecVpToScrimCnt = p->scrimCnt - p->vp;
			vecVpToScrimCnt *= 1.0 / norm(vecVpToScrimCnt);
		}
		else
		{
			vecVpToScrimCnt = Point2d(.0, 1.0);
		}

		Point2d perpVecVpToScrimCnt = Point2d(-1.0 * vecVpToScrimCnt.y, vecVpToScrimCnt.x);
		if(perpVecVpToScrimCnt.x < 0.0)
			perpVecVpToScrimCnt = -1.0 * perpVecVpToScrimCnt;

		Point2d vecScrimCntToBoxCnt = boxCnt - p->scrimCnt;
		double dotPro = vecScrimCntToBoxCnt.dot(vecVpToScrimCnt);
		double dotProPerp = vecScrimCntToBoxCnt.dot(perpVecVpToScrimCnt);

//		if(dir == rightDir)
//		{
//			if(dotProPerp <= 0.0)
//				continue;
//		}
//		else if(dir == leftDir)
//		{
//			if(dotProPerp >= 0.0)
//				continue;
//		}

		double distYardLn = abs(dotPro);
		double distYardLnPerp = abs(dotProPerp);
//		if(boxCnt.y < 300)
//		{
//			cout << "true" << endl;
//			continue;
//		}
		for(unsigned int i = 0; i < players.size(); ++i)
		{
			if(dotPro >= 0.0)
			{
				if((players[i].pType == upWR) || (players[i].pType == upCB))
				{
					//cout << "true" << endl;
					continue;
				}
			}
			else if(dotPro < 0.0)
			{
				//cout << "true" << endl;
				if((players[i].pType == lowWR) || (players[i].pType == lowCB))
					continue;
			}
			double score = 0.0;
			double normDist = p->yardLnsDist / 10.0;
			//normDist = 1.0;
			if( (players[i].pType == lowWR) || (players[i].pType == upWR) )
			{
				if(dir != topBoxes[j].dir)
					score = -1.0;
				else
					score = (distYardLn / normDist) * (topBoxes[j].longestTrksPath / normDist)
					/ (1.0 + distYardLnPerp / normDist);
			}
			else if((players[i].pType == lowCB) || (players[i].pType == upCB) )
			{
				score = (distYardLn / normDist) * (topBoxes[j].longestTrksPath / normDist)
				* (distYardLnPerp / normDist);
			}

			if(score > players[i].score)
			{
				*players[i].pBox = topBoxes[j];
				players[i].score = score;
			}
		}
	}

	double totalScore = 0.0;
	for(unsigned int i = 0; i < players.size(); ++i)
		totalScore += players[i].score;

	return totalScore;
}


void formation::compForm()
{
	double boxXLen = 1.95752 * 2.0 / 3.0;
	double boxYLen = 0.603402 / 2.0;
	//vector<player> lowPlayers, upPlayers;

	int lowStartY = p->scrimCnt.y;
	int lowEndY = imgYLen - int(boxYLen * p->yardLnsDist);

	int upStartY = 1;
	int upEndY = p->scrimCnt.y - int(boxYLen * p->yardLnsDist);

	int startX = 1;
	int endX = imgXLen - int(boxXLen * p->yardLnsDist);

	int lowBnd[4] = {lowStartY, lowEndY, startX, endX};
	//compBestPlayerPos(p, lowBnd, boxXLen, boxYLen, lowPlayers);

	int upBnd[4] = { upStartY, upEndY, startX, endX};
	//compBestPlayerPos(p, upBnd, boxXLen, boxYLen, upPlayers);

	//lowPlayers.insert(lowPlayers.end(), upPlayers.begin(), upPlayers.end());

	//players = lowPlayers;

//	vector<player> leftLowPlayers = lowPlayers, leftUpPlayers = upPlayers;
//	vector<player> rightLowPlayers = lowPlayers, rightUpPlayers = upPlayers;

	#if playersRange == 1
		vector<player> leftLowPlayers, leftUpPlayers;
		vector<player> rightLowPlayers, rightUpPlayers;
		for(unsigned int i = 0; i < players.size(); ++i)
		{
			if((players[i].pType == lowWR) || (players[i].pType == lowCB))
			{
				leftLowPlayers.push_back(players[i]);
				rightLowPlayers.push_back(players[i]);
			}
			else if((players[i].pType == upWR) || (players[i].pType == upCB))
			{
				leftUpPlayers.push_back(players[i]);
				rightUpPlayers.push_back(players[i]);
			}
		}
		struct rect lowRng, upRng;
		compPlayersRange(lowRng, upRng);
		lowRange = lowRng;
		upRange = upRng;

		double leftScore = compOneDirBestPlayerPos2(p, lowBnd, lowRng, boxXLen, boxYLen, leftLowPlayers, leftDir);
		leftScore += compOneDirBestPlayerPos2(p, upBnd, upRng, boxXLen, boxYLen, leftUpPlayers, leftDir);

		double rightScore = compOneDirBestPlayerPos2(p, lowBnd, lowRng, boxXLen, boxYLen, rightLowPlayers, rightDir);
		rightScore += compOneDirBestPlayerPos2(p, upBnd, upRng, boxXLen, boxYLen, rightUpPlayers, rightDir);

		if(leftScore > rightScore)
		{
			leftLowPlayers.insert(leftLowPlayers.end(), leftUpPlayers.begin(), leftUpPlayers.end());
			players = leftLowPlayers;
			dir = leftDir;
		}
		else
		{
			rightLowPlayers.insert(rightLowPlayers.end(), rightUpPlayers.begin(), rightUpPlayers.end());
			players = rightLowPlayers;
			dir = rightDir;
		}

	#elif playersRange == 2
		struct rect lowLeftRng, upLeftRng, lowRightRng, upRightRng;
		compPlayersRange(lowLeftRng, lowRightRng, upLeftRng, upRightRng);
//		lowRange = lowRng;
//		upRange = upRng;
		vector<player> leftPlayers = players, rightPlayers = players;

		double leftScore = 0;
		for(unsigned int i = 0; i < leftPlayers.size(); ++i)
		{
			if(leftPlayers[i].pType == lowWR)
				leftScore += compOneDirBestPlayerPos(p, lowBnd, lowRightRng, boxXLen, boxYLen, leftPlayers[i], leftDir);
			if(leftPlayers[i].pType == lowCB)
				leftScore += compOneDirBestPlayerPos(p, lowBnd, lowLeftRng, boxXLen, boxYLen, leftPlayers[i], leftDir);
			if(leftPlayers[i].pType == upWR)
				leftScore += compOneDirBestPlayerPos(p, upBnd, upRightRng, boxXLen, boxYLen, leftPlayers[i], leftDir);
			if(leftPlayers[i].pType == upCB)
				leftScore += compOneDirBestPlayerPos(p, upBnd, upLeftRng, boxXLen, boxYLen, leftPlayers[i], leftDir);
		}


		double rightScore = 0;
		for(unsigned int i = 0; i < rightPlayers.size(); ++i)
		{
			if(rightPlayers[i].pType == lowWR)
				rightScore += compOneDirBestPlayerPos(p, lowBnd, lowLeftRng, boxXLen, boxYLen, rightPlayers[i], rightDir);
			if(rightPlayers[i].pType == lowCB)
				rightScore += compOneDirBestPlayerPos(p, lowBnd, lowRightRng, boxXLen, boxYLen, rightPlayers[i], rightDir);
			if(rightPlayers[i].pType == upWR)
				rightScore += compOneDirBestPlayerPos(p, upBnd, upLeftRng, boxXLen, boxYLen, rightPlayers[i], rightDir);
			if(rightPlayers[i].pType == upCB)
				rightScore += compOneDirBestPlayerPos(p, upBnd, upRightRng, boxXLen, boxYLen, rightPlayers[i], rightDir);
		}

		if(leftScore > rightScore)
		{
			players = leftPlayers;
			dir = leftDir;
		}
		else
		{
			players = rightPlayers;
			dir = rightDir;
		}

	#endif

}


void formation::compForm(vector<double> &weight, vector<double> &feature)
{
//	double boxXLen = 1.95752 * 2.0 / 3.0;
//	double boxYLen = 0.603402 / 2.0;
	double boxXLen = boxXLenToYardLns;
	double boxYLen = boxYLenToYardLns;
	//vector<player> lowPlayers, upPlayers;
	vector<player> leftLowPlayers, leftUpPlayers;
	vector<player> rightLowPlayers, rightUpPlayers;
	for(unsigned int i = 0; i < players.size(); ++i)
	{
		if((players[i].pType == lowWR) || (players[i].pType == lowCB))
		{
			leftLowPlayers.push_back(players[i]);
			rightLowPlayers.push_back(players[i]);
		}
		else if((players[i].pType == upWR) || (players[i].pType == upCB))
		{
			leftUpPlayers.push_back(players[i]);
			rightUpPlayers.push_back(players[i]);
		}

	}

	int lowStartY = p->scrimCnt.y;
	int lowEndY = imgYLen - int(boxYLen * p->yardLnsDist);

	int upStartY = 1;
	int upEndY = p->scrimCnt.y - int(boxYLen * p->yardLnsDist);

	int startX = 1;
	int endX = imgXLen - int(boxXLen * p->yardLnsDist);

	int lowBnd[4] = {lowStartY, lowEndY, startX, endX};
	//compBestPlayerPos(p, lowBnd, boxXLen, boxYLen, lowPlayers);

	int upBnd[4] = { upStartY, upEndY, startX, endX};
	//compBestPlayerPos(p, upBnd, boxXLen, boxYLen, upPlayers);

	//lowPlayers.insert(lowPlayers.end(), upPlayers.begin(), upPlayers.end());

	//players = lowPlayers;

//	vector<player> leftLowPlayers = lowPlayers, leftUpPlayers = upPlayers;
//	vector<player> rightLowPlayers = lowPlayers, rightUpPlayers = upPlayers;

//	struct rect lowRng, upRng;
//	compPlayersRange(lowRng, upRng);
//	lowRange = lowRng;
//	upRange = upRng;

	double leftScore = compOneDirBestPlayerPos(p, lowBnd, boxXLen, boxYLen, weight, leftLowPlayers, leftDir, feature);
	leftScore += compOneDirBestPlayerPos(p, upBnd, boxXLen, boxYLen, weight, leftUpPlayers, leftDir, feature);

	double rightScore = compOneDirBestPlayerPos(p, lowBnd, boxXLen, boxYLen, weight, rightLowPlayers, rightDir, feature);
	rightScore += compOneDirBestPlayerPos(p, upBnd, boxXLen, boxYLen, weight, rightUpPlayers, rightDir, feature);

	//cout << "feature[0] " << feature[0] << endl;
	//double rightScore = NEGINF;
	//double leftScore = NEGINF;
//	leftLowPlayers.insert(leftLowPlayers.end(), leftUpPlayers.begin(), leftUpPlayers.end());
//	players = leftLowPlayers;
//	dir = leftDir;
//	rightLowPlayers.insert(rightLowPlayers.end(), rightUpPlayers.begin(), rightUpPlayers.end());
//	players = rightLowPlayers;
//	dir = rightDir;

//	cout << "leftScore: " << leftScore << endl;
//	cout << "rightScore: " << rightScore << endl;

	if(leftScore > rightScore)
	{
		leftLowPlayers.insert(leftLowPlayers.end(), leftUpPlayers.begin(), leftUpPlayers.end());
		players = leftLowPlayers;
		dir = leftDir;
		totalScore = leftScore;
	}
	else
	{
		rightLowPlayers.insert(rightLowPlayers.end(), rightUpPlayers.begin(), rightUpPlayers.end());
		players = rightLowPlayers;
		dir = rightDir;
		totalScore = rightScore;
	}

}


void formation::compForm(const std::vector<playerBndBox> &topBoxes)
{
//	double boxXLen = 1.95752 * 2.0 / 3.0;
//	double boxYLen = 0.603402 / 2.0;
	//vector<player> lowPlayers, upPlayers;
	vector<player> leftLowPlayers, leftUpPlayers;
	vector<player> rightLowPlayers, rightUpPlayers;
	for(unsigned int i = 0; i < players.size(); ++i)
	{
		if((players[i].pType == lowWR) || (players[i].pType == lowCB))
		{
			player lowP(players[i]);
			leftLowPlayers.push_back(lowP);
			player upP(players[i]);
			rightLowPlayers.push_back(upP);
		}
		else if((players[i].pType == upWR) || (players[i].pType == upCB))
		{
			player lowP(players[i]);
			leftUpPlayers.push_back(players[i]);
			player upP(players[i]);
			rightUpPlayers.push_back(players[i]);
		}

	}

//	vector<player> leftLowPlayers = lowPlayers, leftUpPlayers = upPlayers;
//	vector<player> rightLowPlayers = lowPlayers, rightUpPlayers = upPlayers;

//	double leftScore = compOneDirBestPlayerPos(p, topBoxes, leftLowPlayers, leftDir);
//	leftScore += compOneDirBestPlayerPos(p, topBoxes, leftUpPlayers, leftDir);
//
//	double rightScore = compOneDirBestPlayerPos(p, topBoxes, rightLowPlayers, rightDir);
//	rightScore += compOneDirBestPlayerPos(p, topBoxes, rightUpPlayers, rightDir);

//	struct rect lowRng, upRng;
//	compPlayersRange(lowRng, upRng);
//	lowRange = lowRng;
//	upRange = upRng;

	double leftScore = compOneDirBestPlayerPos2(p, topBoxes, leftLowPlayers, leftDir);
	leftScore += compOneDirBestPlayerPos2(p, topBoxes, leftUpPlayers, leftDir);
	//cout << "leftLowPlayers[i]: " << leftLowPlayers[0].wrCbScore << endl;;

	double rightScore = compOneDirBestPlayerPos2(p, topBoxes, rightLowPlayers, rightDir);
	rightScore += compOneDirBestPlayerPos2(p, topBoxes, rightUpPlayers, rightDir);

	//double rightScore = 0.0;
	if(leftScore > rightScore)
	{
		leftLowPlayers.insert(leftLowPlayers.end(), leftUpPlayers.begin(), leftUpPlayers.end());
		//cout << "leftLowPlayers[i]: " << leftLowPlayers[0].wrCbScore << endl;;
		for(unsigned int i = 0; i < leftLowPlayers.size(); ++i)
		{
			players[i] = leftLowPlayers[i];
//			cout << "leftLowPlayers[i]: " << leftLowPlayers[i].wrCbScore << endl;
//			cout << "players[i]: " << players[i].wrCbScore << endl;
		}
		//players = leftLowPlayers;
		dir = leftDir;
	}
	else
	{
		rightLowPlayers.insert(rightLowPlayers.end(), rightUpPlayers.begin(), rightUpPlayers.end());
		for(unsigned int i = 0; i < rightLowPlayers.size(); ++i)
		{
			players[i] = rightLowPlayers[i];
		}
		//players = rightLowPlayers;
		dir = rightDir;
	}

}

//int lowStartY = p->scrimCnt.y;
//int lowEndY = imgYLen - int(boxYLen * p->yardLnsDist);
//
//int upStartY = 1;
//int upEndY = p->scrimCnt.y - int(boxYLen * p->yardLnsDist);
//
//int startX = 1;
//int endX = imgXLen - int(boxXLen * p->yardLnsDist);
//
//int lowBnd[4] = {lowStartY, lowEndY, startX, endX};
////compBestPlayerPos(p, lowBnd, boxXLen, boxYLen, lowPlayers);
//
//int upBnd[4] = { upStartY, upEndY, startX, endX};

void formation::compPlayersRange(struct rect &lowRng, struct rect &upRng)
{
//	struct rect r = p->scrimLn;
//	cout<< r.a.x << " " <<  r.a.y << endl;
//	cout<< r.b.x << " " <<   r.b.y << endl;
//	cout<< r.c.x << " " <<   r.c.y << endl;
//	cout<< r.d.x << " " <<   r.d.y << endl;

	Point2d vecVpToScrimCnt;
	if(p->vpExist)
	{
		vecVpToScrimCnt = p->scrimCnt - p->vp;
		vecVpToScrimCnt *= 1.0 / norm(vecVpToScrimCnt);
	}
	else
	{
		vecVpToScrimCnt = Point2d(.0, 1.0);
	}

//	Point2d perpVecVpToScrimCnt = Point2d(-1.0 * vecVpToScrimCnt.y, vecVpToScrimCnt.x);
//	if(perpVecVpToScrimCnt.x < 0.0)
//		perpVecVpToScrimCnt = -1.0 * perpVecVpToScrimCnt;


	Point2d lowCnt = (p->scrimLn.a + p->scrimLn.b) * 0.5;
	if(isLosCloserToLowEnd(p))
		lowCnt -= p->yardLnsDist * 0.5 * vecVpToScrimCnt;
//	lowRng.a = Point2d(lowCnt.x - 2.0 * p->yardLnsDist, lowCnt.y - p->yardLnsDist / 3.0);
//	lowRng.d = Point2d(lowCnt.x + 2.0 * p->yardLnsDist, lowCnt.y - p->yardLnsDist / 3.0);

//	lowRng.a = lowCnt - 2.0 * p->yardLnsDist * perpVecVpToScrimCnt;
//	lowRng.d = lowCnt + 2.0 * p->yardLnsDist * perpVecVpToScrimCnt;

	lowRng.a = Point2d(lowCnt.x - 2.0 * p->yardLnsDist, lowCnt.y);
	lowRng.d = Point2d(lowCnt.x + 2.0 * p->yardLnsDist, lowCnt.y);


	lowRng.b.y = imgYLen;
	lowRng.c.y = imgYLen;
	if(p->vpExist)
	{
		lowRng.b.x = (lowRng.a.x - p->vp.x) / (lowRng.a.y - p->vp.y) * (lowRng.b.y - lowRng.a.y) + lowRng.a.x;
		lowRng.c.x = (lowRng.d.x - p->vp.x) / (lowRng.d.y - p->vp.y) * (lowRng.c.y - lowRng.d.y) + lowRng.d.x;
	}
	else
	{
		lowRng.b.x = lowRng.a.x;
		lowRng.c.x = lowRng.d.x;
	}

	Point2d upCnt = (p->scrimLn.c + p->scrimLn.d) * 0.5;
	if(!isLosCloserToLowEnd(p))
		upCnt += p->yardLnsDist * 0.5 * vecVpToScrimCnt;
//	upRng.a = Point2d(upCnt.x - 2.0 * p->yardLnsDist, upCnt.y + p->yardLnsDist / 3.0);
//	upRng.b = Point2d(upCnt.x + 2.0 * p->yardLnsDist, upCnt.y + p->yardLnsDist / 3.0);
//	upRng.a = upCnt - 2.0 * p->yardLnsDist * perpVecVpToScrimCnt;
//	upRng.b = upCnt + 2.0 * p->yardLnsDist * perpVecVpToScrimCnt;
	upRng.a = Point2d(upCnt.x - 2.0 * p->yardLnsDist, upCnt.y);
	upRng.b = Point2d(upCnt.x + 2.0 * p->yardLnsDist, upCnt.y);


	upRng.c.y = 0.0;
	upRng.d.y = 0.0;
	if(p->vpExist)
	{
		upRng.d.x = (p->vp.x - upRng.a.x) / (p->vp.y - upRng.a.y) * (upRng.d.y - upRng.a.y) + upRng.a.x;
		upRng.c.x = (p->vp.x - upRng.b.x) / (p->vp.y - upRng.b.y) * (upRng.c.y - upRng.b.y) + upRng.b.x;
	}
	else
	{
		upRng.d.x = upRng.a.x;
		upRng.c.x = upRng.b.x;
	}

}


void formation::compPlayersRange(struct rect &lowLeftRng, struct rect &lowRightRng, struct rect &upLeftRng, struct rect &upRightRng)
{
//	struct rect r = p->scrimLn;
//	cout<< r.a.x << " " <<  r.a.y << endl;
//	cout<< r.b.x << " " <<   r.b.y << endl;
//	cout<< r.c.x << " " <<   r.c.y << endl;
//	cout<< r.d.x << " " <<   r.d.y << endl;

	Point2d vecVpToScrimCnt;
	if(p->vpExist)
	{
		vecVpToScrimCnt = p->scrimCnt - p->vp;
		vecVpToScrimCnt *= 1.0 / norm(vecVpToScrimCnt);
	}
	else
	{
		vecVpToScrimCnt = Point2d(.0, 1.0);
	}

//	Point2d perpVecVpToScrimCnt = Point2d(-1.0 * vecVpToScrimCnt.y, vecVpToScrimCnt.x);
//	if(perpVecVpToScrimCnt.x < 0.0)
//		perpVecVpToScrimCnt = -1.0 * perpVecVpToScrimCnt;


	Point2d lowCnt = (p->scrimLn.a + p->scrimLn.b) * 0.5;
	if(isLosCloserToLowEnd(p))
		lowCnt -= p->yardLnsDist * 0.5 * vecVpToScrimCnt;
//	lowRng.a = Point2d(lowCnt.x - 2.0 * p->yardLnsDist, lowCnt.y - p->yardLnsDist / 3.0);
//	lowRng.d = Point2d(lowCnt.x + 2.0 * p->yardLnsDist, lowCnt.y - p->yardLnsDist / 3.0);

//	lowRng.a = lowCnt - 2.0 * p->yardLnsDist * perpVecVpToScrimCnt;
//	lowRng.d = lowCnt + 2.0 * p->yardLnsDist * perpVecVpToScrimCnt;

	lowLeftRng.a = Point2d(lowCnt.x - 2.0 * p->yardLnsDist, lowCnt.y);
	lowLeftRng.d = Point2d(lowCnt.x, lowCnt.y);


	lowLeftRng.b.y = imgYLen;
	lowLeftRng.c.y = imgYLen;
	if(p->vpExist)
	{
		lowLeftRng.b.x = (lowLeftRng.a.x - p->vp.x) / (lowLeftRng.a.y - p->vp.y) * (lowLeftRng.b.y - lowLeftRng.a.y) + lowLeftRng.a.x;
		lowLeftRng.c.x = (lowLeftRng.d.x - p->vp.x) / (lowLeftRng.d.y - p->vp.y) * (lowLeftRng.c.y - lowLeftRng.d.y) + lowLeftRng.d.x;
	}
	else
	{
		lowLeftRng.b.x = lowLeftRng.a.x;
		lowLeftRng.c.x = lowLeftRng.d.x;
	}


	lowRightRng.a = Point2d(lowCnt.x, lowCnt.y);
	lowRightRng.d = Point2d(lowCnt.x + 2.0 * p->yardLnsDist, lowCnt.y);


	lowRightRng.b.y = imgYLen;
	lowRightRng.c.y = imgYLen;
	if(p->vpExist)
	{
		lowRightRng.b.x = (lowRightRng.a.x - p->vp.x) / (lowRightRng.a.y - p->vp.y) * (lowRightRng.b.y - lowRightRng.a.y) + lowRightRng.a.x;
		lowRightRng.c.x = (lowRightRng.d.x - p->vp.x) / (lowRightRng.d.y - p->vp.y) * (lowRightRng.c.y - lowRightRng.d.y) + lowRightRng.d.x;
	}
	else
	{
		lowRightRng.b.x = lowRightRng.a.x;
		lowRightRng.c.x = lowRightRng.d.x;
	}


	Point2d upCnt = (p->scrimLn.c + p->scrimLn.d) * 0.5;
	if(!isLosCloserToLowEnd(p))
		upCnt += p->yardLnsDist * 0.5 * vecVpToScrimCnt;
//	upRng.a = Point2d(upCnt.x - 2.0 * p->yardLnsDist, upCnt.y + p->yardLnsDist / 3.0);
//	upRng.b = Point2d(upCnt.x + 2.0 * p->yardLnsDist, upCnt.y + p->yardLnsDist / 3.0);
//	upRng.a = upCnt - 2.0 * p->yardLnsDist * perpVecVpToScrimCnt;
//	upRng.b = upCnt + 2.0 * p->yardLnsDist * perpVecVpToScrimCnt;
	upLeftRng.a = Point2d(upCnt.x - 2.0 * p->yardLnsDist, upCnt.y);
	upLeftRng.b = Point2d(upCnt.x, upCnt.y);


	upLeftRng.c.y = 0.0;
	upLeftRng.d.y = 0.0;
	if(p->vpExist)
	{
		upLeftRng.d.x = (p->vp.x - upLeftRng.a.x) / (p->vp.y - upLeftRng.a.y) * (upLeftRng.d.y - upLeftRng.a.y) + upLeftRng.a.x;
		upLeftRng.c.x = (p->vp.x - upLeftRng.b.x) / (p->vp.y - upLeftRng.b.y) * (upLeftRng.c.y - upLeftRng.b.y) + upLeftRng.b.x;
	}
	else
	{
		upLeftRng.d.x = upLeftRng.a.x;
		upLeftRng.c.x = upLeftRng.b.x;
	}

	upRightRng.a = Point2d(upCnt.x, upCnt.y);
	upRightRng.b = Point2d(upCnt.x + 2.0 * p->yardLnsDist, upCnt.y);


	upRightRng.c.y = 0.0;
	upRightRng.d.y = 0.0;
	if(p->vpExist)
	{
		upRightRng.d.x = (p->vp.x - upRightRng.a.x) / (p->vp.y - upRightRng.a.y) * (upRightRng.d.y - upRightRng.a.y) + upRightRng.a.x;
		upRightRng.c.x = (p->vp.x - upRightRng.b.x) / (p->vp.y - upRightRng.b.y) * (upRightRng.c.y - upRightRng.b.y) + upRightRng.b.x;
	}
	else
	{
		upRightRng.d.x = upRightRng.a.x;
		upRightRng.c.x = upRightRng.b.x;
	}


}

int playerStartPosInFVec(const playerType &pType, int fNumEachPlayer)
{
	int t = -1;
	switch (pType) {
	case lowWR:
		t = 0;
		break;
	case lowCB:
		t = 1 * fNumEachPlayer;
		break;
	case upWR:
		t = 2 * fNumEachPlayer;
		break;
	case upCB:
		t = 3 * fNumEachPlayer;
		break;
	default:
		t = -1;
		break;

	}

	return t;
}
