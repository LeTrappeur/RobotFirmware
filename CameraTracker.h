#ifndef CAMERATRACKER_H
#define CAMERATRACKER_H

#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>
#include <stdio.h>

//values are set for a blue color in a typical room envir.
const int H_MIN = 54;
const int H_MAX = 118;
const int S_MIN = 70;
const int S_MAX = 134;
const int V_MIN = 40;
const int V_MAX = 108;
//default capture width and height
const int FRAME_WIDTH = 640;
const int FRAME_HEIGHT = 480;
// default movement borders
const int MIN_MOVEMENT_CENTER=250;
const int MAX_MOVEMENT_CENTER=390;
//max number of objects to be detected in frame
const int MAX_NUM_OBJECTS=50;
//minimum and maximum object area
const int MIN_OBJECT_AREA = 20*20;
const int MAX_OBJECT_AREA = FRAME_HEIGHT*FRAME_WIDTH/1.5;

class CameraTracker
{
    public:
        CameraTracker();
        ~CameraTracker();

        void run();
    protected:
    private:
        void morphOps(cv::Mat& thresh);
        void trackFilteredObject(int& x, int& y, cv::Mat threshold, cv::Mat& cameraFeed);
};

#endif // CAMERATRACKER_H
