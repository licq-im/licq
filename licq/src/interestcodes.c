#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gettext.h"

#include <stdio.h>
#include <string.h>

#include "licq_interestcodes.h"

const struct SCategory gInterests[NUM_INTERESTS] =
{
  { tr_NOOP("50's"), 137, 0 },
  { tr_NOOP("60's"), 134, 1 },
  { tr_NOOP("70's"), 135, 2 },
  { tr_NOOP("80's"), 136, 3 },
  { tr_NOOP("Art"), 100, 4 },
  { tr_NOOP("Astronomy"), 128, 5 },
  { tr_NOOP("Audio and Visual"), 147, 6 },
  { tr_NOOP("Business"), 125, 7 },
  { tr_NOOP("Business Services"), 146, 8 },
  { tr_NOOP("Cars"), 101, 9 },
  { tr_NOOP("Celebrity Fans"), 102, 10 },
  { tr_NOOP("Clothing"), 130, 11 },
  { tr_NOOP("Collections"), 103, 12 },
  { tr_NOOP("Computers"), 104, 13 },
  { tr_NOOP("Consumer Electronics"), 140, 14 },
  { tr_NOOP("Culture"), 105, 15 },
  { tr_NOOP("Ecology"), 122, 16 },
  { tr_NOOP("Entertainment"), 139, 17 },
  { tr_NOOP("Finance and Corporate"), 138, 18 },
  { tr_NOOP("Fitness"), 106, 19 },
  { tr_NOOP("Games"), 107, 20 },
  { tr_NOOP("Government"), 124, 21 },
  { tr_NOOP("Health and Beauty"), 142, 22 },
  { tr_NOOP("Hobbies"), 108, 23 },
  { tr_NOOP("Household Products"), 144, 24 },
  { tr_NOOP("Home Automation"), 150, 25 },
  { tr_NOOP("ICQ - Help"), 109, 26 },
  { tr_NOOP("Internet"), 110, 27 },
  { tr_NOOP("Lifestyle"), 111, 28 },
  { tr_NOOP("Mail Order Catalog"), 145, 29 },
  { tr_NOOP("Media"), 143, 30 },
  { tr_NOOP("Movies and TV"), 112, 31 },
  { tr_NOOP("Music"), 113, 32 },
  { tr_NOOP("Mystics"), 126, 33 },
  { tr_NOOP("News and Media"), 123, 34 },
  { tr_NOOP("Outdoors"), 114, 35 },
  { tr_NOOP("Parenting"), 115, 36 },
  { tr_NOOP("Parties"), 131, 37 },
  { tr_NOOP("Pets and Animals"), 116, 38 },
  { tr_NOOP("Publishing"), 149, 39 },
  { tr_NOOP("Religion"), 117, 40 },
  { tr_NOOP("Retail Stores"), 141, 41 },
  { tr_NOOP("Science"), 118, 42 },
  { tr_NOOP("Skills"), 119, 43 },
  { tr_NOOP("Social science"), 133, 44 },
  { tr_NOOP("Space"), 129, 45 },
  { tr_NOOP("Sporting and Athletic"), 148, 46 },
  { tr_NOOP("Sports"), 120, 47 },
  { tr_NOOP("Travel"), 127, 48 },
  { tr_NOOP("Web Design"), 121, 49 },
  { tr_NOOP("Women"), 132, 50 }
};


const struct SCategory *GetInterestByCode(unsigned short _nCode)
{
   // do a simple linear search as there aren't too many interests
   unsigned short i = 0;
   while (i < NUM_INTERESTS && gInterests[i].nCode != _nCode) i++;
   if (i == NUM_INTERESTS) return NULL;
   return &gInterests[i];
}

const struct SCategory *GetInterestByIndex(unsigned short _nIndex)
{
   if (_nIndex >= NUM_INTERESTS) return NULL;
   return (&gInterests[_nIndex]);
}

const struct SCategory *GetInterestByName(const char *_szName)
{
   unsigned short i = 0;
   while (i < NUM_INTERESTS && strcasecmp(gInterests[i].szName, _szName)) i++;
   if (i == NUM_INTERESTS) return NULL;
   return &gInterests[i];
}
