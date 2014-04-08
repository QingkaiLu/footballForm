#ifndef _BLOB_HUDL_H_
#define _BLOB_HUDL_H_

void FindBlobs(const cv::Mat &binary, std::vector < std::vector<cv::Point2i> > &blobs);

std::vector<cv::Rect> filterRect(std::vector<cv::Rect> rectList);

void blob(std::string inputFile, std::string outputFile, std::string imageFile);


#endif
