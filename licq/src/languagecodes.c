// -*- c-basic-offset: 2 -*-
/* ----------------------------------------------------------------------------
 * Licq - A ICQ Client for Unix
 * Copyright (C) 1998 - 2009 Licq developers
 *
 * This program is licensed under the terms found in the LICENSE file.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>

// Localization
#include "gettext.h"

#include "licq_languagecodes.h"

const struct SLanguage gLanguages[NUM_LANGUAGES] =
{
  { tr_NOOP("Unspecified"), LANGUAGE_UNSPECIFIED, 0 },
  { tr_NOOP("Afrikaans"), 55, 1 },
  { tr_NOOP("Albanian"), 58, 2 },
  { tr_NOOP("Arabic"), 1, 3 },
  { tr_NOOP("Armenian"), 59, 4 },
  { tr_NOOP("Azerbaijani"), 68, 5 },
  { tr_NOOP("Belorussian"), 72, 6 },
  { tr_NOOP("Bhojpuri"), 2, 7 },
  { tr_NOOP("Bosnian"), 56, 8 },
  { tr_NOOP("Bulgarian"), 3, 9 },
  { tr_NOOP("Burmese"), 4, 10 },
  { tr_NOOP("Cantonese"), 5, 11 },
  { tr_NOOP("Catalan"), 6, 12 },
  { tr_NOOP("Chamorro"), 61, 13 },
  { tr_NOOP("Chinese"), 7, 14 },
  { tr_NOOP("Croatian"), 8, 15 },
  { tr_NOOP("Czech"), 9, 16 },
  { tr_NOOP("Danish"), 10, 17 },
  { tr_NOOP("Dutch"), 11, 18 },
  { tr_NOOP("English"), 12, 19 },
  { tr_NOOP("Esperanto"), 13, 20 },
  { tr_NOOP("Estonian"), 14, 21 },
  { tr_NOOP("Farsi"), 15, 22 },
  { tr_NOOP("Finnish"), 16, 23 },
  { tr_NOOP("French"), 17, 24 },
  { tr_NOOP("Gaelic"), 18, 25 },
  { tr_NOOP("German"), 19, 26 },
  { tr_NOOP("Greek"), 20, 27 },
  { tr_NOOP("Gujarati"), 70, 28 },
  { tr_NOOP("Hebrew"), 21, 29 },
  { tr_NOOP("Hindi"), 22, 30 },
  { tr_NOOP("Hungarian"), 23, 31 },
  { tr_NOOP("Icelandic"), 24, 32 },
  { tr_NOOP("Indonesian"), 25, 33 },
  { tr_NOOP("Italian"), 26, 34 },
  { tr_NOOP("Japanese"), 27, 35 },
  { tr_NOOP("Khmer"), 28, 36 },
  { tr_NOOP("Korean"), 29, 37 },
  { tr_NOOP("Kurdish"), 69, 38 },
  { tr_NOOP("Lao"), 30, 39 },
  { tr_NOOP("Latvian"), 31, 40 },
  { tr_NOOP("Lithuanian"), 32, 41 },
  { tr_NOOP("Macedonian"), 65, 42 },
  { tr_NOOP("Malay"), 33, 43 },
  { tr_NOOP("Mandarin"), 63, 44 },
  { tr_NOOP("Mongolian"), 62, 45 },
  { tr_NOOP("Norwegian"), 34, 46 },
  { tr_NOOP("Persian"), 57, 47 },
  { tr_NOOP("Polish"), 35, 48 },
  { tr_NOOP("Portuguese"), 36, 49 },
  { tr_NOOP("Punjabi"), 60, 50 },
  { tr_NOOP("Romanian"), 37, 51 },
  { tr_NOOP("Russian"), 38, 52 },
  { tr_NOOP("Serbian"), 39, 53 },
  { tr_NOOP("Sindhi"), 66, 54 },
  { tr_NOOP("Slovak"), 40, 55 },
  { tr_NOOP("Slovenian"), 41, 56 },
  { tr_NOOP("Somali"), 42, 57 },
  { tr_NOOP("Spanish"), 43, 58 },
  { tr_NOOP("Swahili"), 44, 59 },
  { tr_NOOP("Swedish"), 45, 60 },
  { tr_NOOP("Tagalog"), 46, 61 },
  { tr_NOOP("Taiwanese"), 64, 62 },
  { tr_NOOP("Tamil"), 71, 63 },
  { tr_NOOP("Tatar"), 47, 64 },
  { tr_NOOP("Thai"), 48, 65 },
  { tr_NOOP("Turkish"), 49, 66 },
  { tr_NOOP("Ukrainian"), 50, 67 },
  { tr_NOOP("Urdu"), 51, 68 },
  { tr_NOOP("Vietnamese"), 52, 69 },
  { tr_NOOP("Welsh"), 67, 70 },
  { tr_NOOP("Yiddish"), 53, 71 },
  { tr_NOOP("Yoruba"), 54, 72 }
};


const struct SLanguage *GetLanguageByCode(unsigned short _nLanguageCode)
{
   // do a simple linear search as there aren't too many countries
   unsigned short i = 0;
   while (i < NUM_LANGUAGES && gLanguages[i].nCode != _nLanguageCode) i++;
   if (i == NUM_LANGUAGES) return NULL;
   return &gLanguages[i];
}

const struct SLanguage *GetLanguageByIndex(unsigned short _nIndex)
{
   if (_nIndex >= NUM_LANGUAGES) return NULL;
   return (&gLanguages[_nIndex]);
}

const struct SLanguage *GetLanguageByName(const char *_szName)
{
   unsigned short i = 0;
   while (i < NUM_LANGUAGES && strcasecmp(gLanguages[i].szName, _szName)) i++;
   if (i == NUM_LANGUAGES) return NULL;
   return &gLanguages[i];
}
