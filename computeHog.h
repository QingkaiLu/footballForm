#ifndef _COMPUTE_HOG_H_
#define _COMPUTE_HOG_H_

cv::Mat get_hogdescriptor_visual_image(cv::Mat& origImg,
                                   std::vector<float>& descriptorValues,
                                   cv::Size winSize,
                                   cv::Size cellSize,
                                   int scaleFactor,
                                   double viz_factor);

#endif
