#pragma once

#include <fcntl.h>
#include <libv4l2.h>
#include <linux/videodev2.h>
#include <opencv2/videoio/videoio_c.h>
#include <signal.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>

#include <ctime>
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/photo.hpp>
#include <opencv2/video.hpp>
#include <opencv2/videoio.hpp>
#include <string>
#include <vector>

#include "MJPEGWriter.hpp"
#include "config.hpp"
#include "opencv2/opencv.hpp"

extern pthread_mutex_t mutex_capture;
int init();
int gpioInit();
int videoInit();
int captureBackground();
int checkDifference();
int drawOutput();
