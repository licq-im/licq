#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gettext.h"

#include <stdio.h>
#include <string.h>

#include "licq_backgroundcodes.h"

const struct SCategory gBackgrounds[NUM_BACKGROUNDS] =
{
  { tr_NOOP("Elementary School"), 300, 0 },
  { tr_NOOP("High School"), 301, 1 },
  { tr_NOOP("College"), 302, 2 },
  { tr_NOOP("University"), 303, 3 },
  { tr_NOOP("Military"), 304, 4 },
  { tr_NOOP("Past Work Place"), 305, 5 },
  { tr_NOOP("Past Organization"), 306, 6 },
  { tr_NOOP("Other"), 399, 7 }
};


const struct SCategory *GetBackgroundByCode(unsigned short _nCode)
{
   // do a simple linear search as there aren't too many interests
   unsigned short i = 0;
   while (i < NUM_BACKGROUNDS && gBackgrounds[i].nCode != _nCode) i++;
   if (i == NUM_BACKGROUNDS) return NULL;
   return &gBackgrounds[i];
}

const struct SCategory *GetBackgroundByIndex(unsigned short _nIndex)
{
   if (_nIndex >= NUM_BACKGROUNDS) return NULL;
   return (&gBackgrounds[_nIndex]);
}

const struct SCategory *GetBackgroundByName(const char *_szName)
{
   unsigned short i = 0;
   while (i < NUM_BACKGROUNDS &&
          strcasecmp(gBackgrounds[i].szName, _szName))
   {
     i++;
   }
   if (i == NUM_BACKGROUNDS) return NULL;
   return &gBackgrounds[i];
}
