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
    (IN1) (IN0) (OUT1)(OUT0)
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
pthread_mutex_t mutex_capture = PTHREAD_MUTEX_INITIALIZER;
int LP5012;
int level = 0;

using namespace std;
using namespace cv;

int main(int argc, char *argv[]) {
    if (init()) {
        printf(
            "Initialization failed! Check video device accessibility or try "
            "sudo\n");
        return -1;
    }

    for (;;) {
        sleep(1);
    }
    return 0;
}