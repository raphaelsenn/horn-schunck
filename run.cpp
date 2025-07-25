#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include "./src/horn_schunck.hpp"

int main(int argc, char **argv)
{
  cv::VideoCapture cap;
  // Select video source:
  // No arguments: open the default webcam (device 0).
  // One argument: treat it as a video file path and open that file.
  if (argc == 1)
  {
    cap.open(0);
    if (!cap.isOpened()) 
    { 
      std::cout << "[ERROR] Unable to open camera.\n";
      return -1;
    }
    }
  else
  {
    if (argc != 2) 
    { 
      std::cout << "[ERROR] Usage: ./main <video_file>\n";
      return -1;
    }
    cap.open(argv[1]);
    if (!cap.isOpened())
    {
      std::cout << "[ERROR] Unable to open video.\n";
      return -1;
    }
  }
  cv::Mat I1, I2;
  if (!cap.read(I1))
  {
    std::cout << "[ERROR] Could not grab initial frame.\n";
    return -1;
  }
  int WIDTH = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
  int HEIGHT = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));
  std::cout << "Start running a " << WIDTH << "x" << HEIGHT << " video stream.\n";
  while (true)
  {
    cap.read(I2);
    if (I2.empty())
    {
      std::cout << "[ERROR] Blank frame grabbed.\n";
      break;
    }
    // Calculating optical flow based on lucas-kanade 
    cv::Mat flow = hornSchunckOpticalFlow(I1, I2);
    cv::imshow("Lucas-Kanade method for Optical Flow", flow);
    
    I1 = I2.clone();
    if (cv::waitKey(5) >= 0)
      break;
  }  
  return 0;
}