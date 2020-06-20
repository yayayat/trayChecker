#include "init.hpp"

MJPEGWriter MJPEGStream;
VideoCapture cap;
Mat background, raw, frame, medFilter, difference;
Rect roi;
settings cfg;

using namespace std;
using namespace cv;

int main(int argc, char *argv[]) {
  if (init()) {
    cerr << "Initialization failed! Are you super user?" << endl;
    return -1;
  }
  cout << "Initialization compited!";
  for (;;) {
    // wait for a new frame from camera and store it into 'raw'
    cap.read(raw);
    if (raw.empty()) {
      cerr << "ERROR! blank frame grabbed\n";
      continue;
    }
    resize(raw, raw, Size(cfg.res.x, cfg.res.y), 0, 0, INTER_LINEAR);
    cvtColor(raw, frame, COLOR_RGB2GRAY);
    medianBlur(frame, medFilter, 15);
    subtract(background, medFilter, difference);
    rectangle(difference, roi, Scalar(0), FILLED, LINE_8);
    rectangle(raw, roi, Scalar(0, 0, 255), 1, LINE_8);
    Mat buf = Mat::zeros(Size(cfg.res.x, cfg.res.y), raw.type());
    Mat Out = Mat::zeros(Size(cfg.res.x * 2, cfg.res.y * 2), raw.type());
    raw.copyTo(Out(Rect(0, 0, cfg.res.x, cfg.res.y)));
    cvtColor(background, buf, cv::COLOR_GRAY2RGB);
    buf.copyTo(Out(Rect(cfg.res.x, 0, cfg.res.x, cfg.res.y)));
    cvtColor(medFilter, buf, cv::COLOR_GRAY2RGB);
    buf.copyTo(Out(Rect(0, cfg.res.y, cfg.res.x, cfg.res.y)));
    cvtColor(difference, buf, cv::COLOR_GRAY2RGB);
    buf.copyTo(Out(Rect(cfg.res.x, cfg.res.y, cfg.res.x, cfg.res.y)));
    MJPEGStream.write(Out);
  }
  return 0;
}