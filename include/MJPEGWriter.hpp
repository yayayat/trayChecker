#pragma once

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "opencv2/opencv.hpp"
#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <string.h>

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define TIMEOUT_M 200000
#define NUM_CONNECTIONS 10

using namespace cv;
using namespace std;

struct clientFrame {
  uchar *outbuf;
  int outlen;
  int client;
};

struct clientPayload {
  void *context;
  clientFrame cf;
};

class MJPEGWriter {
  int sock;
  fd_set master;
  int timeout;
  int quality; // jpeg compression [1..100]
  std::vector<int> clients;
  pthread_t thread_listen, thread_write;
  pthread_mutex_t mutex_client = PTHREAD_MUTEX_INITIALIZER;
  pthread_mutex_t mutex_cout = PTHREAD_MUTEX_INITIALIZER;
  pthread_mutex_t mutex_writer = PTHREAD_MUTEX_INITIALIZER;
  Mat lastFrame;
  int port;
  int _write(int sock, char *s, int len);
  int _read(int socket, char *buffer);

  static void *listen_Helper(void *context) {
    ((MJPEGWriter *)context)->Listener();
    return NULL;
  }

  static void *writer_Helper(void *context) {
    ((MJPEGWriter *)context)->Writer();
    return NULL;
  }

  static void *clientWrite_Helper(void *payload) {
    void *ctx = ((clientPayload *)payload)->context;
    struct clientFrame cf = ((clientPayload *)payload)->cf;
    ((MJPEGWriter *)ctx)->ClientWrite(cf);
    return NULL;
  }

  void Listener();
  void Writer();
  void ClientWrite(clientFrame &cf);

public:
  MJPEGWriter(int port = 0);
  ~MJPEGWriter();
  bool release();
  bool open();
  bool isOpened();
  void start();
  void stop();
  void write(Mat frame);
};