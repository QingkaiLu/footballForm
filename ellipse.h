#ifndef _ELLIPSE_H_
#define _ELLIPSE_H_

/// Function header
void thresh_callback(std::string fileName, const cv::Mat &src_gray, cv::Mat &img, std::vector<cv::RotatedRect> &ellipses);

void ellipse(std::string inputFileName, std::string outputFileName, std::vector<cv::RotatedRect> &ellipses);

#endif
