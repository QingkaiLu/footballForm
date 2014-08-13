#include <string>
#include <vector>
#include <fstream>

#include <assert.h>

#include "formTree.h"
//#include "hungarian.h"

using namespace std;
using namespace cv;

formTree::formTree(string formFile)
{
	formName = formFile;
	formBestScore = .0;
	string formFilePath = "formModel/" + formFile + ".form";
	setupFormTree(formFilePath);
}

void formTree::setupFormTree(const string &formFile)
{
	ifstream fin(formFile.c_str());

	if(!fin.is_open())
	{
		cout << "Can't open file " << formFile << endl;
		return;
	}

	fin.seekg(0, ios::end);
	if (fin.tellg() == 0) {
		cout << "Empty file " << formFile << endl;
		return;
	}
	fin.seekg(0, ios::beg);
	string tmp;
	fin >> tmp;
	if(tmp == "parts:")
	{
		while(1)
		{
			fin >> tmp;
//			cout << tmp << endl;
			if(tmp.compare("edges:") == 0)
				break;
			struct part p;
			p.partName = tmp;
			parts.push_back(p);
		}
	}

	if(tmp.compare("edges:") == 0)
	{
		while(!fin.eof())
		{
			string parent, child;
			Point2d edge;
			fin >> tmp >> parent >> child >> edge.x >> edge.y >> tmp;
//			cout << tmp << parent << " " << child << " "<< edge.x << " "<< edge.y << " "<< endl;
			if(tmp.empty())
				break;
			for(unsigned int i = 0; i < parts.size(); ++i)
			{
				if(parts[i].partName.compare(parent) == 0)
				{
					for(unsigned int j = 0; j < parts.size(); ++j)
						if(parts[j].partName.compare(child) == 0)
						{
							parts[i].children.push_back(&parts[j]);
							parts[j].parent = &parts[i];
//							parts[j].relLocToPar = edge;
							parts[j].relLocToPar = edge;
//							cout << parts[j].partName << " " << parts[j].relLocToPar.x
//									<< " " << parts[j].relLocToPar.y << endl;
						}
				}
			}
		}
	}
//	while(!fin.eof())
//	{
//		if(tmp.empty())
//			break;
//	}
	fin.close();
}

formTree::formTree(string formFile, direction offSide)
{
	string dir;
	if(offSide == leftDir)
		dir = "Left";
	else if(offSide == rightDir)
		dir = "Right";
	else
	{
		cout << "Wrong direction in detectForms()." << endl;
		return;
	}
	formName = formFile + dir;
	formBestScore = .0;
	string formFilePath = "formModel/" + formFile + "Right.form";
	setupFormTree(formFilePath, offSide);
}

void formTree::setupFormTree(const string &formFile, direction offSide)
{
	ifstream fin(formFile.c_str());

	if(!fin.is_open())
	{
		cout << "Can't open file " << formFile << endl;
		return;
	}

	fin.seekg(0, ios::end);
	if (fin.tellg() == 0) {
		cout << "Empty file " << formFile << endl;
		return;
	}
	fin.seekg(0, ios::beg);
	string tmp;
	fin >> tmp;
	if(tmp == "parts:")
	{
		while(1)
		{
			fin >> tmp;
//			cout << tmp << endl;
			if(tmp.compare("edges:") == 0)
				break;
			struct part p;
			p.partName = tmp;
			parts.push_back(p);
		}
	}

	if(tmp.compare("edges:") == 0)
	{
		while(!fin.eof())
		{
			string parent, child;
			Point2d edge;
			fin >> tmp >> parent >> child >> edge.x >> edge.y >> tmp;
//			cout << tmp << parent << " " << child << " "<< edge.x << " "<< edge.y << " "<< endl;
			if(tmp.empty())
				break;
			for(unsigned int i = 0; i < parts.size(); ++i)
			{
				if(parts[i].partName.compare(parent) == 0)
				{
					for(unsigned int j = 0; j < parts.size(); ++j)
						if(parts[j].partName.compare(child) == 0)
						{
							parts[i].children.push_back(&parts[j]);
							parts[j].parent = &parts[i];
//							parts[j].relLocToPar = edge;
							if(offSide == rightDir)
								parts[j].relLocToPar = edge;
							else if(offSide == leftDir)
							{
								edge.x *= -1.0;
								parts[j].relLocToPar = edge;
							}
//							cout << parts[j].partName << " " << parts[j].relLocToPar.x
//									<< " " << parts[j].relLocToPar.y << endl;
						}
				}
			}
		}
	}
//	while(!fin.eof())
//	{
//		if(tmp.empty())
//			break;
//	}
	fin.close();
}



void formTree::setupPartsLocSetStarModel(const Point2d &rectLosCnt,
		const vector<Point2d> &pLocSetFld, const vector<double> &score)
{
	for(unsigned int i = 0; i < parts.size(); ++i)
	{
		if(parts[i].partName.compare("OL") == 0)
		{
			parts[i].location = rectLosCnt;
			parts[i].appScore = .0;
//			cout << "OL" << endl;
		}
		else
		{
			parts[i].locSet = pLocSetFld;
			parts[i].locSetAppScore = score;
		}
	}
}

void formTree::findBestFormStarModel()
{
	if(parts[0].partName.compare("OL") != 0)
	{
		cout << "OL is not root of tree. " << endl;
		return;
	}
	for(unsigned int i = 1; i < parts.size(); ++i)
	{
		parts[i].score = NEGINF;
		for(unsigned int j = 0; j < parts[i].locSet.size(); ++j)
		{
//			double apScore = parts[i].locSetAppScore[j];
			double apScore = .0;
			Point2d vecFromPar = parts[i].locSet[j] - parts[0].location;
			//convert from pixel distance to feet(/5), then to yard(/3)
			vecFromPar *= 1.0 / 15.0;
			double spScore = -1.0 * norm(vecFromPar - parts[i].relLocToPar);
			double totalScore = apScore + spScore;
			if(totalScore >= parts[i].score)
			{
				parts[i].score = totalScore;
				parts[i].appScore = apScore;
				parts[i].spaScore = spScore;
				parts[i].location = parts[i].locSet[j];

			}
		}

		formBestScore += parts[i].score;
	}
	cout << formName << " " << formBestScore << endl;
}

void formTree::setupPartsLocSetHungarian(const Point2d &rectLosCnt,
		const vector<Point2d> &pLocSetFld)
{
	for(unsigned int i = 0; i < parts.size(); ++i)
	{
		if(parts[i].partName.compare("OL") == 0)
		{
			parts[i].location = rectLosCnt;
			parts[i].appScore = .0;
			break;
		}
	}
	partsLocSet = pLocSetFld;

}

void formTree::setupPartsLocSetHungarian(const std::vector<cv::Point2d> &olLocSet,
		const std::vector<cv::Point2d> &pLocSetFld)
{
	for(unsigned int i = 0; i < parts.size(); ++i)
	{
		if(parts[i].partName.compare("OL") == 0)
		{
			parts[i].locSet = olLocSet;
			parts[i].appScore = .0;
			break;
		}
	}
	partsLocSet = pLocSetFld;
}

void formTree::getScoreMat(string outputFile)
{
	if(parts[0].partName.compare("OL") != 0)
	{
		cout << "1st part is not OL. " << endl;
		return;
	}

	vector<vector<double> > scoreMat;

	double minScore = INF;
	for(unsigned int i = 1; i < parts.size(); ++i)
	{
		vector<double> partScores;
		for(unsigned int j = 0; j < partsLocSet.size(); ++j)
		{
			Point2d vecFromPar = partsLocSet[j] - parts[0].location;
			//convert from pixel distance to feet(/5), then to yard(/3)
			vecFromPar *= 1.0 / 15.0;
			double spScore = -1.0 * norm(vecFromPar - parts[i].relLocToPar);
			partScores.push_back(spScore);
			if(spScore < minScore)
				minScore = spScore;
		}
		scoreMat.push_back(partScores);
	}
//	printScoreMat(scoreMat);
//	cout << endl;
	makeScoreMatSquare(scoreMat, minScore);
	outputFile = outputFile + ".scoreMat";
	printMat(scoreMat, outputFile);
//	cout << endl;

}

void formTree::getScoreMat(std::string outputFile, double &minScore)
{
	if(parts[0].partName.compare("OL") != 0)
	{
		cout << "1st part is not OL. " << endl;
		return;
	}

	vector<vector<double> > scoreMat;

	minScore = INF;
	for(unsigned int i = 1; i < parts.size(); ++i)
	{
		vector<double> partScores;
		for(unsigned int j = 0; j < partsLocSet.size(); ++j)
		{
			Point2d vecFromPar = partsLocSet[j] - parts[0].location;
			//convert from pixel distance to feet(/5), then to yard(/3)
			vecFromPar *= 1.0 / 15.0;
			double spScore = -1.0 * norm(vecFromPar - parts[i].relLocToPar);
			partScores.push_back(spScore);
			if(spScore < minScore)
				minScore = spScore;
		}
		scoreMat.push_back(partScores);
	}
//	printScoreMat(scoreMat);
//	cout << endl;
	makeScoreMatSquare(scoreMat, minScore);
	outputFile = outputFile + ".scoreMat";
	printMat(scoreMat, outputFile);
}

void formTree::findBestFormHungarian(string outputFile)
{
	if(parts[0].partName.compare("OL") != 0)
	{
		cout << "OL is not root of tree. " << endl;
		return;
	}
//	outputFile = outputFile + ".match";
	int matSize = max(parts.size() - 1, partsLocSet.size());
	vector<vector<double> > matchMat;
	readMat(matSize, outputFile + ".match", matchMat);
	printMat(matchMat);
	cout << endl;
	vector<vector<double> > scoreMat;
	readMat(matSize, outputFile + ".scoreMat", scoreMat);
	printMat(scoreMat);
	cout << endl;


	for(unsigned int i = 1; i < parts.size(); ++i)
	{
		for(unsigned int j = 0; j < matchMat[i - 1].size(); ++j)
		{
			if(matchMat[i - 1][j] == 1)
			{
				parts[i].score = scoreMat[i - 1][j];
				if(j < partsLocSet.size())
					parts[i].location = partsLocSet[j];
				else
				{
					cout << "no location for this player. " << endl;
					parts[i].location = Point2d(0, 0);
				}
			}
		}

		formBestScore += parts[i].score;
	}
	cout << formName << " " << formBestScore << endl;


}


void formTree::getScoreMat(std::vector<std::vector<double> > &scoreMat)
{
	if(parts[0].partName.compare("OL") != 0)
	{
		cout << "1st part is not OL. " << endl;
		return;
	}

	double minScore = INF;
	for(unsigned int i = 1; i < parts.size(); ++i)
	{
		vector<double> partScores;
		for(unsigned int j = 0; j < partsLocSet.size(); ++j)
		{
			Point2d vecFromPar = partsLocSet[j] - parts[0].location;
			//convert from pixel distance to feet(/5), then to yard(/3)
			vecFromPar *= 1.0 / 15.0;
			double spScore = -1.0 * norm(vecFromPar - parts[i].relLocToPar);
			partScores.push_back(spScore);
			if(spScore < minScore)
				minScore = spScore;
		}
		scoreMat.push_back(partScores);
	}

	makeScoreMatSquare(scoreMat, minScore);

}

void formTree::findBestFormHungarian()
{
	if(parts[0].partName.compare("OL") != 0)
	{
		cout << "OL is not root of tree. " << endl;
		return;
	}

	int matSize = max(parts.size() - 1, partsLocSet.size());
	formBestScore = NEGINF;
	Point2d olFinalPos;
	for(unsigned int k = 0; k < parts[0].locSet.size(); ++k)
	{
		parts[0].location = parts[0].locSet[k];
		string scoreFile = "Hungarian/tmp";
		double minScore;
		getScoreMat(scoreFile, minScore);
		vector<vector<double> > scoreMat;
		readMat(matSize, scoreFile + ".scoreMat", scoreMat);
		if(system("./../hungarian/hungarian -v 0 -i Hungarian/tmp.scoreMat"))
		{
			cout << "Can not run hungarian." << endl;;
			return;
		}
		vector<vector<double> > matchMat;
		readMat(matSize, "matchResult", matchMat);
		double score = 0;
		readMatchScore("matchScore", score);
		//compensate the score of virtual nodes of Hungarian
		score -= (minScore - 1) * abs(int(parts.size() - 1 - partsLocSet.size()));
		cout << "score: " << score << endl;
		if(score >= formBestScore)
		{
			formBestScore = score;
			olFinalPos = parts[0].location;
			for(unsigned int i = 1; i < parts.size(); ++i)
			{
				for(unsigned int j = 0; j < matchMat[i - 1].size(); ++j)
				{
					if(matchMat[i - 1][j] == 1)
					{
						parts[i].score = scoreMat[i - 1][j];
						if(j < partsLocSet.size())
							parts[i].location = partsLocSet[j];
						else
						{
							cout << "no location for this player. " << endl;
							parts[i].location = Point2d(0, 0);
						}
					}
				}
				//formBestScore += parts[i].score;
			}
		}
	}
	parts[0].location = olFinalPos;
	cout << formName << " " << formBestScore << endl;

}


void formTree::plotFormOrigImg(Mat &img, const Mat &fldToOrgHMat)
{
	vector<Point2d> partsLoc, partsLocOrig;
	for(unsigned int i = 0; i < parts.size(); ++i)
		partsLoc.push_back(parts[i].location);
	perspectiveTransform(partsLoc, partsLocOrig, fldToOrgHMat);
	int fontFace = 0;
	double fontScale = 1;
	int thickness = 2;

	for(unsigned int i = 0; i < parts.size(); ++i)
	{
		cout << parts[i].partName << endl;
		putText(img, parts[i].partName, partsLocOrig[i], fontFace, fontScale, CV_RGB(0, 0, 255), thickness,8);

		int len = 5;
		line(img, partsLocOrig[i] - Point2d(len, 0), partsLocOrig[i] + Point2d(len, 0), CV_RGB(0, 255, 0),2,8,0);
		line(img, partsLocOrig[i] - Point2d(0, len), partsLocOrig[i] + Point2d(0, len), CV_RGB(0, 255, 0),2,8,0);

//		double pixShiftNum = 15;
//		double score = double(int(parts[i].score * 100)) / 100.0;
//		ostringstream convertScore;
//		convertScore << score;
//		string scoreStr = convertScore.str();
//		putText(img, scoreStr, partsLocOrig[i] - Point2d(0, pixShiftNum), fontFace, fontScale, CV_RGB(0, 0, 255), thickness,8);
//
//		double apScore = double(int(parts[i].appScore * 100)) / 100.0;
//		ostringstream convertApScore;
//		convertApScore << apScore;
//		string apScoreStr = convertApScore.str();
//		apScoreStr = "a: " + apScoreStr;
//		putText(img, apScoreStr, partsLocOrig[i] - Point2d(0, pixShiftNum * 2.0), fontFace, fontScale, CV_RGB(0, 0, 255), thickness,8);
//
//		double spScore = double(int(parts[i].spaScore * 100)) / 100.0;
//		ostringstream convertSpScore;
//		convertSpScore << spScore;
//		string spScoreStr = convertSpScore.str();
//		spScoreStr = "s: " + spScoreStr;
//		putText(img, spScoreStr, partsLocOrig[i] - Point2d(0, pixShiftNum * 3.0), fontFace, fontScale, CV_RGB(0, 0, 255), thickness,8);
	}

	putText(img, formName, Point2d(10, 30), fontFace, fontScale, CV_RGB(0, 0, 255), thickness,8);

	double score = double(int(formBestScore * 100)) / 100.0;
	ostringstream convertScore;
	convertScore << score;
	string scoreStr = convertScore.str();
	putText(img, scoreStr, Point2d(10, 55), fontFace, fontScale, CV_RGB(0, 0, 255), thickness,8);

}

void makeScoreMatSquare(vector<vector<double> > &scoreMat, double minScore)
{
	minScore -= 1;
	if(scoreMat.empty() || scoreMat[0].empty())
	{
		cout << "Empty score matrix." << endl;
		return;
	}

	int sizeDif = scoreMat[0].size() - scoreMat.size();
	if(sizeDif == 0)
		return;
	if(sizeDif > 0)
	{
		 vector<double> virtualVec(scoreMat[0].size(), minScore);
		 scoreMat.insert(scoreMat.end(), sizeDif, virtualVec);
	}
	else if(sizeDif < 0)
	{
		sizeDif *= -1;
		for(unsigned int i = 0; i < scoreMat.size(); ++i)
			scoreMat[i].insert(scoreMat[i].end(), sizeDif, minScore);
	}
}

void printMat(const std::vector<std::vector<double> > &scoreMat)
{
	for(unsigned int i = 0; i < scoreMat.size(); ++i)
	{
		for(unsigned int j = 0; j < scoreMat[i].size(); ++j)
			cout << scoreMat[i][j] << " ";
		cout << endl;
	}

}

void printMat(const std::vector<std::vector<double> > &scoreMat, std::string outputPath)
{
	ofstream fout(outputPath.c_str());
	for(unsigned int i = 0; i < scoreMat.size(); ++i)
	{
		for(unsigned int j = 0; j < scoreMat[i].size(); ++j)
			fout << scoreMat[i][j] << " ";
		fout << endl;
	}
	fout.close();
}

void readMat(int n, string filePath, std::vector<std::vector<double> > &m)
{
	ifstream fin(filePath.c_str());

	if(!fin.is_open())
	{
		cout << "Can't open file " << filePath << endl;
		return;
	}

	fin.seekg(0, ios::end);
	if (fin.tellg() == 0) {
		cout << "Empty file " << filePath << endl;
		return;
	}
	fin.seekg(0, ios::beg);

	for(int i = 0; i < n; ++i)
	{
		vector<double> v;
		for(int j = 0; j < n; ++j)
		{
			double t;
			fin >> t;
//			cout << t << " ";
			v.push_back(t);
		}
//		cout << endl;
		m.push_back(v);
	}
	fin.close();

}

void readMatchScore(std::string filePath, double &score)
{
	ifstream fin(filePath.c_str());

	if(!fin.is_open())
	{
		cout << "Can't open file " << filePath << endl;
		return;
	}

	fin.seekg(0, ios::end);
	if (fin.tellg() == 0) {
		cout << "Empty file " << filePath << endl;
		return;
	}
	fin.seekg(0, ios::beg);
	fin >> score;
	fin.close();
}
