#pragma once

#include <cstdlib>
#include <iomanip>
#include <iostream>

#include <libconfig.h++>

struct vec2d {
  int x, y;
};

struct settings {
  int exp;
  vec2d res;
  vec2d ROIpos, ROIsize;
  bool streamOn;
  int port;
  int frameRate;
  bool recOn;
  const char *outputFolder;
  const char *outputContainer;
  const char *outputCodec;
};

int loadConfig(settings &stngs);