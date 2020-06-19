#include "MJPEGWriter.hpp"
#include "init.hpp"

MJPEGWriter MJPEGStream;
VideoCapture cap;
Mat background, raw, frame, medFilter, difference;
Rect roi;

using namespace std;
using namespace cv;

int main(int argc, char *argv[]) {
  if (init()) {
    cout << "Initialization failed! Are you super user?" << endl;
    return -1;
  }
  for (;;) {
    // wait for a new frame from camera and store it into 'raw'
    cap.read(raw);
    if (raw.empty()) {
      cerr << "ERROR! blank frame grabbed\n";
      continue;
    }
    resize(raw, raw, Size(raw.cols * 0.5, raw.rows * 0.5), 0, 0, INTER_LINEAR);
    cvtColor(raw, frame, COLOR_RGB2GRAY);
    medianBlur(frame, medFilter, 15);
    subtract(background, medFilter, difference);
    rectangle(difference, roi, Scalar(0), FILLED, LINE_8);
    rectangle(raw, roi, Scalar(0, 0, 255), 1, LINE_8);
    Mat buf = Mat::zeros(Size(320, 240), raw.type());
    Mat Out = Mat::zeros(Size(640, 480), raw.type());
    raw.copyTo(Out(Rect(0, 0, 320, 240)));
    cvtColor(background, buf, cv::COLOR_GRAY2RGB);
    buf.copyTo(Out(Rect(320, 0, 320, 240)));
    cvtColor(medFilter, buf, cv::COLOR_GRAY2RGB);
    buf.copyTo(Out(Rect(0, 240, 320, 240)));
    cvtColor(difference, buf, cv::COLOR_GRAY2RGB);
    buf.copyTo(Out(Rect(320, 240, 320, 240)));
    MJPEGStream.write(Out);
  }
  return 0;
}