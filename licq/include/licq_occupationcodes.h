#ifndef OCCUPATION_H
#define OCCUPATION_H

#ifdef __cplusplus
extern "C"
{
#endif

#define OCCUPATION_UNSPECIFIED  0
#define NUM_OCCUPATIONS 29

struct SOccupation
{
  char *szName;          /* Name of occupation */
  unsigned short nCode;  /* Occupation code */
  unsigned short nIndex; /* Index in array */
};
extern const struct SOccupation gOccupations[];

const struct SOccupation *GetOccupationByCode(unsigned short _nOccupationCode);
const struct SOccupation *GetOccupationByIndex(unsigned short _nIndex);
const struct SOccupation *GetOccupationByName(const char *_szName);

#ifdef __cplusplus
}
#endif

#endif
