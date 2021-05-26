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
    cerr << "I/O error while reading file." << std::endl;
    return -1;
  } catch (const ParseException &pex) {
    cerr << "Parse error at " << pex.getFile() << ":" << pex.getLine() << " - "
         << pex.getError() << std::endl;
    return -1;
  }
  cfg.setAutoConvert(true);
  cfg.lookupValue("Exposure", stngs.exp);
  cfg.lookupValue("StreamOn", stngs.streamOn);
  cfg.lookupValue("Port", stngs.port);
  cfg.lookupValue("FrameRate", stngs.frameRate);
  cfg.lookupValue("RecordingOn", stngs.recOn);
  cfg.lookupValue("OutputFolder", stngs.outputFolder);
  cfg.lookupValue("OutputContainer", stngs.outputContainer);
  cfg.lookupValue("OutputCodec", stngs.outputCodec);
  cfg.lookupValue("Resolution.x", stngs.res.x);
  cfg.lookupValue("Resolution.y", stngs.res.y);
  const Setting &ROI = cfg.lookup("ROICorners");
  int count = min(ROI.getLength(), 4);
  for (int i = 0; i < count; i++) {
    const Setting &corner = ROI[i];
    stngs.ROICorners[i].x = corner[0];
    stngs.ROICorners[i].y = corner[1];
  }

  return 0;
}