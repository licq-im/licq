#ifndef INTEREST_H
#define INTEREST_H

#ifdef __cplusplus
extern "C"
{
#endif

#define NUM_INTERESTS 51

#ifndef SCategoryDefined
#define SCategoryDefined
struct SCategory
{
  char *szName;          /* Name of the interest */
  unsigned short nCode;  /* Interest code */
  unsigned short nIndex; /* Index in array */
};
#endif
extern const struct SCategory gInterests[];

const struct SCategory *GetInterestByCode(unsigned short _nCode);
const struct SCategory *GetInterestByIndex(unsigned short _nIndex);
const struct SCategory *GetInterestByName(const char *_szName);

#ifdef __cplusplus
}
#endif

#endif // INTEREST_H

