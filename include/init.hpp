#pragma once

#include "MJPEGWriter.hpp"
#include "config.hpp"
#include "opencv2/opencv.hpp"
#include <ctime>
#include <fcntl.h>
#include <iostream>
#include <libv4l2.h>
#include <linux/videodev2.h>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/photo.hpp>
#include <opencv2/video.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/videoio/videoio_c.h>
#include <signal.h>
#include <stdio.h>
#include <string>
#include <sys/ioctl.h>
#include <unistd.h>
#include <vector>
#include <wiringPi.h>
#include <wiringPiI2C.h>

int init();
int gpioInit();
int videoInit();
