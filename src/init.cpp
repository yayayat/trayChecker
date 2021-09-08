#include "init.hpp"

#include "MJPEGWriter.hpp"
#include "config.hpp"

using namespace std;
using namespace cv;

extern MJPEGWriter MJPEGStream;
extern VideoCapture cap;
extern VideoWriter outputVideo;
extern Mat background, raw, transformed, frame, medFilter, difference;
extern Mat M;
extern Rect roi;
extern settings cfg;
extern int LP5012;
extern int level;

static void on_close(int signal) {
    MJPEGStream.stop();
    // outputVideo.release();
    wiringPiI2CWriteReg8(LP5012, 0x17, 0xFF);
    cout << endl
         << "exitting program" << endl;
    exit(EXIT_SUCCESS);
}

int init() {
    videoInit();
    cap.read(raw);
    printf("Camera resolution is \"%d x %d\"\n", raw.cols, raw.rows);

    cfg.ImageCorners[0] = cvPoint(0, 0);
    cfg.ImageCorners[1] = cvPoint(raw.cols, 0);
    cfg.ImageCorners[2] = cvPoint(raw.cols, raw.rows);
    cfg.ImageCorners[3] = cvPoint(0, raw.rows);

    M = getPerspectiveTransform(cfg.ROICorners, cfg.ImageCorners);
    printf("Scaled resolution is \"%d x %d\"\n", cfg.res.x, cfg.res.y);
    sleep(1);
    captureBackground();
    MJPEGStream.write(background);
    MJPEGStream.start();

    printf("Streaming MJPEG @ %d port\n", cfg.port);
    return 0;
}

int LEDS[7] = {0x0C, 0x0F, 0x0E, 0x11, 0x12, 0x14, 0x15};
int LEDR[4] = {0x15, 0x12, 0x0E, 0x0C};

//   \   9     8     7     6   /
//    ╰───────────────────────╯
//     (IN1) (IN0) (OUT1)(OUT0)
// GPIO: 17    27    5     6
// Wpi:  0     2     21    22

static void *blink(void *a) {
    int inState, inStatelast, outState;
    for (;;) {
        inState = digitalRead(2) + (digitalRead(0) << 1);
        if (inState != inStatelast) {
            switch (inState) {
                case 0:
                    outState = 3;
                    break;
                case 1:
                    captureBackground();
                    outState = 2;
                    break;
                case 2:
                    cout << level << endl;
                    outState = level > cfg.differenceThreshold ? 2 : 1;
                    break;
                case 3:
                    outState = 0;
                    break;
            }
            digitalWrite(22, outState & 0x1);
            digitalWrite(21, outState & 0x2);
            inStatelast = inState;
            // cout << level << endl;
        }
        usleep(100000);

        // for (int i = 0; i < 4; i++) {
        //   int buf = level - 0xFF * i;
        //   wiringPiI2CWriteReg8(LP5012, LEDR[i], (buf > 0) ? min(buf, 0xFF) :
        //   0x00);
        // }
        // digitalWrite(21, digitalRead(2));
        // digitalWrite(22, digitalRead(0));
        // level = (level + 1) % 1024;
    }
}

// static void *blink(void *a) {
//   for (;;) {
//     for (int i = 0; i < 4; i++) {
//       for (int j = 0; j < 256; j++) {
//         wiringPiI2CWriteReg8(LP5012, LEDR[i], j);
//         usleep(10000);
//       }
//       for (int j = 255; j >= 0; j--) {
//         wiringPiI2CWriteReg8(LP5012, LEDR[i], j);
//         usleep(10000);
//       }
//     }
//   }
// }

// static void *blink(void *a) {
//   for (;;) {
//     digitalWrite(21, digitalRead(2));
//     digitalWrite(22, digitalRead(0));
//   }
// }

pthread_t blinker;

//     (IN0) (IN1) (OUT0)(OUT1)
// GPIO: 17    27    5     6
// Wpi:  0     2     21    22

int gpioInit() {
    printf("Setting up GPIO..");
    wiringPiSetup();  // Setup the library
    pinMode(0, INPUT);
    pinMode(2, INPUT);
    pinMode(21, OUTPUT);
    pinMode(22, OUTPUT);
    digitalWrite(21, 0);
    digitalWrite(22, 0);
    LP5012 = wiringPiI2CSetup(0x14);
    wiringPiI2CWriteReg8(LP5012, 0x17, 0xFF);
    wiringPiI2CWriteReg8(LP5012, 0x00, 0b01000000);
    pthread_create(&blinker, NULL, &blink, nullptr);
    printf("done\n");
    return 0;
}

int videoInit() {
    printf(
        "KFC tray recognition system v.1.0\n"
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

    int deviceID = 0;          // 0 = open default camera
    int apiID = cv::CAP_V4L2;  // 0 = autodetect default API

    cap.open(deviceID + apiID);  // Get image from camera
    // cap.open("out/9.mov"); // Get image from videofile

    if (!cap.isOpened()) {
        cerr << "ERROR! Unable to open camera\n";
        return -1;
    }

    // sleep(1);

    return 0;
}

int captureBackground() {
    pthread_mutex_lock(&mutex_capture);
    cap.read(raw);
    warpPerspective(raw, transformed, M, raw.size());
    cout << "New background captured!" << endl;
    resize(transformed, background, cv::Size(cfg.res.x, cfg.res.y), 0, 0,
           INTER_LINEAR);
    cvtColor(background, background, cv::COLOR_RGB2GRAY);
    medianBlur(background, background, 15);
    pthread_mutex_unlock(&mutex_capture);
    return 0;
}
