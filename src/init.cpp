#include "init.hpp"
#include "MJPEGWriter.hpp"

using namespace std;
using namespace cv;

extern MJPEGWriter MJPEGStream;
extern VideoCapture cap;
extern Mat background, raw, frame, medFilter, difference;
extern Rect roi;

static void on_close(int signal) {
  MJPEGStream.stop();
  // video.release();
  cout << endl << "exitting program" << endl;
  exit(EXIT_SUCCESS);
}

int init() {
  MJPEGStream = MJPEGWriter(7777);

  v4l2_control c;
  int descriptor = v4l2_open("/dev/video0", O_RDWR);

  cout << "KFC tray recognition system v.1.0\n"
          "by Dmitry Borisov aka yayayat 2020\n";

  //--- INITIALIZE VIDEO DEVICE

  cout << "Setting exposure time: ";
  c.id = V4L2_CID_EXPOSURE_ABSOLUTE;
  c.value = 5000;
  if (v4l2_ioctl(descriptor, VIDIOC_S_CTRL, &c)) {
    cout << "error" << endl;
    return -1;
  } else
    cout << "done" << endl;
  // manual exposure control
  cout << "Setting manual exposure mode: ";
  c.id = V4L2_CID_EXPOSURE_AUTO;
  c.value = V4L2_EXPOSURE_MANUAL;
  if (v4l2_ioctl(descriptor, VIDIOC_S_CTRL, &c)) {
    cout << "error" << endl;
    return -1;
  } else
    cout << "done" << endl;

  //--- INITIALIZE VIDEOCAPTURE

  int deviceID = 0;         // 0 = open default camera
  int apiID = cv::CAP_V4L2; // 0 = autodetect default API
  cap.open(deviceID + apiID);
  if (!cap.isOpened()) {
    cerr << "ERROR! Unable to open camera\n";
    return -1;
  }

  sleep(1);

  cout << "Start grabbing" << endl;
  cap.read(raw);
  cout << "Camera resolution is \"" << raw.cols << " x " << raw.rows << "\""
       << endl;
  resize(raw, background, cv::Size(raw.cols * 0.5, raw.rows * 0.5), 0, 0,
         INTER_LINEAR);
  cout << "Scaled resolution is \"" << background.cols << " x "
       << background.rows << "\"" << endl;

  cvtColor(background, background, cv::COLOR_RGB2GRAY);
  medianBlur(background, background, 15);

  roi = Rect(100, 30, background.cols - 100 * 2, background.rows - 30 * 2);

  sleep(1);

  MJPEGStream.write(background);
  MJPEGStream.start();

  cout << "Streaming MJPEG @ 7777 port" << endl;

  signal(SIGINT, on_close);
  return 0;
  // //--- GRAB AND WRITE LOOP
}