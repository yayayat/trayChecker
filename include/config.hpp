#pragma once

#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <libconfig.h++>

#include "opencv2/opencv.hpp"

struct vec2d {
    int x, y;
};

struct settings {
    int exp;
    vec2d res;
    cv::Point2f ImageCorners[4];
    cv::Point2f ROICorners[4];
    bool streamOn;
    int port;
    int differenceThreshold;
    int frameRate;
    bool recOn;
    const char *outputFolder;
    const char *outputContainer;
    const char *outputCodec;
};

int loadConfig(settings &stngs);