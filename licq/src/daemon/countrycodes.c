#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>

#include "countrycodes.h"

#define NUM_COUNTRIES 47
char *szCountries[NUM_COUNTRIES + 1];
unsigned short nCountryCodes[NUM_COUNTRIES];

void InitCountryCodes(void)
{
   static int s_bInitDone = 0;

   if (s_bInitDone) return;

   szCountries[ 0] = strdup("United States");
   szCountries[ 1] = strdup("Russia (CIS)"); 
   szCountries[ 2] = strdup("Egypt"); 
   szCountries[ 3] = strdup("South Africa"); 
   szCountries[ 4] = strdup("Greece"); 
   szCountries[ 5] = strdup("Netherlands"); 
   szCountries[ 6] = strdup("Belgium"); 
   szCountries[ 7] = strdup("France"); 
   szCountries[ 8] = strdup("Spain"); 
   szCountries[ 9] = strdup("Italy"); 
   szCountries[10] = strdup("Romania"); 
   szCountries[11] = strdup("Switzerland"); 
   szCountries[12] = strdup("Czechoslovakia"); 
   szCountries[13] = strdup("Austria"); 
   szCountries[14] = strdup("United Kingdom"); 
   szCountries[15] = strdup("Denmark"); 
   szCountries[16] = strdup("Sweden"); 
   szCountries[17] = strdup("Norway"); 
   szCountries[18] = strdup("Poland"); 
   szCountries[19] = strdup("Germany"); 
   szCountries[20] = strdup("Mexico"); 
   szCountries[21] = strdup("Argentina"); 
   szCountries[22] = strdup("Brazil"); 
   szCountries[23] = strdup("Chile"); 
   szCountries[24] = strdup("Columbia"); 
   szCountries[25] = strdup("Venezuela"); 
   szCountries[26] = strdup("Australia"); 
   szCountries[27] = strdup("Indonesia"); 
   szCountries[28] = strdup("Philippines"); 
   szCountries[29] = strdup("New Zealand"); 
   szCountries[30] = strdup("Singapore"); 
   szCountries[31] = strdup("Japan"); 
   szCountries[32] = strdup("China"); 
   szCountries[33] = strdup("Turkey"); 
   szCountries[34] = strdup("India"); 
   szCountries[35] = strdup("Pakistan"); 
   szCountries[36] = strdup("Iran"); 
   szCountries[37] = strdup("Canada");
   szCountries[38] = strdup("Algeria"); 
   szCountries[39] = strdup("Portugal"); 
   szCountries[40] = strdup("Ireland"); 
   szCountries[41] = strdup("Finland"); 
   szCountries[42] = strdup("Haiti"); 
   szCountries[43] = strdup("Hong Kong"); 
   szCountries[44] = strdup("Taiwan"); 
   szCountries[45] = strdup("Israel"); 
   szCountries[46] = strdup("Dominican Republic"); 
   szCountries[47] = NULL; 

   nCountryCodes[0] = 0;
   nCountryCodes[1] = 7;
   nCountryCodes[2] =  20;
   nCountryCodes[3] =  27;
   nCountryCodes[4] =  30;
   nCountryCodes[5] =  31;
   nCountryCodes[6] =  32;
   nCountryCodes[7] =  33;
   nCountryCodes[8] =  34;
   nCountryCodes[9] =  39;
   nCountryCodes[10] =  40;
   nCountryCodes[11] =  41;
   nCountryCodes[12] =  42;
   nCountryCodes[13] =  43;
   nCountryCodes[14] =  44;
   nCountryCodes[15] =  45;
   nCountryCodes[16] =  46;
   nCountryCodes[17] =  47;
   nCountryCodes[18] =  48;
   nCountryCodes[19] =  49;
   nCountryCodes[20] =  52;
   nCountryCodes[21] =  54; 
   nCountryCodes[22] =  55; 
   nCountryCodes[23] =  56; 
   nCountryCodes[24] =  57; 
   nCountryCodes[25] =  58; 
   nCountryCodes[26] =  61; 
   nCountryCodes[27] =  62; 
   nCountryCodes[28] =  63; 
   nCountryCodes[29] =  64; 
   nCountryCodes[30] =  65;
   nCountryCodes[31] =  81; 
   nCountryCodes[32] =  86; 
   nCountryCodes[33] =  90; 
   nCountryCodes[34] =  91; 
   nCountryCodes[35] =  92; 
   nCountryCodes[36] =  98; 
   nCountryCodes[37] = 107;
   nCountryCodes[38] = 213; 
   nCountryCodes[39] = 351; 
   nCountryCodes[40] = 353; 
   nCountryCodes[41] = 358;
   nCountryCodes[42] = 509;
   nCountryCodes[43] = 852;
   nCountryCodes[44] = 886;
   nCountryCodes[45] = 972;
   nCountryCodes[46] = 110;

   s_bInitDone = 1;
}


const char *GetCountryByCode(unsigned short _nCountryCode)
{
   // do a simple linear search as there aren't too many countries
   unsigned short i = 0;
   while (i < NUM_COUNTRIES && nCountryCodes[i] != _nCountryCode) i++;
   if (i == NUM_COUNTRIES) return NULL;
   return szCountries[i];
}

const char **GetCountryList(void)
{
   return (const char **)szCountries;
}

const char *GetCountryByIndex(unsigned short _nIndex)
{
   if (_nIndex >= NUM_COUNTRIES) return NULL;
   return (szCountries[_nIndex]);
}

unsigned short GetIndexByCountryCode(unsigned short _nCountryCode)
{
   unsigned short i = 0;
   // make sure the value is sane
   if (_nCountryCode > 1000) return COUNTRY_UNSPECIFIED;

   // do a simple linear search as there aren't too many countries
   while (i < NUM_COUNTRIES && nCountryCodes[i] != _nCountryCode) i++;
   if (i == NUM_COUNTRIES) return COUNTRY_UNKNOWN;
   return i;
}

unsigned short GetCountryCodeByIndex(unsigned short _nIndex)
{
   if (_nIndex >= NUM_COUNTRIES) return COUNTRY_UNSPECIFIED;
   return nCountryCodes[_nIndex];
}

