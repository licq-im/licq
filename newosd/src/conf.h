#ifndef CONF_H
#define CONF_H

#include <string>

#include <glib.h>
#include <aosd.h>

typedef enum
{
  GROUP_TYPE_NONE = 0,
  GROUP_TYPE_ONLINE_NOTIFY,
  GROUP_TYPE_ALL
} UserGroupType;

class Conf
{
private:
  void loadConfig();

  bool markSecure;
  bool mouseActive;
  bool notifyOnly;
  bool wait;

  std::string backColor;
  std::string font;
  std::string ownerModes;
  std::string ownerModesMsg;
  std::string shadowColor;
  std::string textColor;
  std::string textControlColor;

  AosdCoordinate posHorizontal;
  AosdCoordinate posVertical;

  int offsetHorizontal;
  int offsetVertical;
  int shadowOffset;
  int wrapWidth;
  int maxLines;

  unsigned backOpacity;
  unsigned delayPerChar;
  unsigned fadeFull;
  unsigned fadeIn;
  unsigned fadeOut;
  unsigned marginHorizontal;
  unsigned marginVertical;
  unsigned quietTimeout;
  unsigned shadowOpacity;
  unsigned textOpacity;

  UserGroupType autoResponse;
  UserGroupType logonLogoff;
  UserGroupType showMessage;
  UserGroupType statusChange;

  friend class Iface;
};

#endif

/* vim: set ts=2 sw=2 et : */
