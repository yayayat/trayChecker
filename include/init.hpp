#pragma once

#include <signal.h>

#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/photo.hpp>
#include <opencv2/video.hpp>
#include <opencv2/videoio.hpp>
#include <stdio.h>
#include <unistd.h>

#include <fcntl.h>
#include <libv4l2.h>
#include <linux/videodev2.h>

#include <sys/ioctl.h>

#include "MJPEGWriter.hpp"
#include "config.hpp"

int init();