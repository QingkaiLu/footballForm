#include <iostream>
#include "fieldModel.h"

using namespace std;

fieldModel::fieldModel(int fldModel)
{
	if(fldModel == 1)
	{
		//high school football field model with size: 300 * 159(approximation of 160) feet
		//http://www.sportsknowhow.com/football/field-dimensions/high-school-football-field-dimensions.html
		fieldLength = 795;
		fieldWidth = 1800;
		endZoneWidth = 150;
		yardLinesDist = 75;
		hashLinesDist = 15;
		hashToSideLineDist = 265;
		hashLinesLen = 10;
		hashNumToSideLineDist = 120; //(9 * 3 - 6 / 2) * 5
	}
	else if(fldModel == 2)
	{
		//college football field model with size: 300 * 160 feet
		//http://www.sportsknowhow.com/football/field-dimensions/ncaa-football-field-dimensions.html
		fieldLength = 800;
		fieldWidth = 1800;
		endZoneWidth = 150;
		yardLinesDist = 75;
		hashLinesDist = 15;
		hashToSideLineDist = 300;
		hashLinesLen = 10;
		hashNumToSideLineDist = 120;
	}
	else
	{
		cout << "Wrong field model!" << endl;
	}
}
