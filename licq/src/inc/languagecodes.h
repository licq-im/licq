#ifndef LANGUAGE_H
#define LANGUAGE_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define LANGUAGE_UNSPECIFIED  0
#define NUM_LANGUAGES 59

struct SLanguage
{
  char *szName;          /* Name of the country */
  unsigned short nCode;  /* Country code */
  unsigned short nIndex; /* Index in array */
};
extern const struct SLanguage gLanguages[];

const struct SLanguage *GetLanguageByCode(unsigned short _nCode);
const struct SLanguage *GetLanguageByIndex(unsigned short _nIndex);

#ifdef __cplusplus
}
#endif

#endif
