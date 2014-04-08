#include "playerBndBox.h"


playerBndBox::playerBndBox()
{
	longestTrksPath = 0.0;
	dir = nonDir;
}

playerBndBox::playerBndBox(const struct rect &r)
{
	bndRect = r;
	longestTrksPath = 0.0;
	dir = nonDir;
}
void playerBndBox::addTrk(int trkId)
{
	trksInsideIds.push_back(trkId);
}

void playerBndBox::setLongestPath(double len)
{
	longestTrksPath = len;
}

//double playerBndBox::getLongestPath()
//{
//	return longestTrksPath;
//}
void playerBndBox::setDir(direction d)
{
	dir = d;
}
bool playerBndBox::trksIntersect(const playerBndBox &bndBox)
{
	for(unsigned int i = 0; i < trksInsideIds.size(); ++i)
		for(unsigned int j = 0; j < bndBox.trksInsideIds.size(); ++j)
			if(trksInsideIds[i] == bndBox.trksInsideIds[j])
				return true;
	return false;
}

bool compBndBoxes(const playerBndBox &b1, const playerBndBox &b2)
{
	return (b1.longestTrksPath > b2.longestTrksPath);
}

playerBndBox playerBndBox::operator = (const playerBndBox &p)
{
	trksInsideIds = p.trksInsideIds;
	longestTrksPath = p.longestTrksPath;
	bndRect = p.bndRect;
	dir = p.dir;

	return *this;
}

bool playerBndBox::intersectAnotherBndBox(const playerBndBox &pBnd, double ovlpThresh)
{
	vector<int> oneRow(imgXLen, 0);
	vector< vector<int> > imgMat(imgYLen, oneRow);
	int minX = -1, maxX = -1, minY = -1, maxY = -1;
	minX = min(min(bndRect.a.x, bndRect.b.x), min(bndRect.c.x, bndRect.d.x));
	maxX = max(max(bndRect.a.x, bndRect.b.x), max(bndRect.c.x, bndRect.d.x));
	minY = min(min(bndRect.a.y, bndRect.b.y), min(bndRect.c.y, bndRect.d.y));
	maxY = max(max(bndRect.a.y, bndRect.b.y), max(bndRect.c.y, bndRect.d.y));

	if(minX < 1)
		minX = 1;
	if(maxX > imgXLen)
		maxX = imgXLen;

	if(minY < 1)
		minY = 1;
	if(maxY > imgYLen)
		maxY = imgYLen;


	for(int y = minY - 1; y < maxY; ++y)
		for(int x = minX - 1; x < maxX; ++x)
			imgMat[y][x] = 1;

	int minX2 = -1, maxX2 = -1, minY2 = -1, maxY2 = -1;
	minX2 = min(min(pBnd.bndRect.a.x, pBnd.bndRect.b.x), min(pBnd.bndRect.c.x, pBnd.bndRect.d.x));
	maxX2 = max(max(pBnd.bndRect.a.x, pBnd.bndRect.b.x), max(pBnd.bndRect.c.x, pBnd.bndRect.d.x));
	minY2 = min(min(pBnd.bndRect.a.y, pBnd.bndRect.b.y), min(pBnd.bndRect.c.y, pBnd.bndRect.d.y));
	maxY2 = max(max(pBnd.bndRect.a.y, pBnd.bndRect.b.y), max(pBnd.bndRect.c.y, pBnd.bndRect.d.y));

	if(minX2 < 1)
		minX2 = 1;
	if(maxX2 > imgXLen)
		maxX2 = imgXLen;

	if(minY2 < 1)
		minY2 = 1;
	if(maxY2 > imgYLen)
		maxY2 = imgYLen;

	for(int y = minY2 - 1; y < maxY2; ++y)
		for(int x = minX2 - 1; x < maxX2; ++x)
			imgMat[y][x] = 1;

	int onesNum = 0;
	for(int y = 0; y < imgYLen; ++y)
		for(int x = 0; x < imgXLen; ++x)
			if(imgMat[y][x] == 1)
				++onesNum;

	int area1 = (maxX - minX) * (maxY - minY);
	int area2 = (maxX2 - minX2) * (maxY2 - minY2);
	int overlapArea = area1 + area2 - onesNum;
	double overlapRatio = (overlapArea + 0.0) / ((area1 + area2) * 0.5);

	if(overlapRatio >= ovlpThresh)
		return true;

	return false;

}
