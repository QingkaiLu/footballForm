/*
 * player.cpp
 *
 *  Created on: 2013-10-24
 *      Author: qingkai
 */

#include "player.h"
#include "playerBndBox.h"

player::player(playerType pT) {
	// TODO Auto-generated constructor stub
	pType = pT;
	score = NEGINF;
	wrCbScore = NEGINF;
	pBox = new playerBndBox;
}

player::player() {
	// TODO Auto-generated constructor stub
	pType = nonPType;
	score = NEGINF;
	wrCbScore = NEGINF;
	pBox = new playerBndBox;
}

player::player(const player& p)
{
	pType = p.pType;
	score = p.score;
	pBox = new playerBndBox;
	wrCbScore = p.wrCbScore;
	*pBox = *p.pBox;
}

player::~player() {
	// TODO Auto-generated destructor stub
	if(pBox != NULL)
		delete pBox;
}

player player::operator = (const player& p)
{
	pType = p.pType;
	score = p.score;
	//pBox = new playerBndBox;
	*pBox = *p.pBox;
	wrCbScore = p.wrCbScore;

	return *this;
}
