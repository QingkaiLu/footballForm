#ifndef _KNN_H_
#define _KNN_H_

//void myKnn(const std::vector<int> &testData, );

void knnLeavePlayOut(const std::vector<int> &trainTestGames,
		const std::vector<std::vector<playId> > &pIdsAllGames);

//void myKnnLeavePlayOut(const std::vector<int> &trainTestGames,
//		const std::vector<std::vector<playId> > &pIdsAllGames);

void leavePlayOutTestKnn(const std::vector<int> &games);

#endif
