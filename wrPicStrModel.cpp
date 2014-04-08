#include <iostream>
#include <fstream>

#include "wrPicStrModel.h"
#include "play.h"

wrPicStrModel::wrPicStrModel(direction d, string annotPath)
{
	dir = d;
	annotFilePath = annotPath;
	vecScrimToUpRec = Point2d(0.0, 0.0);
	vecScrimToLowRec = Point2d(0.0, 0.0);

	lowRecBndBox.leftUpVert = Point2d(0.0, 0.0);
	lowRecBndBox.xLength = 0.0;
	lowRecBndBox.yLength = 0.0;

	upRecBndBox.leftUpVert = Point2d(0.0, 0.0);
	upRecBndBox.xLength = 0.0;
	upRecBndBox.yLength = 0.0;
	vecLenScrimToUpRec = 0.0;
	vecLenScrimToLowRec = 0.0;
	return;
}

struct playId parseImgPath(string imgPath)
{
	///home/qingkai/workspace/imgAnnotation/annotation/ResultsTrueMos/Game02/right/vid026.jpg
    int resPos = imgPath.find("Game");
    char gameIdx[3];

    imgPath.copy(gameIdx, 2, resPos + 4);
	gameIdx[2] = '\0';
    string gameIdxStr(gameIdx);
	stringstream convertGameIdx(gameIdxStr);

	int gameIndex = -1;
	if ( !(convertGameIdx >> gameIndex) )
		gameIndex = -1;
	//cout<<"gameIndex: "<<gameIndex<<endl;

    string postfix = ".jpg";
    int postfixPos = imgPath.find(postfix);
    char vidNumChar[10];
    imgPath.copy(vidNumChar, 3, (postfixPos - 3));
    vidNumChar[3]='\0';
    string vidNumStr(vidNumChar);
	stringstream convert(vidNumStr);

	int vidNum = -1;
	if ( !(convert >> vidNum) )
		vidNum = -1;
	//cout<<"vidNum: "<<vidNum<<endl;

	struct playId pId;
	pId.gameId = gameIndex;
	pId.vidId = vidNum;

	return pId;
}

bool readAnnotFile(string annotFilePath, vector<bndBox> &lowRecBoxes, vector<bndBox> &upRecBoxes, vector<playId> &pIds)
{

	cout << "reading " << annotFilePath << endl;
	ifstream fin(annotFilePath.c_str());
	if(!fin.is_open())
	{
		cout<<"Can't open file "<<annotFilePath<<endl;
		return false;
	}

	fin.seekg(0, ios::end);
	if (fin.tellg() == 0) {
		cout<<"Empty line file"<<annotFilePath<<endl;
		return false;
	}
	fin.seekg(0, ios::beg);
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
		struct playId pId = parseImgPath(imgPath);
		pIds.push_back(pId);
		fin >> tmp >> tmp >> tmp;
		struct bndBox upRecBox, lowRecBox;
		char comma;
		fin >> lowRecBox.leftUpVert.x >> comma >> lowRecBox.leftUpVert.y >> comma;
		fin >> lowRecBox.xLength >> comma >> lowRecBox.yLength;
		lowRecBoxes.push_back(lowRecBox);
		fin >> tmp >> tmp >> tmp;
		fin >> upRecBox.leftUpVert.x >> comma >> upRecBox.leftUpVert.y >> comma;
		fin >> upRecBox.xLength >> comma >> upRecBox.yLength;
		upRecBoxes.push_back(upRecBox);

	}

	fin.close();
	return true;
}

void wrPicStrModel::constructModel()
{

	vector<bndBox> lowRecBndboxes, upRecBndboxes;
	vector<playId> playIds;
//	vector<double> yardLnDists;
//	vector<Point2d> vecScrimToUpRecs, vecScrimToLowRecs;
//	bndBox lowRecBndBoxes, upRecBndBoxes;
	bool readAnnot = readAnnotFile(annotFilePath, lowRecBndboxes, upRecBndboxes, playIds);
	if(!readAnnot)
	{
		cout << "Can't open annotation file!" << annotFilePath << endl;
		return;
	}
	for(unsigned int i = 0; i < playIds.size(); ++i)
	{
		play p(playIds[i]);
		p.computeYardLnsDist();
		p.getScrimLn();
		Point2d lowBoxCnt = lowRecBndboxes[i].leftUpVert + 0.5 * Point2d(lowRecBndboxes[i].xLength, lowRecBndboxes[i].yLength) ;
		Point2d upBoxCnt = upRecBndboxes[i].leftUpVert + 0.5 * Point2d(upRecBndboxes[i].xLength, upRecBndboxes[i].yLength) ;
		vecScrimToLowRec += (lowBoxCnt - p.scrimCnt) * (1.0 / p.yardLnsDist);
		vecScrimToUpRec += (upBoxCnt - p.scrimCnt) * (1.0 / p.yardLnsDist);

		lowRecBndBox.leftUpVert += lowRecBndboxes[i].leftUpVert;
		lowRecBndBox.xLength += lowRecBndboxes[i].xLength / p.yardLnsDist ;
		lowRecBndBox.yLength += lowRecBndboxes[i].yLength / p.yardLnsDist ;

		upRecBndBox.leftUpVert += upRecBndboxes[i].leftUpVert;
		upRecBndBox.xLength += upRecBndboxes[i].xLength / p.yardLnsDist ;
		upRecBndBox.yLength += upRecBndboxes[i].yLength / p.yardLnsDist ;
		//cout << i << p.yardLnsDist << endl;
//		cout << "gameId: " << playIds[i].gameId << " vidId: " << playIds[i].vidId << endl;
//		cout << p.mos << endl;
//		cout << p.scrimCnt.x << " " <<p.scrimCnt.y << endl;
	}

	vecScrimToLowRec *= (1.0 / playIds.size());
	vecScrimToUpRec *= (1.0 / playIds.size());
	vecLenScrimToLowRec = norm(vecScrimToLowRec);
	vecLenScrimToUpRec = norm(vecScrimToUpRec);

	lowRecBndBox.leftUpVert *= (1.0 / playIds.size());
	lowRecBndBox.xLength *= (1.0 / playIds.size());
	lowRecBndBox.yLength *= (1.0 / playIds.size());

	upRecBndBox.leftUpVert *= (1.0 / playIds.size());
	upRecBndBox.xLength *= (1.0 / playIds.size());
	upRecBndBox.yLength *= (1.0 / playIds.size());

}
