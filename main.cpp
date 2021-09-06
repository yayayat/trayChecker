/*  TERMINAL (LEGACY PROTO)
               ------------- ---------  |
               |(x) (x) (x)| |(x) (x)|  |
               |___________| |_______|  |
GPIO:           17  27  22    GND +12   |
Wpi:            (0) (2) (3)             |
________________________________________|

             DSUB
(12V)  (12V)       (GND) (GND)
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
vector<Mat> lastFrames;
int LP5012;
int level = 0;

using namespace std;
using namespace cv;

int main(int argc, char *argv[]) {
  if (init()) {
    printf("Initialization failed! Check video device accessibility or try "
           "sudo\n");
    return -1;
  }

  for (;;) {
    // wait for a new frame from camera and store it into 'raw'
    cap.read(raw);
    if (raw.empty()) {
      // printf("ERROR! blank frame grabbed\n"
      //        "Reloading video..");
      // cap.open("out/9.mov");
      printf("done\n");
      continue;
    }

    // outputVideo << raw;

    // processing
    warpPerspective(raw, transformed, M, raw.size());
    resize(transformed, transformed, Size(cfg.res.x, cfg.res.y), 0, 0,
           INTER_LINEAR);
    cvtColor(transformed, frame, COLOR_RGB2GRAY);
    medianBlur(frame, medFilter, 15);
    absdiff(background, medFilter, difference);
    threshold(difference, difference, 20, 255, THRESH_BINARY);
    Scalar mn = mean(difference);
    level = mn[0] * 64;

    // converting difference matrix to redchannel
    Mat difChannels[3];
    cvtColor(difference, difference, COLOR_GRAY2RGB);
    split(difference, difChannels);
    difChannels[1] = Mat::zeros(difference.rows, difference.cols,
                                CV_8UC1); // green channel is set to 0
    difChannels[2] = Mat::zeros(difference.rows, difference.cols,
                                CV_8UC1); // blue channel is set to 0
    merge(difChannels, 3, difference);

    // output monitor
    Mat buf = Mat::zeros(Size(cfg.res.x, cfg.res.y), transformed.type());
    Mat Out = Mat::zeros(Size(cfg.res.x * 2 + raw.cols, raw.rows),
                         transformed.type());

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