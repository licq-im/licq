#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>

#include "licq_languagecodes.h"

const struct SLanguage gLanguages[NUM_LANGUAGES] =
{
  { "Unspecified", LANGUAGE_UNSPECIFIED, 0 },
  { "Afrikaans", 55, 1 },
  { "Albanian", 58, 2 },
  { "Arabic", 1, 3 },
  { "Armenian", 59, 4 },
  { "Bhojpuri", 2, 5 },
  { "Bulgarian", 3, 6 },
  { "Burmese", 4, 7 },
  { "Cantonese", 5, 8 },
  { "Catalan", 6, 9 },
  { "Chinese", 7, 10 },
  { "Croatian", 8, 11 },
  { "Czech", 9, 12 },
  { "Danish", 10, 13 },
  { "Dutch", 11, 14 },
  { "English", 12, 15 },
  { "Esperanto", 13, 16 },
  { "Estonian", 14, 17 },
  { "Farsi", 15, 18 },
  { "Finnish", 16, 19 },
  { "French", 17, 20 },
  { "Gaelic", 18, 21 },
  { "German", 19, 22 },
  { "Greek", 20, 23 },
  { "Hebrew", 21, 24 },
  { "Hindi", 22, 25 },
  { "Hungarian", 23, 26 },
  { "Icelandic", 24, 27 },
  { "Indonesian", 25, 28 },
  { "Italian", 26, 29 },
  { "Japanese", 27, 30 },
  { "Khmer", 28, 31 },
  { "Korean", 29, 32 },
  { "Lao", 30, 33 },
  { "Latvian", 31, 34 },
  { "Lithuanian", 32, 35 },
  { "Malay", 33, 36 },
  { "Norwegian", 34, 37 },
  { "Persian", 57, 38 },
  { "Polish", 35, 39 },
  { "Portuguese", 36, 40 },
  { "Romanian", 37, 41 },
  { "Russian", 38, 42 },
  { "Serbian", 39, 43 },
  { "Slovak", 40, 44 },
  { "Slovenian", 41, 45 },
  { "Somali", 42, 46 },
  { "Spanish", 43, 47 },
  { "Swahili", 44, 48 },
  { "Swedish", 45, 49 },
  { "Tagalog", 46, 50 },
  { "Taiwanese", 64, 51 },
  { "Tatar", 47, 52 },
  { "Thai", 48, 53 },
  { "Turkish", 49, 54 },
  { "Ukrainian", 50, 55 },
  { "Urdu", 51, 56 },
  { "Vietnamese", 52, 57 },
  { "Yiddish", 53, 58 },
  { "Yoruba", 54, 59 },
  { "Bosnian", 8, 60 },
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
