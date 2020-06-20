#include "config.hpp"

using namespace std;
using namespace libconfig;

static const char *configFile = "settings.cfg";

int loadConfig(settings &stngs) {
  Config cfg;

  // load and read file
  try {
    cfg.readFile(configFile);
  } catch (const FileIOException &fioex) {
    std::cerr << "I/O error while reading file." << std::endl;
    return -1;
  } catch (const ParseException &pex) {
    std::cerr << "Parse error at " << pex.getFile() << ":" << pex.getLine()
              << " - " << pex.getError() << std::endl;
    return -1;
  }

  const Setting &root = cfg.getRoot();

  // Найти все книжки на полке.
  try {
    stngs.exp = root["Exposure"];
    stngs.res.x = root["Resolution"]["w"];
    stngs.res.y = root["Resolution"]["h"];
    stngs.ROIpos.x = root["SubtractRegion"]["pos"]["x"];
    stngs.ROIpos.y = root["SubtractRegion"]["pos"]["y"];
    stngs.ROIsize.x = root["SubtractRegion"]["size"]["w"];
    stngs.ROIsize.y = root["SubtractRegion"]["size"]["h"];
    stngs.streamOn = root["StreamOn"];
    stngs.port = root["Port"];
    stngs.frameRate = root["FrameRate"];
    stngs.recOn = root["RecordingOn"];
    stngs.outputFolder = root["OutputFolder"];
    stngs.outputContainer = root["OutputContainer"];
    stngs.outputCodec = root["OutputCodec"];
  } catch (const SettingNotFoundException &nfex) {
    // Ignore.
  }

  return 0;
}