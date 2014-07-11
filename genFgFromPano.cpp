#include <string>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "play.h"

using namespace std;

void genFg()
{
	unsigned int gamesVidsNum[10] = {132, 141, 173, 149, 135, 156, 152, 159, 128, 146};
	for(unsigned int i = 2; i < 3; ++i)
	{
		ostringstream convertGameId;
		convertGameId << i ;
		string gameIdStr = convertGameId.str();

		if(i < 10)
			gameIdStr = "0" + gameIdStr;

		for(unsigned int j = 1; j <= gamesVidsNum[i - 2]; ++j)
		{
			struct playId pId;
			pId.gameId = i;
			pId.vidId = j;
			cout << i << " " << j << endl;

			int fldModel = 1;
			play p(pId, fldModel);
			p.setUp();
			if(p.trueDir != nonDir)
			{
				cout << "fg" << endl;
				//p.genRectMosFrmFgBgSub();
				p.genOrigMosFrmFgBgSub();
			}

		}
	}


	return;
}

//int main()
//{
//	genFg();
////	Mat M1(2, 2, CV_32F, 1);
////	cout << M1 - 1<< endl;
//	return 1;
//}

//int main(  )
//{
//    Mat drawing1 = Mat::zeros( Size(400,200), CV_8UC1 );
//    Mat drawing2 = Mat::zeros( Size(400,200), CV_8UC1 );
//
//    drawing1(Range(0,drawing1.rows),Range(0,drawing1.cols/2))=250; imshow("drawing1",drawing1);
//    drawing2(Range(100,150),Range(150,350))=255; imshow("drawing2",drawing2);
//
//    Mat res;
//    bitwise_and(drawing1,drawing2,res);     imshow("AND",res);
//    bitwise_or(drawing1,drawing2,res);      imshow("OR",res);
//    bitwise_xor(drawing1,drawing2,res);     imshow("XOR",res);
//    bitwise_not(drawing1,res);              imshow("NOT",res);
//
//
//    waitKey(0);
//    return(0);
//}
