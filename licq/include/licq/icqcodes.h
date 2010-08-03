/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010 Licq developers
 *
 * Licq is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Licq is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Licq; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef LICQ_ICQCODES_H
#define LICQ_ICQCODES_H

#ifdef __cplusplus
extern "C"
{
#endif


#define NUM_BACKGROUNDS 8

struct SCategory
{
  char* szName;          /* Name of the background */
  unsigned short nCode;  /* Background code */
  unsigned short nIndex; /* Index in array */
};

extern const struct SCategory gBackgrounds[];

const struct SCategory* GetBackgroundByCode(unsigned short _nCode);
const struct SCategory* GetBackgroundByIndex(unsigned short _nIndex);
const struct SCategory* GetBackgroundByName(const char* _szName);


#define COUNTRY_UNSPECIFIED  0
#define COUNTRY_UNKNOWN      0xFFFF
#define NUM_COUNTRIES 244

struct SCountry
{
  char* szName;          /* Name of the country */
  unsigned short nCode;  /* Country code */
  unsigned short nPhone; /* International phone code */
  unsigned short nIndex; /* Index in array */
};
extern const struct SCountry gCountries[];

const struct SCountry* GetCountryByCode(unsigned short _nCountryCode);
const struct SCountry* GetCountryByIndex(unsigned short _nIndex);
const struct SCountry* GetCountryByName(const char* _szName);


#define NUM_HOMEPAGECATS 2985

struct SHomepageCat
{
  char* szName;          /* Name of the category */
  unsigned short nCode;  /* Category code */
  unsigned short nIndex; /* Index in array */
};
extern const struct SHomepageCat gHomepageCategories[];

const struct SHomepageCat* GetHomepageCatByCode(unsigned short _nCode);
const struct SHomepageCat* GetHomepageCatByIndex(unsigned short _nIndex);
const struct SHomepageCat* GetHomepageCatByName(const char* _szName);


#define NUM_INTERESTS 51

extern const struct SCategory gInterests[];

const struct SCategory* GetInterestByCode(unsigned short _nCode);
const struct SCategory* GetInterestByIndex(unsigned short _nIndex);
const struct SCategory* GetInterestByName(const char* _szName);


#define LANGUAGE_UNSPECIFIED  0
#define NUM_LANGUAGES 73

struct SLanguage
{
  char* szName;          /* Name of the country */
  unsigned short nCode;  /* Country code */
  unsigned short nIndex; /* Index in array */
};
extern const struct SLanguage gLanguages[];

const struct SLanguage* GetLanguageByCode(unsigned short _nCode);
const struct SLanguage* GetLanguageByIndex(unsigned short _nIndex);
const struct SLanguage* GetLanguageByName(const char* _szName);


#define OCCUPATION_UNSPECIFIED  0
#define NUM_OCCUPATIONS 29

struct SOccupation
{
  char* szName;          /* Name of occupation */
  unsigned short nCode;  /* Occupation code */
  unsigned short nIndex; /* Index in array */
};
extern const struct SOccupation gOccupations[];

const struct SOccupation* GetOccupationByCode(unsigned short _nOccupationCode);
const struct SOccupation* GetOccupationByIndex(unsigned short _nIndex);
const struct SOccupation* GetOccupationByName(const char* _szName);


#define NUM_ORGANIZATIONS 20

extern const struct SCategory gOrganizations[];

const struct SCategory* GetOrganizationByCode(unsigned short _nCode);
const struct SCategory* GetOrganizationByIndex(unsigned short _nIndex);
const struct SCategory* GetOrganizationByName(const char* _szName);


#define NUM_PROVIDERS 32

struct SProvider
{
  char* szName;          /* Name of provider */
  char* szGateway;       /* E-mail gateway of the provider */
  unsigned short nIndex; /* Index in array */
};
extern const struct SProvider gProviders[];

const struct SProvider* GetProviderByGateway(const char* _szGateway);
const struct SProvider* GetProviderByIndex(unsigned short _nIndex);
const struct SProvider* GetProviderByName(const char* _szName);

#ifdef __cplusplus
}
#endif

#endif // LICQ_ICQCODES_H
