#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gettext.h"

#include <stdio.h>
#include <string.h>

#include "licq_organizationcodes.h"

const struct SCategory gOrganizations[NUM_ORGANIZATIONS] =
{
  { tr_NOOP("Alumni Org."), 200, 0 },
  { tr_NOOP("Charity Org."), 201, 1},
  { tr_NOOP("Club/Social Org."), 202, 2 },
  { tr_NOOP("Community Org."), 203, 3 },
  { tr_NOOP("Cultural Org."), 204, 4 },
  { tr_NOOP("Fan Clubs"), 205, 5 },
  { tr_NOOP("Fraternity/Sorority"), 206, 6 },
  { tr_NOOP("Hobbyists Org."), 207, 7 },
  { tr_NOOP("International Org."), 208, 8 },
  { tr_NOOP("Nature and Environment Org."), 209, 9 },
  { tr_NOOP("Professional Org."), 210, 10 },
  { tr_NOOP("Scientific/Technical Org."), 211, 11 },
  { tr_NOOP("Self Improvement Group"), 212, 12 },
  { tr_NOOP("Spiritual/Religious Org."), 213, 13 },
  { tr_NOOP("Sports Org."), 214, 14 },
  { tr_NOOP("Support Org."), 215, 15 },
  { tr_NOOP("Trade and Business Org."), 216, 16 },
  { tr_NOOP("Union"), 217, 17 },
  { tr_NOOP("Voluntary Org."), 218, 18 },
  { tr_NOOP("Other"), 299, 19 }
};

const struct SCategory *GetOrganizationByCode(unsigned short _nCode)
{
   // do a simple linear search as there aren't too many countries
   unsigned short i = 0;
   while (i < NUM_ORGANIZATIONS && gOrganizations[i].nCode != _nCode) i++;
   if (i == NUM_ORGANIZATIONS) return NULL;
   return &gOrganizations[i];
}

const struct SCategory *GetOrganizationByIndex(unsigned short _nIndex)
{
   if (_nIndex >= NUM_ORGANIZATIONS) return NULL;
   return (&gOrganizations[_nIndex]);
}

const struct SCategory *GetOrganizationByName(const char *_szName)
{
   unsigned short i = 0;
   while (i < NUM_ORGANIZATIONS &&
          strcasecmp(gOrganizations[i].szName, _szName))
   {
     i++;
   }
   if (i == NUM_ORGANIZATIONS) return NULL;
   return &gOrganizations[i];
}
