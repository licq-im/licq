#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#include "licq_languagecodes.h"

const struct SLanguage gLanguages[NUM_LANGUAGES] =
{
  { "Afrikaans", 55, 0 },
  { "Albanian", 58, 1 },
  { "Arabic", 1, 2 },
  { "Armenian", 59, 3 },
  { "Bhojpuri", 2, 4 },
  { "Bulgarian", 3, 5 },
  { "Burmese", 4, 6 },
  { "Cantonese", 5, 7 },
  { "Catalan", 6, 8 },
  { "Chinese", 7, 9 },
  { "Croatian", 8, 10 },
  { "Czech", 9, 11 },
  { "Danish", 10, 12 },
  { "Dutch", 11, 13 },
  { "English", 12, 14 },
  { "Esperanto", 13, 15 },
  { "Estonian", 14, 16 },
  { "Farsi", 15, 17 },
  { "Finnish", 16, 18 },
  { "French", 17, 19 },
  { "Gaelic", 18, 20 },
  { "German", 19, 21 },
  { "Greek", 20, 22 },
  { "Hebrew", 21, 23 },
  { "Hindi", 22, 24 },
  { "Hungarian", 23, 25 },
  { "Icelandic", 24, 26 },
  { "Indonesian", 25, 27 },
  { "Italian", 26, 28 },
  { "Japanese", 27, 29 },
  { "Khmer", 28, 30 },
  { "Korean", 29, 31 },
  { "Lao", 30, 32 },
  { "Latvian", 31, 33 },
  { "Lithuanian", 32, 34 },
  { "Malay", 33, 35 },
  { "Norwegian", 34, 36 },
  { "Persian", 57, 37 },
  { "Polish", 35, 38 },
  { "Portuguese", 36, 39 },
  { "Romanian", 37, 40 },
  { "Russian", 38, 41 },
  { "Serbian", 39, 42 },
  { "Slovak", 40, 43 },
  { "Slovenian", 41, 44 },
  { "Somali", 42, 45 },
  { "Spanish", 43, 46 },
  { "Swahili", 44, 47 },
  { "Swedish", 45, 48 },
  { "Tagalog", 46, 49 },
  { "Tatar", 47, 50 },
  { "Thai", 48, 51 },
  { "Turkish", 49, 52 },
  { "Ukrainian", 50, 53 },
  { "Urdu", 51, 54 },
  { "Vietnamese", 52, 55 },
  { "Yiddish", 53, 56 },
  { "Yoruba", 54, 57 },
  { "Unspecified", 0, 58 }
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

