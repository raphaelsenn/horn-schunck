#pragma once

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>

struct Gradients
{
  cv::Mat Ix;           // Derivative of I1 in x-direction
  cv::Mat Iy;           // Derivative of I2 in y-direction
  cv::Mat It;           // Derivative of I1 in time-direction
  cv::Mat I1_smooth;    // Smooth (gauss) version of I1
  cv::Mat I2_smooth;    // Smooth (gauss) version of I2
};

// Calculates the gradients (Ix, Iy for I1 and It) for two images I1 and I2
Gradients calculateImageGradients(const cv::Mat& I1,  // First image (frame t)
                                  const cv::Mat& I2); // Second image (frame t+1)

// Uses lucas-kanade method to calculate optical flow between two images I1 and I2
cv::Mat hornSchunckOpticalFlow(const cv::Mat& I1,     // First image (frame t)
                               const cv::Mat& I2,     // Second image (frame t+1)
                               int maxIter=1,         // Maximum number of iterations
                               float alpha=10.f,      // Regularization constant (larger values -> smoother flow)
                               bool dense = true);    // Dense or sparse optical flow