#include "conf.h"

#include <licq/daemon.h>
#include <licq/inifile.h>

void Conf::loadConfig()
{
  Licq::IniFile conf("licq_newosd.ini");
  conf.loadFile();

#define GET(key, dst, def); \
  if (!conf.get((key), (dst), (def))) \
    conf.set((key), (dst));

  conf.setSection("Appearance");

  GET("Font", font, "");
  GET("MarkSecureMessages", markSecure, true);

  conf.setSection("Colouring");

  GET("Background", backColor, "");
  GET("Message", textColor, "yellow");
  GET("Control", textControlColor, "gray");
  GET("Shadow", shadowColor, "black");
  GET("BackOpacity", backOpacity, 0);
  GET("TextOpacity", textOpacity, 255);
  GET("ShadowOpacity", shadowOpacity, 255);

#define FIX(key, dst); \
  if ((dst) > 255) \
  { \
    (dst) %= 255; \
    conf.set((key), (dst)); \
  }

  FIX("BackOpacity", backOpacity);
  FIX("TextOpacity", textOpacity);
  FIX("ShadowOpacity", shadowOpacity);
#undef FIX

  conf.setSection("Geometry");

  GET("HPosition", reinterpret_cast<unsigned&>(posHorizontal), 0);
  GET("VPosition", reinterpret_cast<unsigned&>(posVertical), 0);

#define FIX(key, dst); \
  if ((dst) > COORDINATE_MAXIMUM) \
  { \
    (dst) = COORDINATE_MINIMUM; \
    conf.set((key), reinterpret_cast<unsigned&>((dst))); \
  }

  FIX("HPosition", posHorizontal);
  FIX("VPosition", posVertical);
#undef FIX

  GET("HOffset", offsetHorizontal, 0);
  GET("VOffset", offsetVertical, 0);

  GET("HMargin", marginHorizontal, 0);
  GET("VMargin", marginVertical, 0);

  GET("ShadowOffset", shadowOffset, 2);
  GET("Width", wrapWidth, 0);
  GET("Lines", maxLines, 0);

  conf.setSection("Timing");

  GET("FadeIn", fadeIn, 150);
  GET("FadeFull", fadeFull, 3000);
  GET("FadeOut", fadeOut, 150);
  GET("DelayPerCharacter", delayPerChar, 0);

  conf.setSection("Behavior");

  GET("Wait", wait, true);
  GET("MouseActive", mouseActive, true);
  GET("QuietTimeout", quietTimeout, 0);

  conf.setSection("Filtering");

  GET("ShowLogonLogoff", reinterpret_cast<unsigned&>(logonLogoff), 2);
  GET("ShowStatusChange", reinterpret_cast<unsigned&>(statusChange), 2);
  GET("ShowAutoResponseCheck", reinterpret_cast<unsigned&>(autoResponse), 2);
  GET("ShowMessages", reinterpret_cast<unsigned&>(showMessage), 2);

#define FIX(key, dst); \
  if ((dst) > GROUP_TYPE_ALL) \
  { \
    (dst) = GROUP_TYPE_ALL; \
    conf.set((key), reinterpret_cast<unsigned&>((dst))); \
  }

  FIX("ShowLogonLogoff", logonLogoff);
  FIX("ShowStatusChange", statusChange);
  FIX("ShowAutoResponseCheck", autoResponse);
  FIX("ShowMessages", showMessage);
#undef FIX

  GET("ShowMessagesNotification", notifyOnly, false);
  GET("ShowInModes", ownerModes, "");
  GET("ShowMsgsInModes", ownerModesMsg, "");

#undef GET

  conf.writeFile();
}

/* vim: set ts=2 sw=2 et : */
