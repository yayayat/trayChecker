#include "init.hpp"
#include "MJPEGWriter.hpp"
#include "config.hpp"

using namespace std;
using namespace cv;

extern MJPEGWriter MJPEGStream;
extern VideoCapture cap;
extern Mat background, raw, frame, medFilter, difference;
extern Rect roi;
extern settings cfg;

static void on_close(int signal) {
  MJPEGStream.stop();
  // video.release();
  cout << endl << "exitting program" << endl;
  exit(EXIT_SUCCESS);
}

int init() {
  signal(SIGINT, on_close);
  loadConfig(cfg);
  MJPEGStream = MJPEGWriter(cfg.port);
  v4l2_control c;
  int descriptor = v4l2_open("/dev/video0", O_RDWR);

  cout << "KFC tray recognition system v.1.0\n"
          "by Dmitry Borisov aka yayayat 2020\n";

  //--- INITIALIZE VIDEO DEVICE

  cout << "Setting exposure time:";
  c.id = V4L2_CID_EXPOSURE_ABSOLUTE;
  c.value = cfg.exp;
  if (v4l2_ioctl(descriptor, VIDIOC_S_CTRL, &c)) {
    cout << "error" << endl;
    return -1;
  } else
    cout << "\"" << cfg.exp << "\" done" << endl;
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
  resize(raw, background, cv::Size(cfg.res.x, cfg.res.y), 0, 0, INTER_LINEAR);
  cout << "Scaled resolution is \"" << cfg.res.x << " x " << cfg.res.y << "\""
       << endl;

  cvtColor(background, background, cv::COLOR_RGB2GRAY);
  medianBlur(background, background, 15);

  roi = Rect((cfg.res.x - cfg.ROIsize.x) / 2 + cfg.ROIpos.x,
             (cfg.res.y - cfg.ROIsize.y) / 2 + cfg.ROIpos.y, cfg.ROIsize.x,
             cfg.ROIsize.y);
  sleep(1);

  MJPEGStream.write(background);
  MJPEGStream.start();

  cout << "Streaming MJPEG @ " << cfg.port << " port " << endl;
  return 0;
}