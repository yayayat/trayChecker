#include "init.hpp"
#include "MJPEGWriter.hpp"
#include "config.hpp"

using namespace std;
using namespace cv;

extern MJPEGWriter MJPEGStream;
extern VideoCapture cap;
extern Mat background, raw, transformed, frame, medFilter, difference;
extern Mat M;
extern Rect roi;
extern settings cfg;

static void on_close(int signal) {
  MJPEGStream.stop();
  // video.release();
  cout << endl << "exitting program" << endl;
  exit(EXIT_SUCCESS);
}

int init() {
  printf("KFC tray recognition system v.1.0\n"
         "by Dmitry Borisov aka yayayat 2020\n");

  signal(SIGINT, on_close);

  //--- INITIALIZE GPIO
  if (gpioInit())
    return -1;

  loadConfig(cfg);
  MJPEGStream = MJPEGWriter(cfg.port);
  v4l2_control c;

  //--- INITIALIZE VIDEO DEVICE

  int descriptor = v4l2_open("/dev/video0", O_RDWR);
  printf("Setting exposure time %d..", cfg.exp);
  c.id = V4L2_CID_EXPOSURE_ABSOLUTE;
  c.value = cfg.exp;
  if (v4l2_ioctl(descriptor, VIDIOC_S_CTRL, &c)) {
    printf("error\n");
    return -1;
  } else
    printf("done\n");
  // manual exposure control
  printf("Setting manual exposure mode..");
  c.id = V4L2_CID_EXPOSURE_AUTO;
  c.value = V4L2_EXPOSURE_MANUAL;
  if (v4l2_ioctl(descriptor, VIDIOC_S_CTRL, &c)) {
    printf("error\n");
    return -1;
  } else
    printf("done\n");
  //--- INITIALIZE VIDEOCAPTURE

  int deviceID = 0;         // 0 = open default camera
  int apiID = cv::CAP_V4L2; // 0 = autodetect default API

  cap.open(deviceID + apiID); // Get image from camera
  //cap.open("data/sample.m4v"); // Get image from videofile
  
  if (!cap.isOpened()) {
    cerr << "ERROR! Unable to open camera\n";
    return -1;
  }

  sleep(1);

  printf("Start grabbing\n");
  cap.read(raw);
  // raw = imread("data/sample.png");
  printf("Camera resolution is \"%d x %d\"\n", raw.cols, raw.rows);

  cfg.ImageCorners[0] = cvPoint(0, 0);
  cfg.ImageCorners[1] = cvPoint(raw.cols, 0);
  cfg.ImageCorners[2] = cvPoint(raw.cols, raw.rows);
  cfg.ImageCorners[3] = cvPoint(0, raw.rows);

  M = getPerspectiveTransform(cfg.ROICorners, cfg.ImageCorners);
  warpPerspective(raw, transformed, M, raw.size());
  resize(transformed, background, cv::Size(cfg.res.x, cfg.res.y), 0, 0,
         INTER_LINEAR);
  printf("Scaled resolution is \"%d x %d\"\n", cfg.res.x, cfg.res.y);

  cvtColor(background, background, cv::COLOR_RGB2GRAY);
  medianBlur(background, background, 15);
  sleep(1);

  MJPEGStream.write(background);
  MJPEGStream.start();

  printf("Streaming MJPEG @ %d port\n", cfg.port);
  return 0;
}

int gpioInit() {
  printf("Setting up GPIO..");
  wiringPiSetup();    // Setup the library
  pinMode(0, OUTPUT); // Configure GPIO0 as an output
  pinMode(3, INPUT);  // Configure GPIO0 as an input
  printf("done\n");
  return 0;
}
