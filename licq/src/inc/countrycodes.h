#ifndef COUNTRY_H
#define COUNTRY_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

const unsigned short COUNTRY_UNSPECIFIED = 0xFFFF;
const unsigned short COUNTRY_UNKNOWN     = 0xFFFE;

void InitCountryCodes(void);
const char **GetCountryList(void);
const char *GetCountryByCode(unsigned short _nCountryCode);
const char *GetCountryByIndex(unsigned short _nIndex);
unsigned short GetIndexByCountryCode(unsigned short _nCountryCode);
unsigned short GetCountryCodeByIndex(unsigned short _nIndex);

#ifdef __cplusplus
}
#endif

#endif
