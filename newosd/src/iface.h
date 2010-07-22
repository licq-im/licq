#ifndef IFACE_H
#define IFACE_H

#include <ctime>
#include <map>
#include <string>

#include <aosd.h>
#include <aosd-text.h>
#undef Status

namespace Licq
{
class PluginSignal;
}

class Conf;

class Iface
{
public:
  Iface();
  ~Iface();

  void processSignal(Licq::PluginSignal* sig);
  void updateTextRenderData();

private:
  Aosd* aosd;
  TextRenderData trd;
  Conf* conf;
  std::map<unsigned long, time_t> ppidTimers;

  bool filterSignal(Licq::PluginSignal* sig, unsigned long ppid);

  void displayLayout(std::string& msg, bool control);
};

#endif

/* vim: set ts=2 sw=2 et : */
