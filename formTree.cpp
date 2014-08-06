#include <string>
#include <vector>
#include <fstream>

#include "formTree.h"
#include "commonStructs.h"

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
