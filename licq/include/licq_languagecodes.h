#ifndef LANGUAGE_H
#define LANGUAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#define LANGUAGE_UNSPECIFIED  0
#define NUM_LANGUAGES 73

struct SLanguage
{
  char *szName;          /* Name of the country */
  unsigned short nCode;  /* Country code */
  unsigned short nIndex; /* Index in array */
};
extern const struct SLanguage gLanguages[];

const struct SLanguage *GetLanguageByCode(unsigned short _nCode);
const struct SLanguage *GetLanguageByIndex(unsigned short _nIndex);
const struct SLanguage *GetLanguageByName(const char *_szName);

#ifdef __cplusplus
}
#endif

#endif
