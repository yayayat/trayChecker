/*  TERMINAL (LEGACY PROTO)
               ------------- ---------  |
               |(x) (x) (x)| |(x) (x)|  |
               |___________| |_______|  |
GPIO:           17  27  22    GND +12   |
Wpi:            (0) (2) (3)             |
________________________________________|

             DSUB
(VIN+) (VIN+)      (GND) (GND)
╭─────────────────────────────╮
 \ 5     4     3     2     1 /
  \   9     8     7     6   /
   ╰───────────────────────╯
    (IN0) (IN1) (OUT0)(OUT1)
GPIO: 17    27    5     6
Wpi:  0     2     21    22
*/

#include "init.hpp"

MJPEGWriter MJPEGStream;
VideoCapture cap;
Mat background, raw, transformed, frame, medFilter, difference;
Mat M;
Rect roi;
settings cfg;
VideoWriter outputVideo;

using namespace std;
using namespace cv;

int main(int argc, char *argv[]) {
  if (init()) {
    printf("Initialization failed! Check video device accessibility or try "
           "sudo\n");
    return -1;
  }

  // outputVideo.open("out/1.avi", CV_FOURCC('M', 'J', 'P', 'G'), 30.,
  //                  Size(640, 480), true);

  for (;;) {
    // wait for a new frame from camera and store it into 'raw'
    cap.read(raw);
    if (raw.empty()) {
      printf("ERROR! blank frame grabbed\n"
             "Reloading video..");
      cap.open("data/sample.m4v");
      printf("done\n");
      continue;
    }

    // outputVideo << raw;

    warpPerspective(raw, transformed, M, raw.size());
    resize(transformed, transformed, Size(cfg.res.x, cfg.res.y), 0, 0,
           INTER_LINEAR);
    cvtColor(transformed, frame, COLOR_RGB2GRAY);
    medianBlur(frame, medFilter, 15);
    subtract(background, medFilter, difference);
    Mat difChannels[3];

    cvtColor(difference, difference, COLOR_GRAY2RGB);
    split(difference, difChannels);
    difChannels[1] = Mat::zeros(difference.rows, difference.cols,
                                CV_8UC1); // green channel is set to 0
    difChannels[2] = Mat::zeros(difference.rows, difference.cols,
                                CV_8UC1); // red channel is set to 0
    Mat buf = Mat::zeros(Size(cfg.res.x, cfg.res.y), transformed.type());
    Mat Out = Mat::zeros(Size(cfg.res.x * 2 + raw.cols, raw.rows),
                         transformed.type());
    merge(difChannels, 3, difference);

    raw.copyTo(Out(Rect(cfg.res.x * 2, 0, raw.cols, raw.rows)));
    transformed.copyTo(Out(Rect(0, 0, cfg.res.x, cfg.res.y)));
    cvtColor(background, buf, cv::COLOR_GRAY2RGB);
    buf.copyTo(Out(Rect(cfg.res.x, 0, cfg.res.x, cfg.res.y)));
    cvtColor(medFilter, buf, cv::COLOR_GRAY2RGB);
    buf.copyTo(Out(Rect(0, cfg.res.y, cfg.res.x, cfg.res.y)));
    cvtColor(difference, buf, cv::COLOR_BGR2RGB);
    buf.copyTo(Out(Rect(cfg.res.x, cfg.res.y, cfg.res.x, cfg.res.y)));
    MJPEGStream.write(Out);
  }
  return 0;
}

// rectangle(difference, roi, Scalar(0), FILLED, LINE_8);
// fillPoly(raw, &pts, &npts, 1, true, Scalar(0, 0, 255));
// rectangle(raw, roi, Scalar(0, 0, 255), 1, LINE_8);
// polylines(raw, &pts, &npts, 1, true, Scalar(0, 0, 255));

// const Point *pts = (const Point *)contour.data();
// int npts = 4;

// vector<Point> contour;
// contour.push_back(Point(30, 30));
// contour.push_back(Point(90, 30));
// contour.push_back(Point(60, 60));
// contour.push_back(Point(30, 60));