#include <opencv2/opencv.hpp>
#include <vector>
#include "./horn_schunck.hpp"

// Gaussian blur settings for derivative calculation
constexpr int GAUSSIAN_BLUR_SIZE = 3;
constexpr float GAUSSIAN_BLUR_STD = 0.0f;

// Settings to draw optical flow
constexpr float FLOW_ARROW_SCALE = 1.0f;
constexpr int LINE_THICKNESS = 1;
constexpr int LINE_SHIFT = 0;
constexpr int LINE_TYPE = cv::LINE_AA;
constexpr double TIP_LENGTH = 0.2;

Gradients calculateImageGradients(const cv::Mat& I1, 
                                  const cv::Mat& I2)
{
  Gradients g;
  cv::Mat I1_gray, I2_gray;
  
  // Convert I1 and I2 to 32-bit grayscale images
  cv::cvtColor(I1, I1_gray, cv::COLOR_BGR2GRAY);
  cv::cvtColor(I2, I2_gray, cv::COLOR_BGR2GRAY);
  I1_gray.convertTo(I1_gray, CV_32F);
  I2_gray.convertTo(I2_gray, CV_32F);
  
  // Smooth I1_gray and I2_gray for gradient computation 
  cv::GaussianBlur(I1_gray, 
                   g.I1_smooth, 
                   cv::Size(GAUSSIAN_BLUR_SIZE, GAUSSIAN_BLUR_SIZE), 
                   GAUSSIAN_BLUR_STD);
  cv::GaussianBlur(I2_gray, 
                   g.I2_smooth,
                   cv::Size(GAUSSIAN_BLUR_SIZE, GAUSSIAN_BLUR_SIZE), 
                   GAUSSIAN_BLUR_STD);
  
  // Calculate final gradients
  cv::Sobel(g.I1_smooth, g.Ix, CV_32F, 1, 0);
  cv::Sobel(g.I1_smooth, g.Iy, CV_32F, 0, 1);
  g.It = I2_gray - I1_gray;
  return g;
}

cv::Mat hornSchunckOpticalFlow(const cv::Mat& I1, 
                               const cv::Mat& I2,
                               int maxIter,
                               float alpha,
                               bool dense)
{
  // Calculate image gradients
  Gradients g  = calculateImageGradients(I1, I2);

  // Calculating optical flow
  cv::Mat u = cv::Mat::zeros(I1.size(), CV_32F);
  cv::Mat v = cv::Mat::zeros(I1.size(), CV_32F);

  cv::Mat flow; 
  I1.copyTo(flow);
  
  cv::Mat Ix_pow, Iy_pow;
  cv::pow(g.Ix, 2, Ix_pow);
  cv::pow(g.Iy, 2, Iy_pow);
  const cv::Mat denom = Ix_pow + Iy_pow + std::pow(alpha, 2);

  for (std::size_t k = 0; k < maxIter; k++)
  {
    cv::Mat Ix_mul_u, Iy_mul_v;
    cv::multiply(g.Ix, u, Ix_mul_u); 
    cv::multiply(g.Iy, v, Iy_mul_v); 

    const cv::Mat error = Ix_mul_u + Iy_mul_v + g.It;
    cv::Mat du = g.Ix.mul(error) / denom;
    cv::Mat dv = g.Iy.mul(error) / denom;
    
    u -= du;
    v -= dv;
  }
  
  // Compute polar representation
  cv::Mat magnitude, angle;
  cv::cartToPolar(u, v, magnitude, angle, true);
  
  // Normalize magnitude and convert angle to HSV hue range
  // Inspired by: https://docs.opencv.org/4.x/d4/dee/tutorial_optical_flow.html
  cv::Mat norm;
  cv::normalize(magnitude, norm, 0.0f, 1.0f, cv::NORM_MINMAX);
  angle *= ((1.f / 360.f) * (180.f / 255.f));
  
  // Prepare HSV image
  cv::Mat _hsv[3], hsv, hsv8, flow_bgr;
  _hsv[0] = angle;
  _hsv[1] = cv::Mat::ones(angle.size(), CV_32F);
  _hsv[2] = norm;
  cv::merge(_hsv, 3, hsv);
  hsv.convertTo(hsv8, CV_8U, 255.0);
  cv::cvtColor(hsv8, flow_bgr, cv::COLOR_HSV2BGR);
  
  // Draw dense optical flow 
  if (dense)
    return flow_bgr;
  
  // Draw sparse optical flow
  #pragma omp parallel for collapse(2)
  for (std::size_t i = 0; i < I1.rows ; i++)
  {
    for (std::size_t j = 0; j < I2.cols; j++)
    {
      float dx = u.at<float>(i, j);
      float dy = v.at<float>(i, j);

      float mag = std::hypot(dx, dy);
      if (mag < 0.5) continue;

      cv::Point2f start(j, i);
      cv::Point2f end(j + FLOW_ARROW_SCALE * dx, i + FLOW_ARROW_SCALE * dy);
      cv::arrowedLine(flow, 
                      start, 
                      end, 
                      flow_bgr.at<cv::Vec3b>(i, j), 
                      LINE_THICKNESS, 
                      LINE_TYPE, 
                      LINE_SHIFT, 
                      TIP_LENGTH);
    }
  }
  return flow;
}