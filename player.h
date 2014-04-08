/*
 * player.h
 *
 *  Created on: 2013-10-24
 *      Author: qingkai
 */

#ifndef PLAYER_H_
#define PLAYER_H_

#include "commonStructs.h"
class playerBndBox;

class player {
public:
	player(playerType pT);
	player();
	player(const player& p);
	~player();
	player operator = (const player& p);
	//bool intersectAnotherPlayer(const player& p);
public:
	playerType pType;
	playerBndBox *pBox;
	double score;
	double wrCbScore;//score to evaluate the relationship between wide receiver and corner back

};

#endif /* PLAYER_H_ */
