#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gettext.h"

#include <stdio.h>
#include <string.h>

#include "licq_occupationcodes.h"

const struct SOccupation gOccupations[NUM_OCCUPATIONS] =
{
  { tr_NOOP("Unspecified"), OCCUPATION_UNSPECIFIED, 0 },
  { tr_NOOP("Academic"), 1, 1 },
  { tr_NOOP("Administrative"), 2, 2 },
  { tr_NOOP("Art/Entertainment"), 3, 3 },
  { tr_NOOP("College Student"), 4, 4 },
  { tr_NOOP("Community & Social"), 6, 5 },
  { tr_NOOP("Computers"), 5, 6 },
  { tr_NOOP("Education"), 7, 7 },
  { tr_NOOP("Engineering"), 8, 8 },
  { tr_NOOP("Financial Services"), 9, 9 },
  { tr_NOOP("Government"), 10, 10 },
  { tr_NOOP("High School Student"), 11, 11 },
  { tr_NOOP("Home"), 12, 12 },
  { tr_NOOP("ICQ - Providing Help"), 13, 13 },
  { tr_NOOP("Law"), 14, 14 },
  { tr_NOOP("Managerial"), 15, 15 },
  { tr_NOOP("Manufacturing"), 16, 16 },
  { tr_NOOP("Medical/Health"), 17, 17 },
  { tr_NOOP("Military"), 18, 18 },
  { tr_NOOP("Non-Government Organization"), 19, 19 },
  { tr_NOOP("Other Services"), 99, 20 },
  { tr_NOOP("Professional"), 20, 21 },
  { tr_NOOP("Retail"), 21, 22 },
  { tr_NOOP("Retired"), 22, 23 },
  { tr_NOOP("Science & Research"), 23, 24 },
  { tr_NOOP("Sports"), 24, 25 },
  { tr_NOOP("Technical"), 25, 26 },
  { tr_NOOP("University Student"), 26, 27 },
  { tr_NOOP("Web Building"), 27, 28 }
};


const struct SOccupation *GetOccupationByCode(unsigned short _nOccupationCode)
{
   // do a simple linear search as there aren't too many occupations
   unsigned short i = 0;
   while (i < NUM_OCCUPATIONS && gOccupations[i].nCode != _nOccupationCode) i++;
   if (i == NUM_OCCUPATIONS) return NULL;
   return &gOccupations[i];
}

const struct SOccupation *GetOccupationByIndex(unsigned short _nIndex)
{
   if (_nIndex >= NUM_OCCUPATIONS) return NULL;
   return (&gOccupations[_nIndex]);
}

const struct SOccupation *GetOccupationByName(const char *_szName)
{
   unsigned short i = 0;
   while (i < NUM_OCCUPATIONS && strcasecmp(gOccupations[i].szName, _szName)) i++;
   if (i == NUM_OCCUPATIONS) return NULL;
   return &gOccupations[i];
}
