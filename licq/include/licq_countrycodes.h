#ifndef COUNTRY_H
#define COUNTRY_H

#ifdef __cplusplus
extern "C" {
#endif

#define COUNTRY_UNSPECIFIED  0
#define COUNTRY_UNKNOWN      0xFFFF
#define NUM_COUNTRIES 244

struct SCountry
{
  char *szName;          /* Name of the country */
  unsigned short nCode;  /* Country code */
  unsigned short nPhone; /* International phone code */
  unsigned short nIndex; /* Index in array */
};
extern const struct SCountry gCountries[];

const struct SCountry *GetCountryByCode(unsigned short _nCountryCode);
const struct SCountry *GetCountryByIndex(unsigned short _nIndex);
const struct SCountry *GetCountryByName(const char *_szName);

#ifdef __cplusplus
}
#endif

#endif
