/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2013 Licq developers <licq-dev@googlegroups.com>
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

#ifndef LICQ_ICQDATA_H
#define LICQ_ICQDATA_H

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

namespace Licq
{

enum IcqCategoryType
{
  IcqCatTypeBackground = 1,
  IcqCatTypeHomepage = 2,
  IcqCatTypeInterest = 3,
  IcqCatTypeLanguage = 4,
  IcqCatTypeOccupation = 5,
  IcqCatTypeOrganization = 6,
};

struct IcqCategory
{
  const char* szName;    /* Name of the category */
  unsigned short nCode;  /* Category code */
  unsigned short nIndex; /* Index in array */
};

const unsigned NUM_BACKGROUNDS = 8;
const unsigned NUM_HOMEPAGECATS = 2985;
const unsigned NUM_INTERESTS = 51;
const unsigned LANGUAGE_UNSPECIFIED = 0;
const unsigned NUM_LANGUAGES = 73;
const unsigned OCCUPATION_UNSPECIFIED = 0;
const unsigned NUM_OCCUPATIONS = 29;
const unsigned NUM_ORGANIZATIONS = 20;

struct IcqCountry
{
  const char* szName;    /* Name of the country */
  unsigned short nCode;  /* Country code */
  unsigned short nPhone; /* International phone code */
  unsigned short nIndex; /* Index in array */
};

const unsigned COUNTRY_UNSPECIFIED = 0;
const unsigned COUNTRY_UNKNOWN = 0xFFFF;
const unsigned NUM_COUNTRIES = 244;


struct IcqProvider
{
  const char* szName;    /* Name of provider */
  const char* szGateway; /* E-mail gateway of the provider */
  unsigned short nIndex; /* Index in array */
};

const unsigned NUM_PROVIDERS = 32;


class IcqData : private boost::noncopyable
{
public:
  typedef boost::shared_ptr<IcqData> Ptr;

  virtual const struct IcqCategory* getCategoryByCode(enum IcqCategoryType type, unsigned short code) = 0;
  virtual const struct IcqCategory* getCategoryByIndex(enum IcqCategoryType type, unsigned short index) = 0;
  virtual const struct IcqCategory* getCategoryByName(enum IcqCategoryType type, const char* name) = 0;
  virtual const struct IcqCountry* getCountryByCode(unsigned short countryCode) = 0;
  virtual const struct IcqCountry* getCountryByIndex(unsigned short index) = 0;
  virtual const struct IcqCountry* getCountryByName(const char* name) = 0;
  virtual const struct IcqProvider* getProviderByGateway(const char* gateway) = 0;
  virtual const struct IcqProvider* getProviderByIndex(unsigned short index) = 0;
  virtual const struct IcqProvider* getProviderByName(const char* name) = 0;

  const struct IcqCategory* getBackgroundByCode(unsigned short code)
  { return getCategoryByCode(IcqCatTypeBackground, code); }
  const struct IcqCategory* getBackgroundByIndex(unsigned short index)
  { return getCategoryByIndex(IcqCatTypeBackground, index); }
  const struct IcqCategory* getBackgroundByName(const char* name)
  { return getCategoryByName(IcqCatTypeBackground, name); }
  const struct IcqCategory* getHomepageCatByCode(unsigned short code)
  { return getCategoryByCode(IcqCatTypeHomepage, code); }
  const struct IcqCategory* getHomepageCatByIndex(unsigned short index)
  { return getCategoryByIndex(IcqCatTypeHomepage, index); }
  const struct IcqCategory* getHomepageCatByName(const char* name)
  { return getCategoryByName(IcqCatTypeHomepage, name); }
  const struct IcqCategory* getInterestByCode(unsigned short code)
  { return getCategoryByCode(IcqCatTypeInterest, code); }
  const struct IcqCategory* getInterestByIndex(unsigned short index)
  { return getCategoryByIndex(IcqCatTypeInterest, index); }
  const struct IcqCategory* getInterestByName(const char* name)
  { return getCategoryByName(IcqCatTypeInterest, name); }
  const struct IcqCategory* getLanguageByCode(unsigned short code)
  { return getCategoryByCode(IcqCatTypeLanguage, code); }
  const struct IcqCategory* getLanguageByIndex(unsigned short index)
  { return getCategoryByIndex(IcqCatTypeLanguage, index); }
  const struct IcqCategory* getLanguageByName(const char* name)
  { return getCategoryByName(IcqCatTypeLanguage, name); }
  const struct IcqCategory* getOccupationByCode(unsigned short code)
  { return getCategoryByCode(IcqCatTypeOccupation, code); }
  const struct IcqCategory* getOccupationByIndex(unsigned short index)
  { return getCategoryByIndex(IcqCatTypeOccupation, index); }
  const struct IcqCategory* getOccupationByName(const char* name)
  { return getCategoryByName(IcqCatTypeOccupation, name); }
  const struct IcqCategory* getOrganizationByCode(unsigned short code)
  { return getCategoryByCode(IcqCatTypeOrganization, code); }
  const struct IcqCategory* getOrganizationByIndex(unsigned short index)
  { return getCategoryByIndex(IcqCatTypeOrganization, index); }
  const struct IcqCategory* getOrganizationByName(const char* name)
  { return getCategoryByName(IcqCatTypeOrganization, name); }

protected:
  virtual ~IcqData() { /* Empty */ }
};

} // namespace Licq

#endif
