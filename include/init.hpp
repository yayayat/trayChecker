#pragma once

#include <signal.h>

#include "opencv2/opencv.hpp"

#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/photo.hpp>
#include <opencv2/video.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/videoio/videoio_c.h>
#include <stdio.h>
#include <string>
#include <unistd.h>

#include <fcntl.h>
#include <libv4l2.h>
#include <linux/videodev2.h>
#include <wiringPi.h>

#include <sys/ioctl.h>

#include "MJPEGWriter.hpp"
#include "config.hpp"

int init();
int gpioInit();